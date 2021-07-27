#ifndef GWMBANDWIDTHSIZESELECTOR_H
#define GWMBANDWIDTHSIZESELECTOR_H

#include <QMetaType>
#include <qwt_plot.h>
#include "SpatialWeight/gwmspatialweight.h"
#include "SpatialWeight/gwmbandwidthweight.h"

typedef  QList<QPair<double, double> >  BandwidthCriterionList;
Q_DECLARE_METATYPE(BandwidthCriterionList)

struct IBandwidthSizeSelectable
{
    virtual double criterion(GwmBandwidthWeight* weight) = 0;
};

class GwmBandwidthSizeSelector
{
public:
    static void PlotBandwidthResult(QVariant data, QwtPlot* plot);

public:
    GwmBandwidthSizeSelector();

    GwmBandwidthWeight *bandwidth() const;
    void setBandwidth(GwmBandwidthWeight *bandwidth);

    double lower() const;
    void setLower(double lower);

    double upper() const;
    void setUpper(double upper);

    BandwidthCriterionList bandwidthCriterion() const;

public:
    GwmBandwidthWeight* optimize(IBandwidthSizeSelectable* instance);
    bool isCanceled() const;
    void setCanceled(bool newCanceled);
    bool checkCanceled();
    int counter = 0;
protected:
    bool mIsCanceled = false;

private:
    GwmBandwidthWeight* mBandwidth;
    double mLower;
    double mUpper;
    QHash<double, double> mBandwidthCriterion;
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

inline bool GwmBandwidthSizeSelector::isCanceled() const
{
    return mIsCanceled;
}

inline void GwmBandwidthSizeSelector::setCanceled(bool newCanceled)
{
    mIsCanceled = newCanceled;
}

#endif // GWMBANDWIDTHSIZESELECTOR_H
