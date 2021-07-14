#include "gwmtaskthread.h"

GwmTaskThread::GwmTaskThread()
{

}

QString GwmTaskThread::name() const
{
    return tr("Task");
}

bool GwmTaskThread::checkCanceled()
{
    if (isCanceled())
    {
        emit canceled();
        return true;
    }
    else return false;
}
