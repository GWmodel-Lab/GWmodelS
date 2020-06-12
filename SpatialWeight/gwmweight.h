#ifndef GWMWEIGHT_H
#define GWMWEIGHT_H

#include <armadillo>

#include "gwmenumvaluenamemapper.h"

using namespace arma;

class GwmWeight
{
public:
    enum WeightType
    {
        BandwidthWeight
    };

    static GwmEnumValueNameMapper<WeightType> TypeNameMapper;

public:
    GwmWeight() {}
    virtual ~GwmWeight() {}

    virtual GwmWeight* clone() = 0;

public:
    virtual vec weight(vec dist) = 0;
};

#endif // GWMWEIGHT_H
