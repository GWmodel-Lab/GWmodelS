#include "gwmlayerscalablegwritem.h"

GwmLayerScalableGWRItem::GwmLayerScalableGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmScalableGWRTaskThread* taskThread)
    : GwmLayerGWRItem(parent, vector, taskThread)
{
    mPolynomial = taskThread->getPolynomial();
    mCV = taskThread->getCV();
    mScale = taskThread->getScale();
    mPenalty = taskThread->getPenalty();
}

double GwmLayerScalableGWRItem::getCV() const
{
    return mCV;
}

double GwmLayerScalableGWRItem::getScale() const
{
    return mScale;
}

double GwmLayerScalableGWRItem::getPenalty() const
{
    return mPenalty;
}

int GwmLayerScalableGWRItem::getPolynomial() const
{
    return mPolynomial;
}
