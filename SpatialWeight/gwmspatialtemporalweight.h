#ifndef GWMSPATIALTEMPORALWEIGHT_H
#define GWMSPATIALTEMPORALWEIGHT_H

#include "SpatialWeight/gwmspatialweight.h"

class GwmSpatialTemporalWeight : public GwmSpatialWeight
{
public:
    GwmSpatialTemporalWeight();
    GwmSpatialTemporalWeight(GwmWeight* weight, GwmDistance* distance, vec& timestamp, double lambda);
    GwmSpatialTemporalWeight(const GwmSpatialTemporalWeight& spatialTemporalWeight);
    ~GwmSpatialTemporalWeight();

public:
    GwmSpatialTemporalWeight& operator=(const GwmSpatialTemporalWeight& spatialTemporalWeight);
    GwmSpatialTemporalWeight& operator=(const GwmSpatialTemporalWeight&& spatialTemporalWeight);

public:
    vec weightVector(int i) override;
    bool isValid() override;

private:
    double mLambda = 0.0;

    vec mTimestamp;
};

#endif // GWMSPATIALTEMPORALWEIGHT_H
