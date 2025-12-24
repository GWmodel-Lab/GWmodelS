#ifndef GWMALGORITHMMETAGTDR
#define GWMALGORITHMMETAGTDR

#include <string>
#include <qgsvectorlayer.h>
#include <gwmodel.h>
#include "gwmvariableitemmodel.h"

struct GwmAlgorithmMetaGTDR
{
    QgsVectorLayer* layer = nullptr;
    QList<GwmVariable> independentVariables;
    GwmVariable dependentVariable;

    QList<GwmVariable> weightingVariables;  // 用于计算权重的变量
    // Weight
    gwm::Weight::WeightType weightType = gwm::Weight::BandwidthWeight;
    double weightBandwidthSize = DBL_MAX;
    bool weightBandwidthAdaptive = true;
    gwm::BandwidthWeight::KernelFunctionType weightBandwidthKernel = gwm::BandwidthWeight::KernelFunctionType::Gaussian;
    // Distance
    gwm::Distance::DistanceType distanceType = gwm::Distance::DistanceType::OneDimDistance;
    bool distanceCrsGeographic = false;
    double distanceMinkowskiPower = 2.0;
    double distanceMinkowskiTheta = 0.0;
    std::string distanceDmatFilename;
    // Parallel
    gwm::ParallelType parallelType = gwm::ParallelType::SerialOnly;
    std::size_t parallelOmpThreads = 1;
    std::size_t parallelCudaGroupSize = 64;
    std::size_t parallelCudaGpuID = 0;
    // Others
    bool hatmatrix = false;

    bool validate(QString &error) const;

    // Bandwidth Autoselection
    bool bandwidthAuto = true;
    gwm::GTDR::BandwidthCriterionType bandwidthCriterionType = gwm::GTDR::BandwidthCriterionType::AIC;
    double bandwidthOptimizeEps = 1e-6;
    std::size_t bandwidthOptimizeMaxIter = 500;
    double bandwidthOptimizeStep = 0.1;

    //Weight(multidimensional)
    QList<double> weightBandwidthSizes;
    QList<gwm::BandwidthWeight::KernelFunctionType> weightBandwidthKernels;
};


#endif  // GWMALGORITHMMETAGTDR
