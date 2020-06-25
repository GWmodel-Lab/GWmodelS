#include "gwmspatialweight.h"


GwmSpatialWeight::GwmSpatialWeight()
{

}

GwmSpatialWeight::GwmSpatialWeight(GwmWeight *weight, GwmDistance *distance)
{
    mWeight = weight;
    mDistance = distance;
}

GwmSpatialWeight::GwmSpatialWeight(const GwmSpatialWeight &spatialWeight)
{
    mWeight = spatialWeight.mWeight->clone();
    mDistance = spatialWeight.mDistance->clone();
}

GwmSpatialWeight::~GwmSpatialWeight()
{
    if (mWeight) delete mWeight;
    if (mDistance) delete mDistance;
}

bool GwmSpatialWeight::isValid()
{
    return !((mWeight == 0) || (mDistance == 0));
}
