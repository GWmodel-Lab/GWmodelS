#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H


#include "GWmodel/GWmodel.h"
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

class GwmLcrGWRTaskThread:public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable
{
public:
    double mlambda;

    bool mlambdaAdjust;

    double mcnThresh;

    bool madaptive;

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    void createResultLayer(CreateResultLayerData data);

    double criterion(GwmBandwidthWeight *weight);

    bool mIsAutoselectBandwidth = false;
public:
    GwmLcrGWRTaskThread();
protected:
    void run() override;

    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);

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
