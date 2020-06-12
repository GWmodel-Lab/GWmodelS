#ifndef GWMBANDWIDTHSIZESELECTOR_H
#define GWMBANDWIDTHSIZESELECTOR_H

#include <QObject>

#include "SpatialWeight/gwmspatialweight.h"
#include "SpatialWeight/gwmbandwidthweight.h"

interface IBandwidthSizeSelectable
{
    virtual double criterion(GwmBandwidthWeight* weight) = 0;
};

class GwmBandwidthSizeSelector
{
    Q_OBJECT
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

#endif // GWMBANDWIDTHSIZESELECTOR_H
