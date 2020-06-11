#include "gwmspatialmonoscalealgorithm.h"

GwmSpatialMonoscaleAlgorithm::GwmSpatialMonoscaleAlgorithm() : GwmSpatialAlgorithm()
{

}

GwmSpatialWeight GwmSpatialMonoscaleAlgorithm::spatialWeight() const
{
    return mSpatialWeight;
}

void GwmSpatialMonoscaleAlgorithm::setSpatialWeight(const GwmSpatialWeight &spatialWeight)
{
    mSpatialWeight = spatialWeight;
}
