#ifndef GWMMULTISCALEGWRTASKTHREAD_H
#define GWMMULTISCALEGWRTASKTHREAD_H

#include "gwmbandwidthsizeselector.h"
#include "gwmspatialmultiscalealgorithm.h"
#include "iregressionanalysis.h"

class GwmMultiscaleGWRAlgorithm : public GwmSpatialMultiscaleAlgorithm, public IRegressionAnalysis, public IBandwidthSizeSelectable
{
    Q_OBJECT

public:
    enum BandwidthInitilizeType
    {
        Null,
        Initial,
        Specified
    };
    static GwmEnumValueNameMapper<BandwidthInitilizeType> BandwidthInitilizeTypeNameMapper;

    enum BandwidthSelectionCriterionType
    {
        CV,
        AIC
    };
    static GwmEnumValueNameMapper<BandwidthSelectionCriterionType> BandwidthSelectionCriterionTypeNameMapper;

    enum BackFittingCriterionType
    {
        CVR,
        dCVR
    };
    static GwmEnumValueNameMapper<BackFittingCriterionType> BackFittingCriterionTypeNameMapper;

    typedef double (GwmMultiscaleGWRAlgorithm::*BandwidthSizeCriterionFunction)(GwmBandwidthWeight*);
    typedef mat (GwmMultiscaleGWRAlgorithm::*RegressionAllFunction)(const arma::mat&, const arma::vec&);
    typedef vec (GwmMultiscaleGWRAlgorithm::*RegressionVarFunction)(const arma::vec&, const arma::vec&, int, mat&);
    typedef QPair<QString, const mat> CreateResultLayerDataItem;

private:
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

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& S0, double RSS0);

public:
    GwmMultiscaleGWRAlgorithm();

public:
    void run() override;

    int bandwidthSelectRetryTimes() const;
    void setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes);

    int maxIteration() const;
    void setMaxIteration(int maxIteration);

    BackFittingCriterionType criterionType() const;
    void setCriterionType(const BackFittingCriterionType &criterionType);

    double criterionThreshold() const;
    void setCriterionThreshold(double criterionThreshold);

    int adaptiveLower() const;
    void setAdaptiveLower(int adaptiveLower);

    bool hasRegressionLayer()
    {
        return mRegressionLayer != nullptr;
    }


public:     // GwmTaskThread interface
    QString name() const override { return tr("Multiscale GWR"); }


public:     // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight *weight) override
    {
        return (this->*mBandwidthSizeCriterion)(weight);
    }


public:     // IRegressionAnalysis interface
    GwmVariable dependentVariable() const override;

    void setDependentVariable(const GwmVariable &variable) override;

    QList<GwmVariable> independentVariables() const override;

    void setIndependentVariables(const QList<GwmVariable> &variables) override;

    GwmDiagnostic diagnostic() const override;

    mat regression(const mat &x, const vec &y) override;

    QList<BandwidthInitilizeType> bandwidthInitilize() const;
    void setBandwidthInitilize(const QList<BandwidthInitilizeType> &bandwidthInitilize);

    QList<BandwidthSelectionCriterionType> bandwidthSelectionApproach() const;
    void setBandwidthSelectionApproach(const QList<BandwidthSelectionCriterionType> &bandwidthSelectionApproach);

    QList<bool> preditorCentered() const;
    void setPreditorCentered(const QList<bool> &preditorCentered);

    QList<double> bandwidthSelectThreshold() const;
    void setBandwidthSelectThreshold(const QList<double> &bandwidthSelectThreshold);

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool hasHatMatrix);

    mat betas() const;

protected:
    void initPoints();
    void initXY(mat& x, mat& y, const GwmVariable& depVar, const QList<GwmVariable>& indepVars);

    GwmBandwidthWeight* bandwidth(int i)
    {
        return static_cast<GwmBandwidthWeight*>(mSpatialWeights[i].weight());
    }

    mat regressionAllSerial(const mat& x, const vec& y);
    vec regressionVarSerial(const vec& x, const vec& y, const int var, mat& S);

    double findMaxDistance(int var);

    void createResultLayer(initializer_list<CreateResultLayerDataItem> data);

private:
    QgsVectorLayer* mRegressionLayer = nullptr;
    mat mDataPoints;
    mat mRegressionPoints;

    RegressionAllFunction mRegressionAll = &GwmMultiscaleGWRAlgorithm::regressionAllSerial;
    RegressionVarFunction mRegressionVar = &GwmMultiscaleGWRAlgorithm::regressionVarSerial;

    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;

    GwmSpatialWeight mInitSpatialWeight;
    BandwidthSizeCriterionFunction mBandwidthSizeCriterion = &GwmMultiscaleGWRAlgorithm::mBandwidthSizeCriterionAllCVSerial;
    double mBandwidthSizeCriterionAllCVSerial(GwmBandwidthWeight* bandwidthWeight);
    double mBandwidthSizeCriterionAllAICSerial(GwmBandwidthWeight* bandwidthWeight);
    int mBandwidthSelectionCurrentIndex = 0;
    double mBandwidthSizeCriterionVarCVSerial(GwmBandwidthWeight* bandwidthWeight);
    double mBandwidthSizeCriterionVarAICSerial(GwmBandwidthWeight* bandwidthWeight);

    QList<BandwidthInitilizeType> mBandwidthInitilize;
    QList<BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    QList<bool> mPreditorCentered;
    QList<double> mBandwidthSelectThreshold;
    uword mBandwidthSelectRetryTimes = 5;
    int mMaxIteration = 500;
    BackFittingCriterionType mCriterionType = BackFittingCriterionType::CVR;
    double mCriterionThreshold = 1e-6;
    int mAdaptiveLower = 10;

    bool mHasHatMatrix = true;

    mat mX;
    vec mY;
    mat mBetas;
    mat mBetasSE;

    mat mS0;
    cube mSArray;
    cube mC;
    mat mX0;
    vec mY0;

    double mRSS0;

    GwmDiagnostic mDiagnostic;
};

inline GwmVariable GwmMultiscaleGWRAlgorithm::dependentVariable() const
{
    return mDepVar;
}

inline void GwmMultiscaleGWRAlgorithm::setDependentVariable(const GwmVariable &variable)
{
    mDepVar = variable;
}

inline QList<GwmVariable> GwmMultiscaleGWRAlgorithm::independentVariables() const
{
    return mIndepVars;
}

inline void GwmMultiscaleGWRAlgorithm::setIndependentVariables(const QList<GwmVariable> &variables)
{
    mIndepVars = variables;
}

inline GwmDiagnostic GwmMultiscaleGWRAlgorithm::diagnostic() const
{
    return mDiagnostic;
}

inline int GwmMultiscaleGWRAlgorithm::adaptiveLower() const
{
    return mAdaptiveLower;
}

inline void GwmMultiscaleGWRAlgorithm::setAdaptiveLower(int adaptiveLower)
{
    mAdaptiveLower = adaptiveLower;
}

inline double GwmMultiscaleGWRAlgorithm::criterionThreshold() const
{
    return mCriterionThreshold;
}

inline void GwmMultiscaleGWRAlgorithm::setCriterionThreshold(double criterionThreshold)
{
    mCriterionThreshold = criterionThreshold;
}

inline GwmMultiscaleGWRAlgorithm::BackFittingCriterionType GwmMultiscaleGWRAlgorithm::criterionType() const
{
    return mCriterionType;
}

inline void GwmMultiscaleGWRAlgorithm::setCriterionType(const BackFittingCriterionType &criterionType)
{
    mCriterionType = criterionType;
}

inline int GwmMultiscaleGWRAlgorithm::maxIteration() const
{
    return mMaxIteration;
}

inline void GwmMultiscaleGWRAlgorithm::setMaxIteration(int maxIteration)
{
    mMaxIteration = maxIteration;
}

inline int GwmMultiscaleGWRAlgorithm::bandwidthSelectRetryTimes() const
{
    return mBandwidthSelectRetryTimes;
}

inline void GwmMultiscaleGWRAlgorithm::setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes)
{
    mBandwidthSelectRetryTimes = bandwidthSelectRetryTimes;
}

inline QList<bool> GwmMultiscaleGWRAlgorithm::preditorCentered() const
{
    return mPreditorCentered;
}

inline void GwmMultiscaleGWRAlgorithm::setPreditorCentered(const QList<bool> &preditorCentered)
{
    mPreditorCentered = preditorCentered;
}

inline QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> GwmMultiscaleGWRAlgorithm::bandwidthSelectionApproach() const
{
    return GwmMultiscaleGWRAlgorithm::mBandwidthSelectionApproach;
}

inline void GwmMultiscaleGWRAlgorithm::setBandwidthSelectionApproach(const QList<BandwidthSelectionCriterionType> &bandwidthSelectionApproach)
{
    mBandwidthSelectionApproach = bandwidthSelectionApproach;
}

inline QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> GwmMultiscaleGWRAlgorithm::bandwidthInitilize() const
{
    return GwmMultiscaleGWRAlgorithm::mBandwidthInitilize;
}

inline void GwmMultiscaleGWRAlgorithm::setBandwidthInitilize(const QList<BandwidthInitilizeType> &bandwidthInitilize)
{
    mBandwidthInitilize = bandwidthInitilize;
}

inline QList<double> GwmMultiscaleGWRAlgorithm::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

inline void GwmMultiscaleGWRAlgorithm::setBandwidthSelectThreshold(const QList<double> &bandwidthSelectThreshold)
{
    mBandwidthSelectThreshold = bandwidthSelectThreshold;
}

inline bool GwmMultiscaleGWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

inline void GwmMultiscaleGWRAlgorithm::setHasHatMatrix(bool hasHatMatrix)
{
    mHasHatMatrix = hasHatMatrix;
}

inline mat GwmMultiscaleGWRAlgorithm::betas() const
{
    return mBetas;
}

#endif // GWMMULTISCALEGWRTASKTHREAD_H
