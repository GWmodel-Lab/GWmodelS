#ifndef GWMCRSDISTANCE_H
#define GWMCRSDISTANCE_H

#include "SpatialWeight/gwmdistance.h"

class GwmCRSDistance : public GwmDistance
{
public:
    static vec SpatialDistance(const rowvec& out_loc, const mat& in_locs);
    static vec EuclideanDistance(const rowvec& out_loc, const mat& in_locs);
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
    virtual vec distance(const rowvec& target, const mat& dataPoints) override;

private:
    bool mGeographic = false;
};

inline vec GwmCRSDistance::EuclideanDistance(const rowvec& out_loc, const mat& in_locs)
{
    mat diff = (in_locs.each_row() - out_loc);
    return sqrt(sum(diff % diff, 1));
}

inline bool GwmCRSDistance::geographic() const
{
    return mGeographic;
}

inline void GwmCRSDistance::setGeographic(bool geographic)
{
    mGeographic = geographic;
}

inline vec GwmCRSDistance::distance(const rowvec& target, const mat& dataPoints)
{
    return mGeographic ?
                SpatialDistance(target, dataPoints) :
                EuclideanDistance(target, dataPoints);
}

#endif // GWMCRSDISTANCE_H
