#include "gwmpropertygwsstab.h"
#include "ui_gwmpropertygwsstab.h"

#include "TaskThread/gwmgwsstaskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"

QMap<GwmBandwidthWeight::KernelFunctionType, QString> GwmPropertyGWSSTab::kernelFunctionNameDict = {
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Boxcar, QStringLiteral("Boxcar"))
};

QMap<bool, QString> GwmPropertyGWSSTab::bandwidthTypeNameDict = {
    std::make_pair(true, QStringLiteral("Adaptive bandwidth:")),
    std::make_pair(false, QStringLiteral("Fixed bandwidth:"))
};

GwmPropertyGWSSTab::GwmPropertyGWSSTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWSSTab)
{
    ui->setupUi(this);
}

GwmPropertyGWSSTab::~GwmPropertyGWSSTab()
{
    delete ui;
}
