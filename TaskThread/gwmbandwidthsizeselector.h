#ifndef GWMBANDWIDTHSIZESELECTOR_H
#define GWMBANDWIDTHSIZESELECTOR_H

#include "SpatialWeight/gwmspatialweight.h"
#include "SpatialWeight/gwmbandwidthweight.h"

struct IBandwidthSizeSelectable
{
    virtual double criterion(GwmBandwidthWeight* weight) = 0;
};

class GwmBandwidthSizeSelector
{
public:
    GwmBandwidthSizeSelector();

    GwmBandwidthWeight *bandwidth() const;
    void setBandwidth(GwmBandwidthWeight *bandwidth);

    double lower() const;
    void setLower(double lower);

    double upper() const;
    void setUpper(double upper);

    QList<QPair<double, double> > bandwidthCriterion() const;
    void setBandwidthCriterion(const QList<QPair<double, double> > &bandwidthCriterion);

public:
    GwmBandwidthWeight* optimize(IBandwidthSizeSelectable* instance);

private:
    GwmBandwidthWeight* mBandwidth;
    double mLower;
    double mUpper;
    QList<QPair<double, double> > mBandwidthCriterion;
};

inline GwmBandwidthWeight *GwmBandwidthSizeSelector::bandwidth() const
{
    return mBandwidth;
}

inline void GwmBandwidthSizeSelector::setBandwidth(GwmBandwidthWeight *bandwidth)
{
    mBandwidth = bandwidth;
}

inline double GwmBandwidthSizeSelector::lower() const
{
    return mLower;
}

inline void GwmBandwidthSizeSelector::setLower(double lower)
{
    mLower = lower;
}

inline double GwmBandwidthSizeSelector::upper() const
{
    return mUpper;
}

inline void GwmBandwidthSizeSelector::setUpper(double upper)
{
    mUpper = upper;
}

inline QList<QPair<double, double> > GwmBandwidthSizeSelector::bandwidthCriterion() const
{
    return mBandwidthCriterion;
}

inline void GwmBandwidthSizeSelector::setBandwidthCriterion(const QList<QPair<double, double> > &bandwidthCriterion)
{
    mBandwidthCriterion = bandwidthCriterion;
}

#endif // GWMBANDWIDTHSIZESELECTOR_H
