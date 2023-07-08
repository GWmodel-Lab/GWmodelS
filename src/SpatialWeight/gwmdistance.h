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
    explicit GwmDistance(int total) : mTotal(total) {};
    GwmDistance(const GwmDistance& d) { mTotal = d.mTotal; };
    virtual ~GwmDistance() {};

    virtual GwmDistance* clone() = 0;

    virtual DistanceType type() = 0;

    int total() const;
    void setTotal(int total);


public:
    virtual vec distance(int focus) = 0;
    virtual int length() const = 0;

    double maxDistance();
    double minDistance();

    bool isCanceled() const;
    void setCanceled(bool newCanceled);
    bool checkCanceled();

protected:
    int mTotal = 0;
    bool mIsCanceled = false;
};

inline int GwmDistance::total() const
{
    return mTotal;
}

inline void GwmDistance::setTotal(int total)
{
    mTotal = total;
}

inline bool GwmDistance::isCanceled() const
{
    return mIsCanceled;
}

inline void GwmDistance::setCanceled(bool newCanceled)
{
    mIsCanceled = newCanceled;
}

#endif // GWMDISTANCE_H
