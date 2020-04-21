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

mat coordinateRotate(const mat& coords, double theta);
mat euDistMat(const mat& in_locs, const mat& out_locs);
mat euDistSmat(const mat& in_locs);
vec euDistVec(const mat& in_locs, const vec& out_loc);
mat mdDistMat(const mat& in_locs, const mat& out_locs);
mat mdDistSmat(const mat& in_locs);
vec mdDistVec(const mat& in_locs, const vec& out_loc);
mat cdDistMat(const mat& in_locs, const mat& out_locs);
mat cdDistSmat(const mat& in_locs);
vec cdDistVec(const mat& in_locs, const vec& out_loc);
mat mkDistMat(const mat& in_locs, const mat& out_locs, double p);
mat mkDistSmat(const mat& in_locs, double p);
vec mkDistVec(const mat& in_locs, const vec& out_loc, double p);
vec bisqWtVec(const vec& distv, double bw);
mat bisqWtMat(const mat& distm, const vec& bw);
vec gaussWtVec(const vec& distv, double bw);
mat gaussWtMat(const mat& distm, const vec& bw);
vec triWtVec(const vec& distv, double bw);
mat triWtMatt(const mat& distm, const vec& bw);
vec expWtVec(const vec& distv, double bw);
mat expWtMat(const mat& distm, const vec& bw);
vec gwReg(const mat &x, const vec &y, const vec &w, int focus);
vec gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri);
double gwCV(const mat &x, const vec &y, vec &w, int focus);
vec fitted(const mat& X, const mat& beta);
vec ehat(const vec& y, const mat& X, const mat& beta);
double rss(const vec& y, const mat& X, const mat& beta);
vec gwrDiag(const vec& y, const mat& x, const mat& beta, const vec& s_hat);
double AICc(const vec& y, const mat& x, const mat& beta, const vec& s_hat);
vec AICcRss(const vec& y, const mat& x, const mat& beta, const vec& s_hat);
mat CiMat(const mat& x, const vec& w);
double spGcdist(double lon1, double lon2, double lat1, double lat2);
vec spDists(const mat& dp, const vec& loc);
mat gwDist(const mat& dp, const mat& rp, int focus, double p, double theta, bool longlat, bool rp_given);
vec gwWeight(const vec& dist, double bw, int kernel, bool adaptive);
vec gwLocalR2(const mat& dp, const vec& dybar2, const vec& dyhat2, bool dm_given, const mat& dmat, double p, double theta, bool longlat, double bw, int kernel, bool adaptive);

#endif // GWMODEL_H
