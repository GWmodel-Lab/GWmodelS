#ifndef GWMGGWRALGORITHM_H
#define GWMGGWRALGORITHM_H

#include "gwmbasicgwralgorithm.h"

struct GwmGGWRDiagnostic
{
    double RSS;
    double AIC;
    double AICc;
    double RSquare;

    GwmGGWRDiagnostic()
    {
        AIC = 0.0;
        AICc = 0.0;
        RSS = 0.0;
        RSquare = 0.0;
    }

    GwmGGWRDiagnostic(const vec& diag)
    {
        AIC = diag(0);
        AICc = diag(1);
        RSS = diag(2);
        RSquare = diag(3);
    }
};

struct GwmGLMDiagnostic
{
    double NullDev;
    double Dev;
    double AIC;
    double AICc;
    double RSquare;

    GwmGLMDiagnostic()
    {
        AIC = 0.0;
        AICc = 0.0;
        Dev = 0.0;
        NullDev = 0.0;
        RSquare = 0.0;
    }

    GwmGLMDiagnostic(const vec& diag)
    {
        AIC = diag(0);
        AICc = diag(1);
        NullDev = diag(2);
        Dev = diag(3);
        RSquare = diag(4);
    }
};

class GwmGGWRAlgorithm : public GwmBasicGWRAlgorithm
{
public:
    GwmGGWRAlgorithm();
public:
    static QMap<QString, double> TolUnitDict;
    static void initTolUnitDict();

public:
    enum Family
    {
        Poisson,
        Binomial
    };
protected:
    Family mFamily;
    double mTol;
    QString mTolUnit;
    int mMaxiter;

    mat mWtMat1;
    mat mWtMat2;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    CreateResultLayerData mResultList;


protected:
    void run() override;

    bool gwrPoisson();
    bool gwrBinomial();


    mat diag(mat a);

//    void createResultLayer();

public:
    Family getFamily() const;
    double getTol() const;
    int getMaxiter() const;

    mat getWtMat1() const;
    mat getWtMat2() const;

    GwmGGWRDiagnostic getDiagnostic() const;

    GwmGLMDiagnostic getGLMDiagnostic() const;

    bool setFamily(Family family);
    bool setTol(double tol, QString unit);
    bool setMaxiter(int maxiter);
};


inline GwmGGWRAlgorithm::Family GwmGGWRAlgorithm::getFamily() const
{
    return mFamily;
}

inline double GwmGGWRAlgorithm::getTol() const
{
    return mTol;
}

inline int GwmGGWRAlgorithm::getMaxiter() const
{
    return mMaxiter;
}

inline mat GwmGGWRAlgorithm::getWtMat1() const
{
    return mWtMat1;
}

inline mat GwmGGWRAlgorithm::getWtMat2() const
{
    return mWtMat2;
}

inline GwmGGWRDiagnostic GwmGGWRAlgorithm::getDiagnostic() const
{
    return mDiagnostic;
}

inline GwmGLMDiagnostic GwmGGWRAlgorithm::getGLMDiagnostic() const
{
    return mGLMDiagnostic;
}

inline bool GwmGGWRAlgorithm::setFamily(Family family){
    mFamily = family;
    return true;
}

inline bool GwmGGWRAlgorithm::setTol(double tol, QString unit){
    mTolUnit = unit;
    mTol = double(tol) * TolUnitDict[unit];
    return true;
}

inline bool GwmGGWRAlgorithm::setMaxiter(int maxiter){
    mMaxiter = maxiter;
    return true;
}


#endif // GWMGGWRALGORITHM_H
