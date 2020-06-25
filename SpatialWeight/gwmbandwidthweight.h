#ifndef GWMBANDWIDTHWEIGHT_H
#define GWMBANDWIDTHWEIGHT_H

#include "SpatialWeight/gwmweight.h"

class GwmBandwidthWeight : public GwmWeight
{
public:
    enum KernelFunctionType
    {
        Gaussian,
        Exponential,
        Bisquare,
        Tricube,
        Boxcar
    };
    static GwmEnumValueNameMapper<KernelFunctionType> KernelFunctionTypeNameMapper;

    static GwmEnumValueNameMapper<bool> BandwidthTypeNameMapper;

    typedef double (*KernelFunction)(double, double);

    static KernelFunction Kernel[];

    static double GaussianKernelFunction(double dist, double bw);
    static double ExponentialKernelFunction(double dist, double bw);
    static double BisquareKernelFunction(double dist, double bw);
    static double TricubeKernelFunction(double dist, double bw);
    static double BoxcarKernelFunction(double dist, double bw);

public:
    GwmBandwidthWeight();
    GwmBandwidthWeight(double size, bool adaptive, KernelFunctionType kernel);
    GwmBandwidthWeight(const GwmBandwidthWeight& bandwidthWeight);
    GwmBandwidthWeight(const GwmBandwidthWeight* bandwidthWeight);

    virtual GwmWeight * clone() override
    {
        return new GwmBandwidthWeight(*this);
    }

public:
    virtual vec weight(vec dist) override;

    double bandwidth() const;
    void setBandwidth(double bandwidth);

    bool adaptive() const;
    void setAdaptive(bool adaptive);

    KernelFunctionType kernel() const;
    void setKernel(const KernelFunctionType &kernel);

private:
    double mBandwidth;
    bool mAdaptive;
    KernelFunctionType mKernel;
};

inline double GwmBandwidthWeight::GaussianKernelFunction(double dist, double bw) {
  return exp((dist * dist)/((-2)*(bw * bw)));
}

inline double GwmBandwidthWeight::ExponentialKernelFunction(double dist, double bw) {
  return exp(-dist/bw);
}

inline double GwmBandwidthWeight::BisquareKernelFunction(double dist, double bw) {
  return dist > bw ? 0 : (1 - (dist * dist)/(bw * bw)) * (1 - (dist * dist)/(bw * bw));
}

inline double GwmBandwidthWeight::TricubeKernelFunction(double dist, double bw) {
  return dist > bw ?
              0 :
              (1 - (dist * dist * dist)/(bw * bw * bw)) *
              (1 - (dist * dist * dist)/(bw * bw * bw)) *
              (1 - (dist * dist * dist)/(bw * bw * bw));
}

inline double GwmBandwidthWeight::BoxcarKernelFunction(double dist, double bw) {
  return dist > bw ? 0 : 1;
}

inline double GwmBandwidthWeight::bandwidth() const
{
    return mBandwidth;
}

inline void GwmBandwidthWeight::setBandwidth(double bandwidth)
{
    mBandwidth = bandwidth;
}

inline bool GwmBandwidthWeight::adaptive() const
{
    return mAdaptive;
}

inline void GwmBandwidthWeight::setAdaptive(bool adaptive)
{
    mAdaptive = adaptive;
}

inline GwmBandwidthWeight::KernelFunctionType GwmBandwidthWeight::kernel() const
{
    return mKernel;
}

inline void GwmBandwidthWeight::setKernel(const KernelFunctionType &kernel)
{
    mKernel = kernel;
}

#endif // GWMBANDWIDTHWEIGHT_H
