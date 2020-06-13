#ifndef IPARALLELABLE_H
#define IPARALLELABLE_H

struct IParallelalbe
{
    enum ParallelType
    {
        Serial = 0,
        OpenMP = 1 << 0,
        CUDA = 1 << 1
    };

    virtual int parallelAbility() const = 0;
    virtual ParallelType parallelType() const = 0;
    virtual void setParallelType(const ParallelType& type) = 0;
};

struct IOpenmpParallelable : public IParallelalbe
{
    virtual void setOmpThreadNum(const int threadNum) = 0;
};

struct ICudaParallelable : public IParallelalbe
{
    virtual void setGPUId(const int gpuId) = 0;
};

#endif // IPARALLELABLE_H
