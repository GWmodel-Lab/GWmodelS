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
    // 根据带宽和Aic/cv值输出结果图
    static void plotBandwidthResult(QVariant data, QwtPlot *plot);
    // 获取bw和score
    QMap<double,double> getBwScore();

    void setX(const mat& x);
    void setY(const vec& y);

    double getLower() const;
    void setLower(double lower);

protected:
    void run() override;

protected:
    bool createdFromGWRTaskThread = false;
    double mLower = DBL_MAX;

    double gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive);

    double getFixedBwUpper();

    QString createOutputMessage(double bw, double score);

    QMap<double,double> mBwScore;

private:
    double cvAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);
    double aicAll(const mat& x, const vec& y, const mat& dp,double bw,int kernel,bool adaptive);
};

#endif // GWMBANDWIDTHSELECT_H
