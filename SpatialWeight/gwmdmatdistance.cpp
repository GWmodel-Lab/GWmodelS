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

vec GwmDMatDistance::distance(int focus)
{
    QFile dmat(mDMatFile);
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
