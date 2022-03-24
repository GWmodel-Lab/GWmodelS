#include "gwmgeographicalweightedregressionalgorithm.h"
#include <qpair.h>
#include <SpatialWeight/gwmcrsdistance.h>
using namespace arma;

GwmGeographicalWeightedRegressionAlgorithm::GwmGeographicalWeightedRegressionAlgorithm()
{

}


bool GwmGeographicalWeightedRegressionAlgorithm::isValid()
{
    if (mDataLayer == nullptr)
        return false;

    if (mIndepVars.size() < 1)
        return false;

    return true;
}


void GwmGeographicalWeightedRegressionAlgorithm::initPoints()
{
    int nDp = mDataLayer->featureCount();
    mDataPoints = mat(nDp, 2, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; i < nDp && iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        mDataPoints(i, 0) = centroPoint.x();
        mDataPoints(i, 1) = centroPoint.y();
    }
    // Regression Layer
    if (hasRegressionLayer())
    {
        int nRp = mRegressionLayer->featureCount();
        mRegressionPoints = mat(nRp, 2, fill::zeros);
        QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
        QgsFeature f;
        for (int i = 0; i < nRp && iterator.nextFeature(f); i++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegressionPoints(i, 0) = centroPoint.x();
            mRegressionPoints(i, 1) = centroPoint.y();
        }
    }
    else mRegressionPoints = mDataPoints;
    // 设置空间距离中的数据指针
    if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        d->setDataPoints(&mDataPoints);
        d->setFocusPoints(&mRegressionPoints);
    }

}

void GwmGeographicalWeightedRegressionAlgorithm::initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars)
{
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size() + 1;
    // Data layer and X,Y
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {

        double vY = f.attribute(depVar.name).toDouble(&ok);
        if (ok)
        {
            y(i) = vY;
            x(i, 0) = 1.0;
            for (int k = 0; k < indepVars.size(); k++)
            {
                double vX = f.attribute(indepVars[k].name).toDouble(&ok);
                if (ok) x(i, k + 1) = vX;
                else emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
            }
        }
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
}
