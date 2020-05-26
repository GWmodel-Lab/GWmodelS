#include "gwmscalablegwrtaskthread.h"

#include "gsl/gsl_multimin.h"
#include "GWmodel/GWmodel.h"

GwmScalableGWRTaskThread::GwmScalableGWRTaskThread() : GwmGWRTaskThread()
{

}

void GwmScalableGWRTaskThread::run()
{
    if (!setXY())
    {
        return;
    }
    getNeighbours();

    // 解算模型
    emit tick(0, 0);
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols, nBw = mBandwidthSize;
    double P = mPolynomial, band0 = 0.0;
    mat G0(nDp, nBw, fill::zeros);
    switch (mBandwidthKernelFunction)
    {
    case GwmGWRTaskThread::KernelFunction::Gaussian:
        band0 = median(mNeighbourDists.col(qMin<uword>(50, nBw))) / sqrt(3);
        G0 = exp(-pow(mNeighbourDists / band0, 2));
        break;
    case GwmGWRTaskThread::KernelFunction::Exponential:
        band0 = median(mNeighbourDists.col(qMin<uword>(50, nBw))) / 3;
        G0 = exp(-pow(mNeighbourDists / band0, 2));
        break;
    default:
        return;
    }
    emit message(tr("Scalable GWR preparing..."));
    mat XtX = mX.t() * mX, XtY = mX.t() * mY;
    mat Mx0, My0;
    scgwr_pre(mX, mY, nBw, P, band0, G0, mNeighbours, Mx0, My0);

    emit message(tr("Scalable GWR optimizing..."));
    double b_tilde = 1.0, alpha = 0.01;
    mCV = optimize(Mx0, My0, b_tilde, alpha);
    if (mCV < DBL_MAX)
    {
        emit message(tr("Scalable GWR calibrating..."));
        vec parameters = { b_tilde, alpha };
        bool isAllCorrect = scgwr_reg(mX, mY, nBw, P, G0, mNeighbours, parameters, Mx0, My0, mBetas, mSHat, mBetasSE);
        if (isAllCorrect)
        {
            diagnostic();
            createResultLayer();
            emit success();
        }
    }
    else
    {
        emit error(tr("Cannot find optimized b.tilde and alpha"));
        return;
    }
}

void GwmScalableGWRTaskThread::diagnostic()
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
}

void GwmScalableGWRTaskThread::getNeighbours()
{
    uword nDp = mX.n_rows, nBw = mBandwidthSize;
    mNeighbours = umat(nBw, nDp, fill::zeros);
    mNeighbourDists = mat(nBw, nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = distance(i);
        uvec indices = sort_index(d);
        vec d_sorted = sort(d);
        mNeighbours.col(i) = indices.rows(1, nBw + 1);
        mNeighbourDists.col(i) = d_sorted.rows(1, nBw + 1);
    }
    mNeighbours = trans(mNeighbours);
    mNeighbourDists = trans(mNeighbourDists);
}

double scagwr_loocv_multimin_function(const gsl_vector* vars, void* params)
{
    double b_tilde = gsl_vector_get(vars, 0), alpha = gsl_vector_get(vars, 1);
    vec target = { b_tilde, alpha };
    const GwmScalableGWRLoocvParams *p = (GwmScalableGWRLoocvParams*)params;
    const mat *x = p->x, *y = p->y;
    int bw = p->bw;
    double polynomial = p->polynomial;
    const mat *Mx0 = p->Mx0, *My0 = p->My0;
    return scgwr_loocv(target, *x, *y, bw, polynomial, *Mx0, *My0);
}

double GwmScalableGWRTaskThread::optimize(const mat &Mx0, const mat &My0, double& b_tilde, double& alpha)
{
    gsl_multimin_fminimizer* minizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex2, 2);
    gsl_vector* target = gsl_vector_alloc(2);
    gsl_vector_set(target, 0, b_tilde);
    gsl_vector_set(target, 1, alpha);
    gsl_vector* step = gsl_vector_alloc(2);
    gsl_vector_set(step, 0, 0.01);
    gsl_vector_set(step, 1, 0.01);
    GwmScalableGWRLoocvParams params = { &mX, &mY, (int)mBandwidthSize, mPolynomial, &Mx0, &My0 };
    gsl_multimin_function function = { &scagwr_loocv_multimin_function, 2, &params };
    double cv = DBL_MAX;
    int status = gsl_multimin_fminimizer_set(minizer, &function, target, step);
    if (status == GSL_SUCCESS)
    {
        int iter = 0;
        double size;
        do
        {
            iter++;
            status = gsl_multimin_fminimizer_iterate(minizer);
            if (status) break;
            size = gsl_multimin_fminimizer_size(minizer);
            status = gsl_multimin_test_size(size, 1e-6);
        }
        while (status == GSL_CONTINUE && iter < mMaxIter);
        b_tilde = gsl_vector_get(minizer->x, 0);
        alpha = gsl_vector_get(minizer->x, 1);
        cv = minizer->fval;
        emit message(QString().sprintf("Scalable GWR optimizing: b.tilde=%.3lf alpha=%.3lf (CV: %.3lf)", b_tilde, alpha, cv));
    }
    gsl_vector_free(target);
    gsl_vector_free(step);
    gsl_multimin_fminimizer_free(minizer);
    return  cv;
}

double GwmScalableGWRTaskThread::getPenalty() const
{
    return mPenalty;
}

double GwmScalableGWRTaskThread::getScale() const
{
    return mScale;
}

double GwmScalableGWRTaskThread::getCV() const
{
    return mCV;
}

int GwmScalableGWRTaskThread::getPolynomial() const
{
    return mPolynomial;
}
