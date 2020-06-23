#include "gwmpropertygwpcatab.h"
#include "ui_gwmpropertygwpcatab.h"

#include <armadillo>

//#include "TaskThread/gwmgwrmodelselectionthread.h"
#include "TaskThread/gwmbandwidthselecttaskthread.h"

#include <QStandardItemModel>

using namespace arma;

GwmPropertyGWPCATab::GwmPropertyGWPCATab(QWidget *parent, GwmLayerGWPCAItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWPCATab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {

        if (item->bandwidthOptimized())
        {
            mBandwidthSelPlot = new GwmPlot();
            ui->grpBwSelView->layout()->addWidget(mBandwidthSelPlot);
//            ui->grpBwSelView->hide();
        }
        else
        {
            ui->grpBwSelView->hide();
        }
    }
}

GwmPropertyGWPCATab::~GwmPropertyGWPCATab()
{
    delete ui;
}

void GwmPropertyGWPCATab::updateUI()
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
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }
    //ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    ui->lblPCCount->setText(QString("%1").arg(mLayerItem->mk));

    // 计算四分位数
    //QList<GwmVariable> indepVars = mLayerItem->indepVars();
    ui->tbwProp->setRowCount(mLayerItem->mk+1);
    ui->tbwProp->setColumnCount(6);
    ui->tbwProp->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwProp->setHorizontalHeaderLabels(headers);
    //const mat& betas = mLayerItem->betas();
    const mat&betas2 = mLayerItem->mdResult1;
    const mat&betas = mLayerItem->mLocalPV;
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = QString("Comp.%1").arg(r+1);
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwProp->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwProp->setItem(r, c + 1, quantileItem);
        }
    }
    //quantile(sum(betas,1),p)
    vec q = quantile(sum(betas,1),p);
    QString name = QString("Cumulative");
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
    ui->tbwProp->setItem(betas.n_cols, 0, nameItem);
    for (int c = 0; c < 5; c++)
    {
        QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
        quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwProp->setItem(betas.n_cols, c + 1, quantileItem);
    }

    ui->tbwLocalvariance->setRowCount(mLayerItem->mk);
    ui->tbwLocalvariance->setColumnCount(6);
    ui->tbwLocalvariance->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers2 = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwLocalvariance->setHorizontalHeaderLabels(headers);
    //const mat& betas = mLayerItem->betas();
    //const mat&betas2 = mLayerItem->mdResult1;
    //const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas2.n_cols; r++)
    {
        vec q = quantile(betas2.col(r), p);
        QString name = QString("Comp.%1").arg(r+1);
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwLocalvariance->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwLocalvariance->setItem(r, c + 1, quantileItem);
        }
    }
    // 绘制可视化图标
    if(mLayerItem->bandwidthOptimized())
    {
        BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        QVariant data = QVariant::fromValue(bwScores);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }


}
