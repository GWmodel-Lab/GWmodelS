#ifndef GWMMINKWOSKIDISTANCE_H
#define GWMMINKWOSKIDISTANCE_H

#include "SpatialWeight/gwmdistance.h"

class GwmMinkwoskiDistance : public GwmDistance
{
public:
    static mat CoordinateRotate(mat locs, double theta);
    static vec ChessDistance(rowvec in_loc, mat out_locs);
    static vec ManhattonDistance(rowvec in_loc, mat out_locs);
    static vec MinkwoskiDistance(rowvec in_loc, mat out_locs);

public:
    GwmMinkwoskiDistance();
    GwmMinkwoskiDistance(double p, double theta);

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
