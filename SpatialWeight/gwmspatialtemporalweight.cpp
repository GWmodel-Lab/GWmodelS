#include "gwmspatialtemporalweight.h"

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight() : GwmSpatialWeight()
{

}

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight(GwmWeight *weight, GwmDistance *distance, vec &timestamp, double lambda)
    : GwmSpatialWeight(weight, distance)
{
    mLambda = lambda;
    mTimestamp = timestamp;
}

GwmSpatialTemporalWeight::GwmSpatialTemporalWeight(const GwmSpatialTemporalWeight &spatialTemporalWeight)
    : GwmSpatialWeight(spatialTemporalWeight)
{
    mLambda = spatialTemporalWeight.mLambda;
    mTimestamp = spatialTemporalWeight.mTimestamp;
}

GwmSpatialTemporalWeight::~GwmSpatialTemporalWeight()
{

}

GwmSpatialTemporalWeight &GwmSpatialTemporalWeight::operator=(const GwmSpatialTemporalWeight &spatialTemporalWeight)
{
    if (this == &spatialTemporalWeight) return *this;
    GwmSpatialWeight::operator=(spatialTemporalWeight);
    mLambda = spatialTemporalWeight.mLambda;
    mTimestamp = spatialTemporalWeight.mTimestamp;
    return *this;
}

GwmSpatialTemporalWeight &GwmSpatialTemporalWeight::operator=(const GwmSpatialTemporalWeight &&spatialTemporalWeight)
{
    if (this == &spatialTemporalWeight) return *this;
    GwmSpatialWeight::operator=(spatialTemporalWeight);
    mLambda = spatialTemporalWeight.mLambda;
    mTimestamp = spatialTemporalWeight.mTimestamp;
    return *this;
}

vec GwmSpatialTemporalWeight::weightVector(int i)
{
    vec sDist = distance()->distance(i);
    vec tDist = -(mTimestamp.each_row() - mTimestamp.row(i));
    uvec neg = tDist < 0;
    vec stD = sqrt(sDist % sDist + mLambda * (tDist % tDist));
    stD.rows(neg).fill(0);
    return weight()->weight(stD);
}

bool GwmSpatialTemporalWeight::isValid()
{
    return GwmSpatialWeight::isValid() && (distance()->length() == mTimestamp.n_rows);
}
