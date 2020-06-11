#ifndef GWMSPATIALWEIGHT_H
#define GWMSPATIALWEIGHT_H

#include "SpatialWeight/gwmweight.h"
#include "SpatialWeight/gwmdistance.h"

class GwmSpatialWeight
{
public:
    GwmSpatialWeight();
    GwmSpatialWeight(GwmWeight* weight, GwmDistance* distance);
    ~GwmSpatialWeight();

    GwmWeight *weight() const;
    void setWeight(GwmWeight *weight);

    GwmDistance *distance() const;
    void setDistance(GwmDistance *distance);

public:
    vec spatialWeight(rowvec target, mat dataPoints);
    bool isValid();

private:
    GwmWeight* mWeight;
    GwmDistance* mDistance;
};

#endif // GWMSPATIALWEIGHT_H
