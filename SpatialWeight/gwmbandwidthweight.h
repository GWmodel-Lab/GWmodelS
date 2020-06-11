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

public:
    virtual vec weight(vec dist) override;

    double bandwidth() const;
    void setBandwidth(double bandwidth);

    double adaptive() const;
    void setAdaptive(double adaptive);

    KernelFunctionType kernel() const;
    void setKernel(const KernelFunctionType &kernel);

private:
    double mBandwidth;
    double mAdaptive;
    KernelFunctionType mKernel;
};

#endif // GWMBANDWIDTHWEIGHT_H
