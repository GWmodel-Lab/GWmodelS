#include "gwmpoissonmodel.h"
//#include "GWmodel.h"
#include "TaskThread/gwmggwralgorithm.h"

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
    mat res = exp(eta);
    double eps = 2.220446e-16;
    if(res.min() > eps){
        return res;
    }
    for(int i = 0; i < eta.n_rows; i++){
        res(i) = res(i) > eps? res(i) : eps;
    }
    return res;
}

mat GwmPoissonModel::variance(mat mu){
    return mu;
}

mat GwmPoissonModel::devResids(mat y, mat mu, mat weights){
    mat r = mu % weights;  
    if( y.min() > 0){
        r = (weights % (y % log(y/mu) - (y - mu)));
    }
    else{
        for(int i = 0; i < y.n_rows; i++){
            if(y(i) > 0){
                r(i) = (weights(i) * (y(i) * log(y(i)/mu(i)) - (y(i) - mu(i))));
            }
        }
    }
    return 2 * r;
}

double GwmPoissonModel::aic(mat y,mat n,mat mu,mat wt,double dev){
    vec temp = GwmGGWRAlgorithm::dpois(y, mu) % wt;
    return -2 * sum(temp);
}

mat GwmPoissonModel::muEta(mat eta){
    mat res = exp(eta);
    double eps = 2.220446e-16;
    if(res.min() > eps){
        return res;
    }
    for(int i = 0; i < eta.n_rows; i++){
        res(i) = res(i) > eps? res(i) : eps;
    }
    return res;
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
    mat res = log(muStart);
    return res;
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


