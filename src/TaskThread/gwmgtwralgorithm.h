#ifndef GWMGTWRALGORITHM_H
#define GWMGTWRALGORITHM_H

#include <QObject>

#include "TaskThread/gwmspatialtemporalmonoscale.h"
#include "TaskThread/iregressionanalysis.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include <gwmodel.h>

class GwmGTWRAlgorithm : public GwmSpatialTemporalMonoscaleAlgorithm, public IRegressionAnalysis, public IBandwidthSizeSelectable, public gwm::IParallelizable, public gwm::IParallelOpenmpEnabled
{
    Q_OBJECT

public:
    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    static vec Fitted(const mat& x, const mat& betas)
    {
        return sum(betas % x, 1);
    }

    static double RSS(const mat& x, const mat& y, const mat& betas)
    {
        vec r = y - Fitted(x, betas);
        return sum(r % r);
    }

    static double AICc(const mat& x, const mat& y, const mat& betas, const vec& shat)
    {
        double ss = RSS(x, y, betas), n = x.n_rows;
        return n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    }

    typedef QList<QPair<QString, mat> > CreateResultLayerData;
    typedef double (GwmGTWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);
    typedef mat (GwmGTWRAlgorithm::*RegressionFunctionType)(const mat&, const vec&);
    typedef mat (GwmGTWRAlgorithm::*RegressionHatmatrixFunctionType)(const mat&, const vec&, mat&, vec&, vec&);

public:
    GwmGTWRAlgorithm();

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool hasHatMatrix);

    GwmVariable timeVar() const;
    void setTimeVar(const GwmVariable &timeVar);

    QgsVectorLayer *regressionLayer() const;
    void setRegressionLayer(QgsVectorLayer *regressionLayer);

    bool isAutoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool isAutoselectBandwidth);

    bool hasPredict() const;
    void setHasPredict(bool hasPredict);

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    mat betas() const;

    BandwidthCriterionList bandwidthSelectorCriterions() const;

    // QThread interface
protected:
    void run() override;

    // GwmSpatialAlgorithm interface
public:
    bool isValid() override;

    // IRegressionAnalysis interface
public:
    GwmVariable dependentVariable() const override;
    void setDependentVariable(const GwmVariable &variable) override;
    QList<GwmVariable> independentVariables() const override;
    void setIndependentVariables(const QList<GwmVariable> &variables) override;
    GwmDiagnostic diagnostic() const override;
    mat regression(const mat &x, const vec &y) override;

    // IBandwidthSizeSelectable interface
public:
    double criterion(GwmBandwidthWeight *weight) override;

    // gwm::IParallelizable interface
public:
    int parallelAbility() const override;
    gwm::ParallelType parallelType() const override;
    void setParallelType(const gwm::ParallelType &type) override;

    // gwm::IParallelOpenmpEnabled interface
public:
    void setOmpThreadNum(const int threadNum) override;

public:
    bool hasRegressionLayer();
    void setCanceled(bool canceled) override;
    //记录标签
    static int treeChildCount;

protected:
    virtual void initPoints();
    virtual void initXY(mat& x, mat& y, const GwmVariable& depVar, const QList<GwmVariable>& indepVars);

private:
    mat regressionSerial(const mat& x, const vec& y);
#ifdef ENABLE_OpenMP
    mat regressionOmp(const mat& x, const vec& y);
#endif
    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag);
#ifdef ENABLE_OpenMP
    mat regressionHatmatrixOmp(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag);
#endif

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* bandwidthWeight);
#ifdef ENABLE_OpenMP
    double bandwidthSizeCriterionCVOmp(GwmBandwidthWeight* bandwidthWeight);
#endif

    double bandwidthSizeCriterionAICSerial(GwmBandwidthWeight* bandwidthWeight);
#ifdef ENABLE_OpenMP
    double bandwidthSizeCriterionAICOmp(GwmBandwidthWeight* bandwidthWeight);
#endif

    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    void createResultLayer(CreateResultLayerData data, QString name = QStringLiteral("_GTWR"));

protected:
    QgsVectorLayer* mRegressionLayer = nullptr;
    mat mDataPoints;
    mat mRegressionPoints;
    vec mDataTimeStamp;
    vec mRegTimeStamp;

    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmVariable mTimeVar;

    GwmDiagnostic mDiagnostic;

    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool mIsAutoselectBandwidth = false;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = CV;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGTWRAlgorithm::bandwidthSizeCriterionCVSerial;

    mat mX;
    vec mY;
    mat mBetas;
    mat mBetasSE;
    vec mSHat;
    vec mQDiag;

    RegressionFunctionType mRegressionFunction = &GwmGTWRAlgorithm::regressionSerial;
    RegressionHatmatrixFunctionType mRegressionHatmatrixFunction = &GwmGTWRAlgorithm::regressionHatmatrixSerial;

    bool mHasRegressionLayerXY = false;
    bool mHasPredict = false;
    vec mRegressionLayerY;
    mat mRegressionLayerX;

    gwm::ParallelType mParallelType = gwm::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

    bool mHasHatMatrix = true;

    // below are variables and functions added for library functions
private:
    std::unique_ptr<gwm::GTWR> mGTWRCore;

    gwm::BandwidthCriterionList mCriterionList;
    gwm::RegressionDiagnostic mDiagnostic0;
public:
    gwm::SpatialWeight convertSpatialWeight();
    void updateLocalSpatialWeight(gwm::BandwidthWeight* bw);
};

inline GwmVariable GwmGTWRAlgorithm::dependentVariable() const
{
    return mDepVar;
}

inline void GwmGTWRAlgorithm::setDependentVariable(const GwmVariable &variable)
{
    mDepVar = variable;
}

inline QList<GwmVariable> GwmGTWRAlgorithm::independentVariables() const
{
    return mIndepVars;
}

inline void GwmGTWRAlgorithm::setIndependentVariables(const QList<GwmVariable> &variables)
{
    mIndepVars = variables;
}

inline GwmDiagnostic GwmGTWRAlgorithm::diagnostic() const
{
    if (mGTWRCore)
    {
        gwm::RegressionDiagnostic diag = mGTWRCore->diagnostic();
        return { diag.RSS, diag.AIC, diag.AICc, diag.ENP, diag.EDF, diag.RSquare, diag.RSquareAdjust };
    }
    return mDiagnostic;
}

inline bool GwmGTWRAlgorithm::hasRegressionLayer()
{
    return mRegressionLayer != nullptr;
}

inline GwmGTWRAlgorithm::BandwidthSelectionCriterionType GwmGTWRAlgorithm::bandwidthSelectionCriterionType() const
{
    return mBandwidthSelectionCriterionType;
}

inline void GwmGTWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    // mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    // QMap<QPair<BandwidthSelectionCriterionType, gwm::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::SerialOnly), &GwmGTWRAlgorithm::bandwidthSizeCriterionCVSerial),
    // #ifdef ENABLE_OpenMP
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::OpenMP), &GwmGTWRAlgorithm::bandwidthSizeCriterionCVOmp),
    // #endif
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::SerialOnly), &GwmGTWRAlgorithm::bandwidthSizeCriterionAICSerial),
    // #ifdef ENABLE_OpenMP
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::OpenMP), &GwmGTWRAlgorithm::bandwidthSizeCriterionAICOmp),
    // #endif
    // };
    // mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

inline bool GwmGTWRAlgorithm::hasPredict() const
{
    return mHasPredict;
}

inline void GwmGTWRAlgorithm::setHasPredict(bool hasPredict)
{
    mHasPredict = hasPredict;
}

inline bool GwmGTWRAlgorithm::isAutoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

inline void GwmGTWRAlgorithm::setIsAutoselectBandwidth(bool isAutoselectBandwidth)
{
    mIsAutoselectBandwidth = isAutoselectBandwidth;
}

inline QgsVectorLayer *GwmGTWRAlgorithm::regressionLayer() const
{
    return mRegressionLayer;
}

inline void GwmGTWRAlgorithm::setRegressionLayer(QgsVectorLayer *regressionLayer)
{
    mRegressionLayer = regressionLayer;
}

inline GwmVariable GwmGTWRAlgorithm::timeVar() const
{
    return mTimeVar;
}

inline void GwmGTWRAlgorithm::setTimeVar(const GwmVariable &timeVar)
{
    mTimeVar = timeVar;
}

inline bool GwmGTWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

inline void GwmGTWRAlgorithm::setHasHatMatrix(bool hasHatMatrix)
{
    mHasHatMatrix = hasHatMatrix;
}

inline double GwmGTWRAlgorithm::criterion(GwmBandwidthWeight *weight)
{
    return (this->*mBandwidthSelectCriterionFunction)(weight);
}

inline mat GwmGTWRAlgorithm::betas() const
{
    return mBetas;
}

inline BandwidthCriterionList GwmGTWRAlgorithm::bandwidthSelectorCriterions() const
{
    //return mBandwidthSizeSelector.bandwidthCriterion();
    return mCriterionList;
}

inline int GwmGTWRAlgorithm::parallelAbility() const
{
    return gwm::ParallelType::SerialOnly
    #ifdef ENABLE_OpenMP
            | gwm::ParallelType::OpenMP
    #endif
            ;
}

inline gwm::ParallelType GwmGTWRAlgorithm::parallelType() const
{
    return mParallelType;
}

inline void GwmGTWRAlgorithm::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
    if (mGTWRCore)
    {
        mGTWRCore->setOmpThreadNum(threadNum);
    }
}



#endif // GWMGTWRALGORITHM_H
