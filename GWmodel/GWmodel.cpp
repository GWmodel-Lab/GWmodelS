#include "prefix.h"
#include "GWmodel.h"
#include <math.h>
#include <omp.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
using namespace arma;

//distance matrix calculation
//coords must be a matrix with 2 columns
mat coordinateRotate(const mat& coords, double theta)
{
	int n = coords.n_rows;
	mat rotated_coords(n, 2);
	rotated_coords.col(0) = coords.col(0) * cos(theta) - coords.col(1) * sin(theta);
	rotated_coords.col(1) = coords.col(0) * sin(theta) + coords.col(1) * cos(theta);
	return rotated_coords;
}

//Eudclidean distance matrix
mat euDistMat(const mat& in_locs, const mat& out_locs)
{
	int n_in = in_locs.n_rows;
	int n_out = out_locs.n_rows;
	mat eu_dist(n_in, n_out);
	int i = 0, j = 0;
	for (i = 0; i < n_in; i++)
	{
		for (j = 0; j < n_out; j++)
		{
		  eu_dist(i,j) = sum(pow(in_locs.row(i) - out_locs.row(j),2));
		}
	}
	return sqrt(eu_dist);
}
//symmetrical distance matrix
mat euDistSmat(const mat& in_locs)
{
	int n = in_locs.n_rows;
	mat eu_dist(n, n);
	for (int k = 0; k < n * n; k++)
	{
		int i = k / n, j = k % n;
		eu_dist(i, j) = sum(pow(in_locs.row(i) - in_locs.row(j), 2));
		eu_dist(j, i) = eu_dist(i, j);
	}
	return sqrt(eu_dist);
}

vec euDistVec(const mat& in_locs, const vec& out_loc)
{
	int n_in = in_locs.n_rows;
	vec eu_dist(n_in);
	for (int i = 0; i < n_in; i++)
	{
		eu_dist(i) = sum(pow(in_locs.row(i) - trans(out_loc), 2));
	}
	return sqrt(eu_dist);
	// mat v_span(n_in, 1, fill::ones);
	// mat m_diff = in_locs - v_span * trans(out_loc);
	// return sqrt(m_diff % m_diff * mSum);
}

//Manhattan distance matrix
mat mdDistMat(const mat& in_locs, const mat& out_locs)
{
	int n_in = in_locs.n_rows;
	int n_out = out_locs.n_rows;
	mat md_dist(n_in, n_out);
	for (int i = 0; i < n_in; i++)
	{
		for (int j = 0; j < n_out; j++)
		{
			md_dist(i, j) = sum(abs(in_locs.row(i) - out_locs.row(j)));
		}
	}
	return md_dist;
}

//symmetrical distance matrix
mat mdDistSmat(const mat& in_locs)
{
	int n = in_locs.n_rows;
	mat md_dist(n, n);
	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			md_dist(i, j) = sum(abs(in_locs.row(i) - in_locs.row(j)));
			md_dist(j, i) = md_dist(i, j);
		}
	}
	return md_dist;
}
vec mdDistVec(const mat& in_locs, const vec& out_loc)
{
	int n_in = in_locs.n_rows;
	vec md_dist(n_in);
	for (int i = 0; i < n_in; i++)
	{
		md_dist(i) = sum(abs(in_locs.row(i) - trans(out_loc)));
	}
	return md_dist;
}

//Chebyshev distance matrix
mat cdDistMat(const mat& in_locs, const mat& out_locs)
{
	int n_in = in_locs.n_rows;
	int n_out = out_locs.n_rows;
	mat cd_dist(n_in, n_out);
	for (int i = 0; i < n_in; i++)
	{
		for (int j = i; j < n_out; j++)
		{
			cd_dist(i, j) = max(abs(in_locs.row(i) - out_locs.row(j)));
			cd_dist(j, i) = cd_dist(i, j);
		}
	}
	return cd_dist;
}

//symmetrical distance matrix
mat cdDistSmat(const mat& in_locs)
{
	int n = in_locs.n_rows;
	mat cd_dist(n, n);
	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			cd_dist(i, j) = max(abs(in_locs.row(i) - in_locs.row(j)));
			cd_dist(j, i) = cd_dist(i, j);
		}
	}
	return cd_dist;
}
vec cdDistVec(const mat& in_locs, const vec& out_loc)
{
	int n_in = in_locs.n_rows;
	vec cd_dist(n_in);
	for (int i = 0; i < n_in; i++)
	{
		cd_dist(i) = max(abs(in_locs.row(i) - trans(out_loc)));
	}
	return cd_dist;
}

//Minkowski distance matrix
mat mkDistMat(const mat& in_locs, const mat& out_locs, double p)
{
	int n_in = in_locs.n_rows;
	int n_out = out_locs.n_rows;
	mat mk_dist(n_in, n_out);
	for (int i = 0; i < n_in; i++)
	{
		for (int j = 0; j < n_out; j++)
		{
			mk_dist(i, j) = pow(sum(pow(abs(in_locs.row(i) - out_locs.row(j)), p)), 1.0 / p);
		}
	}
	return mk_dist;
}
//sqrt(sum(pow(in_locs.row(i) - trans(out_loc),2)))
//symmetrical distance matrix
mat mkDistSmat(const mat& in_locs, double p)
{
	int n = in_locs.n_rows;
	mat mk_dist(n, n);
	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			mk_dist(i, j) = pow(sum(pow(abs(in_locs.row(i) - in_locs.row(j)), p)), 1.0 / p);
			mk_dist(j, i) = mk_dist(i, j);
		}
	}
	return mk_dist;
}

vec mkDistVec(const mat& in_locs, const vec& out_loc, double p)
{
	int n_in = in_locs.n_rows;
	vec mk_dist(n_in);
	for (int i = 0; i < n_in; i++)
	{
		mk_dist(i) = pow(sum(pow(abs(in_locs.row(i) - trans(out_loc)), p)), 1.0 / p);
	}
	return mk_dist;
}
//Weight matrix
//Bisuqare weight
vec bisqWtVec(const vec& distv, double bw)
{
	int n = distv.n_elem;
	vec wtv;
	wtv.zeros(n);
// #pragma omp parallel for num_threads(omp_get_num_procs() - 1)
	for (int i = 0; i < n; i++)
	{
		if (distv(i) <= bw)
			wtv(i) = pow(1 - pow(distv(i) / bw, 2), 2);
	}
	return wtv;
}
//Calculated by column, the length of bw must be the same the number of columns of distm
mat bisqWtMat(const mat& distm, const vec& bw)
{
	int m = distm.n_cols;
	int n = distm.n_rows;
	mat wtm;
	wtm.zeros(n, m);
// #pragma omp parallel for num_threads(omp_get_num_procs() - 1)
	for (int k = 0; k < m * n; k++)
	{
		int i = k / n, j = k % n;
		if (distm(j, i) <= bw(i))
			wtm(j, i) = (1 - distm(j, i) / bw(i) * distm(j, i) / bw(i)) * (1 - distm(j, i) / bw(i) * distm(j, i) / bw(i));
	}
	return wtm;
}

//Gaussian weight
vec gaussWtVec(const vec& distv, double bw)
{
	int n = distv.n_elem;
	vec wtv;
	wtv.zeros(n);
// #pragma omp parallel for num_threads(omp_get_num_procs() - 1)
	for (int i = 0; i < n; i++)
	{
		wtv(i) = exp(pow(distv(i), 2) / ((-2) * pow(bw, 2)));
	}
	return wtv;
}
mat gaussWtMat(const mat& distm, const vec& bw)
{
	int m = distm.n_cols;
	int n = distm.n_rows;
	mat wtm;
	wtm.zeros(n, m);
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			wtm(j, i) = exp(pow(distm(j, i), 2) / ((-2) * pow(bw(i), 2)));
		}
	}
	return wtm;
}

//Tricube weight
vec triWtVec(const vec& distv, double bw)
{
	int n = distv.n_elem;
	vec wtv;
	wtv.zeros(n);
	for (int i = 0; i < n; i++)
	{
		if (distv(i) <= bw)
			wtv(i) = pow(1 - pow(distv(i), 3) / pow(bw, 3), 3);
	}
	return wtv;
}
mat triWtMatt(const mat& distm, const vec& bw)
{
	int m = distm.n_cols;
	int n = distm.n_rows;
	mat wtm;
	wtm.zeros(n, m);
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (distm(j, i) <= bw(i))
				wtm(j, i) = pow(1 - pow(distm(j, i), 3) / pow(bw(i), 3), 3);
		}
	}
	return wtm;
}
//exponential kernel weight
vec expWtVec(const vec& distv, double bw)
{
	int n = distv.n_elem;
	vec wtv;
	wtv.zeros(n);
	for (int i = 0; i < n; i++)
	{
		wtv(i) = exp(-distv(i) / bw);
	}
	return wtv;
}
mat expWtMat(const mat& distm, const vec& bw)
{
	int m = distm.n_cols;
	int n = distm.n_rows;
	mat wtm;
	wtm.zeros(n, m);
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			wtm(j, i) = exp(-distm(j, i) / bw(i));
		}
	}
	return wtm;
}
//GWR clalibration
vec gwReg(const mat& x, const vec &y, const vec &w, int focus)
{
    QMap<RegressionResult, mat> result;
	mat wspan(1, x.n_cols, fill::ones);
	mat xtw = trans(x % (w * wspan));
	mat xtwx = xtw * x;
    mat xtwy = xtw * y;
	mat xtwx_inv = inv(xtwx);
	vec beta = xtwx_inv * xtwy;
    return beta;
}

vec gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri)
{
    QMap<RegressionResult, mat> result;
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    ci = xtwx_inv * xtw;
    s_ri = x.row(focus) * ci;
    return beta;
}
// Trace of hat matrix + trace of HH' in one function. Used in beta_se

vec trhat2(const mat& S)
{
	int n_obs = S.n_rows;
	double htr = 0.0;
	double htr2 = 0.0;
	vec result(2);
	for (int i = 0; i < n_obs; i++)
	{
		htr += S(i, i);
		htr2 += sum(S.row(i) % S.row(i));
	}
	result(0) = htr;
	result(1) = htr2;
	return result;
}
// Fited values

vec fitted(const mat& X, const mat& beta)
{
	vec fitted = sum(beta % X, 1);
	return fitted;
}

// Residuals

vec ehat(const vec& y, const mat& X, const mat& beta)
{
	vec fitted = sum(beta % X, 1);
	return y - fitted;
}

// Residual sum-of squares
double rss(const vec& y, const mat& X, const mat& beta)
{
	vec r = ehat(y, X, beta);
	return sum(r % r);
}

vec gwrDiag(const vec& y, const mat& x, const mat& beta, const vec& s_hat)
{
	double ss = rss(y, x, beta);
	// vec s_hat = trhat2(S);
	int n = x.n_rows;
	// vec result(9);
	vec result(7);
	double AIC = n * log(ss / n) + n * log(2 * datum::pi) + n + s_hat(0);																//AIC
	double AICc = n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + s_hat(0)) / (n - 2 - s_hat(0))); //AICc
	double edf = n - 2 * s_hat(0) + s_hat(1);																														//edf
	double enp = 2 * s_hat(0) - s_hat(1);																																// enp
    double yss = sum((y - mean(y)) % (y - mean(y)));																															//yss.g
	double r2 = 1 - ss / yss;
	double r2_adj = 1 - (1 - r2) * (n - 1) / (edf - 1);
	result(0) = AIC;
	result(1) = AICc;
	result(2) = edf;
	result(3) = enp;
	result(4) = ss;
	result(5) = r2;
	result(6) = r2_adj;
	// result(7) = s_hat(0);
	// result(8) = s_hat(1);
	return result;
	//return 2*enp + 2*n*log(ss/n) + 2*enp*(enp+1)/(n - enp - 1);
}

// return the AICc - nice tool to give to 'optimise' to find 'best' bandwidth
double AICc(const vec& y, const mat& x, const mat& beta, const vec& s_hat)
{
  double ss = rss(y, x, beta);
  // vec s_hat = trhat2(S);
  int n = x.n_rows;
  double AICc = n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + s_hat(0)) / (n - 2 - s_hat(0))); //AICc
  return AICc;
  //return 2*enp + 2*n*log(ss/n) + 2*enp*(enp+1)/(n - enp - 1);
}

// return the AICc and RSS , used for function model.selection
vec AICcRss(const vec& y, const mat& x, const mat& beta, const vec& s_hat)
{
  vec result(3);
  double ss = rss(y, x, beta);
  result[0] = ss;
  int n = x.n_rows;
  double AIC = n * log(ss / n) + n * log(2 * datum::pi) + n + s_hat(0);
  double AICc = n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + s_hat(0)) / (n - 2 - s_hat(0))); //AICc
  result[1] = AIC;
  result[2] = AICc;
  return result;
  //return 2*enp + 2*n*log(ss/n) + 2*enp*(enp+1)/(n - enp - 1);
}

//Caculate the i row of
mat CiMat(const mat& x, const vec &w)
{
	return inv(trans(x) * diagmat(w) * x) * trans(x) * diagmat(w);
}

double spGcdist(double lon1, double lon2, double lat1, double lat2) {
  
  double F, G, L, sinG2, cosG2, sinF2, cosF2, sinL2, cosL2, S, C;
  double w, R, a, f, D, H1, H2;
  double lat1R, lat2R, lon1R, lon2R, DE2RA;
  
  DE2RA = M_PI/180;
  a = 6378.137;              /* WGS-84 equatorial radius in km */
    f = 1.0/298.257223563;     /* WGS-84 ellipsoid flattening factor */
    
    if (fabs(lat1 - lat2) < DOUBLE_EPS) {
      if (fabs(lon1 - lon2) < DOUBLE_EPS) {
        return 0.0;
        /* Wouter Buytaert bug caught 100211 */
      } else if (fabs((fabs(lon1) + fabs(lon2)) - 360.0) < DOUBLE_EPS) {
        return 0.0;
      }
    }
    lat1R = lat1*DE2RA;
    lat2R = lat2*DE2RA;
    lon1R = lon1*DE2RA;
    lon2R = lon2*DE2RA;
    
    F = ( lat1R + lat2R )/2.0;
    G = ( lat1R - lat2R )/2.0;
    L = ( lon1R - lon2R )/2.0;
    
    /*
    printf("%g %g %g %g; %g %g %g\n",  *lon1, *lon2, *lat1, *lat2, F, G, L);
    */
    
    sinG2 = POWDI( sin( G ), 2 );
    cosG2 = POWDI( cos( G ), 2 );
    sinF2 = POWDI( sin( F ), 2 );
    cosF2 = POWDI( cos( F ), 2 );
    sinL2 = POWDI( sin( L ), 2 );
    cosL2 = POWDI( cos( L ), 2 );
    
    S = sinG2*cosL2 + cosF2*sinL2;
    C = cosG2*cosL2 + sinF2*sinL2;
    
    w = atan( sqrt( S/C ) );
    R = sqrt( S*C )/w;
    
    D = 2*w*a;
    H1 = ( 3*R - 1 )/( 2*C );
    H2 = ( 3*R + 1 )/( 2*S );
    
    return D*( 1 + f*H1*sinF2*cosG2 - f*H2*cosF2*sinG2 ); 
}

vec spDists(const mat& dp, const vec& loc) {
  int N = dp.n_rows, j;
  vec dists(N, fill::zeros);
  double uout = loc(0), vout = loc(1);
  
  for (j = 0; j < N; j++) {
    dists(j) = spGcdist(dp(j, 0), uout, dp(j, 1), vout);
  }
  return dists;
}

mat gwDist(const mat& dp0, const mat& rp0, int focus, double p, double theta, bool longlat, bool rp_given) {
  int ndp = dp0.n_rows, nrp = rp0.n_rows;
  int isFocus = focus > -1;
  mat dists;
  mat dp(dp0), rp(rp0);
  if (p != 2 && theta != 0 && !longlat) {
    dp = coordinateRotate(dp0, theta);
    rp = coordinateRotate(rp0, theta);
  }
  if (isFocus) {
    mat prp = trans(rp.row(focus));
    if (longlat) {
      return spDists(dp, prp);
    } else {
      if (p == 2.0)
        return euDistVec(dp, prp);
      else if(p == 1.0)
        return cdDistVec(dp, prp);
      else if(p == -1.0)
        return mdDistVec(dp, prp);
      else
        return mkDistVec(dp, prp, p);
    }
  } else {
    if (longlat) {
      mat dists(ndp, nrp, fill::zeros);
      for (int i = 0; i < nrp; i++) {
        dists.col(i) = spDists(dp, trans(rp.row(i)));
      }
      return trans(dists);
    } else {
      if (p == 2.0)
        return rp_given ? euDistMat(dp, rp) : euDistSmat(dp);
      else if (p == 1.0)
        return rp_given ? cdDistMat(dp, rp) : cdDistSmat(dp);
      else if (p == -1.0)
        return rp_given ? mdDistMat(dp, rp) : mdDistSmat(dp);
      else
        return rp_given ? mkDistMat(dp, rp, p) : mkDistSmat(dp, p);
    }
  }
}

double gwWeightGaussian(double dist, double bw) {
  return exp(pow(dist, 2)/((-2)*pow(bw, 2)));
}

double gwWeightExponential(double dist, double bw) {
  return exp(-dist/bw);
}

double gwWeightBisquare(double dist, double bw) {
  return dist > bw ? 0 : pow(1 - pow(dist, 2)/pow(bw, 2), 2);
}

double gwWeightTricube(double dist, double bw) {
  return dist > bw ? 0 : pow(1 - pow(dist, 3)/pow(bw, 3), 3);
}

double gwWeightBoxcar(double dist, double bw) {
  return dist > bw ? 0 : 1;
}

typedef double (*KERNEL)(double, double);
const KERNEL GWRKernel[5] = {
  gwWeightGaussian,
  gwWeightExponential,
  gwWeightBisquare,
  gwWeightTricube,
  gwWeightBoxcar
};

vec gwWeight(const vec& dist, double bw, int kernel, bool adaptive) {
  const KERNEL *kerf = GWRKernel + kernel;
  int nr = dist.n_elem;
  vec w(nr, fill::zeros);
  if (adaptive) {
    double dn = bw / nr, fixbw = 0;
    if (dn <= 1) {
      vec vdist = sort(dist);
      fixbw = vdist(int(bw) - 1);
    } else {
      fixbw = dn * max(dist);
    }
    for (int r = 0; r < nr; r++) {
      w(r) = (*kerf)(dist(r), fixbw);
    }
  } else {
    for (int r = 0; r < nr; r++) {
      w(r) = (*kerf)(dist(r), bw);
    }
  }
  return w;
}


vec gwLocalR2(const mat& dp, const vec& dybar2, const vec& dyhat2, bool dm_given, const mat& dmat, double p, double theta, bool longlat, double bw, int kernel, bool adaptive) {
  int n = dp.n_rows;
  vec localR2(n, fill::zeros);
  for (int i = 0; i < n; i++) {
    mat d = dm_given ? dmat.col(i) : gwDist(dp, dp, i, p, theta, longlat, false);
    mat w = gwWeight(d, bw, kernel, adaptive);
    double tss = sum(dybar2 % w);
    double rss = sum(dyhat2 % w);
    localR2(i) = (tss - rss) / tss;
  }
  return localR2;
}

/*zhangtongyao*/
// 给出回归点，计算在给定带宽情况下的CV值
double gwCV(const mat &x, const vec &y, vec &w, int focus)
{
    QMap<RegressionResult, mat> result;
    mat wspan(1, x.n_cols, fill::ones);
    w(focus) = 0.0;
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    return y(focus) - det(x.row(focus) * beta);
}

void scgwr_pre(const mat& x, const vec& y, int bw, int poly, double b0, const mat& g0, const umat& neighbour, mat& Mx0, mat& My0) {
  int n = x.n_rows;
  int k = x.n_cols;
  mat g0s(g0.n_cols + 1, g0.n_rows, fill::ones);
  mat g0t = trans(g0);
  for (int i = 0; i < bw; i++) {
    g0s.row(i + 1) = g0t.row(i);
  }
  g0s = trans(g0s);
  Mx0 = mat((poly + 1)*k*k, n, fill::zeros);
  My0 = mat((poly + 1)*k, n, fill::zeros);
  mat spanXnei(1, poly + 1, fill::ones);
  mat spanXtG(1, k, fill::ones);
  for (int i = 0; i < n; i++) {
    mat g(poly + 1, bw + 1, fill::ones);
    for (int p = 0; p < poly; p++) {
      g.row(p + 1) = pow(g0s.row(i), pow(2.0, poly/2.0)/pow(2.0, p + 1));
    }
    g = trans(g);
    g = g.rows(1, bw);
    mat xnei(bw, k, fill::zeros);
    vec ynei(bw, fill::zeros);
    for (int j = 0; j < bw; j++) {
      int inei = int(neighbour(i, j));
      xnei.row(j) = x.row(inei);
      ynei.row(j) = y(inei);
    }
    for (int k1 = 0; k1 < k; k1++) {
      mat XtG = xnei.col(k1) * spanXnei % g;
      for (int p = 0; p < (poly + 1); p++) {
        mat XtGX = XtG.col(p) * spanXtG % xnei;
        for (int k2 = 0; k2 < k; k2++) {
          int xindex = (k1 * (poly + 1) + p) * k + k2;
          Mx0(xindex, i) = sum(XtGX.col(k2));
        }
        int yindex = p * k + k1;
        vec XtGY = XtG.col(p) % ynei;
        My0(yindex, i) = sum(XtGY);
      }
    }
  }
}


double scgwr_loocv(const vec& target, const mat& x, const vec& y, int bw, int poly, const mat& Mx0, const mat& My0) {
  int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
  double b = target(0) * target(0), a = target(1) * target(1);
  vec R0 = vec(poly1) * b;
  for (int p = 1; p < poly1; p++) {
    R0(p) = pow(b, p + 1);
  }
  vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
  for (int p = 0; p < poly1; p++) {
    for (int k2 = 0; k2 < k; k2++) {
      for (int k1 = 0; k1 < k; k1++) {
        int xindex = k1*poly1*k + p*k + k2;
        Rx(xindex) = R0(p);
      }
      int yindex = p*k + k2;
      Ry(yindex) = R0(p);
    }
  }
  mat Mx = Rx * mat(1, n, fill::ones) % Mx0, My = Ry * mat(1, n, fill::ones) % My0;
  vec yhat(n, 1, fill::zeros);
  for (int i = 0; i < n; i++) {
    mat sumMx(k, k, fill::zeros);
    vec sumMy(k, fill::zeros);
    for (int k2 = 0; k2 < k; k2++) {
      for (int p = 0; p < poly1; p++) {
        for (int k1 = 0; k1 < k; k1++) {
          int xindex = k1*poly1*k + p*k + k2;
          sumMx(k1, k2) += Mx(xindex, i);
        }
        int yindex = p*k + k2;
        sumMy(k2) += My(yindex, i);
      }
    }
    sumMx = sumMx + a * (x.t() * x);
    sumMy = sumMy + a * (x.t() * y);
    if (det(sumMx) < 1e-10) {
      return 1e6;
    } else {
      mat beta = solve(sumMx, sumMy);
      yhat.row(i) = x.row(i) * beta;
    }
  }
  return sum((y - yhat) % (y - yhat));
}


bool scgwr_reg(const mat& x, const vec& y, int bw, int poly, const mat& G0, const umat& neighbour, const vec& parameters, mat& Mx0, mat& My0, mat& betas, vec& s_hat, mat& betasSE) {
  int n = x.n_rows, k = x.n_cols, poly1 = poly + 1;
  double b = parameters(0), a = parameters(1);
  mat XtX = x.t() * x, XtY = x.t() * y;
  /**
   * Calculate Rx, Ry, and R0.
   */
  // printf("Calculate Rx, Ry, and R0 ");
  vec R0 = vec(poly1, fill::ones) * b;
  for (int p = 1; p < poly1; p++) {
    R0(p) = pow(b, p + 1);
  }
  vec Rx(k*k*poly1, fill::zeros), Ry(k*poly1, fill::zeros);
  for (int p = 0; p < poly1; p++) {
    for (int k2 = 0; k2 < k; k2++) {
      for (int k1 = 0; k1 < k; k1++) {
        Rx(k1*poly1*k + p*k + k2) = R0(p);
      }
      Ry(p*k + k2) = R0(p);
    }
  }
  /**
  * Create G0.
  */
  // printf("Create G0 ");
  mat G0s(G0.n_cols + 1, G0.n_rows, fill::ones);
  mat G0t = trans(G0);
  G0s.rows(1, bw) = G0t.rows(0, bw - 1);
  G0s = trans(G0s);
  mat G2(n, bw + 1, fill::zeros);
  for (int p = 0; p < poly; p++) {
    G2 += pow(G0s, pow(2.0, poly/2.0)/pow(2.0, p + 1)) * R0(poly - 1);
  }
  /**
   * Calculate Mx, My.
   */
  // printf("Calculate Mx, My ");
  // mat Mx00(Mx0), My00(My0);
  for (int i = 0; i < n; i++) {
    for (int k1 = 0; k1 < k; k1++) {
      for (int p = 0; p < poly1; p++) {
        for (int k2 = 0; k2 < k; k2++) {
          Mx0((k1 * (poly + 1) + p) * k + k2, i) += x(i, k1) * x(i, k2);
        }
        My0(p * k + k1, i) += x(i, k1) * y(i);
      }
    }
  }
  mat Mx = (Rx * mat(1, n, fill::ones)) % Mx0, My = (Ry * mat(1, n, fill::ones)) % My0;
  /**
   * Regression.
   */
  // printf("Regression ");
  mat Xp(bw + 1, k * poly1, fill::zeros);
  mat rowsumG(poly1, 1, fill::ones);
  mat colsumXp(1, bw + 1, fill::zeros);
  mat spanG(1, k, fill::ones);
  mat spanX(1, poly1, fill::ones);
  betas = mat(n, k, fill::zeros);
  betasSE = mat(n, k, fill::zeros);
  double trS = 0.0, trStS = 0.0;
  bool isAllCorrect = true;
  for (int i = 0; i < n; i++) {
    /**
     * Calculate G.
     */
    mat G = mat(poly1, bw + 1, fill::ones) * R0(0);
    for (int p = 0; p < poly; p++) {
      G.row(p + 1) = pow(G0s.row(i), pow(2.0, poly/2.0)/pow(2.0, p + 1)) * R0(p);
    }
    G = trans(G);
    mat g = G * rowsumG;
    /**
     * Calculate Xp.
     */
    mat xnei(bw + 1, k, fill::zeros);
    vec ynei(bw + 1, fill::zeros);
    xnei.row(0) = x.row(i);
    ynei.row(0) = y.row(i);
    for (int j = 0; j < bw; j++) {
      int inei = int(neighbour(i, j));
      xnei.row(j+1) = x.row(inei);
      ynei.row(j+1) = y(inei);
    }
    /**
     * Calculate sumMx, sumMy.
     */
    mat sumMx(k, k, fill::zeros);
    vec sumMy(k, fill::zeros);
    for (int k2 = 0; k2 < k; k2++) {
      for (int p = 0; p < poly1; p++) {
        for (int k1 = 0; k1 < k; k1++) {
          int xindex = k1*poly1*k + p*k + k2;
          sumMx(k1, k2) += Mx(xindex, i);
        }
        int yindex = p*k + k2;
        sumMy(k2) += My(yindex, i);
      }
    }
    sumMx += a * XtX;
    sumMy += a * XtY;
    try {
        mat invMx = inv(sumMx);
        betas.row(i) = trans(invMx * sumMy);
        /**
         * Calculate Diagnostic statistics, trS and trStS.
         */
        mat StS = invMx * trans(x.row(i));
        trS += det((x.row(i) * g(0, 0)) * StS);
        mat XG2X(k, k, fill::zeros);
        for (int k1 = 0; k1 < k; k1++) {
          for (int k2 = 0; k2 < k; k2++) {
            mat Gi = G2.row(i);
            XG2X(k1, k2) = sum(xnei.col(k1) % trans(Gi % Gi + 2 * a * Gi) % xnei.col(k2)) + a * a * XtX(k1, k2);
          }
        }
        mat XX = invMx * XG2X * invMx;
        mat xi = x.row(i);
        trStS += det(sum(xi * XX * trans(xi)));
        betasSE.row(i) = trans(sqrt(XX.diag()));
    } catch (...) {
        isAllCorrect = false;
    }
  }
  s_hat = { trS, trStS };
  return isAllCorrect;
}

vec gwFitted(mat X, mat beta)
{
    vec fitted = sum(beta % X, 1);
    return fitted;
}

mat diag(mat a){
    int n = a.n_cols;
    mat res = mat(uword(0), uword(0));
    if(a.n_rows > 1){
        for(int i = 0; i < 0; i++){
            res[i] = a.col(i)[i];
        }
    }
    else{
        mat base = eye(n,n);
        for(int i = 0; i < 0; i++){
            res.col(i) = a[i] * base.col(i);
        }
    }
    return res;
}

mat dpois(mat y,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    mat pdf = lgammafn(y);
    res = -mu + y%log(mu) - pdf;
    return res;
}

mat lchoose(mat n,mat k){
    int nrow = n.n_rows;
    mat res = vec(nrow);
    for(int i = 0;i < nrow; i++){
        res.row(i) = lgamma(n[i]+1) - lgamma(n[i]-k[i]+1) - lgamma(k[i]+1);
    }
    return res;
}

mat dbinom(mat y,mat m,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    for(int i = 0;i < n; i++){
        double pdf = gsl_ran_binomial_pdf(int(y[i]), mu[i], int(m[i]));
        res[i] = log(pdf);
    }
    return res;
}

mat lgammafn(mat x){
    int n = x.n_rows;
    mat res = vec(n,fill::zeros);   
    for(int j = 0; j < n ; j++){
        res[j] = lgamma(x[j]);
    }
    return res;
}
