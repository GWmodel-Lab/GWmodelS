#ifndef GWMMINKWOSKIDISTANCE_H
#define GWMMINKWOSKIDISTANCE_H

#include "SpatialWeight/gwmcrsdistance.h"

class GwmMinkwoskiDistance : public GwmCRSDistance
{
public:
    static mat CoordinateRotate(const mat& coords, double theta);
    static vec ChessDistance(const rowvec& out_loc, const mat& in_locs);
    static vec ManhattonDistance(const rowvec& out_loc, const mat& in_locs);
    static vec MinkwoskiDistance(const rowvec& out_loc, const mat& in_locs, double p);

public:
    explicit GwmMinkwoskiDistance(int total);
    explicit GwmMinkwoskiDistance(int total, double p, double theta);
    GwmMinkwoskiDistance(const GwmMinkwoskiDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmMinkwoskiDistance(*this);
    }

    DistanceType type() override { return DistanceType::MinkwoskiDistance; }

    double poly() const;
    void setPoly(double poly);

    double theta() const;
    void setTheta(double theta);

public:
    virtual vec distance(int focus) override;

private:
    double mPoly;
    double mTheta;
};

inline vec GwmMinkwoskiDistance::ChessDistance(const rowvec& out_loc, const mat& in_locs)
{
    return max(abs(in_locs.each_row() - out_loc), 1);
}

inline vec GwmMinkwoskiDistance::ManhattonDistance(const rowvec& out_loc, const mat& in_locs)
{
    return sum(abs(in_locs.each_row() - out_loc), 1);
}

inline vec GwmMinkwoskiDistance::MinkwoskiDistance(const rowvec& out_loc, const mat& in_locs, double p)
{
    vec temp = abs(in_locs.each_row() - out_loc);
    return pow(sum(pow(temp, p), 1), 1.0 / p);
}

inline double GwmMinkwoskiDistance::poly() const
{
    return mPoly;
}

inline void GwmMinkwoskiDistance::setPoly(double poly)
{
    mPoly = poly;
}

inline double GwmMinkwoskiDistance::theta() const
{
    return mTheta;
}

inline void GwmMinkwoskiDistance::setTheta(double theta)
{
    mTheta = theta;
}

#endif // GWMMINKWOSKIDISTANCE_H
