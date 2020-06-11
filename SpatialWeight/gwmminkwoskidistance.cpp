#include "gwmminkwoskidistance.h"

GwmMinkwoskiDistance::GwmMinkwoskiDistance() : GwmDistance()
{

}

GwmMinkwoskiDistance::GwmMinkwoskiDistance(double p, double theta)
{
    mPoly = p;
    mTheta = theta;
}

double GwmMinkwoskiDistance::poly() const
{
    return mPoly;
}

void GwmMinkwoskiDistance::setPoly(double poly)
{
    mPoly = poly;
}

double GwmMinkwoskiDistance::theta() const
{
    return mTheta;
}

void GwmMinkwoskiDistance::setTheta(double theta)
{
    mTheta = theta;
}

vec GwmMinkwoskiDistance::distance(rowvec target, mat dataPoints)
{
    mat dp(dataPoints), rp = target;
    if (mPoly != 2 && mTheta != 0)
    {
        dp = CoordinateRotate(dataPoints, mTheta);
        rp = CoordinateRotate(target, mTheta);
    }
    if (mPoly == 1.0) return ChessDistance(target, dataPoints);
    else if (mPoly == -1.0) return ManhattonDistance(target, dataPoints);
    else return MinkwoskiDistance(target, dataPoints);
}
