#include "gwmspatialalgorithm.h"

GwmSpatialAlgorithm::GwmSpatialAlgorithm() : GwmTaskThread()
{

}

QgsVectorLayer *GwmSpatialAlgorithm::dataLayer() const
{
    return mDataLayer;
}

void GwmSpatialAlgorithm::setDataLayer(QgsVectorLayer *dataLayer)
{
    mDataLayer = dataLayer;
}

QgsVectorLayer *GwmSpatialAlgorithm::resultLayer() const
{
    return mResultLayer;
}
