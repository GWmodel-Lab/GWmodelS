#include "gwmmultiscalegwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

using namespace std;

GwmMultiscaleGWRTaskThread::GwmMultiscaleGWRTaskThread()
    : GwmGWRTaskThread()
{

}

void GwmMultiscaleGWRTaskThread::run()
{
    if (!setXY())
    {
        return;
    }
    uword nDp = mX.n_rows, nVar = mX.n_cols;

    // ********************************
    // Centering and scaling predictors
    // ********************************
    mX0 = mX;
    mY0 = mY;
    for (uword i = 1; i < (nVar - 1); i++)
    {
        if (mPreditorCentered[i - 1])
        {
            mX.col(i) = mX.col(i) - mean(mX.col(i));
        }
    }

    // ***********************
    // Intialize the bandwidth
    // ***********************
    mInitialBandwidthSize = vec(mX.size());
    mInitialBandwidthSize.fill(DBL_MAX);
    for (uword i = 0; i < nVar; i++)
    {
        if (mBandwidthSeled[i] == BandwidthSeledType::Null)
        {
            GwmBandwidthSelectTaskThread bwSelThread(*this);
            QList<GwmLayerAttributeItem*> selIndepVars = QList<GwmLayerAttributeItem*>();
            if (i > 0)
                selIndepVars.append(mIndepVars[i - 1]);
            bwSelThread.setIndepVars(selIndepVars);
            double bw = selectOptimizedBandwidth(bwSelThread);
            mInitialBandwidthSize(i) = bw;
        }
    }

    // *****************************************************
    // Calculate the initial beta0 from the above bandwidths
    // *****************************************************
    GwmBandwidthSelectTaskThread bwSelThread(*this);
    double bwInit0 = selectOptimizedBandwidth(bwSelThread);
    mBandwidthSize = bwInit0;

    bool isStoreS = hasFTest && (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
    const RegressionAll regression = regressionAll[mParallelMethodType];
    bool isAllCorrect = (this->*regression)(hasHatMatrix, S);
    if (!isAllCorrect)
        return;

    // ***********************************************************
    // Select the optimum bandwidths for each independent variable
    // ***********************************************************
    uvec bwChangeNo(nVar, fill::zeros);
    vec resid = mY - fitted(mX, mBetas);
    double RSS0 = sum(resid % resid), RSS = DBL_MAX;
    double criterion = DBL_MAX;
    for (int iteration = 0; iteration < mMaxIteration && criterion > mCriterionThreshold; iteration++)
    {
        for (int i = 0; i < nVar; i++)
        {
            double bwi = DBL_MAX;
            vec fi = mBetas.col(i) % mX.col(i);
            vec yi = resid + fi;
            if (mBandwidthSeled[i] == BandwidthSeledType::Specified)
            {
                bwi = mInitialBandwidthSize[i];
            }
            else
            {
                GwmBandwidthSelectTaskThread bwSelThread(*this);
                bwSelThread.setX(mX.col(i));
                bwi = selectOptimizedBandwidth(bwSelThread);
                if (abs(bwi - mInitialBandwidthSize[i]) > mBandwidthSelectThreshold(i))
                {
                    bwChangeNo(i) = 0;
                }
                else
                {
                    bwChangeNo(i) += 1;
                    if (bwChangeNo(i) >= mBandwidthSelectRetryTimes)
                    {
                        mBandwidthSeled[i] = BandwidthSeledType::Specified;
                    }
                }
            }
            mInitialBandwidthSize[i] = bwi;
            vec betai = regressionVar(mX.col(i), mY, hasHatMatrix, S);
            mBetas.col(i) = betai;
            resid = mY - fitted(mX, mBetas);
        }
        RSS = rss(mY, mX, mBetas);
        criterion = (mCriterionType == CriterionType::CVR) ?
                    abs(RSS - RSS0) :
                    sqrt(abs(RSS - RSS0) / RSS);
        RSS0 = RSS;
    }
    diagnostic(false);
    createResultLayer();
}

double GwmMultiscaleGWRTaskThread::selectOptimizedBandwidth(GwmBandwidthSelectTaskThread &bwSelThread)
{
    connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
    connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
    connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    bwSelThread.start();
    bwSelThread.wait();
    disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
    disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
    disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    return bwSelThread.getBandwidthSize();
}

vec GwmMultiscaleGWRTaskThread::regressionVar(const mat &x, const vec &y, bool hatmatrix, mat &S)
{
    bool isAllCorrect = true;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    if (hatmatrix)
    {
        mat betas(nVar, nDp, fill::zeros);
        bool isStoreS = hasFTest && (nDp <= 8192);
        mat ci, si;
        vec shat(2, fill::zeros), q(nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try
            {
                vec d = distance(i);
                vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
                betas.col(i) = gwRegHatmatrix(x, y, w, i, ci, si);
//                shat(0) += si(0, i);
//                shat(1) += det(si * trans(si));
//                vec p = -trans(si);
//                p(i) += 1.0;
//                q += p % p;
//                S.row(isStoreS ? i : 0) = si;
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            return betas.t();
        }
    }
    else
    {
        mat betas(nVar, nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec d = distance(i);
                vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                betas.col(i) = gwReg(x, y, w, i);
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            return betas.t();
        }
    }
    return vec(nDp, fill::zeros);
}
