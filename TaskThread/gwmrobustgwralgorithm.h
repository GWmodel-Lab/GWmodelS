#ifndef GWMROBUSTGWRTASKTHREAD_H
#define GWMROBUSTGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbasicgwralgorithm.h"

class GwmRobustGWRAlgorithm: public GwmBasicGWRAlgorithm
{
    Q_OBJECT

    typedef double (GwmRobustGWRAlgorithm::*CalcTrQtQFunction)();
    typedef vec (GwmRobustGWRAlgorithm::*CalcDiagBFunction)(int);

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

public:
    GwmRobustGWRAlgorithm();
    //二次权重数组

    bool filtered() const;
    void setFiltered(bool value);

public:
    mat regression(const mat& x, const vec& y) override;

private:

    void createResultLayer(CreateResultLayerData data);

    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }


    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

protected:
    void run() override;
    // 主解算函数
    void gwrModelCalibration();
    // 鲁棒GWR的第一种解法
    void robustGWRCaliFirst();
    // 第二种解法
    void robustGWRCaliSecond();
    // 计算二次权重函数
    vec filtWeight(vec x);

private:
    vec mShat;
    //vec mQDiag;
    mat mS;
    vec mWeightMask;
    vec mYHat;
    vec mResidual;
    vec mStudentizedResidual;
    double mMse;
    vec mWVect;
    bool mFiltered;

    CalcTrQtQFunction mCalcTrQtQFunction = &GwmRobustGWRAlgorithm::calcTrQtQSerial;
    CalcDiagBFunction mCalcDiagBFunction = &GwmRobustGWRAlgorithm::calcDiagBSerial;
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
