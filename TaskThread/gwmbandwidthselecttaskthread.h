#ifndef GWMBANDWIDTHSELECT_H
#define GWMBANDWIDTHSELECT_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>

class GwmBandwidthSelectTaskThread;
typedef double (GwmBandwidthSelectTaskThread::*pfApproach)(const mat& , const vec& , const mat& , double , int , bool );

class GwmBandwidthSelectTaskThread:public GwmGWRTaskThread
{
    Q_OBJECT
public:
    GwmBandwidthSelectTaskThread();

    GwmBandwidthSelectTaskThread(const GwmGWRTaskThread& gwrTaskThread);

public:
    enum Approach
    {
        CV,
        AIC
    };

protected:
    void run() override;

private:
    bool createdFromGWRTaskThread = false;

    Approach mApproach = Approach::CV;

    double cvAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);
    double aicAll(const mat& x, const vec& y, const mat& dp,double bw,int kernel,bool adaptive);

    double gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive);

    double getFixedBwUpper();

    QString createOutputMessage(double bw, double score);
};

#endif // GWMBANDWIDTHSELECT_H
