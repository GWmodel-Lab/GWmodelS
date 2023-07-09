#ifndef GWMSPATIALTEMPORALMONOSCALE_H
#define GWMSPATIALTEMPORALMONOSCALE_H

#include <QObject>

#include "TaskThread/gwmspatialalgorithm.h"
#include "SpatialWeight/gwmspatialtemporalweight.h"

class GwmSpatialTemporalMonoscaleAlgorithm : public GwmSpatialAlgorithm
{
    Q_OBJECT
public:
    GwmSpatialTemporalMonoscaleAlgorithm();

    GwmSpatialTemporalWeight stWeight() const;
    void setSTWeight(const GwmSpatialTemporalWeight &stWeight);

protected:
    GwmSpatialTemporalWeight mSTWeight;
};

inline GwmSpatialTemporalWeight GwmSpatialTemporalMonoscaleAlgorithm::stWeight() const
{
    return mSTWeight;
}

inline void GwmSpatialTemporalMonoscaleAlgorithm::setSTWeight(const GwmSpatialTemporalWeight &sTWeight)
{
    mSTWeight = sTWeight;
}


#endif // GWMSPATIALTEMPORALMONOSCALE_H
