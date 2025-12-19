#include "gwmswimtaskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"
#include "SpatialWeight/gwmcrsdistance.h"
#include <SpatialWeight/gwmminkwoskidistance.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <array>
#include <limits>
#include "gwmapp.h"
#include <armadillo>
using namespace arma;

#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

GwmSWIMTaskThread::GwmSWIMTaskThread(QObject *parent)
    : GwmTaskThread()
{
    mKernelFunction = &GwmBandwidthWeight::GaussianKernelFunction;
}

GwmSWIMTaskThread::~GwmSWIMTaskThread()
{
}

void GwmSWIMTaskThread::setCsvFilePath(const QString& filePath)
{
    mCsvFilePath = filePath;
}

void GwmSWIMTaskThread::setSWIMMode(SWIMMode mode)
{
    mSWIMMode = mode;
}

void GwmSWIMTaskThread::setSpatialWeight(const GwmSpatialWeight& spatialWeight)
{
    mSpatialWeight = spatialWeight;
    applyBandwidthFromWeight(mSpatialWeight.weight<GwmBandwidthWeight>());
}

void GwmSWIMTaskThread::setUseBandwidthAuto(bool enabled)
{
    mUseBandwidthAuto = enabled;
}

void GwmSWIMTaskThread::setBandwidthSelectionCriterion(BandwidthSelectionCriterionType type)
{
    mBandwidthCriterionType = type;
}

int GwmSWIMTaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly
#ifdef ENABLE_OpenMP
           | IParallelalbe::OpenMP
#endif
        ;
}

IParallelalbe::ParallelType GwmSWIMTaskThread::parallelType() const
{
    return mParallelType;
}

void GwmSWIMTaskThread::setParallelType(const ParallelType& type)
{
    mParallelType = type;
}

void GwmSWIMTaskThread::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

void GwmSWIMTaskThread::setFieldMapping(const GwmSWIMFieldMapping& mapping)
{
    mFieldMapping = mapping;
    mFieldMapping.requireOriginCoords = true;
    mFieldMapping.requireDestCoords = true;
    mIndependentVarNames = mapping.independentVarNames;
}

void GwmSWIMTaskThread::setFieldDelimiter(QChar delimiter)
{
    mFieldDelimiter = delimiter;
}

bool GwmSWIMTaskThread::isValid()
{
    if (mCsvFilePath.isEmpty())
    {
        print_error(tr("CSV file path is empty."));
        return false;
    }

    if (!QFile::exists(mCsvFilePath))
    {
        print_error(tr("CSV file does not exist: %1").arg(mCsvFilePath));
        return false;
    }

    if (!mFieldMapping.isComplete())
    {
        print_error(tr("Field mapping is incomplete."));
        return false;
    }

    if (!mSpatialWeight.weight() || !mSpatialWeight.distance())
    {
        print_error(tr("Spatial weight configuration is invalid."));
        return false;
    }

    auto* bw = mSpatialWeight.weight<GwmBandwidthWeight>();
    if (bw && !bw->adaptive() && bw->bandwidth() <= 0.0)
    {
        print_error(tr("Bandwidth must be greater than 0."));
        return false;
    }

    return true;
}

void GwmSWIMTaskThread::run()
{
    qDebug() << "[GwmSWIMTaskThread::run] Starting SWIM calculation";
    emit tick(0, 0);
    mResultList.clear();
    mDiagnostics = GwmSWIMDiagnostics();
    mBandwidthTrace.clear();

    // Step 1: load CSV data
    if (!checkCanceled())
    {
        emit message(tr("Loading CSV data..."));
        if (!loadCsvData())
        {
            emit error(tr("Failed to load CSV data."));
            return;
        }
        if (!prepareRegressionMatrices())
        {
            emit error(tr("Failed to prepare regression matrices."));
            return;
        }
        emit tick(20, 100);
    }

    if (!checkCanceled() && mUseBandwidthAuto)
    {
        emit message(tr("Automatically selecting bandwidth..."));
        if (!selectBandwidthAutomatically())
        {
            emit error(tr("Failed to select bandwidth automatically."));
            return;
        }
    }

    // Step 2: compute weight matrix
    if (!checkCanceled())
    {
        emit message(tr("Calculating weight matrix..."));
        calculateWeightMatrix();
        emit tick(60, 100);
    }

    if (!checkCanceled())
    {
        emit message(tr("Fitting local regression models..."));
        performLocalRegression();
        updateDiagnostics();
        emit tick(80, 100);
    }

    // Step 3: create result layers
    if (!checkCanceled())
    {
        emit message(tr("Creating result layer..."));
        mat observed = mat(mResponseVector);
        mat fitted = mat(mFittedValues);
        mat residuals = mat(mResiduals);

        mResultList.push_back(qMakePair(QStringLiteral("FlowVolume"), observed));
        mResultList.push_back(qMakePair(QStringLiteral("FittedFlow"), fitted));
        mResultList.push_back(qMakePair(QStringLiteral("Residual"), residuals));
        if (!mLocalBetas.empty())
        {
            mResultList.push_back(qMakePair(QStringLiteral("Coefficients"), mLocalBetas));
        }
        mResultList.push_back(qMakePair(QStringLiteral("WeightMatrix"), mWeightMatrix));

        createResultLayer(mResultList);
        emit tick(100, 100);
        emit success();
    }
}

bool GwmSWIMTaskThread::loadCsvData()
{
    QFile file(mCsvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        print_error(tr("Cannot open CSV file: %1").arg(mCsvFilePath));
        return false;
    }

    if (!mFieldMapping.isComplete())
    {
        print_error(tr("Field mapping is not configured."));
        return false;
    }

    QTextStream in(&file);
    QString headerLine = in.readLine();
    if (headerLine.isNull())
    {
        print_error(tr("CSV header is empty."));
        return false;
    }

    QStringList headers = headerLine.split(mFieldDelimiter, Qt::KeepEmptyParts);
    mCsvHeaders = headers;

    if (!mFieldMapping.isValid(headers.size()))
    {
        print_error(tr("Field mapping exceeds available columns."));
        return false;
    }

    mFlowDataList.clear();
    int lineNum = 1;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineNum++;

        if (line.trimmed().isEmpty())
            continue;

        GwmFlowData flowData;
        if (parseCsvLine(line, flowData, mFlowDataList.size()))
        {
            mFlowDataList.append(flowData);
        }
        else
        {
            print_message(tr("Warning: Failed to parse line %1").arg(lineNum));
        }
    }

    file.close();

    if (mFlowDataList.isEmpty())
    {
        print_error(tr("No valid data found in CSV file."));
        return false;
    }

    print_message(tr("Loaded %1 flow records.").arg(mFlowDataList.size()));
    return true;
}

bool GwmSWIMTaskThread::prepareRegressionMatrices()
{
    if (mFlowDataList.isEmpty())
    {
        print_error(tr("No flow data available for regression."));
        return false;
    }

    int n = mFlowDataList.size();
    int indepCount = mFieldMapping.independentVars.size();

    mDesignMatrix = mat(n, indepCount + 1, fill::ones);
    mResponseVector = vec(n, fill::zeros);

    print_message(tr("Preparing regression matrices with %1 records and %2 independent variables").arg(n).arg(indepCount));

    for (int i = 0; i < n; ++i)
    {
        const auto& flow = mFlowDataList[i];
        if (flow.independent_values.size() != indepCount)
        {
            print_error(tr("Flow record %1 does not contain %2 independent variables.")
                            .arg(i)
                            .arg(indepCount));
            return false;
        }

        mResponseVector(i) = flow.flow_volume;
        for (int j = 0; j < indepCount; ++j)
        {
            mDesignMatrix(i, j + 1) = flow.independent_values[j];
        }
    }

    return true;
}

bool GwmSWIMTaskThread::parseCsvLine(const QString& line, GwmFlowData& flowData, int flowIndex)
{
    QStringList fields = line.split(mFieldDelimiter, Qt::KeepEmptyParts);

    auto readInt = [&](int index, int& target) -> bool {
        if (index < 0 || index >= fields.size()) return false;
        bool ok = false;
        target = fields[index].trimmed().toInt(&ok);
        return ok;
    };

    auto readDouble = [&](int index, double& target) -> bool {
        if (index < 0 || index >= fields.size()) return false;
        bool ok = false;
        target = fields[index].trimmed().toDouble(&ok);
        return ok;
    };

    flowData.flow_id = flowIndex;
    flowData.origin_id = flowIndex;
    flowData.dest_id = flowIndex;

    if (!readDouble(mFieldMapping.flowVolume, flowData.flow_volume)) return false;
    if (mFieldMapping.originValue >= 0)
    {
        if (!readDouble(mFieldMapping.originValue, flowData.origin_value)) return false;
    }
    else
    {
        flowData.origin_value = 0.0;
    }

    if (mFieldMapping.destValue >= 0)
    {
        if (!readDouble(mFieldMapping.destValue, flowData.dest_value)) return false;
    }
    else
    {
        flowData.dest_value = 0.0;
    }
    if (!readDouble(mFieldMapping.originX, flowData.origin_x)) return false;
    if (!readDouble(mFieldMapping.originY, flowData.origin_y)) return false;
    if (!readDouble(mFieldMapping.destX, flowData.dest_x)) return false;
    if (!readDouble(mFieldMapping.destY, flowData.dest_y)) return false;

    flowData.independent_values.clear();
    for (int idx : mFieldMapping.independentVars)
    {
        double value = 0.0;
        if (!readDouble(idx, value)) return false;
        flowData.independent_values.append(value);
    }

    static int debugCount = 0;
    if (debugCount < 5) {
        QString debugMsg = QString("Parsed flow data #%1: volume=%2, origin=(%3,%4), dest=(%5,%6)")
        .arg(flowIndex)
            .arg(flowData.flow_volume)
            .arg(flowData.origin_x).arg(flowData.origin_y)
            .arg(flowData.dest_x).arg(flowData.dest_y);

        if (mFieldMapping.originValue >= 0) {
            debugMsg += QString(", origin_value=%1").arg(flowData.origin_value);
        }

        if (mFieldMapping.destValue >= 0) {
            debugMsg += QString(", dest_value=%1").arg(flowData.dest_value);
        }

        if (!flowData.independent_values.isEmpty()) {
            debugMsg += ", indep_vars=[";
            for (int i = 0; i < flowData.independent_values.size(); ++i) {
                if (i > 0) debugMsg += ",";
                debugMsg += QString::number(flowData.independent_values[i]);
            }
            debugMsg += "]";
        }

        print_message(debugMsg);
        debugCount++;
    }

    return true;
}

double GwmSWIMTaskThread::calculateOriginDistance(int i, int j) const
{
    if (i < 0 || i >= mFlowDataList.size() || j < 0 || j >= mFlowDataList.size())
        return 0.0;

    const GwmFlowData& flow1 = mFlowDataList[i];
    const GwmFlowData& flow2 = mFlowDataList[j];

    double dx = flow1.origin_x - flow2.origin_x;
    double dy = flow1.origin_y - flow2.origin_y;
    return std::sqrt(dx * dx + dy * dy);
}

double GwmSWIMTaskThread::calculateDestDistance(int i, int j) const
{
    if (i < 0 || i >= mFlowDataList.size() || j < 0 || j >= mFlowDataList.size())
        return 0.0;

    const GwmFlowData& flow1 = mFlowDataList[i];
    const GwmFlowData& flow2 = mFlowDataList[j];

    double dx = flow1.dest_x - flow2.dest_x;
    double dy = flow1.dest_y - flow2.dest_y;
    return std::sqrt(dx * dx + dy * dy);
}

double GwmSWIMTaskThread::calculateFlowEuclideanDistance(int i, int j) const
{
    if (i < 0 || i >= mFlowDataList.size() || j < 0 || j >= mFlowDataList.size())
        return 0.0;

    const GwmFlowData& flow1 = mFlowDataList[i];
    const GwmFlowData& flow2 = mFlowDataList[j];

    // 四维欧氏距离: (xi, yi, xj, yj)
    double dx1 = flow1.origin_x - flow2.origin_x;
    double dy1 = flow1.origin_y - flow2.origin_y;
    double dx2 = flow1.dest_x - flow2.dest_x;
    double dy2 = flow1.dest_y - flow2.dest_y;

    return std::sqrt(dx1 * dx1 + dy1 * dy1 + dx2 * dx2 + dy2 * dy2);
}

double GwmSWIMTaskThread::calculateFlowSOPDistance(int i, int j) const
{
    // 轨迹距离(SOP): d(i,i') + d(j,j')
    double originDist = calculateOriginDistance(i, j);
    double destDist = calculateDestDistance(i, j);
    return originDist + destDist;
}

double GwmSWIMTaskThread::kernelFunction(double distance, double bandwidth)
{
    return applyKernel(distance, bandwidth);
}

double GwmSWIMTaskThread::applyKernel(double distance, double bandwidth) const
{
    if (!mKernelFunction)
        return 0.0;

    double bw = bandwidth;
    if (bw <= 0.0)
    {
        bw = std::numeric_limits<double>::epsilon();
    }

    return mKernelFunction(distance, bw);
}

double GwmSWIMTaskThread::resolveAdaptiveBandwidth(const QVector<double>& distances) const
{
    if (distances.isEmpty())
        return mBandwidth;

    int n = distances.size();
    double dn = n > 0 ? mBandwidth / static_cast<double>(n) : 0.0;

    if (dn <= 1.0)
    {
        QVector<double> sortedDistances = distances;
        std::sort(sortedDistances.begin(), sortedDistances.end());
        int index = std::max(0, std::min(static_cast<int>(std::round(mBandwidth)) - 1, n - 1));
        double bw = sortedDistances[index];
        if (bw <= 0.0 && !sortedDistances.isEmpty())
        {
            bw = *std::max_element(sortedDistances.begin(), sortedDistances.end());
        }
        return bw;
    }
    else
    {
        double maxDist = *std::max_element(distances.begin(), distances.end());
        return dn * maxDist;
    }
}

GwmSWIMTaskThread::DistanceFunction GwmSWIMTaskThread::distanceFunctionForMode() const
{
    switch (mSWIMMode)
    {
    case SWIMMode::OriginFocused:
        return &GwmSWIMTaskThread::calculateOriginDistance;
    case SWIMMode::DestinationFocused:
        return &GwmSWIMTaskThread::calculateDestDistance;
    case SWIMMode::FlowFocusedEuclidean:
        return &GwmSWIMTaskThread::calculateFlowEuclideanDistance;
    case SWIMMode::FlowFocusedSOP:
    default:
        return &GwmSWIMTaskThread::calculateFlowSOPDistance;
    }
}

QVector<double> GwmSWIMTaskThread::collectDistances(int focusIndex, DistanceFunction func) const
{
    int n = mFlowDataList.size();
    QVector<double> distances(n, 0.0);
    for (int j = 0; j < n; ++j)
    {
        distances[j] = (this->*func)(focusIndex, j);
    }
    return distances;
}

void GwmSWIMTaskThread::fillWeightMatrix(DistanceFunction func)
{
    int n = mFlowDataList.size();

    for (int i = 0; i < n; ++i)
    {
        if (checkCanceled())
        {
            return;
        }

        QVector<double> distances = collectDistances(i, func);
        double bw = mBandwidthAdaptive ? resolveAdaptiveBandwidth(distances) : mBandwidth;
        if (bw <= 0.0)
        {
            bw = std::numeric_limits<double>::epsilon();
        }

        for (int j = 0; j < n; ++j)
        {
            mWeightMatrix(i, j) = applyKernel(distances[j], bw);
        }

        if (i % 100 == 0)
        {
            progress(i, n);
        }
    }
}

void GwmSWIMTaskThread::calculateWeightMatrix()
{
    int n = mFlowDataList.size();
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] n=" << n
             << "bandwidth=" << mBandwidth
             << "adaptive=" << mBandwidthAdaptive;

    mWeightMatrix = mat(n, n, fill::zeros);

    DistanceFunction func = distanceFunctionForMode();
    fillWeightMatrix(func);
}

void GwmSWIMTaskThread::createResultLayer(CreateResultLayerData data)
{
    qDebug() << "[GwmSWIMTaskThread::createResultLayer] Creating result layer, item count:" << data.size();
    print_message(tr("Result layer creation not yet implemented."));
}

bool GwmSWIMTaskThread::selectBandwidthAutomatically()
{
    mBandwidthTrace.clear();
    GwmBandwidthWeight* baseWeight = mSpatialWeight.weight<GwmBandwidthWeight>();
    if (!baseWeight)
    {
        print_error(tr("Spatial weight is not configured correctly."));
        return false;
    }

    const int flowCount = mFlowDataList.size();
    if (flowCount == 0)
    {
        print_error(tr("No flow records available for bandwidth selection."));
        return false;
    }

    DistanceFunction func = distanceFunctionForMode();
    QVector<double> sampleDistances = collectDistances(0, func);

    QVector<double> candidates = baseWeight->adaptive()
                                     ? buildAdaptiveBandwidthCandidates(flowCount)
                                     : buildFixedBandwidthCandidates(sampleDistances);

    auto addCandidate = [&](double value)
    {
        if (value <= 0.0 || !std::isfinite(value))
            return;
        bool exists = std::any_of(candidates.begin(), candidates.end(), [value](double v){
            return std::abs(v - value) < 1e-9;
        });
        if (!exists)
            candidates.append(value);
    };

    addCandidate(baseWeight->bandwidth());
    if (candidates.isEmpty())
    {
        print_error(tr("Failed to build bandwidth candidate set."));
        return false;
    }

    double bestMetric = std::numeric_limits<double>::max();
    double bestBandwidth = baseWeight->bandwidth() > 0.0 ? baseWeight->bandwidth() : candidates.first();

    for (double candidate : candidates)
    {
        double metric = evaluateBandwidthForValue(candidate);
        mBandwidthTrace.append(qMakePair(candidate, metric));
        if (metric < bestMetric)
        {
            bestMetric = metric;
            bestBandwidth = candidate;
        }

        if (checkCanceled())
            break;
    }

    baseWeight->setBandwidth(bestBandwidth);
    applyBandwidthFromWeight(baseWeight);

    qDebug() << "[GwmSWIMTaskThread::selectBandwidthAutomatically] Best bandwidth"
             << bestBandwidth << "criterion=" << bestMetric;

    return true;
}

void GwmSWIMTaskThread::applyBandwidthFromWeight(const GwmBandwidthWeight* weight)
{
    if (!weight)
        return;
    mBandwidth = weight->bandwidth();
    mBandwidthAdaptive = weight->adaptive();
    mKernelType = weight->kernel();
    mKernelFunction = GwmBandwidthWeight::Kernel[mKernelType];
}

QVector<double> GwmSWIMTaskThread::buildAdaptiveBandwidthCandidates(int flowCount) const
{
    QVector<double> candidates;
    if (flowCount <= 0)
        return candidates;

    int minNeighbors = std::max(10, mFieldMapping.independentVars.size() + 2);
    minNeighbors = std::min(minNeighbors, flowCount);
    int maxNeighbors = flowCount;
    int steps = std::min(6, maxNeighbors - minNeighbors + 1);
    if (steps <= 0)
    {
        candidates.append(static_cast<double>(minNeighbors));
        return candidates;
    }

    for (int i = 0; i < steps; ++i)
    {
        double ratio = steps == 1 ? 0.0 : static_cast<double>(i) / (steps - 1);
        int value = static_cast<int>(std::round(minNeighbors + ratio * (maxNeighbors - minNeighbors)));
        value = std::clamp(value, 1, flowCount);
        bool exists = std::any_of(candidates.begin(), candidates.end(), [value](double v){
            return std::abs(v - value) < 1e-9;
        });
        if (!exists)
            candidates.append(static_cast<double>(value));
    }

    return candidates;
}

QVector<double> GwmSWIMTaskThread::buildFixedBandwidthCandidates(const QVector<double>& distances) const
{
    QVector<double> positive;
    positive.reserve(distances.size());
    for (double d : distances)
    {
        if (std::isfinite(d) && d > 0.0)
            positive.append(d);
    }

    QVector<double> candidates;
    if (positive.isEmpty())
        return candidates;

    std::sort(positive.begin(), positive.end());
    static const std::array<double, 6> quantiles = {0.2, 0.35, 0.5, 0.65, 0.8, 0.95};
    for (double q : quantiles)
    {
        int idx = positive.size() == 1
                      ? 0
                      : static_cast<int>(std::round(q * (positive.size() - 1)));
        idx = std::clamp(idx, 0, positive.size() - 1);
        double value = positive[idx];
        if (value <= 0.0)
            continue;
        bool exists = std::any_of(candidates.begin(), candidates.end(), [value](double v){
            return std::abs(v - value) < 1e-9;
        });
        if (!exists)
            candidates.append(value);
    }

    return candidates;
}

double GwmSWIMTaskThread::evaluateBandwidthForValue(double candidate)
{
    if (candidate <= 0.0 || !std::isfinite(candidate))
        return std::numeric_limits<double>::max();

    double savedBandwidth = mBandwidth;
    mBandwidth = candidate;

    calculateWeightMatrix();
    if (checkCanceled())
    {
        mBandwidth = savedBandwidth;
        return std::numeric_limits<double>::max();
    }

    // Refit local Poisson SWIM for this candidate
    performLocalRegression();
    // Update diagnostics (ENP, EDF, deviance, AIC/AICc) before evaluating criterion
    updateDiagnostics();
    double metric = evaluateBandwidthCriterion();
    mBandwidth = savedBandwidth;
    if (!std::isfinite(metric))
        metric = std::numeric_limits<double>::max();
    return metric;
}

double GwmSWIMTaskThread::evaluateBandwidthCriterion() const
{
    const int n = mFlowDataList.size();
    if (n <= 0)
        return std::numeric_limits<double>::max();

    // For Poisson SWIM, use deviance-based criteria.
    // - CV: average deviance
    // - AICc: Nakaya et al. (2005) Poisson AICc(b)
    //   AICc(b) = Deviance(b) + 2 * k(b) * n / (n - k(b) - 1)
    if (!std::isfinite(mDiagnostics.deviance))
        return std::numeric_limits<double>::max();

    if (mBandwidthCriterionType == BandwidthSelectionCriterionType::CV)
    {
        return mDiagnostics.deviance / static_cast<double>(n);
    }

    const double k = mDiagnostics.effectiveParameters;
    if (!std::isfinite(k) || n <= k + 1.0)
        return std::numeric_limits<double>::max();

    double aicc = mDiagnostics.deviance
                  + 2.0 * k * static_cast<double>(n) / (static_cast<double>(n) - k - 1.0);
    return aicc;
}

double GwmSWIMTaskThread::currentRSS() const
{
    if (mResiduals.n_elem == 0)
        return std::numeric_limits<double>::quiet_NaN();
    return dot(mResiduals, mResiduals);
}

void GwmSWIMTaskThread::updateDiagnostics()
{
    mDiagnostics = GwmSWIMDiagnostics();
    mDiagnostics.dataPoints = static_cast<int>(mResponseVector.n_elem);
    const int n = mDiagnostics.dataPoints;

    // Calculate effective number of parameters and effective degrees of freedom
    // using hat matrix trace statistics (following GWR methodology)
    double trS = mShat(0);  // tr(S) - trace of hat matrix
    double trStS = mShat(1);  // tr(S^T * S) - trace of hat matrix squared (approximated)

    // Effective number of parameters: enp = 2 * tr(S) - tr(S^T * S)
    mDiagnostics.effectiveParameters = 2.0 * trS - trStS;

    // Effective degrees of freedom: edf = n - 2 * tr(S) + tr(S^T * S)
    mDiagnostics.effectiveDof = static_cast<double>(n) - 2.0 * trS + trStS;

    // Compute global Poisson deviance using fitted values from SWIM
    if (n > 0 && mFittedValues.n_elem == static_cast<uword>(n))
    {
        double dev = computePoissonDeviance(mResponseVector, mFittedValues);
        if (std::isfinite(dev))
        {
            mDiagnostics.deviance = dev;
        }
    }

    // For compatibility, derive pseudo-R² style measures from deviance
    // Using McFadden's pseudo R²:
    //   R2 = 1 - (Deviance_model / Deviance_null)
    // and adjusted version:
    //   R2_adj = 1 - ((Deviance_model - k) / Deviance_null)
    if (n > 0 && mResponseVector.n_elem == static_cast<uword>(n))
    {
        // Null model: intercept-only Poisson
        vec y = mResponseVector;
        double yMean = mean(y);
        if (yMean > 0.0 && std::isfinite(yMean) && std::isfinite(mDiagnostics.deviance) &&
            std::isfinite(mDiagnostics.effectiveParameters))
        {
            vec mu0(y.n_elem);
            mu0.fill(yMean);
            double devNull = computePoissonDeviance(y, mu0);
            if (std::isfinite(devNull) && devNull > 0.0)
            {
                const double k = mDiagnostics.effectiveParameters;
                // McFadden pseudo R²
                double r2 = 1.0 - mDiagnostics.deviance / devNull;
                // Adjusted McFadden pseudo R²
                double r2Adj = 1.0 - (mDiagnostics.deviance - k) / devNull;

                // Clamp to [0,1] for numerical stability
                auto clamp01 = [](double v) -> double {
                    if (!std::isfinite(v)) return std::numeric_limits<double>::quiet_NaN();
                    if (v < 0.0) return 0.0;
                    if (v > 1.0) return 1.0;
                    return v;
                };

                mDiagnostics.pseudoRSquared = clamp01(r2);
                mDiagnostics.rSquared = mDiagnostics.pseudoRSquared;
                mDiagnostics.adjRSquared = clamp01(r2Adj);
            }

        }
    }

    // For Poisson diagnostics, AIC and AICc are defined via deviance and ENP
    if (n > 0 && std::isfinite(mDiagnostics.deviance) &&
        std::isfinite(mDiagnostics.effectiveParameters))
    {
        const double k = mDiagnostics.effectiveParameters;
        // Standard Poisson AIC: Deviance + 2 * k
        mDiagnostics.aic = mDiagnostics.deviance + 2.0 * k;
        if (n > k + 1.0)
        {
            // Nakaya et al. (2005) Poisson AICc(b)
            mDiagnostics.aicc = mDiagnostics.deviance
                                + 2.0 * k * static_cast<double>(n) / (static_cast<double>(n) - k - 1.0);
        }
    }
}

void GwmSWIMTaskThread::performLocalRegression()
{
    if (mDesignMatrix.n_rows == 0 || mWeightMatrix.n_rows == 0)
    {
        print_message(tr("Skipping regression because matrices are empty."));
        return;
    }

    int n = static_cast<int>(mDesignMatrix.n_rows);
    int p = static_cast<int>(mDesignMatrix.n_cols);

    mLocalBetas = mat(n, p, fill::zeros);
    mFittedValues = vec(n, fill::zeros);
    mResiduals = vec(n, fill::zeros);
    mShat = vec(2, fill::zeros);  // [tr(S), tr(S^T * S)]

    int successCount = 0;
    int skipCount = 0;

    for (int i = 0; i < n; ++i)
    {
        if (checkCanceled())
        {
            return;
        }

        // Kernel weights for this calibration point (row i of weight matrix)
        vec kernelWeights = trans(mWeightMatrix.row(i));
        kernelWeights.transform([](double val) { return val < 0.0 ? 0.0 : val; });

        if (all(kernelWeights == 0.0))
        {
            skipCount++;
            continue;
        }

        vec beta(p, fill::zeros);
        rowvec hatRow;   // optional hat-matrix row for ENP/EDF

        bool ok = fitLocalPoissonIRLS(mDesignMatrix, mResponseVector, kernelWeights,
                                      beta, /*devianceOut*/ mDiagnostics.deviance, &hatRow);
        if (ok && beta.n_elem == static_cast<uword>(p))
        {
            mLocalBetas.row(i) = beta.t();

            // Fitted value at calibration flow i
            double eta_i = dot(mDesignMatrix.row(i), beta);
            double mu_i = std::exp(std::min(eta_i, 20.0));   // cap to avoid overflow
            mFittedValues(i) = mu_i;
            mResiduals(i) = mResponseVector(i) - mu_i;

            successCount++;

            // Accumulate hat-matrix trace statistics (ENP/EDF)
            if (hatRow.n_elem == static_cast<uword>(n))
            {
                double s_ii = hatRow(i);
                if (std::isfinite(s_ii))
                {
                    mShat(0) += s_ii;  // tr(S)
                    double trStS_row = as_scalar(hatRow * hatRow.t());
                    if (std::isfinite(trStS_row))
                        mShat(1) += trStS_row;  // tr(S^T S)
                }
            }
        }

        if (i % 100 == 0)
        {
            progress(i, n);
        }
    }

    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Completed. success=" << successCount
             << "skip=" << skipCount
             << "tr(S)=" << mShat(0)
             << "tr(S^T*S)=" << mShat(1);
}

bool GwmSWIMTaskThread::fitLocalPoissonIRLS(const mat& X,
                                            const vec& y,
                                            const vec& kernelWeights,
                                            vec& betaOut,
                                            double& devianceOut,
                                            rowvec* hatRowOut)
{
    const int n = static_cast<int>(X.n_rows);
    const int p = static_cast<int>(X.n_cols);
    if (n <= 0 || p <= 0 || static_cast<int>(y.n_elem) != n ||
        static_cast<int>(kernelWeights.n_elem) != n)
    {
        return false;
    }

    // Initialize with canonical log-link Poisson GLM:
    // log(mu) = X * beta
    betaOut = vec(p, fill::zeros);

    // Start from log of (y + small) to avoid log(0)
    vec eta = X * betaOut;
    for (int i = 0; i < n; ++i)
    {
        double yi = y(i);
        if (yi > 0.0)
            eta(i) = std::log(yi);
        else
            eta(i) = std::log(0.5);   // small baseline
    }
    // Recompute beta by weighted least squares on initial eta
    mat XtX = X.t() * X;
    vec Xty = X.t() * eta;
    vec betaInit;
    if (!solve(betaInit, XtX, Xty, solve_opts::fast + solve_opts::likely_sympd))
        betaInit = pinv(XtX) * Xty;
    betaOut = betaInit;

    const int maxIter = 50;
    const double tol = 1e-6;
    double prevDev = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < maxIter; ++iter)
    {
        eta = X * betaOut;
        vec mu = exp(eta);
        // Guard against extreme values
        for (int i = 0; i < n; ++i)
        {
            if (!std::isfinite(mu(i)) || mu(i) <= 0.0)
                mu(i) = std::numeric_limits<double>::min();
        }

        // IRLS weights (for Poisson: v_i = mu_i)
        vec varMu = mu;
        // Combined weights: spatial kernel * IRLS weights
        vec w = kernelWeights % varMu;

        // Avoid all-zero weights
        if (all(w == 0.0))
            return false;

        vec sqrtW = sqrt(w);
        mat Xw = X.each_col() % sqrtW;
        vec z = eta + (y - mu) / mu;          // working response
        vec zw = z % sqrtW;

        mat XtWX = Xw.t() * Xw;
        vec XtWz = Xw.t() * zw;

        mat XtWXInv;
        bool solved = solve(XtWXInv, XtWX, eye(p, p), solve_opts::fast + solve_opts::likely_sympd);
        if (!solved)
        {
            XtWXInv = pinv(XtWX);
        }

        vec betaNew = XtWXInv * XtWz;

        // Check convergence
        vec diff = betaNew - betaOut;
        if (norm(diff, 2) < tol)
        {
            betaOut = betaNew;
            break;
        }

        betaOut = betaNew;

        // Optional early stop based on deviance change
        vec muIter = exp(X * betaOut);
        for (int i = 0; i < n; ++i)
        {
            if (!std::isfinite(muIter(i)) || muIter(i) <= 0.0)
                muIter(i) = std::numeric_limits<double>::min();
        }
        double devNow = computePoissonDeviance(y, muIter);
        if (std::isfinite(devNow) && std::abs(devNow - prevDev) < 1e-6)
        {
            prevDev = devNow;
            break;
        }
        prevDev = devNow;
    }

    // Final deviance for this local fit (using kernel weights as frequency-like multipliers)
    vec muFinal = exp(X * betaOut);
    for (int i = 0; i < n; ++i)
    {
        if (!std::isfinite(muFinal(i)) || muFinal(i) <= 0.0)
            muFinal(i) = std::numeric_limits<double>::min();
    }
    // Unweighted deviance at this calibration point
    double dev = computePoissonDeviance(y, muFinal);
    devianceOut = dev;

    // Optional hat-matrix row (approximate, using final IRLS weights)
    if (hatRowOut)
    {
        vec sqrtW = sqrt(kernelWeights % muFinal);
        mat Xw = X.each_col() % sqrtW;
        mat XtWX = Xw.t() * Xw;
        mat XtWXInv;
        bool solved = solve(XtWXInv, XtWX, eye(p, p), solve_opts::fast + solve_opts::likely_sympd);
        if (!solved)
        {
            XtWXInv = pinv(XtWX);
        }
        mat ci = XtWXInv * Xw.t();   // (p x n)
        // For this IRLS fit, take the row corresponding to the calibration point
        // WLS-style: choose the calibration flow as the one with maximum kernel weight
        uword centerIdx = index_max(kernelWeights);
        rowvec X_center = X.row(centerIdx);
        *hatRowOut = X_center * ci;  // (1 x n)
    }

    return true;
}

double GwmSWIMTaskThread::computePoissonDeviance(const vec& y, const vec& mu) const
{
    if (y.n_elem != mu.n_elem || y.n_elem == 0)
        return std::numeric_limits<double>::quiet_NaN();

    double dev = 0.0;
    for (uword i = 0; i < y.n_elem; ++i)
    {
        const double yi = y(i);
        double mui = mu(i);
        if (!std::isfinite(mui) || mui <= 0.0)
            mui = std::numeric_limits<double>::min();

        if (yi > 0.0)
        {
            dev += 2.0 * (yi * std::log(yi / mui) - (yi - mui));
        }
        else
        {
            dev += 2.0 * (0.0 - (0.0 - mui));  // 2 * mui
        }
    }
    return dev;
}

