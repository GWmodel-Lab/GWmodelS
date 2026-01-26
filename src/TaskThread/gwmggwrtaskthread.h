// gwmggwrtaskthread.h is unused
#ifndef GWMGGWRTASKTHREAD_H
#define GWMGGWRTASKTHREAD_H

#include "gwmtaskthread.h"
#include "gwmodel.h"
#include <armadillo>


class GwmGGWRTaskThread
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
    void run();

public:


private:
    gwm::GWRGeneralized mAlgorithm;
    std::unique_ptr<gwm::GWRGeneralized> mGGWRCore;
};

#endif // GWMGGWRTASKTHREAD_H
