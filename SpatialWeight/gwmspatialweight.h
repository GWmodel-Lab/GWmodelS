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
    void setWeight(GwmWeight& weight);
    void setWeight(GwmWeight&& weight);

    GwmDistance *distance() const;
    void setDistance(GwmDistance *distance);
    void setDistance(GwmDistance& distance);
    void setDistance(GwmDistance&& distance);

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
    vec spatialWeight(int i);
    bool isValid();

private:
    GwmWeight* mWeight = nullptr;
    GwmDistance* mDistance = nullptr;
};

inline GwmWeight *GwmSpatialWeight::weight() const
{
    return mWeight;
}

inline void GwmSpatialWeight::setWeight(GwmWeight *weight)
{
    if (mWeight) delete mWeight;
    mWeight = weight;
}

inline void GwmSpatialWeight::setWeight(GwmWeight& weight)
{
    if (mWeight) delete mWeight;
    mWeight = weight.clone();
}

inline void GwmSpatialWeight::setWeight(GwmWeight&& weight)
{
    if (mWeight) delete mWeight;
    mWeight = weight.clone();
}

inline GwmDistance *GwmSpatialWeight::distance() const
{
    return mDistance;
}

inline void GwmSpatialWeight::setDistance(GwmDistance *distance)
{
    if (mDistance) delete mDistance;
    mDistance = distance;
}

inline void GwmSpatialWeight::setDistance(GwmDistance& distance)
{
    if (mDistance) delete mDistance;
    mDistance = distance.clone();
}

inline void GwmSpatialWeight::setDistance(GwmDistance&& distance)
{
    if (mDistance) delete mDistance;
    mDistance = distance.clone();
}

inline vec GwmSpatialWeight::spatialWeight(int i)
{
    return mWeight->weight(mDistance->distance(i));
}

#endif // GWMSPATIALWEIGHT_H
