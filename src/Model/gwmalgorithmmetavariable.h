#ifndef GWMALGORITHMMETAVARIABLE
#define GWMALGORITHMMETAVARIABLE

#include <string>
#include <qgsvectorlayer.h>
#include <gwmodel.h>
#include "gwmvariableitemmodel.h"

struct GwmAlgorithmMetaVariable
{
    QgsVectorLayer* layer = nullptr;
    QList<GwmVariable> variables;
    // Weight
    gwm::Weight::WeightType weightType = gwm::Weight::BandwidthWeight;
    double weightBandwidthSize = DBL_MAX;
    bool weightBandwidthAdaptive = true;
    gwm::BandwidthWeight::KernelFunctionType weightBandwidthKernel = gwm::BandwidthWeight::KernelFunctionType::Gaussian;
    // Distance
    gwm::Distance::DistanceType distanceType = gwm::Distance::DistanceType::CRSDistance;
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
    bool quantile = false;

    bool validate(QString &error) const;
};


#endif  // GWMALGORITHMMETAVARIABLE