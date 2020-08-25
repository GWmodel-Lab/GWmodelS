#ifndef GWMROBUSTGWRTASKTHREAD_H
#define GWMROBUSTGWRTASKTHREAD_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbasicgwralgorithm.h"

class GwmRobustGWRAlgorithm: public GwmBasicGWRAlgorithm
{
    Q_OBJECT

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    typedef mat (GwmRobustGWRAlgorithm::*RegressionHatmatrix)(const mat&, const vec&, mat&, vec&, vec&, mat&);

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

public:
    GwmRobustGWRAlgorithm();
    //二次权重数组

    bool filtered() const;
    void setFiltered(bool value);

    QString name() const override { return tr("Robust GWR"); }

public:
    mat regression(const mat& x, const vec& y) override;

public:
    void setParallelType(const ParallelType &type) override;
    int parallelAbility() const override;

protected:
    void createResultLayer(CreateResultLayerData data);

    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    // 鲁棒GWR的第一种解法
    mat robustGWRCaliFirst(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S);
    // 第二种解法
    mat robustGWRCaliSecond(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S);
    // 计算二次权重函数
    vec filtWeight(vec residual, double mse);

    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
    mat regressionHatmatrixOmp(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
    mat regressionHatmatrixCuda(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

protected:
    void run() override;

private:
    bool mFiltered;

    vec mShat;
    vec mQDiag;
    mat mS;
    vec mWeightMask;

    RegressionHatmatrix mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixSerial;
};


inline bool GwmRobustGWRAlgorithm::filtered() const
{
    return mFiltered;
}

inline void GwmRobustGWRAlgorithm::setFiltered(bool value)
{
    if(value == true){
        this->mFiltered=true;
    }else{
        this->mFiltered=false;
    }
}

inline int GwmRobustGWRAlgorithm::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP | IParallelalbe::CUDA;
}

#endif // GWMROBUSTGWRTASKTHREAD_H
