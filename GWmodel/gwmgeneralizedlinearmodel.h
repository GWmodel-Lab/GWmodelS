#ifndef GWMGENERALIZEDLINEARMODEL_H
#define GWMGENERALIZEDLINEARMODEL_H

#include <armadillo>
#include "GWmodel/gwmlinearmodel.h"
#include "TaskThread/gwmggwralgorithm.h"

using namespace arma;

class GwmGeneralizedLinearModel
{
public:
    GwmGeneralizedLinearModel();

protected:
    mat mX;
    mat mY;
    mat mWeight;
    GwmGGWRAlgorithm::Family mFamily;
    double mEpsilon;
    int mMaxit;
    bool mIntercept;
    mat mOffset;
    GwmLinearModel* mModel;

    mat mMuStart;
    double mDev;
    mat mResiduals;
    double mNullDev;
    double mAIC;

public:
    void fit();

    bool setX(mat X);
    bool setY(mat Y);
    bool setFamily(GwmGGWRAlgorithm::Family family);

    double dev();
    double nullDev();
    double aic();


};

#endif // GWMGENERALIZEDLINEARMODEL_H
