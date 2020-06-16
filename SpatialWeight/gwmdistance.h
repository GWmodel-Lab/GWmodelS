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

    virtual DistanceType type() = 0;

public:
    virtual vec distance(int focus) = 0;

    double maxDistance(int n);
    double minDistance(int n);
};


#endif // GWMDISTANCE_H
