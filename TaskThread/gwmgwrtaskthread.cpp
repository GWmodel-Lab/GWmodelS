#include "gwmgwrtaskthread.h"

GwmGWRTaskThread::GwmGWRTaskThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars)
    : GwmTaskThread()
    , mLayer(layer)
    , mDepVar(depVar)
    , mIndepVars(indepVars)
{

}
