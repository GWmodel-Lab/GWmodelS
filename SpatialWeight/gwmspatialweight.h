#ifndef GWMSPATIALWEIGHT_H
#define GWMSPATIALWEIGHT_H

#include "SpatialWeight/gwmweight.h"
#include "SpatialWeight/gwmdistance.h"

class GwmSpatialWeight
{
public:
    GwmSpatialWeight();
    GwmSpatialWeight(GwmWeight* weight, GwmDistance* distance);
    GwmSpatialWeight(const GwmSpatialWeight& spatialWeight);
    ~GwmSpatialWeight();

    GwmWeight *weight() const;
    void setWeight(GwmWeight *weight);

    GwmDistance *distance() const;
    void setDistance(GwmDistance *distance);

public:
    GwmSpatialWeight& operator=(const GwmSpatialWeight& spatialWeight)
    {
        if (this == &spatialWeight) return *this;
        if (mWeight) delete mWeight;
        if (mDistance) delete mDistance;
        mWeight = spatialWeight.mWeight->clone();
        mDistance = spatialWeight.mDistance->clone();
        return *this;
    }
    GwmSpatialWeight& operator=(const GwmSpatialWeight&& spatialWeight)
    {
        if (this == &spatialWeight) return *this;
        if (mWeight) delete mWeight;
        if (mDistance) delete mDistance;
        mWeight = spatialWeight.mWeight->clone();
        mDistance = spatialWeight.mDistance->clone();
        return *this;
    }

public:
    vec spatialWeight(rowvec target, mat dataPoints);
    bool isValid();

private:
    GwmWeight* mWeight = nullptr;
    GwmDistance* mDistance = nullptr;
};

#endif // GWMSPATIALWEIGHT_H
