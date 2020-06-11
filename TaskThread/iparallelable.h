#ifndef IPARALLELABLE_H
#define IPARALLELABLE_H

interface IParallelalbe
{
    enum ParallelType
    {
        Unable = 0,
        OpenMP = 1 << 0,
        CUDA = 1 << 1
    };

    virtual int parallelAbility() const = 0;
    virtual ParallelType parallelType() const = 0;
    virtual void setParallelType(const ParallelType& type) = 0;
};

interface IOpenmpParallelable : public IParallelalbe
{
    virtual void setThreadNum(const int threadNum) = 0;
};

interface ICudaParallelable : public IParallelalbe
{
    virtual void setGPUId(const int gpuId) = 0;
};

#endif // IPARALLELABLE_H
