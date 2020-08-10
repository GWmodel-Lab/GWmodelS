#ifndef GWMSPATIALTEMPORALWEIGHT_H
#define GWMSPATIALTEMPORALWEIGHT_H

#include "SpatialWeight/gwmspatialweight.h"

class GwmSpatialTemporalWeight : public GwmSpatialWeight
{
public:
    GwmSpatialTemporalWeight();
    GwmSpatialTemporalWeight(GwmWeight* weight, GwmDistance* distance, vec& dataTimeStamp, double lambda);
    GwmSpatialTemporalWeight(const GwmSpatialTemporalWeight& spatialTemporalWeight);
    ~GwmSpatialTemporalWeight();

public:
    GwmSpatialTemporalWeight& operator=(const GwmSpatialTemporalWeight& spatialTemporalWeight);
    GwmSpatialTemporalWeight& operator=(const GwmSpatialTemporalWeight&& spatialTemporalWeight);

public:
    vec distanceVector(int i);
    vec weightVector(int i) override;
    bool isValid() override;

    vec dataTimeStamp() const;
    void setDataTimeStamp(const vec &dataTimeStamp);

    vec focusTimeStamp() const;
    void setFocusTimeStamp(const vec &focusTimeStamp);

private:
    double mLambda = 0.0;

    vec mDataTimeStamp;
    vec mFocusTimeStamp;
};

inline vec GwmSpatialTemporalWeight::dataTimeStamp() const
{
    return mDataTimeStamp;
}

inline void GwmSpatialTemporalWeight::setDataTimeStamp(const vec &timestamp)
{
    mDataTimeStamp = timestamp;
}

inline vec GwmSpatialTemporalWeight::focusTimeStamp() const
{
    return mFocusTimeStamp;
}

inline void GwmSpatialTemporalWeight::setFocusTimeStamp(const vec &focusTimeStamp)
{
    mFocusTimeStamp = focusTimeStamp;
}


#endif // GWMSPATIALTEMPORALWEIGHT_H
