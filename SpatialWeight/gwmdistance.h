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

    int total() const;
    void setTotal(int total);

public:
    virtual vec distance(int focus) = 0;

    double maxDistance();
    double minDistance();

protected:
    int mTotal = 0;
};

inline int GwmDistance::total() const
{
    return mTotal;
}

inline void GwmDistance::setTotal(int total)
{
    mTotal = total;
}


#endif // GWMDISTANCE_H
