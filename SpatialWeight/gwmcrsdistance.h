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
    explicit GwmCRSDistance(int total);
    explicit GwmCRSDistance(int total, bool isGeographic);
    GwmCRSDistance(const GwmCRSDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmCRSDistance(*this);
    }

    DistanceType type() override { return DistanceType::CRSDistance; }

    bool geographic() const;
    void setGeographic(bool geographic);

    mat *focusPoints() const;
    void setFocusPoints(mat *focusPoints);

    mat *dataPoints() const;
    void setDataPoints(mat *dataPoints);

public:
    virtual vec distance(int focus) override;

protected:
    bool mGeographic = false;
    mat* mFocusPoints = nullptr;
    mat* mDataPoints = nullptr;
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

inline mat *GwmCRSDistance::focusPoints() const
{
    return mFocusPoints;
}

inline void GwmCRSDistance::setFocusPoints(mat *focusPoints)
{
    mFocusPoints = focusPoints;
}

inline mat *GwmCRSDistance::dataPoints() const
{
    return mDataPoints;
}

inline void GwmCRSDistance::setDataPoints(mat *dataPoints)
{
    mDataPoints = dataPoints;
}

#endif // GWMCRSDISTANCE_H
