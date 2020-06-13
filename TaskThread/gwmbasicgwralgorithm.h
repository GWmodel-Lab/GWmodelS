#ifndef GWMBASICGWRALGORITHM_H
#define GWMBASICGWRALGORITHM_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"


class GwmBasicGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IIndependentVariableSelectable
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

    typedef double (GwmBasicGWRAlgorithm::*BandwidthSizeCriterionFunction)(GwmBandwidthWeight*);
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


public:     // GwmTaskThread interface
    QString name() const override { return tr("Basic GWR"); };

    FTestResultPack fTestResult()
    {
        return { mF1TestResult, mF2TestResult, mF3TestResult, mF4TestResult };
    }


protected:  // GwmSpatialAlgorithm interface
    bool isValid() override;


protected:  // QThread interface
    void run() override;


public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSizeCriterion)(bandwidthWeight);
    }

public:     // IIndependentVariableSelectable interface
    double criterion(QList<GwmVariable> indepVars) override;

protected:  // IRegressionAnalysis interface
    mat regression(const mat& x, const vec& y) override;

protected:
    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    mat regression(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

private:

    GwmDiagnostic calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
    void createResultLayer(QList<QPair<QString, const mat> > data);
    double bandwidthSizeCriterionAIC(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionCV(GwmBandwidthWeight* bandwidthWeight);
    void fTest(FTestParameters params);

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
    BandwidthSizeCriterionFunction mBandwidthSizeCriterion;

    GwmIndependentVariableSelector mIndepVarSelector;
    bool mIsAutoselectIndepVars = false;
    double mIndepVarSelectionThreshold = 3.0;
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

#endif // GWMBASICGWRALGORITHM_H
