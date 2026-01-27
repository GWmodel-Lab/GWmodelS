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
        mBandwidthTrace.push_back(std::make_pair(candidate, metric));
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

    performLocalRegression();
    double metric = evaluateBandwidthCriterion();
    mBandwidth = savedBandwidth;
    if (!std::isfinite(metric))
        metric = std::numeric_limits<double>::max();
    return metric;
}

double GwmSWIMTaskThread::evaluateBandwidthCriterion() const
{
    double rss = currentRSS();
    if (!std::isfinite(rss))
        return std::numeric_limits<double>::max();

    const int n = mFlowDataList.size();
    if (n <= 0)
        return std::numeric_limits<double>::max();

    if (mBandwidthCriterionType == BandwidthSelectionCriterionType::CV)
    {
        return rss / n;
    }

    const int k = mFieldMapping.independentVars.size() + 1;
    if (n <= k + 1)
        return std::numeric_limits<double>::max();

    double sigma2 = rss / n;
    if (sigma2 <= 0.0)
        sigma2 = std::numeric_limits<double>::min();

    const double pi = 3.14159265358979323846;
    double aic = n * std::log(sigma2) + n * (1.0 + std::log(2.0 * pi));
    double aicc = aic + (2.0 * k * (k + 1.0)) / (n - k - 1.0);
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
    
    double rss = currentRSS();
    mDiagnostics.rss = rss;

    // Calculate effective number of parameters and effective degrees of freedom
    // using hat matrix trace statistics (following GWR methodology)
    double trS = mShat(0);  // tr(S) - trace of hat matrix
    double trStS = mShat(1);  // tr(S^T * S) - trace of hat matrix squared (approximated)
    
    // Effective number of parameters: enp = 2 * tr(S) - tr(S^T * S)
    mDiagnostics.effectiveParameters = 2.0 * trS - trStS;
    
    // Effective degrees of freedom: edf = n - 2 * tr(S) + tr(S^T * S)
    mDiagnostics.effectiveDof = static_cast<double>(n) - 2.0 * trS + trStS;

    if (n > 0 && std::isfinite(rss))
    {
        // Use effective degrees of freedom for variance estimation
        double sigma2 = std::isfinite(mDiagnostics.effectiveDof) && mDiagnostics.effectiveDof > 0.0
            ? rss / mDiagnostics.effectiveDof
            : rss / static_cast<double>(n);
        
        if (sigma2 > 0.0 && std::isfinite(sigma2))
        {
            const double pi = 3.14159265358979323846;
            // AIC = n * log(sigma^2) + n * (1 + log(2*pi)) + 2 * enp
            mDiagnostics.aic = n * std::log(sigma2) + n * (1.0 + std::log(2.0 * pi)) + 2.0 * mDiagnostics.effectiveParameters;
            
            // AICc = AIC + 2 * enp * (enp + 1) / (n - enp - 1)
            if (std::isfinite(mDiagnostics.effectiveParameters) && n - mDiagnostics.effectiveParameters - 1 > 0)
            {
                mDiagnostics.aicc = mDiagnostics.aic + (2.0 * mDiagnostics.effectiveParameters * (mDiagnostics.effectiveParameters + 1.0)) 
                    / (n - mDiagnostics.effectiveParameters - 1.0);
            }
        }
    }

    if (n > 0 && mResponseVector.n_elem == static_cast<uword>(n) && std::isfinite(rss))
    {
        vec centeredY = mResponseVector - mean(mResponseVector);
        double tss = dot(centeredY, centeredY);
        if (tss > 0.0)
        {
            mDiagnostics.rSquared = 1.0 - rss / tss;
            // Adjusted R-squared using effective degrees of freedom
            if (std::isfinite(mDiagnostics.effectiveDof) && mDiagnostics.effectiveDof > 1.0)
            {
                mDiagnostics.adjRSquared = 1.0 - (1.0 - mDiagnostics.rSquared) * (n - 1.0) / (mDiagnostics.effectiveDof - 1.0);
            }
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

        vec weights = trans(mWeightMatrix.row(i));
        vec nonNegativeWeights = weights;
        nonNegativeWeights.transform([](double val) { return val < 0.0 ? 0.0 : val; });

        if (all(nonNegativeWeights == 0.0))
        {
            skipCount++;
            continue;
        }

        vec sqrtW = sqrt(nonNegativeWeights);
        mat Xw = mDesignMatrix.each_col() % sqrtW;
        vec yw = mResponseVector % sqrtW;
        mat XtWX = Xw.t() * Xw;
        vec XtWy = Xw.t() * yw;

        mat XtWXInv;
        bool solved = solve(XtWXInv, XtWX, eye(p, p), solve_opts::fast + solve_opts::likely_sympd);
        if (!solved)
        {
            XtWXInv = pinv(XtWX);
        }

        vec beta = XtWXInv * XtWy;

        if (beta.n_elem == static_cast<uword>(p))
        {
            mLocalBetas.row(i) = beta.t();
            mFittedValues(i) = dot(mDesignMatrix.row(i), beta);
            mResiduals(i) = mResponseVector(i) - mFittedValues(i);
            successCount++;

            // Calculate hat matrix diagonal element s_ii for this observation
            // Following GWR implementation:
            // ci = (X^T * W_i * X)^(-1) * X^T * W_i  (p x n matrix)
            // si = X_i * ci  (1 x n row vector, hat matrix row i)
            // s_ii = si(i)  (diagonal element)
            rowvec X_i = mDesignMatrix.row(i);
            
            // ci = XtWXInv * Xw.t(), where Xw.t() = X^T * W_i (for observation i)
            // Xw is (n x p) where each row j is sqrt(w_ij) * X_j
            // Xw.t() is (p x n) where each column j is sqrt(w_ij) * X_j^T
            mat ci = XtWXInv * Xw.t();  // (p x n)
            
            // si = X_i * ci, hat matrix row i (1 x n)
            rowvec si = X_i * ci;
            
            // s_ii is the i-th element of si (diagonal element)
            double s_ii = si(i);
            
            // Accumulate hat matrix trace statistics
            if (std::isfinite(s_ii))
            {
                mShat(0) += s_ii;  // tr(S) = sum of diagonal elements
                // tr(S^T * S) = sum_i sum_j s_ij^2
                // For row i: sum_j s_ij^2 = dot(si, si) = sum(si % si)
                double trStS_row = as_scalar(si * si.t());  // dot(si, si) = sum(si % si)
                if (std::isfinite(trStS_row))
                {
                    mShat(1) += trStS_row;
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

