#include "gwmminkwoskidistance.h"

GwmMinkwoskiDistance::GwmMinkwoskiDistance() : GwmDistance()
{

}

GwmMinkwoskiDistance::GwmMinkwoskiDistance(double p, double theta)
{
    mPoly = p;
    mTheta = theta;
}

GwmMinkwoskiDistance::GwmMinkwoskiDistance(const GwmMinkwoskiDistance &distance)
{

    mPoly = distance.mPoly;
    mTheta = distance.mTheta;
}

mat GwmMinkwoskiDistance::CoordinateRotate(const mat& coords, double theta)
{
    int n = coords.n_rows;
    mat rotated_coords(n, 2);
    rotated_coords.col(0) = coords.col(0) * cos(theta) - coords.col(1) * sin(theta);
    rotated_coords.col(1) = coords.col(0) * sin(theta) + coords.col(1) * cos(theta);
    return rotated_coords;
}

vec GwmMinkwoskiDistance::distance(const rowvec& target, const mat& dataPoints)
{
    mat dp(dataPoints), rp = target;
    if (mPoly != 2 && mTheta != 0)
    {
        dp = CoordinateRotate(dataPoints, mTheta);
        rp = CoordinateRotate(target, mTheta);
    }
    if (mPoly == 1.0) return ChessDistance(target, dataPoints);
    else if (mPoly == -1.0) return ManhattonDistance(target, dataPoints);
    else return MinkwoskiDistance(target, dataPoints, mPoly);
}
