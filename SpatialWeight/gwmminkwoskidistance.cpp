#include "gwmminkwoskidistance.h"

GwmMinkwoskiDistance::GwmMinkwoskiDistance(int total, double p, double theta) : GwmCRSDistance(total, false)
{
    mPoly = p;
    mTheta = theta;
}

GwmMinkwoskiDistance::GwmMinkwoskiDistance(const GwmMinkwoskiDistance &distance) : GwmCRSDistance(distance)
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

vec GwmMinkwoskiDistance::distance(int focus)
{
    if (focus < mTotal)
    {
        if (mGeographic) return GwmCRSDistance::SpatialDistance(mFocusPoints->row(focus), *mDataPoints);
        if (mDataPoints && mFocusPoints)
        {
            mat dp(*mDataPoints), rp = mFocusPoints->row(focus);
            if (mPoly != 2 && mTheta != 0)
            {
                dp = CoordinateRotate(*mDataPoints, mTheta);
                rp = CoordinateRotate(mFocusPoints->row(focus), mTheta);
            }
            if (mPoly == 1.0) return ChessDistance(mFocusPoints->row(focus), *mDataPoints);
            else if (mPoly == -1.0) return ManhattonDistance(mFocusPoints->row(focus), *mDataPoints);
            else return MinkwoskiDistance(mFocusPoints->row(focus), *mDataPoints, mPoly);
        }
    }
    else return vec(mTotal, fill::zeros);
}
