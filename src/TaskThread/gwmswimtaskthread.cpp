#include "gwmswimtaskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"
#include "SpatialWeight/gwmcrsdistance.h"
#include <SpatialWeight/gwmminkwoskidistance.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include "gwmapp.h"
#include <armadillo>
using namespace arma;

#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

GwmSWIMTaskThread::GwmSWIMTaskThread(QObject *parent)
    : GwmTaskThread()
{
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
    if (spatialWeight.weight())
    {
        if (auto* bw = dynamic_cast<GwmBandwidthWeight*>(spatialWeight.weight()))
        {
            mBandwidth = bw->bandwidth();
            mBandwidthAdaptive = bw->adaptive();
        }
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

    if (mFlowDataList.isEmpty())
    {
        print_error(tr("No flow data loaded."));
        return false;
    }

    return true;
}

void GwmSWIMTaskThread::run()
{
    emit tick(0, 0);

    // Step 1: load CSV data
    if (!checkCanceled())
    {
        emit message(tr("Loading CSV data..."));
        if (!loadCsvData())
        {
            emit error(tr("Failed to load CSV data."));
            return;
        }
        emit tick(20, 100);
    }

    // Step 2: compute weight matrix
    if (!checkCanceled())
    {
        emit message(tr("Calculating weight matrix..."));
        calculateWeightMatrix();
        emit tick(60, 100);
    }

    // Step 3: create result layers
    if (!checkCanceled())
    {
        emit message(tr("Creating result layer..."));
        mat flowVolumeMat = mat(mFlowDataList.size(), 1);
        for (int i = 0; i < mFlowDataList.size(); i++)
        {
            flowVolumeMat(i, 0) = mFlowDataList[i].flow_volume;
        }
        mResultList.push_back(qMakePair(QString("FlowVolume"), flowVolumeMat));

        mResultList.push_back(qMakePair(QString("WeightMatrix"), mWeightMatrix));
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

    return true;
}

double GwmSWIMTaskThread::calculateOriginDistance(int i, int j)
{
    if (i < 0 || i >= mFlowDataList.size() || j < 0 || j >= mFlowDataList.size())
        return 0.0;

    const GwmFlowData& flow1 = mFlowDataList[i];
    const GwmFlowData& flow2 = mFlowDataList[j];

    double dx = flow1.origin_x - flow2.origin_x;
    double dy = flow1.origin_y - flow2.origin_y;
    return std::sqrt(dx * dx + dy * dy);
}

double GwmSWIMTaskThread::calculateDestDistance(int i, int j)
{
    if (i < 0 || i >= mFlowDataList.size() || j < 0 || j >= mFlowDataList.size())
        return 0.0;

    const GwmFlowData& flow1 = mFlowDataList[i];
    const GwmFlowData& flow2 = mFlowDataList[j];

    double dx = flow1.dest_x - flow2.dest_x;
    double dy = flow1.dest_y - flow2.dest_y;
    return std::sqrt(dx * dx + dy * dy);
}

double GwmSWIMTaskThread::calculateFlowEuclideanDistance(int i, int j)
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

double GwmSWIMTaskThread::calculateFlowSOPDistance(int i, int j)
{
    // 轨迹距离(SOP): d(i,i') + d(j,j')
    double originDist = calculateOriginDistance(i, j);
    double destDist = calculateDestDistance(i, j);
    return originDist + destDist;
}

double GwmSWIMTaskThread::kernelFunction(double distance, double bandwidth)
{
    if (bandwidth <= 0.0)
        return 0.0;

    double ratio = distance / bandwidth;
    return std::exp(-0.5 * ratio * ratio);
}

void GwmSWIMTaskThread::calculateWeightMatrix()
{
    int n = mFlowDataList.size();
    mWeightMatrix = mat(n, n, fill::zeros);

    switch (mSWIMMode)
    {
    case SWIMMode::OriginFocused:
        calculateOriginFocusedWeights();
        break;
    case SWIMMode::DestinationFocused:
        calculateDestinationFocusedWeights();
        break;
    case SWIMMode::FlowFocusedEuclidean:
        calculateFlowFocusedEuclideanWeights();
        break;
    case SWIMMode::FlowFocusedSOP:
        calculateFlowFocusedSOPWeights();
        break;
    }
}

void GwmSWIMTaskThread::calculateOriginFocusedWeights()
{
    int n = mFlowDataList.size();

#ifdef ENABLE_OpenMP
    if (mParallelType == IParallelalbe::ParallelType::OpenMP)
    {
        omp_set_num_threads(mOmpThreadNum);
#pragma omp parallel for
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double dist = calculateOriginDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
        }
    }
    else
#endif
    {
        for (int i = 0; i < n; i++)
        {
            if (checkCanceled()) return;
            for (int j = 0; j < n; j++)
            {
                double dist = calculateOriginDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
            if (i % 100 == 0)
            {
                progress(i, n);
            }
        }
    }
}

void GwmSWIMTaskThread::calculateDestinationFocusedWeights()
{
    int n = mFlowDataList.size();

#ifdef ENABLE_OpenMP
    if (mParallelType == IParallelalbe::ParallelType::OpenMP)
    {
        omp_set_num_threads(mOmpThreadNum);
#pragma omp parallel for
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double dist = calculateDestDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
        }
    }
    else
#endif
    {
        for (int i = 0; i < n; i++)
        {
            if (checkCanceled()) return;
            for (int j = 0; j < n; j++)
            {
                double dist = calculateDestDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
            if (i % 100 == 0)
            {
                progress(i, n);
            }
        }
    }
}

void GwmSWIMTaskThread::calculateFlowFocusedEuclideanWeights()
{
    int n = mFlowDataList.size();

#ifdef ENABLE_OpenMP
    if (mParallelType == IParallelalbe::ParallelType::OpenMP)
    {
        omp_set_num_threads(mOmpThreadNum);
#pragma omp parallel for
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double dist = calculateFlowEuclideanDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
        }
    }
    else
#endif
    {
        for (int i = 0; i < n; i++)
        {
            if (checkCanceled()) return;
            for (int j = 0; j < n; j++)
            {
                double dist = calculateFlowEuclideanDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
            if (i % 100 == 0)
            {
                progress(i, n);
            }
        }
    }
}

void GwmSWIMTaskThread::calculateFlowFocusedSOPWeights()
{
    int n = mFlowDataList.size();

#ifdef ENABLE_OpenMP
    if (mParallelType == IParallelalbe::ParallelType::OpenMP)
    {
        omp_set_num_threads(mOmpThreadNum);
#pragma omp parallel for
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double dist = calculateFlowSOPDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
        }
    }
    else
#endif
    {
        for (int i = 0; i < n; i++)
        {
            if (checkCanceled()) return;
            for (int j = 0; j < n; j++)
            {
                double dist = calculateFlowSOPDistance(i, j);
                mWeightMatrix(i, j) = kernelFunction(dist, mBandwidth);
            }
            if (i % 100 == 0)
            {
                progress(i, n);
            }
        }
    }
}

void GwmSWIMTaskThread::createResultLayer(CreateResultLayerData data)
{
    // TODO: 实现结果图层的创建
    // 这里需要根据实际需求创建QgsVectorLayer并添加结果数据
    print_message(tr("Result layer creation not yet implemented."));
}

