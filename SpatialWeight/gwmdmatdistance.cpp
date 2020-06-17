#include "gwmdmatdistance.h"
#include <QFile>

GwmDMatDistance::GwmDMatDistance(int total) : GwmDistance(total)
{

}

GwmDMatDistance::GwmDMatDistance(int total, QString dmatFile) : GwmDistance(total)
{
    mDMatFile = dmatFile;
}

GwmDMatDistance::GwmDMatDistance(const GwmDMatDistance &distance) : GwmDistance(distance)
{
    mDMatFile = distance.mDMatFile;
}

vec GwmDMatDistance::distance(int focus)
{
    QFile dmat(mDMatFile);
    if (focus < mTotal && dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * mTotal * sizeof (double));
        QByteArray values = dmat.read(mTotal * sizeof (double));
        return vec((double*)values.data(), mTotal);
    }
    else return vec(mTotal, fill::zeros);
}
