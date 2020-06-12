#include "gwmminkwoskidistance.h"

mat GwmMinkwoskiDistance::CoordinateRotate(mat coords, double theta)
{
    int n = coords.n_rows;
    mat rotated_coords(n, 2);
    rotated_coords.col(0) = coords.col(0) * cos(theta) - coords.col(1) * sin(theta);
    rotated_coords.col(1) = coords.col(0) * sin(theta) + coords.col(1) * cos(theta);
    return rotated_coords;
}

vec GwmMinkwoskiDistance::ChessDistance(rowvec out_loc, mat in_locs)
{
    int n_in = in_locs.n_rows;
    vec cd_dist(n_in);
    for (int i = 0; i < n_in; i++)
    {
        cd_dist(i) = max(abs(in_locs.row(i) - trans(out_loc)));
    }
    return cd_dist;
}

vec GwmMinkwoskiDistance::ManhattonDistance(rowvec out_loc, mat in_locs)
{
    int n_in = in_locs.n_rows;
    vec md_dist(n_in);
    for (int i = 0; i < n_in; i++)
    {
        md_dist(i) = sum(abs(in_locs.row(i) - trans(out_loc)));
    }
    return md_dist;
}

vec GwmMinkwoskiDistance::MinkwoskiDistance(rowvec out_loc, mat in_locs, double p)
{
    int n_in = in_locs.n_rows;
    vec mk_dist(n_in);
    for (int i = 0; i < n_in; i++)
    {
        mk_dist(i) = pow(sum(pow(abs(in_locs.row(i) - trans(out_loc)), p)), 1.0 / p);
    }
    return mk_dist;
}

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
    else return MinkwoskiDistance(target, dataPoints, mPoly);
}
