#include "gwmcrsdistance.h"

#define POWDI(x,i) pow(x,i)
#define DOUBLE_EPS (1e-8)

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

vec GwmCRSDistance::SpatialDistance(rowvec in_loc, mat out_locs)
{
    int N = out_locs.n_rows, j;
    vec dists(N, fill::zeros);
    double uout = in_loc(0), vout = in_loc(1);

    for (j = 0; j < N; j++) {
      dists(j) = spGcdist(out_locs(j, 0), uout, out_locs(j, 1), vout);
    }
    return dists;
}

vec GwmCRSDistance::EuclideanDistance(rowvec in_loc, mat out_locs)
{
    int n_in = in_loc.n_rows;
    int n_out = out_locs.n_rows;
    mat eu_dist(n_in, n_out);
    int i = 0, j = 0;
    for (i = 0; i < n_in; i++)
    {
        for (j = 0; j < n_out; j++)
        {
          eu_dist(i,j) = sum(pow(in_loc.row(i) - out_locs.row(j),2));
        }
    }
    return sqrt(eu_dist);
}

GwmCRSDistance::GwmCRSDistance() : GwmDistance()
{

}

bool GwmCRSDistance::geographic() const
{
    return mGeographic;
}

void GwmCRSDistance::setGeographic(bool geographic)
{
    mGeographic = geographic;
}

vec GwmCRSDistance::distance(rowvec target, mat dataPoints)
{
    return mGeographic ?
                SpatialDistance(target, dataPoints) :
                EuclideanDistance(target, dataPoints);
}
