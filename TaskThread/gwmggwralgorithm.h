#ifndef GWMGGWRALGORITHM_H
#define GWMGGWRALGORITHM_H

#include "gwmbasicgwralgorithm.h"
#include "gwmggwrbandwidthsizeselector.h"

struct GwmGGWRDiagnostic
{
    double RSS;
    double AIC;
    double AICc;
    double RSquare;

    GwmGGWRDiagnostic()
    {
        AIC = 0.0;
        AICc = 0.0;
        RSS = 0.0;
        RSquare = 0.0;
    }

    GwmGGWRDiagnostic(const vec& diag)
    {
        AIC = diag(0);
        AICc = diag(1);
        RSS = diag(2);
        RSquare = diag(3);
    }
};

struct GwmGLMDiagnostic
{
    double NullDev;
    double Dev;
    double AIC;
    double AICc;
    double RSquare;

    GwmGLMDiagnostic()
    {
        AIC = 0.0;
        AICc = 0.0;
        Dev = 0.0;
        NullDev = 0.0;
        RSquare = 0.0;
    }

    GwmGLMDiagnostic(const vec& diag)
    {
        AIC = diag(0);
        AICc = diag(1);
        NullDev = diag(2);
        Dev = diag(3);
        RSquare = diag(4);
    }
};

class GwmGGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IOpenmpParallelable
{
public:
    enum Family
    {
        Poisson,
        Binomial
    };

    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };

    static QMap<QString, double> TolUnitDict;
    static void initTolUnitDict();

    typedef double (GwmGGWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);
    typedef mat (GwmGGWRAlgorithm::*GGWRRegressionFunction)(const mat& x, const vec& y);
    typedef mat (GwmGGWRAlgorithm::*CalWtFunction)(const mat& x, const vec& y,mat w);

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;


public:
    GwmGGWRAlgorithm();

public:     // GwmTaskThread interface
    QString name() const override { return tr("GGWR"); };

public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSelectCriterionFunction)(bandwidthWeight);
    }


public:     // IRegressionAnalysis interface
    arma::mat regression(const arma::mat &x, const arma::vec &y)
    {
        return (this->*mGGWRRegressionFunction)(x, y);
    }


public:     // IParallelalbe interface
    int parallelAbility() const;

    ParallelType parallelType() const;
    void setParallelType(const ParallelType &type) override;


public:     // IOpenmpParallelable interface
    void setOmpThreadNum(const int threadNum);

public:
    static vec gwReg(const mat& x, const vec &y, const vec &w, int focus);

    static vec gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri);

    static mat dpois(mat y,mat mu);
    static mat dbinom(mat y,mat m,mat mu);
    static mat lchoose(mat n,mat k);
    static mat lgammafn(mat x);

    static mat CiMat(const mat& x, const vec& w);

protected:
    void run() override;

    mat regressionPoissonSerial(const mat& x, const vec& y);
    mat regressionBinomialSerial(const mat& x, const vec& y);

    mat regressionPoissonOmp(const mat& x, const vec& y);
    mat regressionBinomialOmp(const mat& x, const vec& y);

    mat diag(mat a);

    mat PoissonWtSerial(const mat& x, const vec& y,mat w);
    mat BinomialWtSerial(const mat& x, const vec& y,mat w);

    mat PoissonWtOmp(const mat& x, const vec& y,mat w);
    mat BinomialWtOmp(const mat& x, const vec& y,mat w);

    void createResultLayer(CreateResultLayerData data,QString name = QStringLiteral("_GWR"));

private:

    double bandwidthSizeGGWRCriterionCVSerial(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeGGWRCriterionAICSerial(GwmBandwidthWeight* bandwidthWeight);

    double bandwidthSizeGGWRCriterionCVOmp(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeGGWRCriterionAICOmp(GwmBandwidthWeight* bandwidthWeight);

public:
    Family getFamily() const;
    double getTol() const;
    int getMaxiter() const;

    mat getWtMat1() const;
    mat getWtMat2() const;

    GwmGGWRDiagnostic getDiagnostic() const;

    GwmGLMDiagnostic getGLMDiagnostic() const;

    bool setFamily(Family family);
    void setTol(double tol, QString unit);
    void setMaxiter(int maxiter);

    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    BandwidthCriterionList bandwidthSelectorCriterions() const;


protected:
    Family mFamily;
    double mTol;
    QString mTolUnit;
    int mMaxiter;

    bool mHasHatMatrix = true;
    bool mHasFTest = false;

    vec mQDiag;
    mat mBetasSE;

    vec mShat;
    mat mS;

    mat mWtMat1;
    mat mWtMat2;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    CreateResultLayerData mResultList;

    mat mWt2;
    mat myAdj;

    double mLLik = 0;

    GGWRRegressionFunction mGGWRRegressionFunction = &GwmGGWRAlgorithm::regressionPoissonSerial;
    CalWtFunction mCalWtFunction = &GwmGGWRAlgorithm::PoissonWtSerial;

    bool mIsAutoselectBandwidth = false;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial;
    GwmGGWRBandwidthSizeSelector mBandwidthSizeSelector;
};


inline GwmGGWRAlgorithm::Family GwmGGWRAlgorithm::getFamily() const
{
    return mFamily;
}

inline double GwmGGWRAlgorithm::getTol() const
{
    return mTol;
}

inline int GwmGGWRAlgorithm::getMaxiter() const
{
    return mMaxiter;
}

inline mat GwmGGWRAlgorithm::getWtMat1() const
{
    return mWtMat1;
}

inline mat GwmGGWRAlgorithm::getWtMat2() const
{
    return mWtMat2;
}

inline GwmGGWRDiagnostic GwmGGWRAlgorithm::getDiagnostic() const
{
    return mDiagnostic;
}

inline GwmGLMDiagnostic GwmGGWRAlgorithm::getGLMDiagnostic() const
{
    return mGLMDiagnostic;
}

inline void GwmGGWRAlgorithm::setTol(double tol, QString unit){
    mTolUnit = unit;
    mTol = double(tol) * TolUnitDict[unit];
}

inline void GwmGGWRAlgorithm::setMaxiter(int maxiter){
    mMaxiter = maxiter;
}

inline BandwidthCriterionList GwmGGWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}

#endif // GWMGGWRALGORITHM_H
