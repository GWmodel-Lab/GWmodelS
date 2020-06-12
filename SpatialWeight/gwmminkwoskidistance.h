#ifndef GWMMINKWOSKIDISTANCE_H
#define GWMMINKWOSKIDISTANCE_H

#include "SpatialWeight/gwmdistance.h"

class GwmMinkwoskiDistance : public GwmDistance
{
public:
    static mat CoordinateRotate(mat coords, double theta);
    static vec ChessDistance(rowvec out_loc, mat in_locs);
    static vec ManhattonDistance(rowvec out_loc, mat in_locs);
    static vec MinkwoskiDistance(rowvec out_loc, mat in_locs, double p);

public:
    GwmMinkwoskiDistance();
    GwmMinkwoskiDistance(double p, double theta);
    GwmMinkwoskiDistance(const GwmMinkwoskiDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmMinkwoskiDistance(*this);
    }

    double poly() const;
    void setPoly(double poly);

    double theta() const;
    void setTheta(double theta);

public:
    virtual vec distance(rowvec target, mat dataPoints) override;

private:
    double mPoly;
    double mTheta;
};

#endif // GWMMINKWOSKIDISTANCE_H
