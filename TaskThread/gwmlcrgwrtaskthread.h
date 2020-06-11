#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

class GwmLcrGWRTaskThread;
typedef double (GwmLcrGWRTaskThread::*pfGwmLcrBandwidthSelectionApproach)(double , int , bool, double, bool, double );

class GwmLcrGWRTaskThread:public GwmGWRTaskThread
{
public:
    GwmLcrGWRTaskThread();
    double mlambda;
    //
    bool mlambdaAdjust;
    //
    double mcnThresh;
    //
    bool madaptive;
protected:
    void run() override;

    void createResultLayer() override;
    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);
    //返回cv.contrib的函数
    vec LcrCVContrib(double bw, int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //黄金分割函数
    double gold(pfGwmLcrBandwidthSelectionApproach p,double xL, double xU, bool adaptBw, int kernel, bool adaptive,double lambda, bool lambdaAdjust,double cnThreshd);
    //带宽选择函数
    double LcrBandWidthSelect(int kernel, double lambda, bool lambdaAdjust, double cnThresh, bool adaptive);
    //
    double getFixedBwUpper();
    //
};

#endif // GWMLCRGWRTASKTHREAD_H
