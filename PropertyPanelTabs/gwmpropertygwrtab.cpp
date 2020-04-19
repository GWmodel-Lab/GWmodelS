#include "gwmpropertygwrtab.h"
#include "ui_gwmpropertygwrtab.h"

QMap<GwmGWRTaskThread::KernelFunction, QString> GwmPropertyGWRTab::kernelFunctionNameDict = {
    std::make_pair(GwmGWRTaskThread::KernelFunction::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Boxcar, QStringLiteral("Boxcar"))
};

QMap<GwmGWRTaskThread::BandwidthType, QString> GwmPropertyGWRTab::bandwidthTypeNameDict = {
    std::make_pair(GwmGWRTaskThread::BandwidthType::Adaptive, QStringLiteral("Adaptive bandwidth:")),
    std::make_pair(GwmGWRTaskThread::BandwidthType::Fixed, QStringLiteral("Fixed bandwidth:"))
};

GwmPropertyGWRTab::GwmPropertyGWRTab(QWidget *parent, GwmLayerGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWRTab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {
        updateUI();
    }
}

GwmPropertyGWRTab::~GwmPropertyGWRTab()
{
    delete ui;
}

void GwmPropertyGWRTab::updateUI()
{
    ui->lblKernelFunction->setText(kernelFunctionNameDict[mLayerItem->bandwidthKernelFunction()]);
    ui->lblBandwidthType->setText(bandwidthTypeNameDict[mLayerItem->bandwidthType()]);
    if (mLayerItem->bandwidthType() == GwmGWRTaskThread::BandwidthType::Adaptive)
    {
        QString bwSizeString = QString("%1 (number of nearest neighbours)").arg(int(mLayerItem->bandwidthSize()));
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    else
    {
        QString bwSizeString = QString("%1 %2")
                .arg(mLayerItem->bandwidthSize(), 0, 'f', 12)
                .arg(mLayerItem->bandwidthUnit());
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    if (true)
    {
        ui->lblRegressionPoints->setText(tr("the same location as observations are used."));
    }
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    GwmGWRDiagnostic diagnostic = mLayerItem->diagnostic();
    ui->lblENP->setText(QString("%1").arg(diagnostic.ENP, 0, 'f', 6));
    ui->lblEDF->setText(QString("%1").arg(diagnostic.EDF, 0, 'f', 6));
    ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
    ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
    ui->lblRSS->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
    ui->lblRSquare->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
    ui->lblRSquareAdjusted->setText(QString("%1").arg(diagnostic.RSquareAdjust, 0, 'f', 6));
}
