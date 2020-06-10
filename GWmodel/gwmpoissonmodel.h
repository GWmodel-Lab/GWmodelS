#ifndef GWMPOISSONMODEL_H
#define GWMPOISSONMODEL_H

#include "gwmlinearmodel.h"

class GwmPoissonModel : public GwmLinearModel
{
public:
    GwmPoissonModel();

public:
    mat mMuStart;
    mat mY;
    mat mWeight;

public:
    mat initialize() override;
    mat variance(mat mu) override;
    mat linkinv(mat eta) override;
    mat devResids(mat y,mat mu,mat weights) override;
    double aic(mat y,mat n,mat mu,mat wt,double dev) override;
    mat muEta(mat eta) override;
    bool valideta(mat eta) override;
    bool validmu(mat mu) override;
    mat linkfun(mat muStart) override;

    mat muStart() override;
    mat weights() override;
    mat getY() override;
    bool setMuStart(mat muStart) override;
    bool setY(mat y) override;
    bool setWeight(mat weight) override;
};

#endif // GWMPOISSONMODEL_H
