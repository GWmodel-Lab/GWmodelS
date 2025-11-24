#ifndef GWMSWIMTASKTHREAD_H
#define GWMSWIMTASKTHREAD_H

#include "TaskThread/gwmtaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmspatialweight.h"
#include <armadillo>
#include <QString>
#include <QList>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <QPair>

using namespace arma;

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
    QVector<double> independent_values;
};

enum class SWIMMode
{
    OriginFocused,
    DestinationFocused,
    FlowFocusedEuclidean,
    FlowFocusedSOP
};

struct GwmSWIMFieldMapping
{
    int flowVolume = -1;
    int originValue = -1;
    int destValue = -1;
    int originX = -1;
    int originY = -1;
    int destX = -1;
    int destY = -1;
    QList<int> independentVars;
    QStringList independentVarNames;
    bool requireOriginCoords = true;
    bool requireDestCoords = true;

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

    // Data configuration
    void setCsvFilePath(const QString& filePath);
    QString csvFilePath() const { return mCsvFilePath; }

    // Mode configuration
    void setSWIMMode(SWIMMode mode);
    SWIMMode swimMode() const { return mSWIMMode; }

    // Spatial weight configuration
    void setSpatialWeight(const GwmSpatialWeight& spatialWeight);
    GwmSpatialWeight spatialWeight() const { return mSpatialWeight; }

    // Parallel configuration
    int parallelAbility() const override;
    ParallelType parallelType() const override;
    void setParallelType(const ParallelType& type) override;
    void setOmpThreadNum(const int threadNum) override;

    void setFieldMapping(const GwmSWIMFieldMapping& mapping);
    GwmSWIMFieldMapping fieldMapping() const { return mFieldMapping; }
    void setFieldDelimiter(QChar delimiter);
    QChar fieldDelimiter() const { return mFieldDelimiter; }

    // Result accessors
    mat weightMatrix() const { return mWeightMatrix; }
    mat flowMatrix() const { return mFlowMatrix; }
    QList<GwmFlowData> flowData() const { return mFlowDataList; }
    CreateResultLayerData resultList() const { return mResultList; }

    QString name() const override { return tr("SWIM"); }

    bool isValid();

protected:
    void run() override;

private:
    // CSV helpers
    bool loadCsvData();
    bool parseCsvLine(const QString& line, GwmFlowData& flowData, int flowIndex);

    // Distance helpers
    double calculateOriginDistance(int i, int j);
    double calculateDestDistance(int i, int j);
    double calculateFlowEuclideanDistance(int i, int j);
    double calculateFlowSOPDistance(int i, int j);

    // Weight helpers
    void calculateWeightMatrix();
    void calculateOriginFocusedWeights();
    void calculateDestinationFocusedWeights();
    void calculateFlowFocusedEuclideanWeights();
    void calculateFlowFocusedSOPWeights();

    // Kernel helper
    double kernelFunction(double distance, double bandwidth);

    // Result helper
    void createResultLayer(CreateResultLayerData data);

private:
    QString mCsvFilePath;
    SWIMMode mSWIMMode = SWIMMode::OriginFocused;
    GwmSpatialWeight mSpatialWeight;

    QList<GwmFlowData> mFlowDataList;
    mat mWeightMatrix;
    mat mFlowMatrix;
    CreateResultLayerData mResultList;

    // Parallel parameters
    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

    // Bandwidth parameters
    double mBandwidth = 0.0;
    bool mBandwidthAdaptive = false;

    GwmSWIMFieldMapping mFieldMapping;
    QChar mFieldDelimiter = '\t';
    QStringList mIndependentVarNames;
};

inline bool GwmSWIMFieldMapping::isComplete() const
{
    if (flowVolume < 0) return false;
    if (requireOriginCoords && (originX < 0 || originY < 0)) return false;
    if (requireDestCoords && (destX < 0 || destY < 0)) return false;
    if (independentVars.isEmpty()) return false;
    return true;
}

inline bool GwmSWIMFieldMapping::isValid(int columnCount) const
{
    if (!isComplete()) return false;
    if (columnCount < 0) return true;
    auto checkIndex = [&](int idx) -> bool {
        return idx >= 0 && idx < columnCount;
    };
    if (!checkIndex(flowVolume)) return false;
    if (requireOriginCoords)
    {
        if (!checkIndex(originX) || !checkIndex(originY)) return false;
    }
    if (requireDestCoords)
    {
        if (!checkIndex(destX) || !checkIndex(destY)) return false;
    }
    if (originValue >= 0 && !checkIndex(originValue)) return false;
    if (destValue >= 0 && !checkIndex(destValue)) return false;
    for (int idx : independentVars)
    {
        if (!checkIndex(idx)) return false;
    }
    return true;
}

#endif // GWMSWIMTASKTHREAD_H

