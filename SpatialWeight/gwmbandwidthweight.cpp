#include "gwmbandwidthweight.h"

GwmBandwidthWeight::KernelFunction GwmBandwidthWeight::Kernel[] =
{
    &GwmBandwidthWeight::GaussianKernelFunction,
    &GwmBandwidthWeight::ExponentialKernelFunction,
    &GwmBandwidthWeight::BisquareKernelFunction,
    &GwmBandwidthWeight::TricubeKernelFunction,
    &GwmBandwidthWeight::BoxcarKernelFunction
};

GwmBandwidthWeight::GwmBandwidthWeight() : GwmWeight()
{

}

GwmBandwidthWeight::GwmBandwidthWeight(double size, bool adaptive, GwmBandwidthWeight::KernelFunctionType kernel)
{
    mBandwidth = size;
    mAdaptive = adaptive;
    mKernel = kernel;
}

GwmBandwidthWeight::GwmBandwidthWeight(const GwmBandwidthWeight &bandwidthWeight)
{
    mBandwidth = bandwidthWeight.mBandwidth;
    mAdaptive = bandwidthWeight.mAdaptive;
    mKernel = bandwidthWeight.mKernel;
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
