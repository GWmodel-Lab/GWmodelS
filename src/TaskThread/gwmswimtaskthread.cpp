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
    mKernelType = GwmBandwidthWeight::KernelFunctionType::Gaussian;
    mKernelFunction = &GwmBandwidthWeight::GaussianKernelFunction;
    if (auto* bw = mSpatialWeight.weight<GwmBandwidthWeight>())
    {
        mBandwidth = bw->bandwidth();
        mBandwidthAdaptive = bw->adaptive();
        mKernelType = bw->kernel();
        mKernelFunction = GwmBandwidthWeight::Kernel[mKernelType];
    }
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

    // Step 1: load CSV data
    if (!checkCanceled())
    {
        emit message(tr("Loading CSV data..."));
        qDebug() << "[GwmSWIMTaskThread::run] Step 1: Loading CSV data";
        if (!loadCsvData())
        {
            qDebug() << "[GwmSWIMTaskThread::run] ERROR: Failed to load CSV data";
            emit error(tr("Failed to load CSV data."));
            return;
        }
        qDebug() << "[GwmSWIMTaskThread::run] CSV data loaded successfully";
        qDebug() << "[GwmSWIMTaskThread::run] Flow data count:" << mFlowDataList.size();
        qDebug() << "[GwmSWIMTaskThread::run] CSV headers count:" << mCsvHeaders.size();
        
        if (!prepareRegressionMatrices())
        {
            qDebug() << "[GwmSWIMTaskThread::run] ERROR: Failed to prepare regression matrices";
            emit error(tr("Failed to prepare regression matrices."));
            return;
        }
        qDebug() << "[GwmSWIMTaskThread::run] Regression matrices prepared";
        qDebug() << "[GwmSWIMTaskThread::run] Design matrix size:" << mDesignMatrix.n_rows << "x" << mDesignMatrix.n_cols;
        qDebug() << "[GwmSWIMTaskThread::run] Response vector size:" << mResponseVector.n_elem;
        emit tick(20, 100);
    }

    // Step 2: compute weight matrix
    if (!checkCanceled())
    {
        emit message(tr("Calculating weight matrix..."));
        qDebug() << "[GwmSWIMTaskThread::run] Step 2: Calculating weight matrix";
        qDebug() << "[GwmSWIMTaskThread::run] Bandwidth:" << mBandwidth << "Adaptive:" << mBandwidthAdaptive;
        qDebug() << "[GwmSWIMTaskThread::run] SWIM Mode:" << static_cast<int>(mSWIMMode);
        calculateWeightMatrix();
        qDebug() << "[GwmSWIMTaskThread::run] Weight matrix calculated";
        qDebug() << "[GwmSWIMTaskThread::run] Weight matrix size:" << mWeightMatrix.n_rows << "x" << mWeightMatrix.n_cols;
        if (mWeightMatrix.n_rows > 0 && mWeightMatrix.n_cols > 0)
        {
            qDebug() << "[GwmSWIMTaskThread::run] Weight matrix min:" << mWeightMatrix.min();
            qDebug() << "[GwmSWIMTaskThread::run] Weight matrix max:" << mWeightMatrix.max();
            qDebug() << "[GwmSWIMTaskThread::run] Weight matrix mean:" << mean(mean(mWeightMatrix));
            qDebug() << "[GwmSWIMTaskThread::run] Weight matrix sum:" << accu(mWeightMatrix);
        }
        emit tick(60, 100);
    }

    if (!checkCanceled())
    {
        emit message(tr("Fitting local regression models..."));
        qDebug() << "[GwmSWIMTaskThread::run] Step 3: Fitting local regression models";
        performLocalRegression();
        qDebug() << "[GwmSWIMTaskThread::run] Local regression completed";
        if (mFittedValues.n_elem > 0)
        {
            qDebug() << "[GwmSWIMTaskThread::run] Fitted values count:" << mFittedValues.n_elem;
            qDebug() << "[GwmSWIMTaskThread::run] Fitted values min:" << mFittedValues.min();
            qDebug() << "[GwmSWIMTaskThread::run] Fitted values max:" << mFittedValues.max();
            qDebug() << "[GwmSWIMTaskThread::run] Fitted values mean:" << mean(mFittedValues);
        }
        if (mResiduals.n_elem > 0)
        {
            qDebug() << "[GwmSWIMTaskThread::run] Residuals count:" << mResiduals.n_elem;
            qDebug() << "[GwmSWIMTaskThread::run] Residuals min:" << mResiduals.min();
            qDebug() << "[GwmSWIMTaskThread::run] Residuals max:" << mResiduals.max();
            qDebug() << "[GwmSWIMTaskThread::run] Residuals mean:" << mean(mResiduals);
        }
        if (mLocalBetas.n_rows > 0 && mLocalBetas.n_cols > 0)
        {
            qDebug() << "[GwmSWIMTaskThread::run] Local betas size:" << mLocalBetas.n_rows << "x" << mLocalBetas.n_cols;
        }
        emit tick(80, 100);
    }

    // Step 3: create result layers
    if (!checkCanceled())
    {
        emit message(tr("Creating result layer..."));
        qDebug() << "[GwmSWIMTaskThread::run] Step 4: Creating result layer";
        mat observed = mat(mResponseVector);
        mat fitted = mat(mFittedValues);
        mat residuals = mat(mResiduals);
        qDebug() << "[GwmSWIMTaskThread::run] Creating result data structures";
        qDebug() << "[GwmSWIMTaskThread::run] Observed matrix size:" << observed.n_rows << "x" << observed.n_cols;
        qDebug() << "[GwmSWIMTaskThread::run] Fitted matrix size:" << fitted.n_rows << "x" << fitted.n_cols;
        qDebug() << "[GwmSWIMTaskThread::run] Residuals matrix size:" << residuals.n_rows << "x" << residuals.n_cols;
        
        mResultList.push_back(qMakePair(QStringLiteral("FlowVolume"), observed));
        mResultList.push_back(qMakePair(QStringLiteral("FittedFlow"), fitted));
        mResultList.push_back(qMakePair(QStringLiteral("Residual"), residuals));
        if (!mLocalBetas.empty())
        {
            qDebug() << "[GwmSWIMTaskThread::run] Adding coefficients to result list";
            mResultList.push_back(qMakePair(QStringLiteral("Coefficients"), mLocalBetas));
        }
        qDebug() << "[GwmSWIMTaskThread::run] Adding weight matrix to result list";
        mResultList.push_back(qMakePair(QStringLiteral("WeightMatrix"), mWeightMatrix));
        qDebug() << "[GwmSWIMTaskThread::run] Result list size:" << mResultList.size();
        
        createResultLayer(mResultList);
        qDebug() << "[GwmSWIMTaskThread::run] Result layer created";
        emit tick(100, 100);
        qDebug() << "[GwmSWIMTaskThread::run] Calculation completed successfully";
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
        qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] ERROR: Flow data list is empty";
        return false;
    }

    int n = mFlowDataList.size();
    int indepCount = mFieldMapping.independentVars.size();
    qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] Preparing matrices with" << n << "records and" << indepCount << "independent variables";
    
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
            qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] ERROR: Flow record" << i << "has" << flow.independent_values.size() << "independent values, expected" << indepCount;
            return false;
        }

        mResponseVector(i) = flow.flow_volume;
        for (int j = 0; j < indepCount; ++j)
        {
            mDesignMatrix(i, j + 1) = flow.independent_values[j];
        }
    }

    qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] Matrices prepared successfully";
    qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] Design matrix:" << mDesignMatrix.n_rows << "x" << mDesignMatrix.n_cols;
    qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] Response vector:" << mResponseVector.n_elem << "elements";
    if (n > 0)
    {
        qDebug() << "[GwmSWIMTaskThread::prepareRegressionMatrices] Sample response value[0]:" << mResponseVector(0);

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
    if (mFieldMapping.requireOriginCoords)
    {
        if (!readDouble(mFieldMapping.originX, flowData.origin_x)) return false;
        if (!readDouble(mFieldMapping.originY, flowData.origin_y)) return false;
    }
    else
    {
        flowData.origin_x = 0.0;
        flowData.origin_y = 0.0;
    }

    if (mFieldMapping.requireDestCoords)
    {
        if (!readDouble(mFieldMapping.destX, flowData.dest_x)) return false;
        if (!readDouble(mFieldMapping.destY, flowData.dest_y)) return false;
    }
    else
    {
        flowData.dest_x = 0.0;
        flowData.dest_y = 0.0;
    }

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
    qDebug() << "[GwmSWIMTaskThread::fillWeightMatrix] Filling weight matrix for" << n << "flows";
    
    for (int i = 0; i < n; ++i)
    {
        if (checkCanceled())
        {
            qDebug() << "[GwmSWIMTaskThread::fillWeightMatrix] Calculation canceled at flow" << i;
            return;
        }

        QVector<double> distances = collectDistances(i, func);
        double bw = mBandwidthAdaptive ? resolveAdaptiveBandwidth(distances) : mBandwidth;
        if (bw <= 0.0)
        {
            bw = std::numeric_limits<double>::epsilon();
        }

        if (i == 0 || i == n - 1)
        {
            qDebug() << "[GwmSWIMTaskThread::fillWeightMatrix] Flow" << i << "bandwidth:" << bw << "distance range:" << *std::min_element(distances.begin(), distances.end()) << "-" << *std::max_element(distances.begin(), distances.end());
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
    
    qDebug() << "[GwmSWIMTaskThread::fillWeightMatrix] Weight matrix filled completely";
}

void GwmSWIMTaskThread::calculateWeightMatrix()
{
    int n = mFlowDataList.size();
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Starting weight matrix calculation";
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Flow data count:" << n;
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Bandwidth:" << mBandwidth << "Adaptive:" << mBandwidthAdaptive;
    
    mWeightMatrix = mat(n, n, fill::zeros);
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Weight matrix initialized:" << mWeightMatrix.n_rows << "x" << mWeightMatrix.n_cols;
    
    DistanceFunction func = distanceFunctionForMode();
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Distance function selected for mode:" << static_cast<int>(mSWIMMode);
    
    fillWeightMatrix(func);
    qDebug() << "[GwmSWIMTaskThread::calculateWeightMatrix] Weight matrix filled";
}

void GwmSWIMTaskThread::createResultLayer(CreateResultLayerData data)
{
    qDebug() << "[GwmSWIMTaskThread::createResultLayer] Creating result layer";
    qDebug() << "[GwmSWIMTaskThread::createResultLayer] Result data count:" << data.size();
    for (int i = 0; i < data.size(); ++i)
    {
        qDebug() << "[GwmSWIMTaskThread::createResultLayer] Result" << i << ":" << data[i].first << "size:" << data[i].second.n_rows << "x" << data[i].second.n_cols;
    }
    print_message(tr("Result layer creation not yet implemented."));
    qDebug() << "[GwmSWIMTaskThread::createResultLayer] Result layer creation completed (placeholder)";
}

void GwmSWIMTaskThread::performLocalRegression()
{
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Starting local regression";
    
    if (mDesignMatrix.n_rows == 0 || mWeightMatrix.n_rows == 0)
    {
        print_message(tr("Skipping regression because matrices are empty."));
        qDebug() << "[GwmSWIMTaskThread::performLocalRegression] ERROR: Matrices are empty";
        qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Design matrix:" << mDesignMatrix.n_rows << "x" << mDesignMatrix.n_cols;
        qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Weight matrix:" << mWeightMatrix.n_rows << "x" << mWeightMatrix.n_cols;
        return;
    }

    int n = static_cast<int>(mDesignMatrix.n_rows);
    int p = static_cast<int>(mDesignMatrix.n_cols);
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Regression parameters: n=" << n << "p=" << p;
    
    mLocalBetas = mat(n, p, fill::zeros);
    mFittedValues = vec(n, fill::zeros);
    mResiduals = vec(n, fill::zeros);
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Result matrices initialized";

    int successCount = 0;
    int skipCount = 0;
    
    for (int i = 0; i < n; ++i)
    {
        if (checkCanceled())
        {
            qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Calculation canceled at iteration" << i;
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

        vec beta;
        bool solved = solve(beta, XtWX, XtWy, solve_opts::fast + solve_opts::likely_sympd);
        if (!solved)
        {
            beta = pinv(XtWX) * XtWy;
        }

        if (beta.n_elem == static_cast<uword>(p))
        {
            mLocalBetas.row(i) = beta.t();
            mFittedValues(i) = dot(mDesignMatrix.row(i), beta);
            mResiduals(i) = mResponseVector(i) - mFittedValues(i);
            successCount++;
        }

        if (i % 100 == 0)
        {
            progress(i, n);
            if (i == 0 || i == n - 1)
            {
                qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Progress:" << i << "/" << n << "Success:" << successCount << "Skip:" << skipCount;
            }
        }
    }
    
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Regression completed";
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Success count:" << successCount << "Skip count:" << skipCount;
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Local betas size:" << mLocalBetas.n_rows << "x" << mLocalBetas.n_cols;
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Fitted values size:" << mFittedValues.n_elem;
    qDebug() << "[GwmSWIMTaskThread::performLocalRegression] Residuals size:" << mResiduals.n_elem;
}

