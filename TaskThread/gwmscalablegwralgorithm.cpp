#include "gwmscalablegwralgorithm.h"

#include <QPair>
#include "gsl/gsl_multimin.h"

#include <SpatialWeight/gwmbandwidthweight.h>


double GwmScalableGWRAlgorithm::Loocv(const vec &target, const mat &x, const vec &y, int bw, int poly, const mat &Mx0, const mat &My0)
{
    int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
    double b = target(0) * target(0), a = target(1) * target(1);
    vec R0 = vec(poly1) * b;
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
        if (det(sumMx) < 1e-10) {
            return DBL_MAX;
        } else {
            mat beta = solve(sumMx, sumMy);
            yhat.row(i) = x.row(i) * beta;
        }
    }
    return sum((y - yhat) % (y - yhat));
}

double GwmScalableGWRAlgorithm::AICvalue(const vec &target, const mat &x, const vec &y, int bw, int poly, const mat &Mx0, const mat &My0)
{
    int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
    double b = target(0) * target(0), a = target(1) * target(1);
    vec R0 = vec(poly1) * b;
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
    initPoints();
    initXY(mX, mY, mDepVar, mIndepVars);
    findNeighbours();

    // 修正带宽
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols, nBw = bandwidth->bandwidth();
    if (nBw >= nDp)
    {
        nBw = nDp - 1;
        bandwidth->setBandwidth(nBw);
    }

    // 解算模型
    emit tick(0, 0);
    double band0 = 0.0;
    switch (bandwidth->kernel())
    {
    case GwmBandwidthWeight::KernelFunctionType::Gaussian:
        band0 = median(mNeighbourDists.col(qMin<uword>(50, nBw) - 1)) / sqrt(3);
        mG0 = exp(-pow(mNeighbourDists / band0, 2));
        break;
    case GwmBandwidthWeight::KernelFunctionType::Exponential:
        band0 = median(mNeighbourDists.col(qMin<uword>(50, nBw) - 1)) / 3;
        mG0 = exp(-pow(mNeighbourDists / band0, 2));
        break;
    default:
        return;
    }
    emit message(tr("Scalable GWR preparing..."));
    prepare();

    emit message(tr("Scalable GWR optimizing..."));
    double b_tilde = 1.0, alpha = 0.01;
    mCV = optimize(mMx0, mMy0, b_tilde, alpha);
    if (mCV < DBL_MAX)
    {
        emit message(tr("Scalable GWR calibrating..."));
        mScale = b_tilde * b_tilde;
        mPenalty = alpha * alpha;
        mBetas = regression(mX, mY);
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
        emit error(tr("Cannot find optimized b.tilde and alpha"));
        return;
    }
    emit success();
}

void GwmScalableGWRAlgorithm::findNeighbours()
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    uword nDp = mDataPoints.n_rows, nBw = bandwidth->bandwidth() >= nDp ? (nDp - 1) : bandwidth->bandwidth();
    umat neighboursIndex(nBw, nDp, fill::zeros);
    mat neighboursDists(nBw, nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        uvec indices = sort_index(d);
        vec d_sorted = sort(d);
        neighboursIndex.col(i) = indices(span(1, nBw));
        neighboursDists.col(i) = d_sorted(span(1, nBw));
    }
    mNeighbourDists = trans(neighboursDists);
    mNeighbours = trans(neighboursIndex);
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
            b_tilde = gsl_vector_get(minizer->x, 0);
            alpha = gsl_vector_get(minizer->x, 1);
            cv = minizer->fval;
            emit message(QString().sprintf("Scalable GWR optimizing: b.tilde=%.3lf alpha=%.3lf (CV: %.3lf)", b_tilde, alpha, cv));
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

void GwmScalableGWRAlgorithm::prepare()
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    int bw = bandwidth->bandwidth();
    const mat &x = mX, &y = mY, &g0 = mG0;
    int n = x.n_rows;
    int k = x.n_cols;
    mat g0s(g0.n_cols + 1, g0.n_rows, fill::ones);
    mat g0t = trans(g0);
    for (int i = 0; i < bw; i++) {
        g0s.row(i + 1) = g0t.row(i);
    }
    g0s = trans(g0s);
    mMx0 = mat((mPolynomial + 1)*k*k, n, fill::zeros);
    mMy0 = mat((mPolynomial + 1)*k, n, fill::zeros);
    mat spanXnei(1, mPolynomial + 1, fill::ones);
    mat spanXtG(1, k, fill::ones);
    for (int i = 0; i < n; i++) {
        mat g(mPolynomial + 1, bw + 1, fill::ones);
        for (int p = 0; p < mPolynomial; p++) {
            g.row(p + 1) = pow(g0s.row(i), pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1));
        }
        g = trans(g);
        g = g.rows(1, bw);
        mat xnei(bw, k, fill::zeros);
        vec ynei(bw, fill::zeros);
        for (int j = 0; j < bw; j++) {
            int inei = int(mNeighbours(i, j));
            xnei.row(j) = x.row(inei);
            ynei.row(j) = y(inei);
        }
        for (int k1 = 0; k1 < k; k1++) {
            mat XtG = xnei.col(k1) * spanXnei % g;
            for (int p = 0; p < (mPolynomial + 1); p++) {
                mat XtGX = XtG.col(p) * spanXtG % xnei;
                for (int k2 = 0; k2 < k; k2++) {
                    int xindex = (k1 * (mPolynomial + 1) + p) * k + k2;
                    mMx0(xindex, i) = sum(XtGX.col(k2));
                }
                int yindex = p * k + k1;
                vec XtGY = XtG.col(p) % ynei;
                mMy0(yindex, i) = sum(XtGY);
            }
        }
    }
}

arma::mat GwmScalableGWRAlgorithm::regression(const arma::mat &x, const arma::vec &y)
{
    GwmBandwidthWeight* bandwidth = mSpatialWeight.weight<GwmBandwidthWeight>();
    int bw = bandwidth->bandwidth();
    int n = x.n_rows, k = x.n_cols, poly1 = mPolynomial + 1;
    double b = mScale, a = mPenalty;
    mat XtX = x.t() * x, XtY = x.t() * y;
    /**
     * Calculate Rx, Ry, and R0.
     */
    // printf("Calculate Rx, Ry, and R0 ");
    vec R0 = vec(poly1, fill::ones) * b;
    for (int p = 1; p < poly1; p++) {
        R0(p) = pow(b, p + 1);
    }
    vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
    for (int p = 0; p < poly1; p++) {
        for (int k2 = 0; k2 < k; k2++) {
            for (int k1 = 0; k1 < k; k1++) {
                Rx(k1*poly1*k + p*k + k2) = R0(p);
            }
            Ry(p*k + k2) = R0(p);
        }
    }
    /**
    * Create G0.
    */
    // printf("Create G0 ");
    mat G0s(mG0.n_cols + 1, mG0.n_rows, fill::ones);
    mat G0t = trans(mG0);
    G0s.rows(1, bw) = G0t.rows(0, bw - 1);
    G0s = trans(G0s);
    mat G2(n, bw + 1, fill::zeros);
    for (int p = 0; p < mPolynomial; p++) {
        G2 += pow(G0s, pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1)) * R0(mPolynomial - 1);
    }
    /**
     * Calculate Mx, My.
     */
    // printf("Calculate Mx, My ");
    // mat Mx00(Mx0), My00(My0);
    for (int i = 0; i < n; i++) {
        for (int k1 = 0; k1 < k; k1++) {
            for (int p = 0; p < poly1; p++) {
                for (int k2 = 0; k2 < k; k2++) {
                    mMx0((k1 * (mPolynomial + 1) + p) * k + k2, i) += x(i, k1) * x(i, k2);
                }
                mMy0(p * k + k1, i) += x(i, k1) * y(i);
            }
        }
    }
    mat Mx = (Rx * mat(1, n, fill::ones)) % mMx0, My = (Ry * mat(1, n, fill::ones)) % mMy0;
    /**
     * Regression.
     */
    // printf("Regression ");
    mat Xp(bw + 1, k * poly1, fill::zeros);
    mat rowsumG(poly1, 1, fill::ones);
    mat colsumXp(1, bw + 1, fill::zeros);
    mat spanG(1, k, fill::ones);
    mat spanX(1, poly1, fill::ones);
    mat betas = mat(n, k, fill::zeros);
    mBetasSE = mat(n, k, fill::zeros);
    double trS = 0.0, trStS = 0.0;
    bool isAllCorrect = true;
    for (int i = 0; i < n; i++) {
        /**
       * Calculate G.
       */
        mat G = mat(poly1, bw + 1, fill::ones) * R0(0);
        for (int p = 0; p < mPolynomial; p++) {
            G.row(p + 1) = pow(G0s.row(i), pow(2.0, mPolynomial/2.0)/pow(2.0, p + 1)) * R0(p);
        }
        G = trans(G);
        mat g = G * rowsumG;
        /**
       * Calculate Xp.
       */
        mat xnei(bw + 1, k, fill::zeros);
        vec ynei(bw + 1, fill::zeros);
        xnei.row(0) = x.row(i);
        ynei.row(0) = y.row(i);
        for (int j = 0; j < bw; j++) {
            int inei = int(mNeighbours(i, j));
            xnei.row(j+1) = x.row(inei);
            ynei.row(j+1) = y(inei);
        }
        /**
       * Calculate sumMx, sumMy.
       */
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
        sumMx += a * XtX;
        sumMy += a * XtY;
        try {
            mat invMx = inv(sumMx);
            betas.row(i) = trans(invMx * sumMy);
            /**
           * Calculate Diagnostic statistics, trS and trStS.
           */
            mat StS = invMx * trans(x.row(i));
            trS += det((x.row(i) * g(0, 0)) * StS);
            mat XG2X(k, k, fill::zeros);
            for (int k1 = 0; k1 < k; k1++) {
                for (int k2 = 0; k2 < k; k2++) {
                    mat Gi = G2.row(i);
                    XG2X(k1, k2) = sum(xnei.col(k1) % trans(Gi % Gi + 2 * a * Gi) % xnei.col(k2)) + a * a * XtX(k1, k2);
                }
            }
            mat XX = invMx * XG2X * invMx;
            mat xi = x.row(i);
            trStS += det(sum(xi * XX * trans(xi)));
            mBetasSE.row(i) = trans(sqrt(XX.diag()));
        } catch (...) {
            isAllCorrect = false;
        }
    }
    mShat = { trS, trStS };
    return betas;
}

void GwmScalableGWRAlgorithm::createResultLayer(initializer_list<CreateResultLayerDataItem> data)
{
    QgsVectorLayer* srcLayer = mDataLayer;
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
