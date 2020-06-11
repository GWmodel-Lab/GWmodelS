#include "gwmbandwidthweight.h"

GwmBandwidthWeight::KernelFunction GwmBandwidthWeight::Kernel[] =
{
    &GwmBandwidthWeight::GaussianKernelFunction,
    &GwmBandwidthWeight::ExponentialKernelFunction,
    &GwmBandwidthWeight::BisquareKernelFunction,
    &GwmBandwidthWeight::TricubeKernelFunction,
    &GwmBandwidthWeight::BoxcarKernelFunction
};

double GwmBandwidthWeight::GaussianKernelFunction(double dist, double bw) {
  return exp(pow(dist, 2)/((-2)*pow(bw, 2)));
}

double GwmBandwidthWeight::ExponentialKernelFunction(double dist, double bw) {
  return exp(-dist/bw);
}

double GwmBandwidthWeight::BisquareKernelFunction(double dist, double bw) {
  return dist > bw ? 0 : pow(1 - pow(dist, 2)/pow(bw, 2), 2);
}

double GwmBandwidthWeight::TricubeKernelFunction(double dist, double bw) {
  return dist > bw ? 0 : pow(1 - pow(dist, 3)/pow(bw, 3), 3);
}

double GwmBandwidthWeight::BoxcarKernelFunction(double dist, double bw) {
  return dist > bw ? 0 : 1;
}

GwmBandwidthWeight::GwmBandwidthWeight() : GwmWeight()
{

}

GwmBandwidthWeight::GwmBandwidthWeight(double size, bool adaptive, GwmBandwidthWeight::KernelFunctionType kernel)
{
    mBandwidth = size;
    mAdaptive = adaptive;
    mKernel = kernel;
}

vec GwmBandwidthWeight::weight(vec dist)
{
    const KernelFunction *kerf = Kernel + mKernel;
    int nr = dist.n_elem;
    vec w(nr, fill::zeros);
    if (mAdaptive) {
      double dn = mBandwidth / nr, fixbw = 0;
      if (dn <= 1) {
        vec vdist = sort(dist);
        fixbw = vdist(mBandwidth > 0 ? int(mBandwidth) - 1 : 0);
      } else {
        fixbw = dn * max(dist);
      }
      for (int r = 0; r < nr; r++) {
        w(r) = (*kerf)(dist(r), fixbw);
      }
    } else {
      for (int r = 0; r < nr; r++) {
        w(r) = (*kerf)(dist(r), mBandwidth);
      }
    }
    return w;
}

double GwmBandwidthWeight::bandwidth() const
{
    return mBandwidth;
}

void GwmBandwidthWeight::setBandwidth(double bandwidth)
{
    mBandwidth = bandwidth;
}

double GwmBandwidthWeight::adaptive() const
{
    return mAdaptive;
}

void GwmBandwidthWeight::setAdaptive(double adaptive)
{
    mAdaptive = adaptive;
}

GwmBandwidthWeight::KernelFunctionType GwmBandwidthWeight::kernel() const
{
    return mKernel;
}

void GwmBandwidthWeight::setKernel(const KernelFunctionType &kernel)
{
    mKernel = kernel;
}
