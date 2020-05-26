#include "gwmgeneralizedlinearmodel.h"
#include "gwmpoissonmodel.h"
#include "gwmbinomialmodel.h"
#include "GWmodel.h"

GwmGeneralizedLinearModel::GwmGeneralizedLinearModel()
{
    mWeight = vec(uword(0));
    mOffset = vec(uword(0));
    mIntercept = true;
    mMaxit = 25;
    mEpsilon = 1e-8;
}

void GwmGeneralizedLinearModel::fit(){
    int nVars = mX.n_cols;
    int nObs = mY.n_rows;
    bool empty = (nVars == 0);
    mat coefold;
    mat eta = vec(uword(0));
    mat mu;
    if(mWeight.is_empty()){
        mWeight = ones(nObs);
    }
    if(mOffset.is_empty()){
        mOffset = zeros(nObs);
    }
    if(mFamily == GwmGGWRTaskThread::Family::Poisson){
        //初始化模型
        mModel = new GwmPoissonModel();
        mModel->setY(mY);
        mModel->setWeight(mWeight);
    }
    else{
        mModel = new GwmBinomialModel();
        mModel->setY(mY);
        mModel->setWeight(mWeight);
    }
    mat n = mModel->initialize();
    mMuStart = mModel->muStart();
    if(empty){
        eta = zeros(nObs) + mOffset;
//        if(!mModel->valideta(eta)){

//        }
        mu = mModel->linkinv(eta);
        vec devtemp = mModel->devResids(mY,mu,mWeight);
        mDev = sum(devtemp);
        mat mueta = mModel->muEta(eta);
        mat w = sqrt((mWeight * (mueta % mueta))/mModel->variance(mu)); //^2符号的作用？
        mResiduals = (mY - mu)/mueta;
    }
    else{
        eta = mModel->linkfun(mMuStart);
        mu = mModel->linkinv(eta);
        vec devoldtemp = mModel->devResids(mY,mu,mWeight);
        double devold = sum(devoldtemp);
        mat start = mat(uword(0), uword(0));
        mat coef;
        bool boundary = false;
        bool conv = false;
        for(int iter = 0; iter < mMaxit; iter++){
            mat varmu = mModel->variance(mu);
            mat muetaval = mModel->muEta(eta);
            mat z = (eta - mOffset) + (mY - mu)/muetaval;
            mat w = sqrt((mWeight % (muetaval % muetaval))/varmu);
            for (int i = 0; i < mX.n_rows; i++){
                start.col(i) = gwReg(mX,z,w,i);
            }
            eta = gwFitted(mX , start) ; //?不是很确定
            mu = mModel->linkinv(eta + mOffset);
            vec devtemp = mModel->devResids(mY,mu,mWeight);
            mDev = sum(devtemp);
            if (abs(mDev - devold)/(0.1 + abs(mDev)) < mEpsilon) {
                       conv = true;
                       coef = start;
                       break;
                   } else {
                       devold = mDev;
                       coef = coefold = start;
            }
        }
    }
    mat wtdmu = (mIntercept)? sum(mWeight % mY)/sum(mWeight) : mModel->linkinv(mOffset);
    vec nulldevtemp = mModel->devResids(mY,wtdmu,mWeight);
    mNullDev = sum(nulldevtemp);
    int rank = empty? 0 : mX.n_rows;
    mAIC = mModel->aic(mY,n,mu,mWeight,mDev) + 2 * rank;
}


bool GwmGeneralizedLinearModel::setX(mat X){
    mX = X;
    return true;
}

bool GwmGeneralizedLinearModel::setY(mat Y){
    mY =  Y;
    return true;
}

bool GwmGeneralizedLinearModel::setFamily(GwmGGWRTaskThread::Family family){
    mFamily = family;
    return true;
}

double GwmGeneralizedLinearModel::aic(){
    return mAIC;
}

double GwmGeneralizedLinearModel::nullDev(){
    return mNullDev;
}

double GwmGeneralizedLinearModel::dev(){
    return mDev;
}

