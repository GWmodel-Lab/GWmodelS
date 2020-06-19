#ifndef GWMROBUSTGWRTASKTHREAD_H
#define GWMROBUSTGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbasicgwralgorithm.h"

class GwmRobustGWRTaskThread: public GwmBasicGWRAlgorithm
{
public:
    GwmRobustGWRTaskThread();
    //二次权重数组
    vec mWVect;

    bool filtered;

    void setFiltered(bool value);

    // 计算用的矩阵
//    mat mX;
//    vec mY;
    vec mWeightMask;
//    mat mBetas;
//    mat mRowSumBetasSE;
//    mat mBetasSE;
//    mat mBetasTV;
//    mat mDataPoints;
//    mat mRegPoints;
//    vec mSHat;
//    vec mQDiag;
    vec mYHat;
    vec mResidual;
    vec mStudentizedResidual;
//    vec mLocalRSquare;

    double mMse;

    mat regression(const mat& x, const vec& y) override;
    bool hasHatMatrix() const;

    bool hasFTest() const;

    void setHasHatMatrix(bool hasHatMatrix);

    void setHasFTest(bool hasFTest);

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    void createResultLayer(CreateResultLayerData data);

    GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    //mat mBetasSE;
    vec mShat;
    //vec mQDiag;
    mat mS;

    struct FTestParameters
    {
        int nDp = 0;
        int nVar = 0;
        double trS = 0.0;
        double trStS = 0.0;
        double gwrRSS = 0.0;
        double trQ = 0.0;
        double trQtQ = 0.0;
    };

    void fTest(FTestParameters params);

    typedef double (GwmRobustGWRTaskThread::*CalcTrQtQFunction)();
    typedef vec (GwmRobustGWRTaskThread::*CalcDiagBFunction)(int);
    CalcDiagBFunction mCalcDiagBFunction = &GwmRobustGWRTaskThread::calcDiagBSerial;

    double calcTrQtQSerial();
    vec calcDiagBSerial(int i);

    CalcTrQtQFunction mCalcTrQtQFunction = &GwmRobustGWRTaskThread::calcTrQtQSerial;

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

};

#endif // GWMROBUSTGWRTASKTHREAD_H
