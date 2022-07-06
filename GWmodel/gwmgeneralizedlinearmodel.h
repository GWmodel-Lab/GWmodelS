#ifndef GWMGENERALIZEDLINEARMODEL_H
#define GWMGENERALIZEDLINEARMODEL_H

#include <armadillo>
#include "GWmodel/gwmlinearmodel.h"
#include "TaskThread/gwmgeneralizedgwralgorithm.h"

using namespace arma;

class GwmGeneralizedLinearModel
{
public:
    GwmGeneralizedLinearModel();

protected:
    mat mX;
    mat mY;
    mat mWeight;
    GwmGeneralizedGWRAlgorithm::Family mFamily;
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
    bool mIsCanceled = false;

public:
    void fit();

    bool setX(mat X);
    bool setY(mat Y);
    bool setFamily(GwmGeneralizedGWRAlgorithm::Family family);

    double dev();
    double nullDev();
    double aic();
    bool isCanceled() const;
    void setCanceled(bool newCanceled);
    bool checkCanceled();


};
inline bool GwmGeneralizedLinearModel::isCanceled() const
{
    return mIsCanceled;
}

inline void GwmGeneralizedLinearModel::setCanceled(bool newCanceled)
{
    mIsCanceled = newCanceled;
}

#endif // GWMGENERALIZEDLINEARMODEL_H
