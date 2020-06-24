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

public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSelectCriterionFunction)(bandwidthWeight);
    }
public:     // GwmTaskThread interface
    QString name() const override { return tr("GGWR"); };

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

    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCV;

    GwmGGWRBandwidthSizeSelector mBandwidthSizeSelector;

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

    bool gwrPoisson();
    bool gwrBinomial();

    mat diag(mat a);

    void PoissonWt(const mat& x, const vec& y,mat w,bool verbose = true);

    void BinomialWt(const mat& x, const vec& y,mat w,bool verbose = true);
//    void createResultLayer();

private:

    double bandwidthSizeGGWRCriterionCV(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeGGWRCriterionAIC(GwmBandwidthWeight* bandwidthWeight);

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


#endif // GWMGGWRALGORITHM_H
