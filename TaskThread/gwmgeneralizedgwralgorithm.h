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

class GwmGeneralizedGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IOpenmpParallelable
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

    typedef double (GwmGeneralizedGWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);
    typedef mat (GwmGeneralizedGWRAlgorithm::*GGWRRegressionFunction)(const mat& x, const vec& y);
    typedef mat (GwmGeneralizedGWRAlgorithm::*CalWtFunction)(const mat& x, const vec& y,mat w);

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;


public:
    GwmGeneralizedGWRAlgorithm();

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

    void CalGLMModel(const mat& x, const vec& y);

    void createResultLayer(CreateResultLayerData data,QString name = QStringLiteral("_GGWR"));

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
    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;

    bool autoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool value);

    QgsVectorLayer* regressionLayer() const;
    void setRegressionLayer(QgsVectorLayer* layer);

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool value);



protected:
    Family mFamily;
    double mTol;
    QString mTolUnit;
    int mMaxiter;

    bool mHasHatMatrix = true;

    mat mBetasSE;

    vec mShat;
    mat mS;
    double mGwDev;

    mat mWtMat1;
    mat mWtMat2;

    GwmGGWRDiagnostic mDiagnostic;
    GwmGLMDiagnostic mGLMDiagnostic;
    CreateResultLayerData mResultList;

    mat mWt2;
    mat myAdj;

    double mLLik = 0;

    GGWRRegressionFunction mGGWRRegressionFunction = &GwmGeneralizedGWRAlgorithm::regressionPoissonSerial;
    CalWtFunction mCalWtFunction = &GwmGeneralizedGWRAlgorithm::PoissonWtSerial;

    bool mIsAutoselectBandwidth = false;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::AIC;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial;
    GwmGGWRBandwidthSizeSelector mBandwidthSizeSelector;

    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;
};


inline GwmGeneralizedGWRAlgorithm::Family GwmGeneralizedGWRAlgorithm::getFamily() const
{
    return mFamily;
}

inline double GwmGeneralizedGWRAlgorithm::getTol() const
{
    return mTol;
}

inline int GwmGeneralizedGWRAlgorithm::getMaxiter() const
{
    return mMaxiter;
}

inline mat GwmGeneralizedGWRAlgorithm::getWtMat1() const
{
    return mWtMat1;
}

inline mat GwmGeneralizedGWRAlgorithm::getWtMat2() const
{
    return mWtMat2;
}

inline GwmGGWRDiagnostic GwmGeneralizedGWRAlgorithm::getDiagnostic() const
{
    return mDiagnostic;
}

inline GwmGLMDiagnostic GwmGeneralizedGWRAlgorithm::getGLMDiagnostic() const
{
    return mGLMDiagnostic;
}

inline void GwmGeneralizedGWRAlgorithm::setTol(double tol, QString unit){
    mTolUnit = unit;
    mTol = double(tol) * TolUnitDict[unit];
}

inline void GwmGeneralizedGWRAlgorithm::setMaxiter(int maxiter){
    mMaxiter = maxiter;
}

inline BandwidthCriterionList GwmGeneralizedGWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}

inline bool GwmGeneralizedGWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

inline void GwmGeneralizedGWRAlgorithm::setHasHatMatrix(bool value)
{
    mHasHatMatrix = value;
}

inline QgsVectorLayer *GwmGeneralizedGWRAlgorithm::regressionLayer() const
{
    return mRegressionLayer;
}

inline void GwmGeneralizedGWRAlgorithm::setRegressionLayer(QgsVectorLayer *layer)
{
    mRegressionLayer = layer;
}

inline GwmGeneralizedGWRAlgorithm::BandwidthSelectionCriterionType GwmGeneralizedGWRAlgorithm::bandwidthSelectionCriterionType() const
{
    return mBandwidthSelectionCriterionType;
}

inline bool GwmGeneralizedGWRAlgorithm::autoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

inline void GwmGeneralizedGWRAlgorithm::setIsAutoselectBandwidth(bool value)
{
    mIsAutoselectBandwidth = value;
}

inline int GwmGeneralizedGWRAlgorithm::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP | IParallelalbe::CUDA;
}

inline IParallelalbe::ParallelType GwmGeneralizedGWRAlgorithm::parallelType() const
{
    return mParallelType;
}

inline void GwmGeneralizedGWRAlgorithm::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

#endif // GWMGGWRALGORITHM_H
