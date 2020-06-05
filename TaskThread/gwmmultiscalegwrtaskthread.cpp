#include "gwmmultiscalegwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

//#include <iomanip>

using namespace std;

void preview(string filename, const mat& obj, string title, bool append = false)
{
    ofstream fout(filename, append ? ios::app : ios::out);
    if (fout.is_open())
    {
        fout.setf(ios::fixed);
        fout.precision(3);
        fout.width(12);
        fout << title << std::endl;
        obj.raw_print(fout);
    }
    fout.close();
}

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
    for (uword i = 1; i < nVar; i++)
    {
        if (mPreditorCentered[i - 1])
        {
            mX.col(i) = mX.col(i) - mean(mX.col(i));
        }
    }
    preview("PSDM/x1.txt", mX, "x1");

    // ***********************
    // Intialize the bandwidth
    // ***********************
    for (uword i = 0; i < nVar; i++)
    {
        if (mBandwidthSeled[i] == BandwidthSeledType::Null)
        {
            GwmBandwidthSelectTaskThread bwSelThread;
            bwSelThread.setLayer(mLayer);
            bwSelThread.setDepVar(mDepVar);
            QList<GwmLayerAttributeItem*> selIndepVars = QList<GwmLayerAttributeItem*>();
            if (i > 0)
                selIndepVars.append(mIndepVars[i - 1]);
            bwSelThread.setIndepVars(selIndepVars);
            bwSelThread.setBandwidthType(mBandwidthType);
            bwSelThread.setBandwidthKernelFunction(mBandwidthKernelFunction);
            bwSelThread.setBandwidthSelectionApproach(mBandwidthSelectionApproach);
            double bw = selectOptimizedBandwidth(bwSelThread, false);
            mInitialBandwidthSize(i) = bw;
        }
    }

    // *****************************************************
    // Calculate the initial beta0 from the above bandwidths
    // *****************************************************
    GwmBandwidthSelectTaskThread bwSelThread(*this);
    double bwInit0 = selectOptimizedBandwidth(bwSelThread);
    mBandwidthSize = 31;

    bool isStoreS = hasFTest && (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
    const RegressionAll regression = regressionAll[mParallelMethodType];
    bool isAllCorrect = (this->*regression)(hasHatMatrix, S);
    if (!isAllCorrect)
        return;

    preview("PSDM/betas.txt", mBetas, "betas");

    // ***********************************************************
    // Select the optimum bandwidths for each independent variable
    // ***********************************************************
    emit message(QString("-------- Select the Optimum Bandwidths for each Independent Varialbe --------"));
    uvec bwChangeNo(nVar, fill::zeros);
    vec resid = mY - fitted(mX, mBetas);
    preview("PSDM/resid.txt", resid, "resid");
    double RSS0 = sum(resid % resid), RSS = DBL_MAX;
    double criterion = DBL_MAX;
    for (int iteration = 1; iteration <= mMaxIteration && criterion > mCriterionThreshold; iteration++)
    {
        for (int i = 0; i < nVar; i++)
        {
            QString varName = i == 0 ? QStringLiteral("Intercept") : mIndepVars[i-1]->attributeName();
            double bwi = DBL_MAX;
            vec fi = mBetas.col(i) % mX.col(i);
            preview("PSDM/fi.txt", fi, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), !(iteration == 1 && i == 0));
            vec yi = resid + fi;
            preview("PSDM/yi.txt", yi, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), !(iteration == 1 && i == 0));
            if (mBandwidthSeled[i] == BandwidthSeledType::Specified)
            {
                bwi = mInitialBandwidthSize[i];
            }
            else
            {
                QString varName = i == 0 ? QStringLiteral("Intercept") : mIndepVars[i-1]->attributeName();
                emit message(QString("Now select an optimum bandwidth for the variable: %1").arg(varName));
                GwmBandwidthSelectTaskThread bwSelThread(*this);
                bwSelThread.setX(mX.col(i));
                bwSelThread.setY(yi);
                bwi = selectOptimizedBandwidth(bwSelThread, false);
                double bwi0 = mInitialBandwidthSize[i];
                emit message(QString("The newly selected bandwidth for variable %1 is %2 (last is %3, difference is %4)")
                             .arg(varName).arg(bwi).arg(bwi0).arg(abs(bwi - bwi0)));
                if (abs(bwi - mInitialBandwidthSize[i]) > mBandwidthSelectThreshold(i))
                {
                    bwChangeNo(i) = 0;
                    emit message(QString("The bandwidth for variable %1 will be continually selected in the next iteration").arg(varName));
                }
                else
                {
                    bwChangeNo(i) += 1;
                    if (bwChangeNo(i) >= mBandwidthSelectRetryTimes)
                    {
                        mBandwidthSeled[i] = BandwidthSeledType::Specified;
                        emit message(QString("The bandwidth for variable %1 seems to be converged and will be kept the same in the following iterations.").arg(varName));
                    }
                    else
                    {
                        emit message(QString("The bandwidth for variable %1 seems to be converged for %2 times. It will be continually optimized in the next %3 times.")
                                     .arg(varName).arg(bwChangeNo(i)).arg(mBandwidthSelectRetryTimes - bwChangeNo(i)));
                    }
                }
            }
            mInitialBandwidthSize[i] = bwi;
            mBetas.col(i) = regressionVar(mX.col(i), yi, bwi, hasHatMatrix, S);
            resid = mY - fitted(mX, mBetas);
            preview("PSDM/resid.txt", resid, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), true);
        }
        preview("PSDM/betas.txt", mBetas, QString("Iteration %1").arg(iteration - 1).toStdString(), true);
        RSS = rss(mY, mX, mBetas);
        criterion = (mCriterionType == CriterionType::CVR) ?
                    abs(RSS - RSS0) :
                    sqrt(abs(RSS - RSS0) / RSS);
        QString criterionName = mCriterionType == CriterionType::CVR ? "change value of RSS (CVR)" : "differential change value of RSS (dCVR)";
        emit message(QString("Iteration %1 the %2 is %3").arg(iteration).arg(criterionName).arg(criterion));
        RSS0 = RSS;
        emit message(QString("---- End of Iteration %1 ----").arg(iteration));
    }
    emit message(QString("-------- [End] Select the Optimum Bandwidths for each Independent Varialbe --------"));
    isBandwidthSizeAutoSel = false;
    diagnostic(false);
    createResultLayer();

    emit success();
}

double GwmMultiscaleGWRTaskThread::selectOptimizedBandwidth(GwmBandwidthSelectTaskThread &bwSelThread, bool verbose)
{
    if (verbose)
    {
        connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    }
    bwSelThread.start();
    bwSelThread.wait();
    if (verbose)
    {
        disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    }
    return bwSelThread.getBandwidthSize();
}

vec GwmMultiscaleGWRTaskThread::regressionVar(const mat &x, const vec &y, double bw, bool hatmatrix, mat &S)
{
    bool isAllCorrect = true;
    arma::uword nDp = x.n_rows, nVar = x.n_cols;
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
                vec w = gwWeight(d, bw, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
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
                vec w = gwWeight(d, bw, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
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

double GwmMultiscaleGWRTaskThread::criterionThreshold() const
{
    return mCriterionThreshold;
}

void GwmMultiscaleGWRTaskThread::setCriterionThreshold(double criterionThreshold)
{
    mCriterionThreshold = criterionThreshold;
}

GwmMultiscaleGWRTaskThread::CriterionType GwmMultiscaleGWRTaskThread::criterionType() const
{
    return mCriterionType;
}

void GwmMultiscaleGWRTaskThread::setCriterionType(const GwmMultiscaleGWRTaskThread::CriterionType &criterionType)
{
    mCriterionType = criterionType;
}

int GwmMultiscaleGWRTaskThread::bandwidthSelectRetryTimes() const
{
    return mBandwidthSelectRetryTimes;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes)
{
    mBandwidthSelectRetryTimes = bandwidthSelectRetryTimes;
}

vec GwmMultiscaleGWRTaskThread::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectThreshold(const vec &bandwidthSelectThreshold)
{
    mBandwidthSelectThreshold = bandwidthSelectThreshold;
}

QList<bool> GwmMultiscaleGWRTaskThread::preditorCentered() const
{
    return mPreditorCentered;
}

void GwmMultiscaleGWRTaskThread::setPreditorCentered(const QList<bool> &preditorCentered)
{
    mPreditorCentered = preditorCentered;
}

QList<GwmMultiscaleGWRTaskThread::BandwidthSeledType> GwmMultiscaleGWRTaskThread::bandwidthSeled() const
{
    return mBandwidthSeled;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSeled(const QList<BandwidthSeledType> &bandwidthSeled)
{
    mBandwidthSeled = bandwidthSeled;
}

vec GwmMultiscaleGWRTaskThread::initialBandwidthSize() const
{
    return mInitialBandwidthSize;
}

void GwmMultiscaleGWRTaskThread::setInitialBandwidthSize(const vec &initialBandwidthSize)
{
    mInitialBandwidthSize = initialBandwidthSize;
}
