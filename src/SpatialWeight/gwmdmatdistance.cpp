#include "gwmdmatdistance.h"
#include <QFile>

GwmDMatDistance::GwmDMatDistance(int total, QString dmatFile) : GwmDistance(total)
{
    mDMatFile = dmatFile;
    mRowSize = total;
}

GwmDMatDistance::GwmDMatDistance(const GwmDMatDistance &distance) : GwmDistance(distance)
{
    mDMatFile = distance.mDMatFile;
    mRowSize = distance.mRowSize;
}

vec GwmDMatDistance::distance(int focus)
{
    QFile dmat(mDMatFile);
    if (focus < mTotal && dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * mRowSize * sizeof (double));
        QByteArray values = dmat.read(mRowSize * sizeof (double));
        return vec((double*)values.data(), mRowSize);
    }
    else return vec(mRowSize, fill::zeros);
}
