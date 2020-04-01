#include "gwmgwrtaskthread.h"

GwmGWRTaskThread::GwmGWRTaskThread(QgsVectorLayer* layer, int depVarIndex, QList<int> indepVarIndex)
    : GwmTaskThread()
    , mLayer(layer)
    , mDepVarIndex(depVarIndex)
    , mIndepVarIndex(indepVarIndex)
{

}
