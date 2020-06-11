#include "gwmgwsstaskthread.h"

GwmGWSSTaskThread::GwmGWSSTaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

bool GwmGWSSTaskThread::quantile() const
{
    return mQuantile;
}

void GwmGWSSTaskThread::setQuantile(bool quantile)
{
    mQuantile = quantile;
}
