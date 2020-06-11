#ifndef GWMCRSDISTANCE_H
#define GWMCRSDISTANCE_H

#include "SpatialWeight/gwmdistance.h"

class GwmCRSDistance : public GwmDistance
{
public:
    static vec SpatialDistance(rowvec in_loc, mat out_locs);
    static vec EuclideanDistance(rowvec in_loc, mat out_locs);

public:
    GwmCRSDistance();

    bool geographic() const;
    void setGeographic(bool geographic);

public:
    virtual vec distance(rowvec target, mat dataPoints) override;

private:
    bool mGeographic = false;
};

#endif // GWMCRSDISTANCE_H
