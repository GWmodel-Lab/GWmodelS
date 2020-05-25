#ifndef GWMSCALABLEGWRTASKTHREAD_H
#define GWMSCALABLEGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"

struct GwmScalableGWRLoocvParams
{
    const mat* x;
    const mat* y;
    const int bw;
    const double polynomial;
    const mat* Mx0;
    const mat* My0;
};

class GwmScalableGWRTaskThread : public GwmGWRTaskThread
{
    Q_OBJECT
public:
    GwmScalableGWRTaskThread();

    void run() override;
    void diagnostic() override;

protected:
    void getNeighbours();
    double optimize(const mat& x, const mat& y, uword bw, double P, const mat& Mx0, const mat& My0, double& b_tilde, double& alpha);

protected:
    int mPolynomial = 4;
    int mMaxIter = 500;
    double mCV = 0.0;
    double mScale = 1.0;
    double mPenalty = 0.01;

    umat mNeighbours;
    mat mNeighbourDists;
};

#endif // GWMSCALABLEGWRTASKTHREAD_H
