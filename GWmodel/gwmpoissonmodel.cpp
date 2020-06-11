#include "gwmpoissonmodel.h"
#include "GWmodel.h"

GwmPoissonModel::GwmPoissonModel()
{

}

mat GwmPoissonModel::initialize(){
    mat n = ones(mY.n_rows);
    mMuStart = mY + 0.1;
    return n;
}

mat GwmPoissonModel::muStart(){
    return mMuStart;
}


mat GwmPoissonModel::linkinv(mat eta){
    return exp(eta);
}

mat GwmPoissonModel::variance(mat mu){
    return mu;
}

mat GwmPoissonModel::devResids(mat y, mat mu, mat weights){
    mat r = mu % weights;
    r = (weights % (y % log(y/mu) - (y - mu)));
    return 2 * r;
}

double GwmPoissonModel::aic(mat y,mat n,mat mu,mat wt,double dev){
    vec temp = dpois(y, mu) % wt;
    return -2 * sum(temp);
}

mat GwmPoissonModel::muEta(mat eta){
    return exp(eta);
}

bool GwmPoissonModel::valideta(mat eta){
    return true;
}

bool GwmPoissonModel::validmu(mat mu){
    if(mu.is_finite() && mu.min() > 0){
        return true;
    }
    return false;
}

mat GwmPoissonModel::linkfun(mat muStart){
    return log(muStart);
}

bool GwmPoissonModel::setY(mat y){
    mY = y;
    return true;
}

bool GwmPoissonModel::setMuStart(mat muStart){
    mMuStart = muStart;
    return true;
}

bool GwmPoissonModel::setWeight(mat weight){
    mWeight = weight;
    return true;
}

mat GwmPoissonModel::weights(){
    return mWeight;
}

mat GwmPoissonModel::getY(){
    return mY;
}


