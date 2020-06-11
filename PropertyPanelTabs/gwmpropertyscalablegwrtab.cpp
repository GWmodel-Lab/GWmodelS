#include "gwmpropertyscalablegwrtab.h"
#include "ui_gwmpropertyscalablegwrtab.h"

#include <armadillo>

#include "TaskThread/gwmgwrmodelselectionthread.h"
#include "TaskThread/gwmbandwidthselecttaskthread.h"

#include <QStandardItemModel>

using namespace arma;

QMap<GwmGWRTaskThread::KernelFunction, QString> GwmPropertyScalableGWRTab::kernelFunctionNameDict = {
    std::make_pair(GwmGWRTaskThread::KernelFunction::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Boxcar, QStringLiteral("Boxcar"))
};

QMap<GwmGWRTaskThread::BandwidthType, QString> GwmPropertyScalableGWRTab::bandwidthTypeNameDict = {
    std::make_pair(GwmGWRTaskThread::BandwidthType::Adaptive, QStringLiteral("Adaptive bandwidth:")),
    std::make_pair(GwmGWRTaskThread::BandwidthType::Fixed, QStringLiteral("Fixed bandwidth:"))
};

GwmPropertyScalableGWRTab::GwmPropertyScalableGWRTab(QWidget *parent, GwmLayerScalableGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyScalableGWRTab),
    mLayerItem(item)
{
    ui->setupUi(this);
}

GwmPropertyScalableGWRTab::~GwmPropertyScalableGWRTab()
{
    delete ui;
}

void GwmPropertyScalableGWRTab::updateUI()
{
    if (!mLayerItem)
        return;
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
    ui->lblNumberDataPoints->setText(QString().sprintf("%d", mLayerItem->dataPointsSize()));

    if (mLayerItem->getHasHatmatrix())
    {
        GwmGWRDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblENP->setText(QString("%1").arg(diagnostic.ENP, 0, 'f', 6));
        ui->lblEDF->setText(QString("%1").arg(diagnostic.EDF, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblRSquare->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
        ui->lblRSquareAdjusted->setText(QString("%1").arg(diagnostic.RSquareAdjust, 0, 'f', 6));
    }

    // 计算四分位数
    QList<int> indepVarsIndex = mLayerItem->getIndepVarIndex();
    QList<GwmLayerAttributeItem*> indepVars, originIndepVars = mLayerItem->getIndepVarsOrigin();
    for (int index : indepVarsIndex)
    {
        for (GwmLayerAttributeItem* item : originIndepVars)
        {
            if (item->attributeIndex() == index)
                indepVars.append(item);
        }
    }
    ui->tbwCoefficient->setRowCount(indepVarsIndex.size() + 1);
    ui->tbwCoefficient->setColumnCount(6);
    ui->tbwCoefficient->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwCoefficient->setHorizontalHeaderLabels(headers);
    const mat& betas = mLayerItem->betas();
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = (r == 0) ? QStringLiteral("Intercept") : indepVars[r - 1]->attributeName();
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwCoefficient->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwCoefficient->setItem(r, c + 1, quantileItem);
        }
    }

    // LOOCV 结果
    ui->lblBtilde->setText(QString().sprintf("%.6lf", mLayerItem->getScale()));
    ui->lblAlpha->setText(QString().sprintf("%.6lf", mLayerItem->getPenalty()));
    ui->lblCV->setText(QString().sprintf("%.6lf", mLayerItem->getCV()));
}
