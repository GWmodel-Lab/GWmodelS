#include "gwmspatialweight.h"


GwmSpatialWeight::GwmSpatialWeight()
{

}

GwmSpatialWeight::GwmSpatialWeight(GwmWeight *weight, GwmDistance *distance)
{
    mWeight = weight;
    mDistance = distance;
}

GwmSpatialWeight::~GwmSpatialWeight()
{
    if (mWeight) delete mWeight;
    if (mDistance) delete mDistance;
}

GwmWeight *GwmSpatialWeight::weight() const
{
    return mWeight;
}

void GwmSpatialWeight::setWeight(GwmWeight *weight)
{
    mWeight = weight;
}

GwmDistance *GwmSpatialWeight::distance() const
{
    return mDistance;
}

void GwmSpatialWeight::setDistance(GwmDistance *distance)
{
    mDistance = distance;
}

vec GwmSpatialWeight::spatialWeight(rowvec target, mat dataPoints)
{
    return mWeight->weight(mDistance->distance(target, dataPoints));
}

bool GwmSpatialWeight::isValid()
{
    return !((mWeight == 0) || (mDistance == 0));
}
