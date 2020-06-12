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

GwmWeight *GwmSpatialWeight::weight() const
{
    return mWeight;
}

void GwmSpatialWeight::setWeight(GwmWeight *weight)
{
    if (mWeight) delete mWeight;
    mWeight = weight;
}

GwmDistance *GwmSpatialWeight::distance() const
{
    return mDistance;
}

void GwmSpatialWeight::setDistance(GwmDistance *distance)
{
    if (mDistance) delete mDistance;
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
