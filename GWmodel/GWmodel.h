#ifndef GWMODEL_H
#define GWMODEL_H

#include <armadillo>
#include <QMap>
using namespace arma;

#define POWDI(x,i) pow(x,i)

#define GAUSSIAN 0
#define EXPONENTIAL 1
#define BISQUARE 2
#define TRICUBE 3
#define BOXCAR 4

enum RegressionResult
{
    Beta,
    S_ri,
    Ci
};

const mat mSum(2, 1, fill::ones);

mat coordinateRotate(mat coords, double theta);
mat euDistMat(mat in_locs, mat out_locs);
mat euDistSmat(mat in_locs);
vec euDistVec(mat in_locs, vec out_loc);
mat mdDistMat(mat in_locs, mat out_locs);
mat mdDistSmat(mat in_locs);
vec mdDistVec(mat in_locs, vec out_loc);
mat cdDistMat(mat in_locs, mat out_locs);
mat cdDistSmat(mat in_locs);
vec cdDistVec(mat in_locs, vec out_loc);
mat mkDistMat(mat in_locs, mat out_locs, double p);
mat mkDistSmat(mat in_locs, double p);
vec mkDistVec(mat in_locs, vec out_loc, double p);
vec bisqWtVec(vec distv, double bw);
mat bisqWtMat(mat distm, vec bw);
vec gaussWtVec(vec distv, double bw);
mat gaussWtMat(mat distm, vec bw);
vec triWtVec(vec distv, double bw);
mat triWtMatt(mat distm, vec bw);
vec expWtVec(vec distv, double bw);
mat expWtMat(mat distm, vec bw);
QMap<RegressionResult, mat> gwReg(const mat &x, const vec &y, const vec &w, bool hatmatrix, int focus);
vec trhat2(mat S);
vec fitted(mat X, mat beta);
vec ehat(vec y, mat X, mat beta);
double rss(vec y, mat X, mat beta);
vec gwrDiag(vec y, mat x, mat beta, vec s_hat);
double AICc(vec y, mat x, mat beta, vec s_hat);
vec AICcRss(vec y, mat x, mat beta, vec s_hat);
mat CiMat(mat x, vec w);
double spGcdist(double lon1, double lon2, double lat1, double lat2);
vec spDists(mat dp, vec loc);
mat gwDist(mat dp, mat rp, int focus, double p, double theta, bool longlat, bool rp_given);
mat gwWeight(mat dist, double bw, int kernel, bool adaptive);
vec gwLocalR2(mat dp, vec dybar2, vec dyhat2, bool dm_given, mat dmat, double p, double theta, bool longlat, double bw, int kernel, bool adaptive);

#endif // GWMODEL_H
