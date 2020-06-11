#include "gwmgwrtaskthread.h"

#include <exception>
#include <gsl/gsl_cdf.h>
#include <omp.h>

#include "GWmodel/GWmodel.h"

#include "TaskThread/gwmbandwidthselecttaskthread.h"
#include "TaskThread/gwmgwrmodelselectionthread.h"

using namespace std;

QMap<QString, double> GwmGWRTaskThread::fixedBwUnitDict = {
    make_pair(QString("m"), 1.0),
    make_pair(QString("km"), 1000.0),
    make_pair(QString("mile"), 1609.344)
};
QMap<QString, double> GwmGWRTaskThread::adaptiveBwUnitDict = {
    make_pair(QString("x1"), 1),
    make_pair(QString("x10"), 10),
    make_pair(QString("x100"), 100),
    make_pair(QString("x1000"), 1000)
};

RegressionAll GwmGWRTaskThread::regressionAll[] = {
    &GwmGWRTaskThread::regressionAllSerial,
    &GwmGWRTaskThread::regressionAllOmp
};
CalcTrQtQ GwmGWRTaskThread::calcTrQtQ[] = {
    &GwmGWRTaskThread::calcTrQtQSerial,
    &GwmGWRTaskThread::calcTrQtQOmp
};
CalcDiagB GwmGWRTaskThread::calcDiagB[] = {
    &GwmGWRTaskThread::calcDiagBSerial,
    &GwmGWRTaskThread::calcDiagBOmp
};

GwmGWRTaskThread::GwmGWRTaskThread()
    : GwmTaskThread()
{
    mX = mat(uword(0), uword(0));
    mY = vec(uword(0));
    mWeightMask = vec(uword(0));
    mDataPoints = mat(uword(0), 2);
    mRegPoints = mat(uword(0), 2);
    mBetas = mat(uword(0), uword(0));
    mRowSumBetasSE = mat(uword(0), uword(0));
    mBetasSE = mat(uword(0), uword(0));
    mBetasTV = mat(uword(0), uword(0));
    mSHat = vec(uword(0));
    mQDiag = vec(uword(0));
    mYHat = vec(uword(0));
    mResidual = vec(uword(0));
    mStudentizedResidual = vec(uword(0));
    mLocalRSquare = vec(uword(0));
}

GwmGWRTaskThread::GwmGWRTaskThread(const GwmGWRTaskThread &taskThread)
{
    mLayer = taskThread.mLayer;
    mDepVar = taskThread.mDepVar;
    mIndepVars = taskThread.mIndepVars;
    mDepVarIndex = taskThread.mDepVarIndex;
    mIndepVarsIndex = taskThread.mIndepVarsIndex;
    mModelSelThreshold = taskThread.mModelSelThreshold;
    isEnableIndepVarAutosel = taskThread.isEnableIndepVarAutosel;
    mFeatureList = taskThread.mFeatureList;
    hasHatMatrix = taskThread.hasHatMatrix;
    mBandwidthType = taskThread.mBandwidthType;
    mBandwidthSize = taskThread.mBandwidthSize;
    mBandwidthSizeOrigin = taskThread.mBandwidthSizeOrigin;
    mBandwidthUnit = taskThread.mBandwidthUnit;
    isBandwidthSizeAutoSel = taskThread.isBandwidthSizeAutoSel;
    mBandwidthSelectionApproach = taskThread.mBandwidthSelectionApproach;
    mBandwidthKernelFunction = taskThread.mBandwidthKernelFunction;
    mDistSrcType = taskThread.mDistSrcType;
    mDistSrcParameters = taskThread.mDistSrcParameters;
    mCRSRotateTheta = taskThread.mCRSRotateTheta;
    mCRSRotateP = taskThread.mCRSRotateP;
    mParallelMethodType = taskThread.mParallelMethodType;
    mParallelParameter = taskThread.mParallelParameter;
    mX = mat(taskThread.mX);
    mY = vec(taskThread.mY);
    mWeightMask = vec(mWeightMask);
    mDataPoints = mat(taskThread.mDataPoints);
    mRegPoints = mat(taskThread.mRegPoints);
    mBetas = mat(uword(0), uword(0));
    mRowSumBetasSE = mat(uword(0), uword(0));
    mBetasSE = mat(uword(0), uword(0));
    mBetasTV = mat(uword(0), uword(0));
    mSHat = vec(uword(0));
    mQDiag = vec(uword(0));
    mYHat = vec(uword(0));
    mResidual = vec(uword(0));
    mStudentizedResidual = vec(uword(0));
    mLocalRSquare = vec(uword(0));
}

void GwmGWRTaskThread::run()
{
    // 优选模型
    if (isEnableIndepVarAutosel)
    {
        emit message(tr("Selecting optimized model..."));
        GwmGWRModelSelectionThread modelSelThread(*this);
        connect(&modelSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&modelSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&modelSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        modelSelThread.start();
        modelSelThread.wait();
        disconnect(&modelSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&modelSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&modelSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        QPair<QList<int>, double> optimizedModel = modelSelThread.modelSelection();
        if (optimizedModel.second != DBL_MAX)
        {
            mIndepVarsIndex = optimizedModel.first;
            mModelSelModels = modelSelThread.getModelInDepVars();
            mModelSelAICcs = modelSelThread.getModelAICcs();

            // 绘图
            QMap<QString, QVariant> data;
            QList<QVariant> indepVarNames, modelSelModels, modelSelAICcs;
            for (GwmLayerAttributeItem* item : mIndepVars)
            {
                indepVarNames.append(item->attributeName());
            }
            data["indepVarNames"] = indepVarNames;
            for (QStringList modelStringList : mModelSelModels)
            {
                modelSelModels.append(modelStringList);
            }
            data["modelSelModels"] = modelSelModels;
            for (double aic : mModelSelAICcs)
            {
                modelSelAICcs.append(aic);
            }
            data["modelSelAICcs"] = modelSelAICcs;
            emit plot(data, &GwmGWRModelSelectionThread::plotModelOrder);
            emit plot(data, &GwmGWRModelSelectionThread::plotModelAICcs);
        }
        else
        {
            emit error(tr("Cannot select optimized model."));
            return;
        }
    }

    // 设置矩阵
    if (!setXY())
    {
        return;
    }

    // 优选带宽
    if (isBandwidthSizeAutoSel)
    {
        emit message(tr("Selecting optimized bandwidth..."));
        GwmBandwidthSelectTaskThread bwSelThread(*this);
        connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        bwSelThread.start();
        bwSelThread.wait();
        disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        mBandwidthSize = bwSelThread.getBandwidthSize();
        mBandwidthSelScore = bwSelThread.getBwScore();
        // 绘图
        QList<QVariant> plotData;
        for (auto i = mBandwidthSelScore.constBegin(); i != mBandwidthSelScore.constEnd(); i++)
        {
            plotData.append(QVariant(QPointF(i.key(), i.value())));
        }
        emit plot(QVariant(plotData), &GwmBandwidthSelectTaskThread::plotBandwidthResult);
    }

    // 解算模型
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, nDp);
    mWeightMask = vec(nDp, fill::ones);
    bool isAllCorrect = gwrCalibration();

    // Create Result Layer
    if (isAllCorrect)
    {
        createResultLayer();
    }
    emit success();
}

bool GwmGWRTaskThread::gwrCalibration()
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    if (hasHatMatrix)
    {
        bool isStoreS = hasFTest && (nDp <= 8192);

        // 解算
        mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
        const RegressionAll regression = regressionAll[mParallelMethodType];
        isAllCorrect = (this->*regression)(hasHatMatrix, S);

        // 诊断和检验
        if (isAllCorrect)
        {
            diagnostic();
            // F Test
            if (hasHatMatrix && hasFTest)
            {
                // Q
                double trQtQ = DBL_MAX;
                if (isStoreS)
                {
                    mat EmS = eye(nDp, nDp) - S;
                    mat Q = trans(EmS) * EmS;
                    trQtQ = sum(diagvec(trans(Q) * Q));
                }
                else
                {
                    const CalcTrQtQ calc = calcTrQtQ[mParallelMethodType];
                    trQtQ = (this->*calc)();
                }
                GwmFTestParameters fTestParams;
                fTestParams.nDp = mX.n_rows;
                fTestParams.nVar = mX.n_cols;
                fTestParams.trS = mSHat(0);
                fTestParams.trStS = mSHat(1);
                fTestParams.trQ = sum(mQDiag);
                fTestParams.trQtQ = trQtQ;
                fTestParams.bw = mBandwidthSize;
                fTestParams.adaptive = mBandwidthType == BandwidthType::Adaptive;
                fTestParams.kernel = mBandwidthKernelFunction;
                fTestParams.gwrRSS = sum(mResidual % mResidual);
                f1234Test(fTestParams);
            }
        }
    }
    else
    {
        mat _(0, 0);
        isAllCorrect = regressionAllSerial(hasHatMatrix, _);
    }
    return isAllCorrect;
}

bool GwmGWRTaskThread::regressionAllSerial(bool hatmatrix, mat& S)
{
    bool isAllCorrect = true;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    if (hatmatrix)
    {
        mat betas(nVar, nDp, fill::zeros), betasSE(nVar, nDp, fill::zeros);
        bool isStoreS = hasFTest && (nDp <= 8192);
        mat ci, si;
        vec shat(2, fill::zeros), q(nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try
            {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
                betas.col(i) = gwRegHatmatrix(mX, mY, weight, i, ci, si);
                betasSE.col(i) = sum(ci % ci, 1);
                shat(0) += si(0, i);
                shat(1) += det(si * trans(si));
                vec p = -trans(si);
                p(i) += 1.0;
                q += p % p;
                S.row(isStoreS ? i : 0) = si;
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mSHat = shat;
            mQDiag = q;
            mBetas = betas.t();
            mBetasSE = betasSE.t();
        }
    }
    else
    {
        mat betas(nVar, nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                betas.col(i) = gwReg(mX, mY, weight, i);
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mBetas = betas.t();
        }
    }
    return isAllCorrect;
}

bool GwmGWRTaskThread::regressionAllOmp(bool hatmatrix, mat &S)
{
    int nThread = mParallelParameter.toInt();
    bool isAllCorrect = true;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    if (hatmatrix)
    {
        mat betas(nVar, nDp, fill::zeros), betasSE(nVar, nDp, fill::zeros);
        bool isStoreS = hasFTest && (nDp <= 8192);
        mat qdiag_all(nDp, nThread, fill::zeros);
        vec s1(nDp, fill::zeros), s2(nDp, fill::zeros);
        int current = 0;
#pragma omp parallel for num_threads(nThread)
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            int thread_id = omp_get_thread_num();
            mat ci, si, betas(nDp, nVar, fill::zeros), betasSE(nDp, nVar, fill::zeros);
            vec qdiag(nDp, fill::zeros);
            try
            {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
                betas.col(i) = gwRegHatmatrix(mX, mY, weight, i, ci, si);
                betasSE.col(i) = sum(ci % ci, 1);
                s1(i) = si(0, i);
                s2(i) = det(si * trans(si));
                vec p = -trans(si);
                p(i) += 1.0;
                qdiag_all.col(thread_id) += p % p;
                S.row(isStoreS ? i : 0) = si;
                emit tick(++current, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mSHat(0) = sum(s1);
            mSHat(1) = sum(s2);
            mQDiag = sum(qdiag_all, 1);
            mBetas = betas.t();
            mBetasSE = betasSE.t();
        }
    }
    else
    {
        mat betas(nVar, nDp, fill::zeros);
        int current = 0;
#pragma omp parallel for num_threads(nThread)
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                betas.col(i) = gwReg(mX, mY, weight, i);
                emit tick(++current, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mBetas = betas.t();
        }
    }
    return isAllCorrect;
}

double GwmGWRTaskThread::calcTrQtQSerial()
{
    double trQtQ = 0.0;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calculating the trace of matrix Q..."));
    emit tick(0, nDp);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword i = 0; i < nDp; i++)
    {
        vec di = distance(i);
        vec wi = gwWeight(di, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
        mat xtwi = trans(mX % (wi * wspan));
        mat xtwxR = inv(xtwi * mX);
        mat ci = xtwxR * xtwi;
        mat si = mX.row(i) * inv(xtwi * mX) * xtwi;
        vec pi = -trans(si);
        pi(i) += 1.0;
        double qi = sum(pi % pi);
        trQtQ += qi * qi;
        for (arma::uword j = i + 1; j < nDp; j++)
        {
            vec dj = distance(j);
            vec wj = gwWeight(dj, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
            mat xtwj = trans(mX % (wj * wspan));
            mat sj = mX.row(j) * inv(xtwj * mX) * xtwj;
            vec pj = -trans(sj);
            pj(j) += 1.0;
            double qj = sum(pi % pj);
            trQtQ += qj * qj * 2.0;
        }
        emit tick(i + 1, nDp);
    }
    return trQtQ;
}

double GwmGWRTaskThread::calcTrQtQOmp()
{
    int nThread = mParallelParameter.toInt();
    vec trQtQ_all(nThread, fill::zeros);
    int nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calculating the trace of matrix Q..."));
    emit tick(0, nDp);
    mat wspan(1, nVar, fill::ones);
    int current = 0;
#pragma omp parallel for num_threads(nThread)
    for (int i = 0; i < nDp; i++)
    {
        int thread_id = omp_get_thread_num();
        vec di = distance(i);
        vec wi = gwWeight(di, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
        mat xtwi = trans(mX % (wi * wspan));
        mat si = mX.row(i) * inv(xtwi * mX) * xtwi;
        vec pi = -trans(si);
        pi(i) += 1.0;
        double qi = sum(pi % pi);
        trQtQ_all(thread_id) += qi * qi;
        for (arma::uword j = i + 1; j < nDp; j++)
        {
            vec dj = distance(j);
            vec wj = gwWeight(dj, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive) % mWeightMask;
            mat xtwj = trans(mX % (wj * wspan));
            mat sj = mX.row(j) * inv(xtwj * mX) * xtwj;
            vec pj = -trans(sj);
            pj(j) += 1.0;
            double qj = sum(pi % pj);
            trQtQ_all(thread_id) += qj * qj * 2.0;
        }
        emit tick(++current, nDp);
    }
    return sum(trQtQ_all);
}

QString GwmGWRTaskThread::name() const
{
    return tr("GWR");
}

bool GwmGWRTaskThread::isValid(QString &message)
{
    if (!mLayer)
    {
        message = (tr("Layer is not selected."));
        return false;
    }
    if (!mDepVar)
    {
        message = (tr("Dependent variable is not selected."));
        return false;
    }
    QgsField depField = mLayer->fields()[mDepVarIndex];
    if (!isNumeric(depField.type()))
    {
        message = (tr("Dependent variable is not numeric."));
        return false;
    }
    if (mIndepVarsIndex.size() > 0)
    {
        for (int iIndepVar : mIndepVarsIndex)
        {
            QgsField indepField = mLayer->fields()[iIndepVar];
            if (!isNumeric(indepField.type()))
            {
                message = (tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
                return false;
            }
        }
    }
    else
    {
        message = (tr("No independent variables."));
        return false;
    }
    if (mDistSrcType == DistanceSourceType::DMatFile)
    {
        QString filename = mDistSrcParameters.toString();
        if (filename.isNull() || filename.isEmpty())
        {
            message = tr("Distance matrix file is not selected.");
            return false;
        }

        QFile dmat(filename);
        if (dmat.open(QIODevice::ReadOnly))
        {
            QDataStream fin(&dmat);
            fin.setByteOrder(QDataStream::LittleEndian);
            int featureCount = mLayer->featureCount();
            int nrow = 0, ncol = 0;
            fin >> nrow >> ncol;
            if (nrow != featureCount)
            {
                message = tr("The number of rows of selected distance matrix is not equal to the number of data points.");
                return false;
            }
            if (ncol != featureCount)
            {
                message = tr("The number of columns of selected distance matrix is not equal to the number of regression points.");
                return false;
            }
        }
        else
        {
            message = tr("Distance matrix file cannot be opened.");
            return false;
        }
    }
    if (mRegressionLayer)
    {
        if (isEnableIndepVarAutosel)
        {
            message = tr("Cannot automaticly select indenpendent variables if regression points are given.");
            return false;
        }
        if (isBandwidthSizeAutoSel)
        {
            message = tr("Cannot automaticly optimize bandwidth if regression points are given.");
            return false;
        }
    }
    if (!isBandwidthSizeAutoSel)
    {
        if (mBandwidthType == BandwidthType::Adaptive)
        {
            if (mBandwidthSize < 20.0 || mBandwidthSize < (mIndepVarsIndex.size() + 1))
            {
                message = tr("Bandwidth size is too small.");
                return false;
            }
        }
        else if (mBandwidthSize == BandwidthType::Fixed)
        {
            if (mBandwidthSize < 1e-6)
            {
                message = tr("Bandwidth size is too small.");
                return false;
            }
        }
        else
        {
            message = tr("Unknown bandwidth type.");
            return false;
        }
    }

    return true;
}

QgsVectorLayer *GwmGWRTaskThread::layer() const
{
    return mLayer;
}

void GwmGWRTaskThread::setLayer(QgsVectorLayer *layer)
{
    mLayer = layer;
    mFeatureList.clear();
}

QList<GwmLayerAttributeItem *> GwmGWRTaskThread::indepVars() const
{
    return mIndepVars;
}

void GwmGWRTaskThread::setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars)
{
    mIndepVars = indepVars;
    mIndepVarsIndex.clear();
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        int iIndepVar = item->attributeIndex();
        mIndepVarsIndex.append(iIndepVar);
    }
}

GwmGWRTaskThread::BandwidthType GwmGWRTaskThread::bandwidthType() const
{
    return mBandwidthType;
}

void GwmGWRTaskThread::setBandwidthType(const BandwidthType &bandwidthType)
{
    mBandwidthType = bandwidthType;
}

bool GwmGWRTaskThread::getIsBandwidthSizeAutoSel() const
{
    return isBandwidthSizeAutoSel;
}

void GwmGWRTaskThread::setIsBandwidthSizeAutoSel(bool value)
{
    isBandwidthSizeAutoSel = value;
}

GwmGWRTaskThread::KernelFunction GwmGWRTaskThread::getBandwidthKernelFunction() const
{
    return mBandwidthKernelFunction;
}

void GwmGWRTaskThread::setBandwidthKernelFunction(const KernelFunction &bandwidthKernelFunction)
{
    mBandwidthKernelFunction = bandwidthKernelFunction;
}

GwmGWRTaskThread::DistanceSourceType GwmGWRTaskThread::getDistSrcType() const
{
    return mDistSrcType;
}

void GwmGWRTaskThread::setDistSrcType(const DistanceSourceType &distSrcType)
{
    mDistSrcType = distSrcType;
}

QVariant GwmGWRTaskThread::getDistSrcParameters() const
{
    return mDistSrcParameters;
}

void GwmGWRTaskThread::setDistSrcParameters(const QVariant &distSrcParameters)
{
    mDistSrcParameters = distSrcParameters;
}

GwmGWRTaskThread::ParallelMethod GwmGWRTaskThread::getParallelMethodType() const
{
    return mParallelMethodType;
}

void GwmGWRTaskThread::setParallelMethodType(const ParallelMethod &parallelMethodType)
{
    mParallelMethodType = parallelMethodType;
}

QVariant GwmGWRTaskThread::getParallelParameter() const
{
    return mParallelParameter;
}

void GwmGWRTaskThread::setParallelParameter(const QVariant &parallelParameter)
{
    mParallelParameter = parallelParameter;
}

QgsVectorLayer *GwmGWRTaskThread::getResultLayer() const
{
    return mResultLayer;
}

bool GwmGWRTaskThread::enableIndepVarAutosel() const
{
    return isEnableIndepVarAutosel;
}

void GwmGWRTaskThread::setEnableIndepVarAutosel(bool value)
{
    isEnableIndepVarAutosel = value;
}

QgsFeatureList GwmGWRTaskThread::getFeatureList() const
{
    return mFeatureList;
}

GwmGWRDiagnostic GwmGWRTaskThread::getDiagnostic() const
{
    return mDiagnostic;
}

mat GwmGWRTaskThread::getBetas() const
{
    return mBetas;
}

GwmGWRTaskThread::BandwidthSelectionApproach GwmGWRTaskThread::getBandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

void GwmGWRTaskThread::setBandwidthSelectionApproach(const BandwidthSelectionApproach &bandwidthSelectionApproach)
{
    mBandwidthSelectionApproach = bandwidthSelectionApproach;
}

QList<QStringList> GwmGWRTaskThread::getModelSelModels() const
{
    return mModelSelModels;
}

QList<double> GwmGWRTaskThread::getModelSelAICcs() const
{
    return mModelSelAICcs;
}

int GwmGWRTaskThread::getDepVarIndex() const
{
    return mDepVarIndex;
}

QList<int> GwmGWRTaskThread::getIndepVarsIndex() const
{
    return mIndepVarsIndex;
}

QMap<double, double> GwmGWRTaskThread::getBwScore() const
{
    return mBandwidthSelScore;
}

double GwmGWRTaskThread::getModelSelThreshold() const
{
    return mModelSelThreshold;
}

void GwmGWRTaskThread::setModelSelThreshold(double modelSelThreshold)
{
    mModelSelThreshold = modelSelThreshold;
}

QList<GwmFTestResult> GwmGWRTaskThread::fTestResults() const
{
    QList<GwmFTestResult> f124Results = {
        mF1Result,
        mF2Result,
        mF4Result
    };
    return f124Results + mF3Result;
}

bool GwmGWRTaskThread::getHasFTest() const
{
    return hasFTest;
}

void GwmGWRTaskThread::setHasFTest(bool value)
{
    hasFTest = value;
}

bool GwmGWRTaskThread::getHasHatMatrix() const
{
    return hasHatMatrix;
}

void GwmGWRTaskThread::setHasHatMatrix(bool value)
{
    hasHatMatrix = value;
}

QgsVectorLayer *GwmGWRTaskThread::getRegressionLayer() const
{
    return mRegressionLayer;
}

void GwmGWRTaskThread::setRegressionLayer(QgsVectorLayer *regressionLayer)
{
    mRegressionLayer = regressionLayer;
}

double GwmGWRTaskThread::getBandwidthSize() const
{
    return mBandwidthSize;
}

double GwmGWRTaskThread::getBandwidthSizeOrigin() const
{
    return mBandwidthSizeOrigin;
}

QString GwmGWRTaskThread::getBandwidthUnit() const
{
    return mBandwidthUnit;
}

GwmLayerAttributeItem *GwmGWRTaskThread::depVar() const
{
    return mDepVar;
}

void GwmGWRTaskThread::setDepVar(GwmLayerAttributeItem *depVar)
{
    mDepVar = depVar;
    mDepVarIndex = mDepVar->attributeIndex();
}

void GwmGWRTaskThread::setBandwidth(GwmGWRTaskThread::BandwidthType type, double size, QString unit)
{
    mBandwidthSizeOrigin = size;
    mBandwidthSize = size;
    mBandwidthType = type;
    mBandwidthUnit = unit;
    if (type == BandwidthType::Fixed)
    {
        mBandwidthSize = size * fixedBwUnitDict[unit];
    }
    else
    {
        mBandwidthSize = size * adaptiveBwUnitDict[unit];
    }
    qDebug() << "[GwmGWRTaskThread::setBandwidth]"
             << "mBandwidthSizeOrigin" << mBandwidthSizeOrigin
             << "mBandwidthUnit" << unit
             << "mBandwidthSize" << mBandwidthSize
             << "mBandwidthType" << mBandwidthType;
}

bool GwmGWRTaskThread::isNumeric(QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
    case QVariant::Double:
        return true;
    default:
        return false;
    }
}

bool GwmGWRTaskThread::setXY()
{
    // 提取 FeatureID
    emit message("Setting matrices.");
    emit tick(0, 0);
    QgsFeatureIterator it = (mRegressionLayer ? mRegressionLayer : mLayer)->getFeatures();
    QgsFeature f;
    while (it.nextFeature(f))
    {
        mFeatureList.append(f);
    }
    // 初始化所需矩阵
    int nDp = mLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = mIndepVarsIndex.size() + 1;
    mX = mat(nDp, nVar, fill::zeros);
    mY = vec(nDp, fill::zeros);
    mWeightMask = vec(nDp, fill::ones);
    mBetas = mat(nRp, nVar, fill::zeros);
    if (hasHatMatrix)
    {
        mBetasSE = mat(nDp, nVar, fill::zeros);
        mSHat = vec(2, fill::zeros);
        mQDiag = vec(nDp, fill::zeros);
        mRowSumBetasSE = mat(nDp, 1, fill::ones);
    }
    mDataPoints = mat(nDp, 2);

    bool ok = false;
    it = mLayer->getFeatures();
    for (int row = 0; it.nextFeature(f); row++)
    {
        double vY = f.attribute(mDepVarIndex).toDouble(&ok);
        if (ok)
        {
            mY(row, 0) = vY;
            mX(row, 0) = 1.0;
            // 设置 X 矩阵
            for (int i = 0; i < mIndepVarsIndex.size(); i++)
            {
                int index = mIndepVarsIndex[i];
                double vX = f.attribute(index).toDouble(&ok);
                if (ok)
                {
                    mX(row, i + 1) = vX;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
            // 设置坐标
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mDataPoints(row, 0) = centroPoint.x();
            mDataPoints(row, 1) = centroPoint.y();
        }
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }

    // 回归点
    if (mRegressionLayer)
    {
        mRegPoints = mat(nRp, 2);
        it = mRegressionLayer->getFeatures();
        for (int row = 0; it.nextFeature(f); row++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegPoints(row, 0) = centroPoint.x();
            mRegPoints(row, 1) = centroPoint.y();
        }
    }
    else mRegPoints = mDataPoints;
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    return true;
}

vec GwmGWRTaskThread::distance(int focus)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(focus);
    case DistanceSourceType::DMatFile:
        return distanceDmat(focus);
    default:
        return distanceCRS(focus);
    }
}

vec GwmGWRTaskThread::distanceCRS(int focus)
{
    bool longlat = mLayer->crs().isGeographic();
    return gwDist(mDataPoints, mRegPoints, focus, 2.0, 0.0, longlat, mRegressionLayer != nullptr);
}

vec GwmGWRTaskThread::distanceMinkowski(int focus)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    double theta = parameters["theta"].toDouble();
    return gwDist(mDataPoints, mRegPoints, focus, p, theta, false, mRegressionLayer != nullptr);
}

vec GwmGWRTaskThread::distanceDmat(int focus)
{
    QString filename = mDistSrcParameters.toString();
    qint64 featureCount = mFeatureList.size();
    QFile dmat(filename);
    if (dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * featureCount * sizeof (double));
        QByteArray values = dmat.read(featureCount * sizeof (double));
        return vec((double*)values.data(), featureCount);
    }
    else
    {
        return vec(featureCount, fill::zeros);
    }
}

void GwmGWRTaskThread::diagnostic(bool doLocalR2)
{
    emit message(tr("Calculating diagnostic informations..."));

    // 诊断信息
    vec vDiags = gwrDiag(mY, mX, mBetas, mSHat);
    mDiagnostic = GwmGWRDiagnostic(vDiags);
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    double trS = mSHat(0), trStS = mSHat(1);
    double nDp = mFeatureList.size();
    double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
    mStudentizedResidual = mResidual / sqrt(sigmaHat * mQDiag);
    mBetasSE = sqrt(sigmaHat * mBetasSE);
    mBetasTV = mBetas / mBetasSE;
    vec dybar2 = (mY - sum(mY) / nDp) % (mY - sum(mY) / nDp);
    vec dyhat2 = (mY - mYHat) % (mY - mYHat);

    // Local RSquare
    mLocalRSquare = vec(mFeatureList.size(), fill::zeros);
    if (doLocalR2)
    {
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            vec dist = distance(i);
            mat w = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            mLocalRSquare(i) = (tss - rss) / tss;
        }
    }
}

void GwmGWRTaskThread::f1234Test(const GwmFTestParameters& params)
{
    emit message("F Test");
    GwmFTestResult f1, f2, f4;
    double v1 = params.trS, v2 = params.trStS;
    int nDp = params.nDp, nVar = params.nVar;
    emit tick(0, nVar + 3);
    double edf = 1.0 * nDp - 2 * v1 + v2;
    double RSSg = params.gwrRSS;
    vec betao = solve(mX, mY);
    vec residual = mY - mX * betao;
    double RSSo = sum(residual % residual);
    double DFo = nDp - nVar;
    double delta1 = 1.0 * nDp - 2 * v1 + v2;
    double sigma2delta1 = RSSg / delta1;
    double sigma2 = RSSg / nDp;
    double trQ = params.trQ, trQtQ = params.trQtQ;
    double lDelta1 = trQ;
    double lDelta2 = trQtQ;
    // F1 Test
    f1.s = (RSSg/lDelta1)/(RSSo/DFo);
    f1.df1 = lDelta1 * lDelta1 / lDelta2;
    f1.df2 = DFo;
    f1.p = gsl_cdf_fdist_P(f1.s, f1.df1, f1.df2);
    emit tick(1, nVar + 3);
    // F2 Test
    f2.s = ((RSSo-RSSg)/(DFo-lDelta1))/(RSSo/DFo);
    f2.df1 = (DFo-lDelta1) * (DFo-lDelta1) / (DFo - 2 * lDelta1 + lDelta2);
    f2.df2 = DFo;
    f2.p = gsl_cdf_fdist_Q(f2.s, f2.df1, f2.df2);
    emit tick(2, nVar + 3);
    // F3 Test
//    vec vJndp = vec(nDp, fill::ones) * (1.0 / nDp);
    vec vk2(nVar, fill::zeros);
    for (int i = 0; i < nVar; i++)
    {
        vec betasi = mBetas.col(i);
        vec betasJndp = vec(nDp, fill::ones) * (sum(betasi) * 1.0 / nDp);
        vk2(i) = (1.0 / nDp) * det(trans(betasi - betasJndp) * betasi);
    }
    QList<GwmFTestResult> f3;
    f3.reserve(nVar);
    for (int i = 0; i < nVar; i++)
    {
        double g1 = 0.0, g2 = 0.0, numdf = 0.0;
        if (nDp > 8192)
        {
            mat B(nDp, nDp, fill::zeros);
            mat ek = eye(nVar, nVar);
            mat wspan(1, nVar, fill::ones);
            for (int j = 0; j < nDp; j++)
            {
                vec d = distance(j);
                vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                mat xtw = trans(mX % (w * wspan));
                B.row(j) = ek.row(i) * inv(xtw * mX) * xtw;
            }
            mat Bj = 1.0 / nDp * (B.t() * (eye(nDp, nDp) - (1.0 / nDp) * mat(nDp, nDp, fill::ones)) * B);
            vec b = diagvec(Bj);
            g1 = sum(b);
            g2 = sum(b % b);
            qDebug("Var %d tr(B)   = %lf", i, g1);
            qDebug("Var %d tr(B.B) = %lf", i, g2);
            numdf = g1 * g1 / g2;
        }
        else
        {
            const CalcDiagB calculator = calcDiagB[mParallelMethodType];
            vec diagB = (this->*calculator)(i);
            g1 = diagB(0);
            g2 = diagB(1);
            numdf = g1 * g1 / g2;
        }
        GwmFTestResult f3i;
        f3i.s = (vk2(i) / g1) / sigma2delta1;
        f3i.df1 = numdf;
        f3i.df2 = f1.df1;
        f3i.p = gsl_cdf_fdist_Q(f3i.s, numdf, f1.df1);
        f3.append(f3i);
        emit tick(3 + i, nVar + 3);
    }
    // F4 Test
    f4.s = RSSg / RSSo;
    f4.df1 = delta1;
    f4.df2 = DFo;
    f4.p = gsl_cdf_fdist_P(f4.s, f4.df1, f4.df2);
    emit tick(nVar + 3, nVar + 3);
    // 保存结果
    mF1Result = f1;
    mF2Result = f2;
    mF3Result = f3;
    mF4Result = f4;
}

vec GwmGWRTaskThread::calcDiagBSerial(int i)
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    vec diagB(nDp, fill::zeros), c(nDp, fill::zeros);
    mat ek = eye(nVar, nVar);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword j = 0; j < nDp; j++)
    {
        vec d = distance(j);
        vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        mat xtw = trans(mX % (w * wspan));
        mat C = trans(xtw) * inv(xtw * mX);
        c += C.col(i);
    }
    for (arma::uword k = 0; k < nDp; k++)
    {
        vec d = distance(k);
        vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        mat xtw = trans(mX % (w * wspan));
        mat C = trans(xtw) * inv(xtw * mX);
        vec b = C.col(i);
        diagB += (b % b - (1.0 / nDp) * (b % c));
    }
    diagB = 1.0 / nDp * diagB;
    return { sum(diagB), sum(diagB % diagB) };
}

vec GwmGWRTaskThread::calcDiagBOmp(int i)
{
    int nThread = mParallelParameter.toInt();
    int nDp = mX.n_rows, nVar = mX.n_cols;
    mat cAll(nDp, nThread, fill::zeros), diagBAll(nDp, nThread, fill::zeros);
    mat ek = eye(nVar, nVar);
    mat wspan(1, nVar, fill::ones);
#pragma omp parallel for num_threads(nThread)
    for (int j = 0; j < nDp; j++)
    {
        int thread = omp_get_thread_num();
        vec d = distance(j);
        vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        mat xtw = trans(mX % (w * wspan));
        cAll.col(thread) += trans(ek.row(i) * inv(xtw * mX) * xtw);
    }
    vec c = sum(cAll, 1);
#pragma omp parallel for num_threads(nThread)
    for (int k = 0; k < nDp; k++)
    {
        int thread = omp_get_thread_num();
        vec d = distance(k);
        vec w = gwWeight(d, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        mat xtw = trans(mX % (w * wspan));
        vec b = trans(ek.row(i) * inv(xtw * mX) * xtw);
        diagBAll.col(thread) += (b % b - (1.0 / nDp) * (b % c));
    }
    vec diagB = sum(diagBAll, 1);
    diagB = 1.0 / nDp * diagB;
    return { sum(diagB), sum(diagB % diagB) };
}

void GwmGWRTaskThread::createResultLayer()
{
    emit message("Creating result layer...");
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    if (mBandwidthType == BandwidthType::Fixed)
    {
        layerName += QString("_B%1%2").arg(mBandwidthSizeOrigin, 0, 'f', 3).arg(mBandwidthSize);
    }
    else
    {
        layerName += QString("_B%1").arg(int(mBandwidthSize));
    }
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    QgsFields fields;
    fields.append(QgsField(QStringLiteral("Intercept"), QVariant::Double, QStringLiteral("double")));
    for (int index : mIndepVarsIndex)
    {
        QString srcName = mLayer->fields().field(index).name();
        QString name = srcName;
        fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
    }
    if (hasHatMatrix)
    {
        fields.append(QgsField(QStringLiteral("y"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("yhat"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("residual"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("Stud_residual"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("Intercept_SE"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_SE");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
        fields.append(QgsField(QStringLiteral("Intercept_TV"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_TV");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
        fields.append(QgsField(QStringLiteral("Local_R2"), QVariant::Double, QStringLiteral("double")));
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    mResultLayer->startEditing();
    if (hasHatMatrix)
    {
        int indepSize = mIndepVarsIndex.size() + 1;
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            int curCol = 0;
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mY(f));
            feature.setAttribute(fields[curCol++].name(), mYHat(f));
            feature.setAttribute(fields[curCol++].name(), mResidual(f));
            feature.setAttribute(fields[curCol++].name(), mStudentizedResidual(f));
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasSE(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasTV(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mLocalRSquare(f));
            mResultLayer->addFeature(feature);
        }
    }
    else
    {
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < fields.size(); a++)
            {
                QString attributeName = fields[a].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            mResultLayer->addFeature(feature);
        }
    }
    mResultLayer->commitChanges();
}
