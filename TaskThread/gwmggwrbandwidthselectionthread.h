#ifndef GWMGGWRBANDWIDTHSELECTIONTHREAD_H
#define GWMGGWRBANDWIDTHSELECTIONTHREAD_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmbandwidthselecttaskthread.h"
#include "gwmggwrtaskthread.h"

class GwmGGWRBandWidthSelectionThread;
typedef double (GwmGGWRBandWidthSelectionThread::*pfApproach1)(const mat& , const vec& , const mat& , double , int , bool );

class GwmGGWRBandWidthSelectionThread : public GwmBandwidthSelectTaskThread
{
//public:

//    enum Family
//    {
//        Poisson,
//        Binomial
//    };

public:
    GwmGGWRBandWidthSelectionThread();

    GwmGGWRBandWidthSelectionThread(const GwmGGWRTaskThread& gwrTaskThread);

    GwmGGWRTaskThread::Family mFamily;

    mat cvContrib(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);

    double mTol;
    int mMaxiter;

protected:
    void run();

protected:
    double cvAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);

    double aicAll(const mat& x, const vec& y, const mat& dp,double bw,int kernel,bool adaptive);

    void PoissonWt(const mat& x, const vec& y,double bw,mat w,bool verbose = true);

    void BinomialWt(const mat& x, const vec& y,double bw,mat w,bool verbose = true);

    double gold(pfApproach1 p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive);

    mat mWt2;
    double mLLik;
    mat myAdj;
};

#endif // GWMGGWRBANDWIDTHSELECTIONTHREAD_H
