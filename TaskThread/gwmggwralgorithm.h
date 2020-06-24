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

class GwmGGWRAlgorithm : public GwmBasicGWRAlgorithm
{
public:
    GwmGGWRAlgorithm();
public:
    static QMap<QString, double> TolUnitDict;
    static void initTolUnitDict();

public:
    enum Family
    {
        Poisson,
        Binomial
    };

    typedef double (GwmGGWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);
    typedef bool (GwmGGWRAlgorithm::*GGWRRegressionFunction)();
    typedef mat (GwmGGWRAlgorithm::*CalWtFunction)(const mat& x, const vec& y,mat w);

public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSelectCriterionFunction)(bandwidthWeight);
    }
public:     // GwmTaskThread interface
    QString name() const override { return tr("GGWR"); };

public:     // IParallelalbe interface

    void setParallelType(const ParallelType &type) override;

protected:
    Family mFamily;
    double mTol;
    QString mTolUnit;
    int mMaxiter;

    mat mWtMat1;
    mat mWtMat2;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    CreateResultLayerData mResultList;

    mat mWt2;
    mat myAdj;

    double mLLik = 0;

    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial;

    GwmGGWRBandwidthSizeSelector mBandwidthSizeSelector;

    GGWRRegressionFunction mGGWRRegressionFunction = &GwmGGWRAlgorithm::gwrPoissonSerial;
    CalWtFunction mCalWtFunction = &GwmGGWRAlgorithm::PoissonWtSerial;

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

    bool gwrPoissonSerial();
    bool gwrBinomialSerial();

    bool gwrPoissonOmp();
    bool gwrBinomialOmp();

    mat diag(mat a);

    mat PoissonWtSerial(const mat& x, const vec& y,mat w);

    mat BinomialWtSerial(const mat& x, const vec& y,mat w);

    mat PoissonWtOmp(const mat& x, const vec& y,mat w);
    mat BinomialWtOmp(const mat& x, const vec& y,mat w);
//    void createResultLayer();

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
    bool setTol(double tol, QString unit);
    bool setMaxiter(int maxiter);

    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    BandwidthCriterionList bandwidthSelectorCriterions() const;

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

inline bool GwmGGWRAlgorithm::setFamily(Family family){
    mFamily = family;
    QMap<QPair<Family, IParallelalbe::ParallelType>, GGWRRegressionFunction> mapper = {
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::gwrPoissonSerial),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::gwrPoissonOmp),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::gwrPoissonSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::gwrBinomialSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::gwrBinomialOmp),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::gwrBinomialSerial)
    };
    mGGWRRegressionFunction = mapper[qMakePair(family, mParallelType)];
    QMap<QPair<Family, IParallelalbe::ParallelType>, CalWtFunction> mapper1 = {
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::PoissonWtSerial),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::PoissonWtOmp),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::PoissonWtSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::BinomialWtSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::BinomialWtOmp),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::BinomialWtSerial)
    };
    mCalWtFunction = mapper1[qMakePair(family, mParallelType)];
    return true;
}

inline bool GwmGGWRAlgorithm::setTol(double tol, QString unit){
    mTolUnit = unit;
    mTol = double(tol) * TolUnitDict[unit];
    return true;
}

inline bool GwmGGWRAlgorithm::setMaxiter(int maxiter){
    mMaxiter = maxiter;
    return true;
}

inline BandwidthCriterionList GwmGGWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}

#endif // GWMGGWRALGORITHM_H
