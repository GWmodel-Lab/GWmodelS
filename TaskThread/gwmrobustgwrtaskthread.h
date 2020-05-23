#ifndef GWMROBUSTGWRTASKTHREAD_H
#define GWMROBUSTGWRTASKTHREAD_H

#include "gwmgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

class GwmRobustGWRTaskThread:public GwmGWRTaskThread
{
public:
    GwmRobustGWRTaskThread();
    //二次权重数组
    vec result;
    double mse;
    bool filtered;

    void setFiltered(bool value);
protected:
    void run() override;
    // 主解算函数
    bool gwrModelCalibration();
    // 鲁棒GWR的第一种解法
    bool robustGWRCaliFirst();
    // 第二种解法
    bool robustGWRCaliSecond();
    // 计算二次权重函数
    vec filtWeight(vec x);

};

#endif // GWMROBUSTGWRTASKTHREAD_H
