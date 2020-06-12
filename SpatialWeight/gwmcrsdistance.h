#ifndef GWMCRSDISTANCE_H
#define GWMCRSDISTANCE_H

#include "SpatialWeight/gwmdistance.h"

class GwmCRSDistance : public GwmDistance
{
public:
    static vec SpatialDistance(rowvec out_loc, mat in_locs);
    static vec EuclideanDistance(rowvec out_loc, mat in_locs);
    static double SpGcdist(double lon1, double lon2, double lat1, double lat2);

public:
    GwmCRSDistance();
    GwmCRSDistance(bool isGeographic);
    GwmCRSDistance(const GwmCRSDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmCRSDistance(*this);
    }

    bool geographic() const;
    void setGeographic(bool geographic);

public:
    virtual vec distance(rowvec target, mat dataPoints) override;

private:
    bool mGeographic = false;
};

#endif // GWMCRSDISTANCE_H
