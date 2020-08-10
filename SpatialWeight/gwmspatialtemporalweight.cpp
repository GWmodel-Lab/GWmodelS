#include "gwmspatialtemporalweight.h"

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight() : GwmSpatialWeight()
{

}

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight(GwmWeight *weight, GwmDistance *distance, vec &timestamp, double lambda)
    : GwmSpatialWeight(weight, distance)
{
    mLambda = lambda;
    mDataTimeStamp = timestamp;
}

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight(const GwmSpatialTemporalWeight &spatialTemporalWeight)
    : GwmSpatialWeight(spatialTemporalWeight)
{
    mLambda = spatialTemporalWeight.mLambda;
    mDataTimeStamp = spatialTemporalWeight.mDataTimeStamp;
}

GwmSpatialTemporalWeight::~GwmSpatialTemporalWeight()
{

}

GwmSpatialTemporalWeight &GwmSpatialTemporalWeight::operator=(const GwmSpatialTemporalWeight &spatialTemporalWeight)
{
    if (this == &spatialTemporalWeight) return *this;
    GwmSpatialWeight::operator=(spatialTemporalWeight);
    mLambda = spatialTemporalWeight.mLambda;
    mDataTimeStamp = spatialTemporalWeight.mDataTimeStamp;
    return *this;
}

GwmSpatialTemporalWeight &GwmSpatialTemporalWeight::operator=(const GwmSpatialTemporalWeight &&spatialTemporalWeight)
{
    if (this == &spatialTemporalWeight) return *this;
    GwmSpatialWeight::operator=(spatialTemporalWeight);
    mLambda = spatialTemporalWeight.mLambda;
    mDataTimeStamp = spatialTemporalWeight.mDataTimeStamp;
    return *this;
}

vec GwmSpatialTemporalWeight::distanceVector(int i)
{
    vec sDist = distance()->distance(i);
    vec tDist = -(mDataTimeStamp.each_row() - mFocusTimeStamp.row(i));
    uvec neg = tDist < 0;
    vec stD = sqrt(sDist % sDist + mLambda * (tDist % tDist));
    stD.rows(neg).fill(0);
    return stD;
}

vec GwmSpatialTemporalWeight::weightVector(int i)
{
    return weight()->weight(distanceVector(i));
}

bool GwmSpatialTemporalWeight::isValid()
{
    return GwmSpatialWeight::isValid() && (distance()->length() == mDataTimeStamp.n_rows);
}
