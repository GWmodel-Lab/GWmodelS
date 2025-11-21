#ifndef GWMSWIMTASKTHREAD_H
#define GWMSWIMTASKTHREAD_H

#include "TaskThread/gwmtaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmspatialweight.h"
#include <armadillo>
#include <QString>
#include <QList>
#include <QMap>

using namespace arma;

// SWIM数据流结构
struct GwmFlowData
{
    int flow_id;
    int origin_id;
    int dest_id;
    double flow_volume;
    double origin_value;
    double dest_value;
    double origin_x;
    double origin_y;
    double dest_x;
    double dest_y;
};

// SWIM模式枚举
enum class SWIMMode
{
    OriginFocused,      // 以起点为中心
    DestinationFocused, // 以终点为中心
    FlowFocusedEuclidean, // 以流动为中心 - 四维欧氏距离
    FlowFocusedSOP      // 以流动为中心 - 轨迹距离(SOP)
};

struct GwmSWIMFieldMapping
{
    int flowId = -1;
    int originId = -1;
    int destId = -1;
    int flowVolume = -1;
    int originValue = -1;
    int destValue = -1;
    int originX = -1;
    int originY = -1;
    int destX = -1;
    int destY = -1;

    bool isComplete() const;
    bool isValid(int columnCount) const;
};

class GwmSWIMTaskThread : public GwmTaskThread, public IOpenmpParallelable
{
    Q_OBJECT

public:
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

public:
    explicit GwmSWIMTaskThread(QObject *parent = nullptr);
    ~GwmSWIMTaskThread();

    // 数据设置
    void setCsvFilePath(const QString& filePath);
    QString csvFilePath() const { return mCsvFilePath; }

    // 模式设置
    void setSWIMMode(SWIMMode mode);
    SWIMMode swimMode() const { return mSWIMMode; }

    // 空间权重设置
    void setSpatialWeight(const GwmSpatialWeight& spatialWeight);
    GwmSpatialWeight spatialWeight() const { return mSpatialWeight; }

    // 并行设置
    int parallelAbility() const override;
    ParallelType parallelType() const override;
    void setParallelType(const ParallelType& type) override;
    void setOmpThreadNum(const int threadNum) override;

    void setFieldMapping(const GwmSWIMFieldMapping& mapping);
    GwmSWIMFieldMapping fieldMapping() const { return mFieldMapping; }
    void setFieldDelimiter(QChar delimiter);
    QChar fieldDelimiter() const { return mFieldDelimiter; }

    // 结果获取
    mat weightMatrix() const { return mWeightMatrix; }
    mat flowMatrix() const { return mFlowMatrix; }
    QList<GwmFlowData> flowData() const { return mFlowDataList; }
    CreateResultLayerData resultList() const { return mResultList; }

    QString name() const override { return tr("SWIM"); }

    bool isValid();

protected:
    void run() override;

private:
    // 数据加载
    bool loadCsvData();
    bool parseCsvLine(const QString& line, GwmFlowData& flowData);

    // 距离计算
    double calculateOriginDistance(int i, int j);  // 起点间距离
    double calculateDestDistance(int i, int j);    // 终点间距离
    double calculateFlowEuclideanDistance(int i, int j);  // 四维欧氏距离
    double calculateFlowSOPDistance(int i, int j); // 轨迹距离(SOP)

    // 权重计算
    void calculateWeightMatrix();
    void calculateOriginFocusedWeights();
    void calculateDestinationFocusedWeights();
    void calculateFlowFocusedEuclideanWeights();
    void calculateFlowFocusedSOPWeights();

    // 核函数
    double kernelFunction(double distance, double bandwidth);

    // 结果创建
    void createResultLayer(CreateResultLayerData data);

private:
    QString mCsvFilePath;
    SWIMMode mSWIMMode = SWIMMode::OriginFocused;
    GwmSpatialWeight mSpatialWeight;

    QList<GwmFlowData> mFlowDataList;
    mat mWeightMatrix;
    mat mFlowMatrix;
    CreateResultLayerData mResultList;

    // 并行参数
    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

    // 带宽参数（从spatialWeight中获取）
    double mBandwidth = 0.0;
    bool mBandwidthAdaptive = false;

    GwmSWIMFieldMapping mFieldMapping;
    QChar mFieldDelimiter = '\t';
};

inline bool GwmSWIMFieldMapping::isComplete() const
{
    return flowId >= 0 && originId >= 0 && destId >= 0 &&
           flowVolume >= 0 && originValue >= 0 && destValue >= 0 &&
           originX >= 0 && originY >= 0 && destX >= 0 && destY >= 0;
}

inline bool GwmSWIMFieldMapping::isValid(int columnCount) const
{
    if (!isComplete()) return false;
    if (columnCount < 0) return true;
    QList<int> indices = {flowId, originId, destId, flowVolume, originValue,
                          destValue, originX, originY, destX, destY};
    for (int idx : indices)
    {
        if (idx < 0 || idx >= columnCount) return false;
    }
    return true;
}

#endif // GWMSWIMTASKTHREAD_H

