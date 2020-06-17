#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H

//#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

//class GwmLcrGWRTaskThread;
//typedef double (GwmLcrGWRTaskThread::*pfGwmLcrBandwidthSelectionApproach)(double , int , bool, double, bool, double );

class GwmLcrGWRTaskThread:public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable
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
    //bool isValid(QString &message) override;
    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    void createResultLayer(CreateResultLayerData data);

    double criterion(GwmBandwidthWeight *weight);

    bool mIsAutoselectBandwidth = false;

//    double findMaxDistance()
//    {
//        int nDp = mDataPoints.n_rows;
//        double maxD = 0.0;
//        for (int i = 0; i < nDp; i++)
//        {
//            double d = max(mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints));
//            maxD = d > maxD ? d : maxD;
//        }
//        return maxD;
//    }


protected:
    void run() override;

    //void createResultLayer() override;
    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);
    //返回cv.contrib的函数
    //vec LcrCVContrib(double bw, int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //黄金分割函数
    //double gold(pfGwmLcrBandwidthSelectionApproach p,double xL, double xU, bool adaptBw, int kernel, bool adaptive,double lambda, bool lambdaAdjust,double cnThreshd);
    //带宽选择函数
    //double LcrBandWidthSelect(int kernel, double lambda, bool lambdaAdjust, double cnThresh, bool adaptive);
    //
    double getFixedBwUpper();
    //

    // IRegressionAnalysis interface
public:
    arma::mat regression(const arma::mat &x, const arma::vec &y);

    GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    GwmDiagnostic getDialnostic() const{
        return mDiagnostic;
    }
    bool getIsAutoselectBandwidth() const;

    GwmBandwidthSizeSelector selector;

    inline BandwidthCriterionList GwmLcrGWRTaskThread::bandwidthSelectorCriterions() const
    {
        return selector.bandwidthCriterion();
    }
    inline void GwmLcrGWRTaskThread::setIsAutoselectBandwidth(bool value)
    {
        mIsAutoselectBandwidth = value;
    }
    inline bool GwmLcrGWRTaskThread::autoselectBandwidth() const
    {
        return mIsAutoselectBandwidth;
    }
    double trs = 0;
    double trsts = 0;
    double getMcnThresh() const;
    double getMlambda() const;

    bool hashatmatix = false;
    bool getHashatmatix() const;
    void setHashatmatix(bool value);

    bool isValid() override;
};

#endif // GWMLCRGWRTASKTHREAD_H
