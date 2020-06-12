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
    GwmDistance() {}
    virtual ~GwmDistance() {}

    virtual GwmDistance* clone() = 0;

public:
    virtual vec distance(rowvec target, mat dataPoints) = 0;
};


#endif // GWMDISTANCE_H
