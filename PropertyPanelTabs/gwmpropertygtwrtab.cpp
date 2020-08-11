#include "gwmpropertygtwrtab.h"
#include "ui_gwmpropertygtwrtab.h"

#include <armadillo>

#include "TaskThread/gwmgwrmodelselectionthread.h"
#include "TaskThread/gwmbandwidthselecttaskthread.h"

#include <QStandardItemModel>

using namespace arma;

QMap<GwmGWRTaskThread::KernelFunction, QString> GwmPropertyGTWRTab::kernelFunctionNameDict = {
    std::make_pair(GwmGWRTaskThread::KernelFunction::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Boxcar, QStringLiteral("Boxcar"))
};

QMap<GwmGWRTaskThread::BandwidthType, QString> GwmPropertyGTWRTab::bandwidthTypeNameDict = {
    std::make_pair(GwmGWRTaskThread::BandwidthType::Adaptive, QStringLiteral("Adaptive bandwidth:")),
    std::make_pair(GwmGWRTaskThread::BandwidthType::Fixed, QStringLiteral("Fixed bandwidth:"))
};

GwmPropertyGTWRTab::GwmPropertyGTWRTab(QWidget *parent, GwmLayerGTWRItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGTWRTab),
    mLayerItem(item),
    mBandwidthSelPlot(nullptr)
{
    ui->setupUi(this);
    if (item)
    {
        if (item->bandwidthOptimized())
        {
            mBandwidthSelPlot = new GwmPlot();
            ui->grpBwSelView->layout()->addWidget(mBandwidthSelPlot);
        }
        else
        {
            ui->grpBwSelView->hide();
        }
        if (!item->hatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
    }
}

GwmPropertyGTWRTab::~GwmPropertyGTWRTab()
{
    delete ui;
}

void GwmPropertyGTWRTab::updateUI()
{
    if (!mLayerItem)
        return;

    GwmBandwidthWeight weight = mLayerItem->weight();
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));
    if (weight.adaptive())
    {
        QString bwSizeString = QString("%1 (number of nearest neighbours)").arg(int(weight.bandwidth()));
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    else
    {
        QString bwSizeString = QString("%1 %2")
                .arg(weight.bandwidth(), 0, 'f', 12)
                .arg(weight.bandwidth());
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    if (mLayerItem->regressionPointGiven())
    {
        ui->lblRegressionPoints->setText(tr("A seperate set of regression points is used."));
    }
    else
    {
        ui->lblRegressionPoints->setText(tr("The same location as observations are used."));
    }
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    if (mLayerItem->hatmatrix())
    {
        GwmDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblENP->setText(QString("%1").arg(diagnostic.ENP, 0, 'f', 6));
        ui->lblEDF->setText(QString("%1").arg(diagnostic.EDF, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblRSquare->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
        ui->lblRSquareAdjusted->setText(QString("%1").arg(diagnostic.RSquareAdjust, 0, 'f', 6));
    }

    // 计算四分位数
    QList<GwmVariable> indepVars = mLayerItem->indepVars();
    ui->tbwCoefficient->setRowCount(indepVars.size() + 1);
    ui->tbwCoefficient->setColumnCount(6);
    ui->tbwCoefficient->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwCoefficient->setHorizontalHeaderLabels(headers);
    const mat& betas = mLayerItem->betas();
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = (r == 0) ? QStringLiteral("Intercept") : indepVars[r - 1].name;
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

    if(mLayerItem->bandwidthOptimized())
    {
        BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        QVariant data = QVariant::fromValue(bwScores);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }
}
