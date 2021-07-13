#ifndef GWMBASICGWRALGORITHM_H
#define GWMBASICGWRALGORITHM_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"
#include "TaskThread/iparallelable.h"

#ifdef ENABLE_CUDA
#include "GWmodelCUDA/IGWmodelCUDA.h"
#endif

class GwmBasicGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IIndependentVariableSelectable, public IOpenmpParallelable, public ICudaParallelable
{
    Q_OBJECT

public:

    struct FTestResultPack
    {
        GwmFTestResult f1;
        GwmFTestResult f2;
        QList<GwmFTestResult> f3;
        GwmFTestResult f4;
    };

    struct FTestParameters
    {
        int nDp = 0;
        int nVar = 0;
        double trS = 0.0;
        double trStS = 0.0;
        double gwrRSS = 0.0;
        double trQ = 0.0;
        double trQtQ = 0.0;
    };

    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };
    static GwmEnumValueNameMapper<BandwidthSelectionCriterionType> BandwidthSelectionCriterionTypeNameMapper;

    typedef double (GwmBasicGWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);
    typedef double (GwmBasicGWRAlgorithm::*IndepVarsSelectCriterionFunction)(const QList<GwmVariable>&);
    typedef double (GwmBasicGWRAlgorithm::*CalcTrQtQFunction)();
    typedef vec (GwmBasicGWRAlgorithm::*CalcDiagBFunction)(int);
    typedef mat (GwmBasicGWRAlgorithm::*Regression)(const mat&, const vec&);
    typedef mat (GwmBasicGWRAlgorithm::*RegressionHatmatrix)(const mat&, const vec&, mat&, vec&, vec&, mat&);

    typedef QList<QPair<QString, mat> > CreateResultLayerData;

private:
    GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

public:
    GwmBasicGWRAlgorithm();

    bool autoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool value);

    bool autoselectIndepVars() const;
    void setIsAutoselectIndepVars(bool value);

    double indepVarSelectionThreshold() const;
    void setIndepVarSelectionThreshold(double indepVarSelectionThreshold);

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool value);

    bool hasFTest() const;
    void setHasFTest(bool value);

    int groupSize() const;
    void setGroupSize(int groupSize);

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    FTestResultPack fTestResult() const
    {
        return { mF1TestResult, mF2TestResult, mF3TestResult, mF4TestResult };
    }

    BandwidthCriterionList bandwidthSelectorCriterions() const;

    IndepVarsCriterionList indepVarSelectorCriterions() const;

    bool hasPredict() const;
    void setHasPredict(bool hasPredict);

    void setCanceled(bool canceled);

protected:  // QThread interface
    void run() override;


public:     // GwmTaskThread interface
    QString name() const override { return tr("Basic GWR"); };


public:  // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // GwmGeographicalWeightedRegressionAlgorithm
    virtual void initPoints();
    virtual void initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars) override;


public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSelectCriterionFunction)(bandwidthWeight);
    }

public:     // IIndependentVariableSelectable interface
    double criterion(const QList<GwmVariable>& indepVars) override
    {
        return (this->*mIndepVarsSelectCriterionFunction)(indepVars);
    }

protected:  // IRegressionAnalysis interface
    virtual mat regression(const mat& x, const vec& y) override;

public:     // IParallelalbe interface
    int parallelAbility() const override;
    ParallelType parallelType() const override;

    void setParallelType(const ParallelType &type) override;

public:     // IOpenmpParallelable interface
    void setOmpThreadNum(const int threadNum) override;


public:     // ICudaParallelable interface
    void setGPUId(const int gpuId) override;

protected:
    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    rowvec distanceParam1(int i)
    {
        return (mSpatialWeight.distance()->type() == GwmDistance::DMatDistance ? vec(1).fill(i) : mDataPoints.row(i));
    }
protected:

    void createResultLayer(CreateResultLayerData data,QString name = QStringLiteral("_GWR"));

#ifdef ENABLE_CUDA
    void initCuda(IGWmodelCUDA *cuda, const mat &x, const vec &y);
#endif

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionAICSerial(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionCVOmp(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionAICOmp(GwmBandwidthWeight* bandwidthWeight); 
#ifdef ENABLE_CUDA
    double bandwidthSizeCriterionCVCuda(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionAICCuda(GwmBandwidthWeight* bandwidthWeight);
#endif

    double indepVarsSelectCriterionSerial(const QList<GwmVariable>& indepVars);
    double indepVarsSelectCriterionOmp(const QList<GwmVariable>& indepVars);
#ifdef ENABLE_CUDA
    double indepVarsSelectCriterionCuda(const QList<GwmVariable>& indepVars);
#endif

    mat regressionSerial(const mat& x, const vec& y);
    mat regressionOmp(const mat& x, const vec& y);
#ifdef ENABLE_CUDA
    mat regressionCuda(const mat& x, const vec& y);
#endif

    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
    mat regressionHatmatrixOmp(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
#ifdef ENABLE_CUDA
    mat regressionHatmatrixCuda(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
#endif

    void fTest(FTestParameters params);

    double calcTrQtQSerial();
    double calcTrQtQOmp();
#ifdef ENABLE_CUDA
    double calcTrQtQCuda();
#endif

    vec calcDiagBSerial(int i);
    vec calcDiagBOmp(int i);
#ifdef ENABLE_CUDA
    vec calcDiagBCuda(int i);
#endif

protected:
    GwmFTestResult mF1TestResult;
    GwmFTestResult mF2TestResult;
    QList<GwmFTestResult> mF3TestResult;
    GwmFTestResult mF4TestResult;

    bool mHasHatMatrix = true;
    bool mHasFTest = false;
    bool mHasPredict = false;

    vec mQDiag;
    mat mBetasSE;

    vec mShat;
    mat mS;

    bool mHasRegressionLayerXY = false;
    vec mRegressionLayerY;
    mat mRegressionLayerX;

    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool mIsAutoselectBandwidth = false;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::AIC;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVSerial;

    GwmIndependentVariableSelector mIndepVarSelector;
    bool mIsAutoselectIndepVars = false;
    double mIndepVarSelectionThreshold = 3.0;
    IndepVarsSelectCriterionFunction mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial;
    int mIndepVarSelectModelsTotalNum = 0;
    int mIndepVarSelectModelsCurrent = 0;

    Regression mRegressionFunction = &GwmBasicGWRAlgorithm::regressionSerial;
    RegressionHatmatrix mRegressionHatmatrixFunction = &GwmBasicGWRAlgorithm::regressionHatmatrixSerial;

    CalcTrQtQFunction mCalcTrQtQFunction = &GwmBasicGWRAlgorithm::calcTrQtQSerial;
    CalcDiagBFunction mCalcDiagBFunction = &GwmBasicGWRAlgorithm::calcDiagBSerial;

    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;
    int mGpuId = 0;
    int mGroupSize = 64;

};

inline bool GwmBasicGWRAlgorithm::hasFTest() const
{
    return mHasFTest;
}

inline void GwmBasicGWRAlgorithm::setHasFTest(bool value)
{
    mHasFTest = value;
}

inline bool GwmBasicGWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

inline void GwmBasicGWRAlgorithm::setHasHatMatrix(bool value)
{
    mHasHatMatrix = value;
}

inline double GwmBasicGWRAlgorithm::indepVarSelectionThreshold() const
{
    return mIndepVarSelectionThreshold;
}

inline void GwmBasicGWRAlgorithm::setIndepVarSelectionThreshold(double indepVarSelectionThreshold)
{
    mIndepVarSelectionThreshold = indepVarSelectionThreshold;
}

inline GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType GwmBasicGWRAlgorithm::bandwidthSelectionCriterionType() const
{
    return mBandwidthSelectionCriterionType;
}

inline bool GwmBasicGWRAlgorithm::autoselectIndepVars() const
{
    return mIsAutoselectIndepVars;
}

inline void GwmBasicGWRAlgorithm::setIsAutoselectIndepVars(bool value)
{
    mIsAutoselectIndepVars = value;
}

inline bool GwmBasicGWRAlgorithm::autoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

inline void GwmBasicGWRAlgorithm::setIsAutoselectBandwidth(bool value)
{
    mIsAutoselectBandwidth = value;
}

inline IndepVarsCriterionList GwmBasicGWRAlgorithm::indepVarSelectorCriterions() const
{
    return mIndepVarSelector.indepVarsCriterion();
}

inline int GwmBasicGWRAlgorithm::parallelAbility() const
{
    return IParallelalbe::SerialOnly
            | IParallelalbe::OpenMP
        #ifdef ENABLE_CUDA
            | IParallelalbe::CUDA
        #endif
            ;
}

inline IParallelalbe::ParallelType GwmBasicGWRAlgorithm::parallelType() const
{
    return mParallelType;
}

inline void GwmBasicGWRAlgorithm::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

inline void GwmBasicGWRAlgorithm::setGPUId(const int gpuId)
{
    mGpuId = gpuId;
}

inline BandwidthCriterionList GwmBasicGWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}

inline bool GwmBasicGWRAlgorithm::hasPredict() const
{
    return mHasPredict;
}

inline void GwmBasicGWRAlgorithm::setHasPredict(bool hasPredict)
{
    mHasPredict = hasPredict;
}

#endif // GWMBASICGWRALGORITHM_H
