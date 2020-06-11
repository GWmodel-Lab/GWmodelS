#ifndef GWMDISTANCE_H
#define GWMDISTANCE_H

#include <armadillo>
#include "gwmenumvaluenamemapper.h"

using namespace arma;

class GwmDistance
{
public:
    enum DistanceType
    {
        CRSDistance,
        MinkwoskiDistance,
        DMatDistance
    };

    static GwmEnumValueNameMapper<DistanceType> TypeNameMapper;

public:
    GwmDistance();
    virtual ~GwmDistance();

public:
    virtual vec distance(rowvec target, mat dataPoints) = 0;
};

GwmEnumValueNameMapper<GwmDistance::DistanceType> GwmDistance::TypeNameMapper =
{
    std::make_pair(GwmDistance::DistanceType::CRSDistance, "CRSDistance"),
    std::make_pair(GwmDistance::DistanceType::MinkwoskiDistance, "MinkwoskiDistance"),
    std::make_pair(GwmDistance::DistanceType::DMatDistance, "DMatDistance")
};

GwmDistance::GwmDistance()
{

}

GwmDistance::~GwmDistance()
{

}


#endif // GWMDISTANCE_H
