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
        mScale = b_tilde * b_tilde;
        mPenalty = alpha * alpha;
        emit message(tr("Scalable GWR calibrating..."));
        vec parameters = { mScale, mPenalty };
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

void GwmScalableGWRTaskThread::diagnostic(bool doLocalR2)
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
        mNeighbours.col(i) = indices(span(1, nBw));
        mNeighbourDists.col(i) = d_sorted(span(1, nBw));
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
    gsl_multimin_fminimizer* minizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex, 2);
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

void GwmScalableGWRTaskThread::createResultLayer()
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

void GwmScalableGWRTaskThread::setPolynomial(int polynomial)
{
    mPolynomial = polynomial;
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
