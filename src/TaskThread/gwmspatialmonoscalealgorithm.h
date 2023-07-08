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

    GwmSpatialWeight spatialWeight() const;
    void setSpatialWeight(const GwmSpatialWeight &spatialWeight);

protected:
    GwmSpatialWeight mSpatialWeight;
};


inline GwmSpatialWeight GwmSpatialMonoscaleAlgorithm::spatialWeight() const
{
    return mSpatialWeight;
}

inline void GwmSpatialMonoscaleAlgorithm::setSpatialWeight(const GwmSpatialWeight &spatialWeight)
{
    mSpatialWeight = spatialWeight;
}


#endif // GWMSPATIALMONOSCALEALGORITHM_H
