#ifndef GWMGWRTASKTHREAD_H
#define GWMGWRTASKTHREAD_H

#include "gwmtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>

#include "Model/gwmlayerattributeitem.h"

using namespace arma;

struct GwmGWRDiagnostic
{
    double RSS;
    double AIC;
    double AICc;
    double ENP;
    double EDF;
    double RSquare;
    double RSquareAdjust;

    GwmGWRDiagnostic()
    {
        RSS = 0.0;
        AIC = 0.0;
        AICc = 0.0;
        ENP = 0.0;
        EDF = 0.0;
        RSquare = 0.0;
        RSquareAdjust = 0.0;
    }

    GwmGWRDiagnostic(const vec& diag)
    {
        AIC = diag(0);
        AICc = diag(1);
        EDF = diag(2);
        ENP = diag(3);
        RSS = diag(4);
        RSquare = diag(5);
        RSquareAdjust = diag(6);
    }
};

class GwmGWRTaskThread : public GwmTaskThread
{
    Q_OBJECT

public:
    static QMap<QString, double> fixedBwUnitDict;
    static QMap<QString, double> adaptiveBwUnitDict;
    static void initUnitDict();

public:

    enum BandwidthType
    {
        Adaptive,
        Fixed
    };

    enum ParallelMethod
    {
        None,
        Multithread,
        GPU
    };

    enum KernelFunction
    {
        Gaussian,
        Exponential,
        Bisquare,
        Tricube,
        Boxcar
    };

    enum DistanceSourceType
    {
        CRS,
        Minkowski,
        DMatFile
    };

    enum BandwidthSelectionApproach
    {
        CV,
        AIC
    };

public:
    GwmGWRTaskThread();
    GwmGWRTaskThread(const GwmGWRTaskThread& taskThread);

protected:
    void run() override;
    QString name() const override;

public:
    bool isValid(QString& message);
    void setBandwidth(BandwidthType type, double size, QString unit);

    QgsVectorLayer *layer() const;
    void setLayer(QgsVectorLayer *layer);

    QList<GwmLayerAttributeItem *> indepVars() const;
    void setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars);

    GwmLayerAttributeItem *depVar() const;
    void setDepVar(GwmLayerAttributeItem *depVar);

    BandwidthType bandwidthType() const;
    void setBandwidthType(const BandwidthType &bandwidthType);

    double getBandwidthSize() const;

    double getBandwidthSizeOrigin() const;

    QString getBandwidthUnit() const;

    bool getIsBandwidthSizeAutoSel() const;
    void setIsBandwidthSizeAutoSel(bool value);

    KernelFunction getBandwidthKernelFunction() const;
    void setBandwidthKernelFunction(const KernelFunction &bandwidthKernelFunction);

    DistanceSourceType getDistSrcType() const;
    void setDistSrcType(const DistanceSourceType &distSrcType);

    QVariant getDistSrcParameters() const;
    void setDistSrcParameters(const QVariant &distSrcParameters);

    ParallelMethod getParallelMethodType() const;
    void setParallelMethodType(const ParallelMethod &parallelMethodType);

    QVariant getParallelParameter() const;
    void setParallelParameter(const QVariant &parallelParameter);

    QgsVectorLayer *getResultLayer() const;

    bool enableIndepVarAutosel() const;
    void setEnableIndepVarAutosel(bool value);

    QgsFeatureList getFeatureList() const;

    GwmGWRDiagnostic getDiagnostic() const;

    mat getBetas() const;

    BandwidthSelectionApproach getBandwidthSelectionApproach() const;
    void setBandwidthSelectionApproach(const BandwidthSelectionApproach &bandwidthSelectionApproach);

    QList<QStringList> getModelSelModels() const;

    QList<double> getModelSelAICcs() const;

    int getDepVarIndex() const;

    QList<int> getIndepVarsIndex() const;

protected:
    // 图层和 X Y 属性列
    QgsVectorLayer* mLayer = nullptr;
    GwmLayerAttributeItem* mDepVar = nullptr;
    QList<GwmLayerAttributeItem*> mIndepVars;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    bool isEnableIndepVarAutosel = false;
    QgsFeatureList mFeatureList;  // 预收集的要素列表

    bool hasHatMatrix = true;

    // 优选模型的结果
    QList<QStringList> mModelSelModels;
    QList<double> mModelSelAICcs;

    // 带宽配置
    BandwidthType mBandwidthType = BandwidthType::Adaptive;
    double mBandwidthSize = 0.0;
    double mBandwidthSizeOrigin = 0.0;
    QString mBandwidthUnit = QStringLiteral("x1");
    bool isBandwidthSizeAutoSel = true;
    BandwidthSelectionApproach mBandwidthSelectionApproach = BandwidthSelectionApproach::CV;
    KernelFunction mBandwidthKernelFunction = KernelFunction::Gaussian;

    // 距离配置
    DistanceSourceType mDistSrcType = DistanceSourceType::CRS;
    QVariant mDistSrcParameters = QVariant();

    // 坐标系配置
    double mCRSRotateTheta = 0.0;
    double mCRSRotateP = 0.0;

    // 并行配置
    ParallelMethod mParallelMethodType = ParallelMethod::None;
    QVariant mParallelParameter = 0;

    // 计算用的矩阵
    mat mX;
    vec mY;
    mat mBetas;
    mat mRowSumBetasSE;
    mat mBetasSE;
    mat mBetasTV;
    mat mDataPoints;
    vec mSHat;
    vec mQDiag;
    vec mYHat;
    vec mResidual;
    vec mStudentizedResidual;
    vec mLocalRSquare;

    // 结果
    GwmGWRDiagnostic mDiagnostic;
    QgsVectorLayer* mResultLayer;

protected:
    bool isNumeric(QVariant::Type type);
    bool setXY();

    vec distance(int focus);
    vec distanceCRS(int focus);
    vec distanceMinkowski(int focus);

    void diagnostic();

    void createResultLayer();
};

#endif // GWMGWRTASKTHREAD_H
