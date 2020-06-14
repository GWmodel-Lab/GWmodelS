#ifndef GWMSCALABLEGWRTASKTHREAD_H
#define GWMSCALABLEGWRTASKTHREAD_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"

class GwmScalableGWRTaskThread : public GwmGeographicalWeightedRegressionAlgorithm
{
    Q_OBJECT

public:
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

    typedef QPair<QString, const mat> CreateResultLayerDataItem;

private:
    static GwmDiagnostic CalcDiagnostic(const vec& y, const mat& x, const mat& betas, const vec &shat);


public:
    GwmScalableGWRTaskThread();

    void run() override;

    int getPolynomial() const;
    void setPolynomial(int polynomial);

    double getCV() const;
    double getScale() const;
    double getPenalty() const;


public:     // GwmTaskThread interface
    QString name() const override { return tr("Scalable GWR"); }


public:     // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // IRegressionAnalysis interface
    arma::mat regression(const arma::mat &x, const arma::vec &y) override;

private:
    umat getNeighbours(mat& dists);
    double optimize(const mat& Mx0, const mat& My0, double& b_tilde, double& alpha);
    void prepare();

    void createResultLayer(initializer_list<CreateResultLayerDataItem> data);


private:
    int mPolynomial = 4;
    int mMaxIter = 500;
    double mCV = 0.0;
    double mScale = 1.0;
    double mPenalty = 0.01;

    mat mG0;
    umat mNeighbours;
    mat mNeighbourDists;
    mat mMx0;
    mat mMy0;
    vec mShat;
    mat mBetasSE;
};

inline void GwmScalableGWRTaskThread::setPolynomial(int polynomial)
{
    mPolynomial = polynomial;
}

inline double GwmScalableGWRTaskThread::getPenalty() const
{
    return mPenalty;
}

inline double GwmScalableGWRTaskThread::getScale() const
{
    return mScale;
}

inline double GwmScalableGWRTaskThread::getCV() const
{
    return mCV;
}

inline int GwmScalableGWRTaskThread::getPolynomial() const
{
    return mPolynomial;
}

#endif // GWMSCALABLEGWRTASKTHREAD_H
