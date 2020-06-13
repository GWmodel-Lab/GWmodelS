#ifndef GWMBASICGWRALGORITHM_H
#define GWMBASICGWRALGORITHM_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"
#include "TaskThread/iparallelable.h"

class GwmBasicGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IIndependentVariableSelectable, public IOpenmpParallelable
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

public:
    GwmBasicGWRAlgorithm();

    bool autoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool value);

    bool autoselectIndepVars() const;
    void setIsAutoselectIndepVars(bool value);

    double indepVarSelectionThreshold() const;
    void setIndepVarSelectionThreshold(double indepVarSelectionThreshold);

    QgsVectorLayer* regressionLayer() const;
    void setRegressionLayer(QgsVectorLayer* layer);

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool value);

    bool hasFTest() const;
    void setHasFTest(bool value);

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    FTestResultPack fTestResult() const
    {
        return { mF1TestResult, mF2TestResult, mF3TestResult, mF4TestResult };
    }

    BandwidthCriterionList bandwidthSelectorCriterions() const;

    IndepVarsCriterionList indepVarSelectorCriterions() const;

public:     // GwmTaskThread interface
    QString name() const override { return tr("Basic GWR"); };


public:  // GwmSpatialAlgorithm interface
    bool isValid() override;


protected:  // QThread interface
    void run() override;


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
    mat regression(const mat& x, const vec& y) override;
    mat regression(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

public:     // IParallelalbe interface
    int parallelAbility() const override;
    ParallelType parallelType() const override;
    void setParallelType(const ParallelType &type) override;

public:     // IOpenmpParallelable interface
    void setOmpThreadNum(const int threadNum) override;

protected:
    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

private:
    GwmDiagnostic calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
    void createResultLayer(QList<QPair<QString, const mat> > data);
    double bandwidthSizeCriterionAIC(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionCV(GwmBandwidthWeight* bandwidthWeight);

    double indepVarsSelectCriterionSerial(const QList<GwmVariable>& indepVars);
    double indepVarsSelectCriterionOmp(const QList<GwmVariable>& indepVars);

    void fTest(FTestParameters params);

    double findMaxDistance();

    CalcTrQtQFunction calcTrQtQ()
    {
        return &GwmBasicGWRAlgorithm::calcTrQtQSerial;
    }
    double calcTrQtQSerial();

    CalcDiagBFunction calcDiagB()
    {
        return &GwmBasicGWRAlgorithm::calcDiagBSerial;
    }
    vec calcDiagBSerial(int i);

protected:
    GwmFTestResult mF1TestResult;
    GwmFTestResult mF2TestResult;
    QList<GwmFTestResult> mF3TestResult;
    GwmFTestResult mF4TestResult;


private:
    bool mHasHatMatrix = true;
    bool mHasFTest = false;

    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool mIsAutoselectBandwidth = false;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::AIC;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction;

    GwmIndependentVariableSelector mIndepVarSelector;
    bool mIsAutoselectIndepVars = false;
    double mIndepVarSelectionThreshold = 3.0;
    IndepVarsSelectCriterionFunction mIndepVarsSelectCriterionFunction;

    IParallelalbe::ParallelType mParallelType;
    int mOmpThreadNum = 8;
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

inline QgsVectorLayer *GwmBasicGWRAlgorithm::regressionLayer() const
{
    return mRegressionLayer;
}

inline void GwmBasicGWRAlgorithm::setRegressionLayer(QgsVectorLayer *layer)
{
    mRegressionLayer = layer;
}

inline GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType GwmBasicGWRAlgorithm::bandwidthSelectionCriterionType() const
{
    return mBandwidthSelectionCriterionType;
}

inline void GwmBasicGWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
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
    return IParallelalbe::Serial | IParallelalbe::OpenMP;
}

inline IParallelalbe::ParallelType GwmBasicGWRAlgorithm::parallelType() const
{
    return mParallelType;
}

inline void GwmBasicGWRAlgorithm::setParallelType(const IParallelalbe::ParallelType &type)
{
    mParallelType = (type & parallelAbility()) ? type : mParallelType;
}

inline void GwmBasicGWRAlgorithm::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

inline BandwidthCriterionList GwmBasicGWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}

#endif // GWMBASICGWRALGORITHM_H
