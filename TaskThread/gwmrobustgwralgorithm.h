#ifndef GWMROBUSTGWRTASKTHREAD_H
#define GWMROBUSTGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbasicgwralgorithm.h"

class GwmRobustGWRAlgorithm: public GwmBasicGWRAlgorithm
{
    Q_OBJECT

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    typedef mat (GwmRobustGWRAlgorithm::*RegressionHatmatrix)(const mat&, const vec&, mat&, vec&, vec&, mat&);

public:
    GwmRobustGWRAlgorithm();
    //二次权重数组

    bool filtered() const;
    void setFiltered(bool value);

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

    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
    mat regressionHatmatrixOmp(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);
    mat regressionHatmatrixCuda(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

protected:
    void run() override;
private:
    vec mShat;

    vec mQDiag;

    mat mS;

    vec mWeightMask;

    vec mYHat;

    vec mResidual;

    vec mStudentizedResidual;

    double mMse;

    vec mWVect;

    bool mFiltered;

    RegressionHatmatrix mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixSerial;

    // 鲁棒GWR的第一种解法
    mat robustGWRCaliFirst(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S);
    // 第二种解法
    mat robustGWRCaliSecond(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S);
    // 计算二次权重函数
    vec filtWeight(vec x);
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

#endif // GWMROBUSTGWRTASKTHREAD_H
