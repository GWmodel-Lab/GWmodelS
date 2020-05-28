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

struct GwmFTestParameters
{
    int nDp;
    int nVar;
    double trS;
    double trStS;
    double gwrRSS;
    double bw;
    bool adaptive;
    int kernel;
    double trQ;
    double trQtQ;

    GwmFTestParameters ()
    {
        trS = 0.0;
        trStS = 0.0;
        nDp = 0.0;
        gwrRSS = 0.0;
        nVar = 0.0;
        bw = DBL_MAX;
        adaptive = false;
        kernel = 0;
        trQ = 0.0;
        trQtQ = 0.0;
    }
};

struct GwmFTestResult
{
    double s;
    double df1;
    double df2;
    double p;

    GwmFTestResult()
    {
        s = 0.0;
        df1 = 0.0;
        df2 = 0.0;
        p = 0.0;
    }

    GwmFTestResult(double s, double n, double d, double p)
    {
        s = s;
        df1 = n;
        df2 = d;
        p = p;
    }
};

class GwmGWRTaskThread;

typedef bool (GwmGWRTaskThread::*RegressionAll)(bool, mat&);
typedef double (GwmGWRTaskThread::*CalcTrQtQ)();
typedef vec (GwmGWRTaskThread::*CalcDiagB)(int);

class GwmGWRTaskThread : public GwmTaskThread
{
    Q_OBJECT

public:
    static QMap<QString, double> fixedBwUnitDict;
    static QMap<QString, double> adaptiveBwUnitDict;

    static RegressionAll regressionAll[];
    static CalcTrQtQ calcTrQtQ[];
    static CalcDiagB calcDiagB[];

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
    QString name() const override;
    void run() override;

    virtual bool regressionAllSerial(bool hatmatrix, mat& S);
    virtual bool regressionAllOmp(bool hatmatrix, mat& S);
    bool gwrCalibration();
//    bool regressionAllCuda(bool hatmatrix, mat& S);

    virtual double calcTrQtQSerial();
    virtual double calcTrQtQOmp();
//    double trQtQCuda();

    bool isNumeric(QVariant::Type type);
    virtual bool setXY();

    vec distance(int focus);
    vec distanceCRS(int focus);
    vec distanceMinkowski(int focus);
    vec distanceDmat(int focus);

    virtual void diagnostic();

    virtual void f1234Test(const GwmFTestParameters& params);
    virtual vec calcDiagBSerial(int i);
    virtual vec calcDiagBOmp(int i);

    virtual void createResultLayer();

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

    QMap<double, double> getBwScore() const;

    double getModelSelThreshold() const;
    void setModelSelThreshold(double modelSelThreshold);

    QList<GwmFTestResult> fTestResults() const;

    bool getHasFTest() const;
    void setHasFTest(bool value);

    bool getHasHatMatrix() const;
    void setHasHatMatrix(bool value);

    QgsVectorLayer *getRegressionLayer() const;
    void setRegressionLayer(QgsVectorLayer *regressionLayer);

protected:
    // 图层和 X Y 属性列
    QgsVectorLayer* mLayer = nullptr;
    GwmLayerAttributeItem* mDepVar = nullptr;
    QList<GwmLayerAttributeItem*> mIndepVars;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    bool isEnableIndepVarAutosel = false;
    QgsVectorLayer* mRegressionLayer = nullptr;  // 回归点
    QgsFeatureList mFeatureList;  // 预收集的要素列表，如果有回归点则是回归点要素，否则是数据点要素

    // 检验配置
    bool hasHatMatrix = true;
    bool hasFTest = false;

    // 优选模型的结果
    double mModelSelThreshold;
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

    // 优选带宽的结果
    QMap<double, double> mBandwidthSelScore = QMap<double, double>();

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
    vec mWeightMask;
    mat mBetas;
    mat mRowSumBetasSE;
    mat mBetasSE;
    mat mBetasTV;
    mat mDataPoints;
    mat mRegPoints;
    vec mSHat;
    vec mQDiag;
    vec mYHat;
    vec mResidual;
    vec mStudentizedResidual;
    vec mLocalRSquare;

    // F检验结果
    GwmFTestResult mF1Result;
    GwmFTestResult mF2Result;
    QList<GwmFTestResult> mF3Result;
    GwmFTestResult mF4Result;

    // 结果
    GwmGWRDiagnostic mDiagnostic;
    QgsVectorLayer* mResultLayer;
};

#endif // GWMGWRTASKTHREAD_H
