#ifndef GWMGGWRTASKTHREAD_H
#define GWMGGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"

class GwmGGWRTaskThread : public GwmGWRTaskThread
{
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
    int mMaxiter;
    bool isCV;

    mat mWtMat1;
    mat mWtMat2;

protected:
    void run() override;

    vec distance(int focus, mat regPoints);
    vec distanceCRS(int focus, mat regPoints);
    vec distanceMinkowski(int focus, mat regPoints);
    vec distanceDmat(int focus, mat regPoints);

    void gwrPoisson();
    void gwrBinomial();

    mat diag(mat a);

public:
    Family getFamily() const;
    double getTol() const;
    int getMaxiter() const;
    bool getIsCV() const;

    mat getWtMat1() const;
    mat getWtMat2() const;

    bool setFamily(Family family);
    bool setTol(double tol);
    bool setMaxiter(int maxiter);
    bool setIsCV(bool iscv);
};

#endif // GWMGGWRTASKTHREAD_H
