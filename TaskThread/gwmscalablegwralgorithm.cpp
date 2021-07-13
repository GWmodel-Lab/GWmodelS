#include "gwmscalablegwralgorithm.h"

#include <QPair>
#include "gsl/gsl_multimin.h"

#include <SpatialWeight/gwmbandwidthweight.h>


double GwmScalableGWRAlgorithm::Loocv(const vec &target, const mat &x, const vec &y, int bw, int poly, const mat &Mx0, const mat &My0)
{
    int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
    double b = target(0) * target(0), a = target(1) * target(1);
    vec R0 = vec(poly1, fill::ones) * b;
    for (int p = 1; p < poly1; p++) {
        R0(p) = pow(b, p + 1);
    }
    R0 = R0 / sum(R0);

    vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
    for (int p = 0; p < poly1; p++) {
        for (int k2 = 0; k2 < k; k2++) {
            for (int k1 = 0; k1 < k; k1++) {
                int xindex = k1*poly1*k + p*k + k2;
                Rx(xindex) = R0(p);
            }
            int yindex = p*k + k2;
            Ry(yindex) = R0(p);
        }
    }
    mat Mx = Rx * mat(1, n, fill::ones) % Mx0, My = Ry * mat(1, n, fill::ones) % My0;
    vec yhat(n, 1, fill::zeros);
    for (int i = 0; i < n; i++) {
        mat sumMx(k, k, fill::zeros);
        vec sumMy(k, fill::zeros);
        for (int k2 = 0; k2 < k; k2++) {
            for (int p = 0; p < poly1; p++) {
                for (int k1 = 0; k1 < k; k1++) {
                    int xindex = k1*poly1*k + p*k + k2;
                    sumMx(k1, k2) += Mx(xindex, i);
                }
                int yindex = p*k + k2;
                sumMy(k2) += My(yindex, i);
            }
        }
        sumMx += + a * (x.t() * x);
        sumMy += + a * (x.t() * y);
        try {
            mat beta = solve(sumMx, sumMy);
            yhat.row(i) = x.row(i) * beta;
        } catch (...) {
            return DBL_MAX;
        }
    }
    return sum((y - yhat) % (y - yhat));
}

double GwmScalableGWRAlgorithm::AICvalue(const vec &target, const mat &x, const vec &y, int bw, int poly, const mat &Mx0, const mat &My0)
{
    int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
    double b = target(0) * target(0), a = target(1) * target(1);
    vec R0 = vec(poly1, fill::ones) * b;
    for (int p = 1; p < poly1; p++) {
        R0(p) = pow(b, p + 1);
    }
    R0 = R0 / sum(R0);

    vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
    for (int p = 0; p < poly1; p++) {
        for (int k2 = 0; k2 < k; k2++) {
            for (int k1 = 0; k1 < k; k1++) {
                int xindex = k1*poly1*k + p*k + k2;
                Rx(xindex) = R0(p);
            }
            int yindex = p*k + k2;
            Ry(yindex) = R0(p);
        }
    }
    mat Mx = Rx * mat(1, n, fill::ones) % Mx0, My = Ry * mat(1, n, fill::ones) % My0;
//    mat Mx2 = 2 * a * Mx + ((Rx % Rx) * mat(1, n, fill::ones) % Mx0);

    vec yhat(n, 1, fill::zeros);
    double trS = 0.0/*, trStS = 0.0*/;
    for (int i = 0; i < n; i++) {
        mat sumMx(k, k, fill::zeros)/*, sumMx2(k, k, fill::zeros)*/;
        vec sumMy(k, fill::zeros);
        for (int k2 = 0; k2 < k; k2++) {
            for (int p = 0; p < poly1; p++) {
                for (int k1 = 0; k1 < k; k1++) {
                    int xindex = k1*poly1*k + p*k + k2;
                    sumMx(k1, k2) += Mx(xindex, i);
//                    sumMx2(k1, k2) += Mx2(xindex, i);
                }
                int yindex = p*k + k2;
                sumMy(k2) += My(yindex, i);
            }
        }
        sumMx += a * (x.t() * x);
//        sumMx2 += a * a * (x.t() * x);
        sumMy += a * (x.t() * y);
        if (det(sumMx) < 1e-10) {
            return DBL_MAX;
        } else {
            mat sumMxR = inv(sumMx.t() * sumMx);
            vec trS00 = sumMxR * trans(x.row(i));
            mat trS0 = x.row(i) * trS00;
            trS += trS0[0];

//            vec trStS00 = sumMx2 * trS00;
//            double trStS0 = sum(sum(trS00 * trStS00));
//            trStS += trStS0;

            vec beta = sumMxR * (sumMx.t() * sumMy);
            yhat.row(i) = x.row(i) * beta;
        }
    }
    double sse = sum((y - yhat) % (y - yhat));
    double sig = sqrt(sse / n);
    double AICc = 2 * n * log(sig) + n *log(2*M_PI) +n*(n+trS)/(n-2-trS);
    return isfinite(AICc) ? AICc : DBL_MAX;
}

GwmDiagnostic GwmScalableGWRAlgorithm::CalcDiagnostic(const vec &y, const mat &x, const mat &betas, const vec &shat)
{
    vec r = y - sum(betas % x, 1);
    double rss = sum(r % r);
    int n = x.n_rows;
    double AIC = n * log(rss / n) + n * log(2 * datum::pi) + n + shat(0);
    double AICc = n * log(rss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    double edf = n - 2 * shat(0) + shat(1);
    double enp = 2 * shat(0) - shat(1);
    double yss = sum((y - mean(y)) % (y - mean(y)));
    double r2 = 1 - rss / yss;
    double r2_adj = 1 - (1 - r2) * (n - 1) / (edf - 1);
    return { rss, AIC, AICc, enp, edf, r2, r2_adj };
}

GwmScalableGWRAlgorithm::GwmScalableGWRAlgorithm() : GwmGeographicalWeightedRegressionAlgorithm()
{

}

void GwmScalableGWRAlgorithm::run()
{

    mDpSpatialWeight = mSpatialWeight;

    if(!checkCanceled())
    {
        emit message(tr("Initilizing points, matrix and neighours..."));
        initPoints();
        initXY(mX, mY, mDepVar, mIndepVars);
        findDataPointNeighbours();
    }

    // 修正带宽
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    arma::uword nDp = mX.n_rows, nBw = bandwidth->bandwidth();
    if (nBw >= nDp && !checkCanceled())
    {
        nBw = nDp - 1;
        bandwidth->setBandwidth(nBw);
    }
    if(!checkCanceled())
    {
    // 解算模型
        emit tick(0, 0);
        double band0 = 0.0;
        switch (bandwidth->kernel())
        {
        case GwmBandwidthWeight::KernelFunctionType::Gaussian:
            band0 = median(mDpNNDists.col(qMin<uword>(50, nBw) - 1)) / sqrt(3);
            mG0 = exp(-pow(mDpNNDists / band0, 2));
            break;
        case GwmBandwidthWeight::KernelFunctionType::Exponential:
            band0 = median(mDpNNDists.col(qMin<uword>(50, nBw) - 1)) / 3;
            mG0 = exp(-pow(mDpNNDists / band0, 2));
            break;
        default:
            return;
        }
        emit message(tr("Scalable GWR preparing..."));
        prepare();

        emit message(tr("Scalable GWR optimizing..."));
        double b_tilde = 1.0, alpha = 0.01;
        mCV = optimize(mMx0, mMy0, b_tilde, alpha);
        if (mCV < DBL_MAX && !checkCanceled())
        {
            emit message(tr("Scalable GWR calibrating..."));
            mScale = b_tilde * b_tilde;
            mPenalty = alpha * alpha;
            if (!hasRegressionLayer() && !checkCanceled())
            {
                mBetas = regressionHatmatrixSerial(mX, mY);
                mDiagnostic = CalcDiagnostic(mY, mX, mBetas, mShat);
                double trS = mShat(0), trStS = mShat(1);
                double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
                vec yhat = sum(mX % mBetas, 1);
                vec residual = mY - yhat;
                mBetasSE = sqrt(sigmaHat * mBetasSE);
                mat betasTV = mBetas / mBetasSE;
                createResultLayer({
                    qMakePair(QString("%1"), mBetas),
                    qMakePair(QString("y"), mY),
                    qMakePair(QString("yhat"), yhat),
                    qMakePair(QString("residual"), residual),
                    qMakePair(QString("%1_SE"), mBetasSE),
                    qMakePair(QString("%1_TV"), betasTV)
                });
            }
            else
            {
                if(!checkCanceled())
                {
                   mBetas = regressionSerial(mX, mY);
                }
                if (hasRegressionLayerXY && mHasPredict && !checkCanceled())
                {
                    vec yhat = Fitted(mRegressionLayerX, mBetas);
                    vec residual = mRegressionLayerY - yhat;
                    createResultLayer({
                        qMakePair(QString(mDepVar.name), mRegressionLayerY),
                        qMakePair(QString("%1"), mBetas),
                        qMakePair(QString("yhat"), yhat),
                        qMakePair(QString("residual"), residual)
                    });
                }
                else
                {
                    createResultLayer({
                        qMakePair(QString("%1"), mBetas),
                    });
                }
            }
        }
        else
        {
            emit error(tr("Cannot find optimized b.tilde and alpha"));
            return;
        }
        emit success();

    }
    if (checkCanceled())
    {
        return;
    }

}

void GwmScalableGWRAlgorithm::findDataPointNeighbours()
{
    GwmBandwidthWeight* bandwidth = mDpSpatialWeight.weight<GwmBandwidthWeight>();
    uword nDp = mDataPoints.n_rows, nBw = bandwidth->bandwidth() < nDp ? bandwidth->bandwidth() : nDp;
    if (mParameterOptimizeCriterion == ParameterOptimizeCriterionType::CV)
    {
        nBw -= 1;
    }
    umat nnIndex(nBw, nDp, fill::zeros);
    mat nnDists(nBw, nDp, fill::zeros);
    for (uword i = 0; i < nDp & !checkCanceled(); i++)
    {
        vec d = mDpSpatialWeight.distance()->distance(i);
        uvec i_sorted = sort_index(d);
        vec d_sorted = sort(d);
        nnIndex.col(i) = i_sorted(span(0, nBw - 1));
        nnDists.col(i) = d_sorted(span(0, nBw - 1));
    }
    if (mParameterOptimizeCriterion == ParameterOptimizeCriterionType::CV)
    {
        mDpNNDists = trans(nnDists);
        mDpNNIndex = trans(nnIndex);
    }
    else
    {
        mDpNNDists = join_rows(vec(nDp, fill::zeros), trans(nnDists));
        mDpNNIndex = join_rows(linspace<uvec>(0, nDp - 1, nDp), trans(nnIndex));
    }

    mDpNNDists.submat(0, 0, 2, 2).print(std::cout, "nn");
}

mat GwmScalableGWRAlgorithm::findNeighbours(const GwmSpatialWeight &spatialWeight, umat &nnIndex)
{
    GwmBandwidthWeight* bandwidth = spatialWeight.weight<GwmBandwidthWeight>();
    uword nDp = mDpSpatialWeight.distance()->total();
    uword nRp = spatialWeight.distance()->total();
    uword nBw = bandwidth->bandwidth() < nDp ? bandwidth->bandwidth() : nDp;
    umat index(nBw, nRp, fill::zeros);
    mat dists(nBw, nRp, fill::zeros);
    for (uword i = 0; i < nRp & !checkCanceled(); i++)
    {
        vec d = spatialWeight.distance()->distance(i);
        uvec i_sorted = sort_index(d);
        vec d_sorted = sort(d);
        index.col(i) = i_sorted(span(0, nBw - 1));
        dists.col(i) = d_sorted(span(0, nBw - 1));
    }
    nnIndex = index.t();
    return dists.t();
}

double scagwr_loocv_multimin_function(const gsl_vector* vars, void* params)
{
    double b_tilde = gsl_vector_get(vars, 0), alpha = gsl_vector_get(vars, 1);
    vec target = { b_tilde, alpha };
    const GwmScalableGWRAlgorithm::LoocvParams *p = (GwmScalableGWRAlgorithm::LoocvParams*) params;
    const mat *x = p->x, *y = p->y;
    int bw = p->bw;
    double polynomial = p->polynomial;
    const mat *Mx0 = p->Mx0, *My0 = p->My0;
    return GwmScalableGWRAlgorithm::Loocv(target, *x, *y, bw, polynomial, *Mx0, *My0);
}

double scagwr_aic_multimin_function(const gsl_vector* vars, void* params)
{
    double b_tilde = gsl_vector_get(vars, 0), alpha = gsl_vector_get(vars, 1);
    vec target = { b_tilde, alpha };
    const GwmScalableGWRAlgorithm::LoocvParams *p = (GwmScalableGWRAlgorithm::LoocvParams*) params;
    const mat *x = p->x, *y = p->y;
    int bw = p->bw;
    double polynomial = p->polynomial;
    const mat *Mx0 = p->Mx0, *My0 = p->My0;
    return GwmScalableGWRAlgorithm::AICvalue(target, *x, *y, bw, polynomial, *Mx0, *My0);
}

double GwmScalableGWRAlgorithm::optimize(const mat &Mx0, const mat &My0, double& b_tilde, double& alpha)
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    gsl_multimin_fminimizer* minizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex, 2);
    gsl_vector* target = gsl_vector_alloc(2);
    gsl_vector_set(target, 0, b_tilde);
    gsl_vector_set(target, 1, alpha);
    gsl_vector* step = gsl_vector_alloc(2);
    gsl_vector_set(step, 0, 0.01);
    gsl_vector_set(step, 1, 0.01);
    LoocvParams params = { &mX, &mY, (int)bandwidth->bandwidth(), mPolynomial, &Mx0, &My0 };
    gsl_multimin_function function = { mParameterOptimizeCriterion == CV ? &scagwr_loocv_multimin_function : &scagwr_aic_multimin_function, 2, &params };
    double cv = DBL_MAX;
    int status = gsl_multimin_fminimizer_set(minizer, &function, target, step);
    if (status == GSL_SUCCESS && !checkCanceled())
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
            b_tilde = gsl_vector_get(minizer->x, 0);
            alpha = gsl_vector_get(minizer->x, 1);
            cv = minizer->fval;
            emit message(QString().sprintf("Scalable GWR optimizing: b.tilde=%.3lf alpha=%.3lf (CV: %.3lf)", b_tilde, alpha, cv));
        }
        while (status == GSL_CONTINUE && iter < mMaxIter && !checkCanceled());
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

void GwmScalableGWRAlgorithm::prepare()
{
//    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    int knn = mDpNNIndex.n_cols;
    const mat &x = mX, &y = mY, &G0 = mG0;
    int n = x.n_rows;
    int k = x.n_cols;
    mMx0 = mat((mPolynomial + 1)*k*k, n, fill::zeros);
    mMxx0 = mat((mPolynomial + 1)*k*k, n, fill::zeros);
    mMy0 = mat((mPolynomial + 1)*k, n, fill::zeros);
    mat spanXnei(1, mPolynomial + 1, fill::ones);
    mat spanXtG(1, k, fill::ones);
    for (int i = 0; i < n & !checkCanceled(); i++) {
        mat G(mPolynomial + 1, knn, fill::ones);
        for (int p = 0; p < mPolynomial & !checkCanceled(); p++) {
            G.row(p + 1) = pow(G0.row(i), pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1));
        }
        G = trans(G);
        mat xnei = x.rows(mDpNNIndex.row(i));
        vec ynei = y.rows(mDpNNIndex.row(i));
        for (int k1 = 0; k1 < k & !checkCanceled(); k1++) {
            mat XtG = xnei.col(k1) * spanXnei % G;
            mat XtG2 = xnei.col(k1) * spanXnei % G % G;
            for (int p = 0; p < (mPolynomial + 1) & !checkCanceled(); p++) {
                mat XtGX = XtG.col(p) * spanXtG % xnei;
                mat XtG2X = XtG2.col(p) * spanXtG % xnei;
                for (int k2 = 0; k2 < k & !checkCanceled(); k2++) {
                    int xindex = (k1 * (mPolynomial + 1) + p) * k + k2;
                    mMx0(xindex, i) = sum(XtGX.col(k2));
                    mMxx0(xindex, i) = sum(XtG2X.col(k2));
                }
                int yindex = p * k + k1;
                vec XtGY = XtG.col(p) % ynei;
                mMy0(yindex, i) = sum(XtGY);
            }
        }
    }
}

mat GwmScalableGWRAlgorithm::regressionSerial(const arma::mat &x, const arma::vec &y)
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    arma::uword nDp = mDataPoints.n_rows, nRp = mRegressionPoints.n_rows, nVar = mX.n_cols, nBw = bandwidth->bandwidth();
    double band0 = 0.0;
    mat G0;
    umat rpNNIndex;
    mat rpNNDists = findNeighbours(mSpatialWeight, rpNNIndex);
    switch (bandwidth->kernel())
    {
    case GwmBandwidthWeight::KernelFunctionType::Gaussian:
        band0 = median(rpNNDists.col(qMin<uword>(50, nBw) - 1)) / sqrt(3);
        G0 = exp(-pow(rpNNDists / band0, 2));
        break;
    case GwmBandwidthWeight::KernelFunctionType::Exponential:
        band0 = median(rpNNDists.col(qMin<uword>(50, nBw) - 1)) / 3;
        G0 = exp(-pow(rpNNDists / band0, 2));
        break;
    default:
        return mat(nRp, nVar, fill::zeros);
    }

    mMx0 = mat((mPolynomial + 1)*nVar*nVar, nRp, fill::zeros);
    mMxx0 = mat((mPolynomial + 1)*nVar*nVar, nRp, fill::zeros);
    mMy0 = mat((mPolynomial + 1)*nVar, nRp, fill::zeros);
    mat spanXnei(1, mPolynomial + 1, fill::ones);
    mat spanXtG(1, nVar, fill::ones);
    for (arma::uword i = 0; i < nRp & !checkCanceled(); i++)
    {
        mat G(mPolynomial + 1, nBw, fill::ones);
        for (int p = 0; p < mPolynomial & !checkCanceled(); p++) {
            G.row(p + 1) = pow(G0.row(i), pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1));
        }
        G = trans(G);
        mat xnei = x.rows(rpNNIndex.row(i));
        vec ynei = y.rows(rpNNIndex.row(i));
        for (arma::uword k1 = 0; k1 < nVar & !checkCanceled(); k1++) {
            mat XtG = xnei.col(k1) * spanXnei % G;
            mat XtG2 = xnei.col(k1) * spanXnei % G % G;
            for (int p = 0; p < (mPolynomial + 1) & !checkCanceled(); p++) {
                mat XtGX = XtG.col(p) * spanXtG % xnei;
                mat XtG2X = XtG2.col(p) * spanXtG % xnei;
                for (arma::uword k2 = 0; k2 < nVar & !checkCanceled(); k2++) {
                    int xindex = (k1 * (mPolynomial + 1) + p) * nVar + k2;
                    mMx0(xindex, i) = sum(XtGX.col(k2));
                    mMxx0(xindex, i) = sum(XtG2X.col(k2));
                }
                int yindex = p * nVar + k1;
                vec XtGY = XtG.col(p) % ynei;
                mMy0(yindex, i) = sum(XtGY);
            }
        }
    }

    int poly1 = mPolynomial + 1;
    double b = mScale, a = mPenalty;
    vec R0 = vec(poly1, fill::ones) * b;
    for (int p = 1; p < poly1 & !checkCanceled(); p++) {
        R0(p) = pow(b, p + 1);
    }
    R0 = R0 / sum(R0);
    vec Rx(nVar*nVar*poly1, fill::zeros), Ry(nVar*poly1, fill::zeros);
    for (int p = 0; p < poly1 & !checkCanceled(); p++) {
        for (uword k2 = 0; k2 < nVar & !checkCanceled(); k2++) {
            for (uword k1 = 0; k1 < nVar & !checkCanceled(); k1++) {
                uword xindex = k1*poly1*nVar + p*nVar + k2;
                Rx(xindex) = R0(p);
            }
            uword yindex = p*nVar + k2;
            Ry(yindex) = R0(p);
        }
    }
    mat Mx = Rx * mat(1, nRp, fill::ones) % mMx0, My = Ry * mat(1, nRp, fill::ones) % mMy0;
    mat Mx2 = 2 * a * Mx + ((Rx % Rx) * mat(1, nRp, fill::ones) % mMx0);

    mat betas(nVar, nRp, fill::zeros);
    for (uword i = 0; i < nRp & !checkCanceled(); i++) {
        mat sumMx(nVar, nVar, fill::zeros), sumMx2(nVar, nVar, fill::zeros);
        vec sumMy(nVar, fill::zeros);
        for (uword k2 = 0; k2 < nVar & !checkCanceled(); k2++) {
            for (int p = 0; p < poly1 & !checkCanceled(); p++) {
                for (uword k1 = 0; k1 < nVar & !checkCanceled(); k1++) {
                    int xindex = k1*poly1*nVar + p*nVar + k2;
                    sumMx(k1, k2) += Mx(xindex, i);
                    sumMx2(k1, k2) += Mx2(xindex, i);
                }
                int yindex = p*nVar + k2;
                sumMy(k2) += My(yindex, i);
            }
        }
        sumMx += a * (x.t() * x);
        sumMx2 += a * a * (x.t() * x);
        sumMy += a * (x.t() * y);
        mat sumMxR = inv(sumMx.t() * sumMx);
        betas.col(i) = sumMxR * (sumMx.t() * sumMy);
    }

    return betas.t();
}

arma::mat GwmScalableGWRAlgorithm::regressionHatmatrixSerial(const arma::mat &x, const arma::vec &y)
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    int bw = bandwidth->bandwidth();
    int n = x.n_rows, k = x.n_cols, poly1 = mPolynomial + 1;
    double b = mScale, a = mPenalty;
    mat XtX = x.t() * x, XtY = x.t() * y;
    mat betas(k, n, fill::zeros);

    double band0 = 0.0;
    umat dpNNIndex;
    mat dpNNDists = findNeighbours(mDpSpatialWeight, dpNNIndex);
    switch (bandwidth->kernel())
    {
    case GwmBandwidthWeight::KernelFunctionType::Gaussian:
        band0 = median(dpNNDists.col(qMin<uword>(50, bw) - 1)) / sqrt(3);
        mG0 = exp(-pow(dpNNDists / band0, 2));
        break;
    case GwmBandwidthWeight::KernelFunctionType::Exponential:
        band0 = median(dpNNDists.col(qMin<uword>(50, bw) - 1)) / 3;
        mG0 = exp(-pow(dpNNDists / band0, 2));
        break;
    default:
        return betas.t();
    }

    mMx0 = mat((mPolynomial + 1)*k*k, n, fill::zeros);
    mMxx0 = mat((mPolynomial + 1)*k*k, n, fill::zeros);
    mMy0 = mat((mPolynomial + 1)*k, n, fill::zeros);
    mat spanXnei(1, mPolynomial + 1, fill::ones);
    mat spanXtG(1, k, fill::ones);
    for (int i = 0; i < n & !checkCanceled(); i++) {
        mat G(mPolynomial + 1, bw, fill::ones);
        for (int p = 0; p < mPolynomial & !checkCanceled(); p++) {
            G.row(p + 1) = pow(mG0.row(i), pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1));
        }
        G = trans(G);
        mat xnei = x.rows(dpNNIndex.row(i));
        vec ynei = y.rows(dpNNIndex.row(i));
        for (int k1 = 0; k1 < k & !checkCanceled(); k1++) {
            mat XtG = xnei.col(k1) * spanXnei % G;
            mat XtG2 = xnei.col(k1) * spanXnei % G % G;
            for (int p = 0; p < (mPolynomial + 1) & !checkCanceled(); p++) {
                mat XtGX = XtG.col(p) * spanXtG % xnei;
                mat XtG2X = XtG2.col(p) * spanXtG % xnei;
                for (int k2 = 0; k2 < k & !checkCanceled(); k2++) {
                    int xindex = (k1 * (mPolynomial + 1) + p) * k + k2;
                    mMx0(xindex, i) = sum(XtGX.col(k2));
                    mMxx0(xindex, i) = sum(XtG2X.col(k2));
                }
                int yindex = p * k + k1;
                vec XtGY = XtG.col(p) % ynei;
                mMy0(yindex, i) = sum(XtGY);
            }
        }
    }

    vec R0 = vec(poly1, fill::ones) * b;
    for (int p = 1; p < poly1 & !checkCanceled(); p++) {
        R0(p) = pow(b, p + 1);
    }
    R0 = R0 / sum(R0);
    vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
    for (int p = 0; p < poly1 & !checkCanceled(); p++) {
        for (int k2 = 0; k2 < k & !checkCanceled(); k2++) {
            for (int k1 = 0; k1 < k & !checkCanceled(); k1++) {
                int xindex = k1*poly1*k + p*k + k2;
                Rx(xindex) = R0(p);
            }
            int yindex = p*k + k2;
            Ry(yindex) = R0(p);
        }
    }
    mat Mx = Rx * mat(1, n, fill::ones) % mMx0, My = Ry * mat(1, n, fill::ones) % mMy0;
    mat Mx2 = 2 * a * Mx + ((Rx % Rx) * mat(1, n, fill::ones) % mMxx0);

    mat bse(k, n, fill::zeros);
    double trS = 0.0, trStS = 0.0;
    for (int i = 0; i < n & !checkCanceled(); i++) {
        mat sumMx(k, k, fill::zeros), sumMx2(k, k, fill::zeros);
        vec sumMy(k, fill::zeros);
        for (int k2 = 0; k2 < k & !checkCanceled(); k2++) {
            for (int p = 0; p < poly1 & !checkCanceled(); p++) {
                for (int k1 = 0; k1 < k & !checkCanceled(); k1++) {
                    int xindex = k1*poly1*k + p*k + k2;
                    sumMx(k1, k2) += Mx(xindex, i);
                    sumMx2(k1, k2) += Mx2(xindex, i);
                }
                int yindex = p*k + k2;
                sumMy(k2) += My(yindex, i);
            }
        }
        sumMx += a * (x.t() * x);
        sumMx2 += a * a * (x.t() * x);
        sumMy += a * (x.t() * y);
        try {
            mat sumMxR = inv(trans(sumMx) * sumMx);
            vec trS00 = sumMxR * trans(x.row(i));
            mat trS0 = x.row(i) * trS00;
            trS += trS0[0];

            vec trStS00 = sumMx2 * trS00;
            double trStS0 = sum(trS00 % trStS00);
            trStS += trStS0;

            vec beta = sumMxR * (sumMx.t() * sumMy);
            betas.col(i) = beta;

            mat bse00 = sumMxR * sumMx2 * sumMxR;
            vec bse0 = sqrt(diagvec(bse00));
            bse.col(i) = bse0;
        } catch (std::runtime_error e) {
            emit error(e.what());
        }
    }
    mBetasSE = bse.t();
    mShat = { trS, trStS };
    return betas.t();
}

void GwmScalableGWRAlgorithm::initPoints()
{
    GwmGeographicalWeightedRegressionAlgorithm::initPoints();
    if (mDpSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mDpSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mDpSpatialWeight.distance());
        d->setDataPoints(&mDataPoints);
        d->setFocusPoints(&mDataPoints);
    }
}

void GwmScalableGWRAlgorithm::initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars)
{
    GwmGeographicalWeightedRegressionAlgorithm::initXY(x, y, depVar, indepVars);
    if (hasRegressionLayer())
    {
        // 检查回归点图层是否包含了所有变量
        QStringList fieldNameList = mRegressionLayer->fields().names();
        bool flag = fieldNameList.contains(depVar.name);
        for (auto field : indepVars)
        {
            flag = flag && fieldNameList.contains(field.name);
        }
        hasRegressionLayerXY = flag;
        if (flag)
        {
            // 设置回归点X和回归点Y
            int regressionPointsSize = mRegressionLayer->featureCount();
            mRegressionLayerY = vec(regressionPointsSize, fill::zeros);
            mRegressionLayerX = mat(regressionPointsSize, indepVars.size() + 1, fill::zeros);
            QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
            QgsFeature f;
            bool ok = false;
            for (int i = 0; iterator.nextFeature(f); i++)
            {
                double vY = f.attribute(depVar.name).toDouble(&ok);
                if (ok)
                {
                    mRegressionLayerY(i) = vY;
                    mRegressionLayerX(i, 0) = 1.0;
                    for (int k = 0; k < indepVars.size(); k++)
                    {
                        double vX = f.attribute(indepVars[k].name).toDouble(&ok);
                        if (ok) mRegressionLayerX(i, k + 1) = vX;
                    }
                }
            }
        }
    }
}

void GwmScalableGWRAlgorithm::createResultLayer(initializer_list<CreateResultLayerDataItem> data)
{
    QgsVectorLayer* srcLayer = hasRegressionLayer() ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWR");
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    // 设置字段
    QgsFields fields;
    for (QPair<QString, const mat&> item : data)
    {
        QString title = item.first;
        const mat& value = item.second;
        if (value.n_cols > 1)
        {
            for (uword k = 0; k < value.n_cols; k++)
            {
                QString variableName = k == 0 ? QStringLiteral("Intercept") : mIndepVars[k - 1].name;
                QString fieldName = title.arg(variableName);
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }
        }
        else
        {
            fields.append(QgsField(title, QVariant::Double, QStringLiteral("double")));
        }
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    // 设置要素几何
    mResultLayer->startEditing();
    QgsFeatureIterator iterator = srcLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsFeature feature(fields);
        feature.setGeometry(f.geometry());

        // 设置属性
        int k = 0;
        for (QPair<QString, const mat&> item : data)
        {
            for (uword d = 0; d < item.second.n_cols; d++)
            {
                feature.setAttribute(k, item.second(i, d));
                k++;
            }
        }

        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();
}

bool GwmScalableGWRAlgorithm::hasPredict() const
{
    return mHasPredict;
}

void GwmScalableGWRAlgorithm::setHasPredict(bool hasPredict)
{
    mHasPredict = hasPredict;
}

bool GwmScalableGWRAlgorithm::isValid()
{
    if (GwmGeographicalWeightedRegressionAlgorithm::isValid())
    {
        GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
        if (!(bandwidth->kernel() == GwmBandwidthWeight::Gaussian || bandwidth->kernel() == GwmBandwidthWeight::Exponential))
            return false;

        if (bandwidth->bandwidth() <= mIndepVars.size())
            return false;

        return true;
    }
    else return false;
}
