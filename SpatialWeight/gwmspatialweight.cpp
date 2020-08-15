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

GwmSpatialWeight &GwmSpatialWeight::operator=(const GwmSpatialWeight &&spatialWeight)
{
    if (this == &spatialWeight) return *this;
    if (mWeight) delete mWeight;
    if (mDistance) delete mDistance;
    mWeight = spatialWeight.mWeight->clone();
    mDistance = spatialWeight.mDistance->clone();
    return *this;
}

GwmSpatialWeight &GwmSpatialWeight::operator=(const GwmSpatialWeight &spatialWeight)
{
    if (this == &spatialWeight) return *this;
    if (mWeight) delete mWeight;
    if (mDistance) delete mDistance;
    mWeight = spatialWeight.mWeight->clone();
    mDistance = spatialWeight.mDistance->clone();
    return *this;
}

bool GwmSpatialWeight::isValid()
{
    return !((mWeight == 0) || (mDistance == 0));
}
