#ifndef GWMSPATIALMONOSCALEALGORITHM_H
#define GWMSPATIALMONOSCALEALGORITHM_H

#include <QObject>

#include "TaskThread/gwmspatialalgorithm.h"
#include "SpatialWeight/gwmspatialweight.h"

class GwmSpatialMonoscaleAlgorithm : public GwmSpatialAlgorithm
{
    Q_OBJECT
public:
    GwmSpatialMonoscaleAlgorithm();

    gwm::SpatialWeight spatialWeight() const;
    void setSpatialWeight(const gwm::SpatialWeight &spatialWeight);

protected:
    gwm::SpatialWeight mSpatialWeight;
};


inline gwm::SpatialWeight GwmSpatialMonoscaleAlgorithm::spatialWeight() const
{
    return mSpatialWeight;
}

inline void GwmSpatialMonoscaleAlgorithm::setSpatialWeight(const gwm::SpatialWeight &spatialWeight)
{
    mSpatialWeight = spatialWeight;
}


#endif // GWMSPATIALMONOSCALEALGORITHM_H
