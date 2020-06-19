#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H


#include "GWmodel/GWmodel.h"
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

class GwmLcrGWRTaskThread:public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable
{
public:
    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
public:
    GwmLcrGWRTaskThread();

    double cnThresh() const;
    void setCnThresh(double cnThresh);

    double lambda() const;
    void setLambda(double lambda);

    bool hasHatmatix() const;
    void setHasHatmatix(bool value);

    bool lambdaAdjust() const;
    void setLambdaAdjust(bool lambdaAdjust);

    GwmDiagnostic dialnostic() const{
        return mDiagnostic;
    }
    bool isAutoselectBandwidth() const;

    BandwidthCriterionList GwmLcrGWRTaskThread::bandwidthSelectorCriterions() const
    {
        return selector.bandwidthCriterion();
    }

    void GwmLcrGWRTaskThread::setIsAutoselectBandwidth(bool value)
    {
        mIsAutoselectBandwidth = value;
    }

    bool GwmLcrGWRTaskThread::autoselectBandwidth() const
    {
        return mIsAutoselectBandwidth;
    }
public:
    void run() override;

    bool isValid() override;

    arma::mat regression(const arma::mat &x, const arma::vec &y);

    double criterion(GwmBandwidthWeight *weight);
protected:
    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);

    void createResultLayer(CreateResultLayerData data);
private:
    double mLambda;

    bool mLambdaAdjust;

    double mCnThresh;

    GwmBandwidthSizeSelector selector;

    bool mHasHatmatix = false;

    double mTrS = 0;

    double mTrStS = 0;

    bool mIsAutoselectBandwidth = false;
};

#endif // GWMLCRGWRTASKTHREAD_H
