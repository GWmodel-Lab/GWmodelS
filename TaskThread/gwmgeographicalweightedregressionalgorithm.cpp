#include "gwmgeographicalweightedregressionalgorithm.h"
#include <qpair.h>
using namespace arma;

GwmGeographicalWeightedRegressionAlgorithm::GwmGeographicalWeightedRegressionAlgorithm()
{

}

void GwmGeographicalWeightedRegressionAlgorithm::run()
{
    mat x;
    vec y;
    init(x, y);
    // 解算模型
    if (hasHatMatrix)
    {
        uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
        mat betasSE, S(isStoreS() ? nDp : 1, nDp, fill::zeros);
        vec shat, qDiag;
        mBetas = regression(x, y, betasSE, shat, qDiag, S);
        // 诊断
        mDiagnostic = calcDiagnostic(x, y, mBetas, shat);
        vec yhat = Fitted(x, mBetas);
        vec res = y - yhat;
        double trS = shat(0), trStS = shat(1);
        double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
        vec stu_res = res / sqrt(sigmaHat * qDiag);
        betasSE = sqrt(sigmaHat * betasSE);
        mat betasTV = mBetas / betasSE;
        vec dybar2 = (y - mean(y)) % (y - mean(y));
        vec dyhat2 = (y - yhat) % (y - yhat);
        vec localR2 = vec(nDp, fill::zeros);
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeight.spatialWeight(mDataPoints.row(i), mDataPoints);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            localR2(i) = (tss - rss) / tss;
        }

        QList<QPair<QString, const mat&> > resultLayerData = {
            qMakePair(QString("%1"), mBetas),
            qMakePair(QString("y"), y),
            qMakePair(QString("yhat"), yhat),
            qMakePair(QString("residual"), res),
            qMakePair(QString("Stud_residual"), stu_res),
            qMakePair(QString("%1_SE"), betasSE),
            qMakePair(QString("%1_TV"), betasTV),
            qMakePair(QString("localR2"), localR2)
        };

        createResultLayer(resultLayerData);
    }
    else
    {
        mBetas = regression(x, y);
        QList<QPair<QString, const mat&> > resultLayerData = {
            qMakePair(QString("%1"), mBetas)
        };
        createResultLayer(resultLayerData);
    }
}

bool GwmGeographicalWeightedRegressionAlgorithm::isValid()
{
    if (mDataLayer == nullptr)
        return false;

    if (mIndepVars.size() < 1)
        return false;

    if (mRegressionLayer && hasHatMatrix)
        return false;

    if (mRegressionLayer && hasFTest)
        return false;

    return true;
}

mat GwmGeographicalWeightedRegressionAlgorithm::regression(const mat &x, const vec &y)
{
    uword nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    const mat& points = hasRegressionLayer() ? mRegressionPoints : mDataPoints;
    mat betas(nVar, nRp, fill::zeros);
    for (uword i = 0; i < nRp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(points.row(i), mDataPoints);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            emit tick(i + 1, nRp);
        }
        catch (exception e)
        {
            emit error(e.what());
        }
    }
    return betas.t();
}

mat GwmGeographicalWeightedRegressionAlgorithm::regression(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(mDataPoints.row(i), mDataPoints);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            betasSE.col(i) = sum(ci % ci, 1);
            mat si = x.row(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
            vec p = - si.t();
            p(i) += 1.0;
            qDiag += p % p;
            S.row(isStoreS() ? i : 0) = si;
        }
        catch (std::exception e)
        {
            emit error(e.what());
        }
        emit tick(i + 1, nDp);
    }
    betasSE = betasSE.t();
    return betas.t();
}

void GwmGeographicalWeightedRegressionAlgorithm::init(mat &x, mat &y)
{
    int nDp = mDataLayer->featureCount(), nRp = hasRegressionLayer() ? mRegressionLayer->featureCount() : nDp;
    int nVar = mIndepVars.size() + 1;
    // Data layer and X,Y
    mDataPoints = mat(nDp, 2, fill::zeros);
    mBetas = mat(nRp, nVar, fill::zeros);
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        mDataPoints(i, 0) = centroPoint.x();
        mDataPoints(i, 1) = centroPoint.y();
        double vY = f.attribute(mDepVar.index).toDouble(&ok);
        if (ok)
        {
            y(i) = vY;
            x(i, 0) = 1.0;
            for (int k = 0; k < mIndepVars.size(); k++)
            {
                double vX = f.attribute(mIndepVars[k].index).toDouble(&ok);
                if (ok) x(i, k + 1) = vX;
                else emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
            }
        }
        emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
    // Regression Layer
    if (hasRegressionLayer())
    {
        mRegressionPoints = mat(nRp, 2, fill::zeros);
        iterator = mRegressionLayer->getFeatures();
        for (int i = 0; iterator.nextFeature(f); i++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegressionPoints(i, 0) = centroPoint.x();
            mRegressionPoints(i, 1) = centroPoint.y();
        }
    }
    else mRegressionPoints = mDataPoints;
}

GwmDiagnostic GwmGeographicalWeightedRegressionAlgorithm::calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
{
    vec r = y - sum(betas % x, 1);
    double rss = sum(r % r);
    int n = x.n_rows;
    double AIC = n * log(rss / n) + n * log(2 * datum::pi) + n + shat(0);																//AIC
    double AICc = n * log(rss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0))); //AICc
    double edf = n - 2 * shat(0) + shat(1);																														//edf
    double enp = 2 * shat(0) - shat(1);																																// enp
    double yss = sum((y - mean(y)) % (y - mean(y)));																															//yss.g
    double r2 = 1 - rss / yss;
    double r2_adj = 1 - (1 - r2) * (n - 1) / (edf - 1);
    return { rss, AIC, AICc, enp, edf, r2, r2_adj };
}

void GwmGeographicalWeightedRegressionAlgorithm::createResultLayer(QList<QPair<QString, const mat &> > data)
{
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
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
