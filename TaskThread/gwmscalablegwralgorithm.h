#ifndef GWMSCALABLEGWRTASKTHREAD_H
#define GWMSCALABLEGWRTASKTHREAD_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"

class GwmScalableGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm
{
    Q_OBJECT

public:
    enum ParameterOptimizeCriterionType
    {
        AIC,
        CV
    };

    struct LoocvParams
    {
        const mat* x;
        const mat* y;
        const int bw;
        const int polynomial;
        const mat* Mx0;
        const mat* My0;
    };
    static double Loocv(const vec& target, const mat& x, const vec& y, int bw, int poly, const mat& Mx0, const mat& My0);
    static double AICvalue(const vec& target, const mat& x, const vec& y, int bw, int poly, const mat& Mx0, const mat& My0);

    typedef QPair<QString, const mat> CreateResultLayerDataItem;

private:
    static GwmDiagnostic CalcDiagnostic(const vec& y, const mat& x, const mat& betas, const vec &shat);


public:
    GwmScalableGWRAlgorithm();

    void run() override;

    int polynomial() const;
    void setPolynomial(int polynomial);

    double cv() const;
    double scale() const;
    double penalty() const;

    bool hasPredict() const;
    void setHasPredict(bool hasPredict);

    ParameterOptimizeCriterionType parameterOptimizeCriterion() const;
    void setParameterOptimizeCriterion(const ParameterOptimizeCriterionType &parameterOptimizeCriterion);


public:     // GwmTaskThread interface
    QString name() const override { return tr("Scalable GWR"); }


public:     // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // IRegressionAnalysis interface
    arma::mat regression(const arma::mat &x, const arma::vec &y) override
    {
        return regressionHatmatrixSerial(x, y);
    }

protected:
    void initPoints() override;
    void initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars) override;

private:
    void findDataPointNeighbours();
    mat findNeighbours(const GwmSpatialWeight& spatialWeight, umat &nnIndex);
    double optimize(const mat& Mx0, const mat& My0, double& b_tilde, double& alpha);
    void prepare();

    mat regressionSerial(const arma::mat& x, const arma::vec& y);
    mat regressionHatmatrixSerial(const arma::mat &x, const arma::vec &y);

    void createResultLayer(initializer_list<CreateResultLayerDataItem> data);

    bool mHasPredict = false;

private:
    int mPolynomial = 4;
    int mMaxIter = 500;
    double mCV = 0.0;
    double mScale = 1.0;
    double mPenalty = 0.01;

    bool hasRegressionLayerXY = false;
    vec mRegressionLayerY;
    mat mRegressionLayerX;

    bool mHasHatMatrix = true;

    GwmSpatialWeight mDpSpatialWeight;

    ParameterOptimizeCriterionType mParameterOptimizeCriterion = ParameterOptimizeCriterionType::CV;

    mat mG0;
    umat mDpNNIndex;
    mat mDpNNDists;
//    umat mRpNNIndex;
//    mat mRpNNDists;
    mat mMx0;
    mat mMxx0;
    mat mMy0;
    vec mShat;
    mat mBetasSE;

public:
    static int treeChildCount;
};



inline void GwmScalableGWRAlgorithm::setPolynomial(int polynomial)
{
    mPolynomial = polynomial;
}

inline double GwmScalableGWRAlgorithm::penalty() const
{
    return mPenalty;
}

inline double GwmScalableGWRAlgorithm::scale() const
{
    return mScale;
}

inline double GwmScalableGWRAlgorithm::cv() const
{
    return mCV;
}

inline int GwmScalableGWRAlgorithm::polynomial() const
{
    return mPolynomial;
}

inline GwmScalableGWRAlgorithm::ParameterOptimizeCriterionType GwmScalableGWRAlgorithm::parameterOptimizeCriterion() const
{
    return mParameterOptimizeCriterion;
}

inline void GwmScalableGWRAlgorithm::setParameterOptimizeCriterion(const ParameterOptimizeCriterionType &parameterOptimizeCriterion)
{
    mParameterOptimizeCriterion = parameterOptimizeCriterion;
}

#endif // GWMSCALABLEGWRTASKTHREAD_H
