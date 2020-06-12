#include "gwmdmatdistance.h"
#include <QFile>

GwmDMatDistance::GwmDMatDistance() : GwmDistance()
{

}

GwmDMatDistance::GwmDMatDistance(QString dmatFile, int featureCount)
{
    mDMatFile = dmatFile;
    mFeatureCount = featureCount;
}

GwmDMatDistance::GwmDMatDistance(const GwmDMatDistance &distance)
{
    mDMatFile = distance.mDMatFile;
    mFeatureCount = distance.mFeatureCount;
}

QString GwmDMatDistance::dMatFile() const
{
    return mDMatFile;
}

void GwmDMatDistance::setDMatFile(const QString &dMatFile)
{
    mDMatFile = dMatFile;
}

vec GwmDMatDistance::distance(rowvec params, mat dataPoints)
{
    QFile dmat(mDMatFile);
    int focus = params(0);
    if (dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * mFeatureCount * sizeof (double));
        QByteArray values = dmat.read(mFeatureCount * sizeof (double));
        return vec((double*)values.data(), mFeatureCount);
    }
    else
    {
        return vec(mFeatureCount, fill::zeros);
    }
}

int GwmDMatDistance::featureCount() const
{
    return mFeatureCount;
}

void GwmDMatDistance::setFeatureCount(int featureCount)
{
    mFeatureCount = featureCount;
}
