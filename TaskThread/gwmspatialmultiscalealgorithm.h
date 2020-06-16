#ifndef GWMSPATIALMULTISCALEALGORITHM_H
#define GWMSPATIALMULTISCALEALGORITHM_H

#include <QObject>

#include "SpatialWeight/gwmspatialweight.h"
#include "TaskThread/gwmspatialalgorithm.h"

class GwmSpatialMultiscaleAlgorithm : public GwmSpatialAlgorithm
{
    Q_OBJECT
public:
    GwmSpatialMultiscaleAlgorithm();

    QList<GwmSpatialWeight> spatialWeights() const;
    virtual void setSpatialWeights(const QList<GwmSpatialWeight> &spatialWeights);
//    void setSpatialWeights(int i, const GwmSpatialWeight& spatialWeight);

protected:
    QList<GwmSpatialWeight> mSpatialWeights;
};

inline QList<GwmSpatialWeight> GwmSpatialMultiscaleAlgorithm::spatialWeights() const
{
    return mSpatialWeights;
}

inline void GwmSpatialMultiscaleAlgorithm::setSpatialWeights(const QList<GwmSpatialWeight> &spatialWeights)
{
    mSpatialWeights = spatialWeights;
}

#endif // GWMSPATIALMULTISCALEALGORITHM_H
