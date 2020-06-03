#ifndef GWMGGWRTASKTHREAD_H
#define GWMGGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"

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

class GwmGGWRTaskThread : public GwmGWRTaskThread
{
public:
    static QMap<QString, double> TolUnitDict;
    static void initTolUnitDict();

public:
    enum Family
    {
        Poisson,
        Binomial
    };

public:
    GwmGGWRTaskThread();
    GwmGGWRTaskThread(const GwmGGWRTaskThread &taskThread);

protected:
    Family mFamily;
    double mTol;
    QString mTolUnit;
    int mMaxiter;
    bool isCV;

    mat mWtMat1;
    mat mWtMat2;

    GwmGGWRDiagnostic mDiagnostic;


protected:
    void run() override;

    vec distance(int focus, mat regPoints);
    vec distanceCRS(int focus, mat regPoints);
    vec distanceMinkowski(int focus, mat regPoints);
    vec distanceDmat(int focus, mat regPoints);

    bool gwrPoisson();
    bool gwrBinomial();

    void diagnosticGGWR();

    mat diag(mat a);

    void createResultLayer();

public:
    Family getFamily() const;
    double getTol() const;
    int getMaxiter() const;
    bool getIsCV() const;

    mat getWtMat1() const;
    mat getWtMat2() const;

    GwmGGWRDiagnostic getDiagnostic() const;

    bool setFamily(Family family);
    bool setTol(double tol, QString unit);
    bool setMaxiter(int maxiter);
    bool setIsCV(bool iscv);
};

#endif // GWMGGWRTASKTHREAD_H
