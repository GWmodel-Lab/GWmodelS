#include "gwmpropertygwsstab.h"
#include "ui_gwmpropertygwsstab.h"

#include <QStandardItemModel>
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
    std::make_pair(true, QStringLiteral("Adaptive")),
    std::make_pair(false, QStringLiteral("Fixed:"))
};

GwmPropertyGWSSTab::GwmPropertyGWSSTab(QWidget *parent, GwmLayerGWSSItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWSSTab),
    mLayerItem(item)
{
    ui->setupUi(this);
}

GwmPropertyGWSSTab::~GwmPropertyGWSSTab()
{
    delete ui;
}

void GwmPropertyGWSSTab::updateUI()
{
    if (!mLayerItem)
        return;
    GwmBandwidthWeight weight = mLayerItem->bandwidth();
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));

    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }

    QList<GwmVariable> indepVars = mLayerItem->variables();
    ui->tbwCVValue->setRowCount(indepVars.size());
    ui->tbwCVValue->setColumnCount(3);
    ui->tbwCVValue->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Local Mean bw") << tr("Local Median bw");
    ui->tbwCVValue->setHorizontalHeaderLabels(headers);
    const mat& bws = mLayerItem->bws();
    for (uword r = 0; r < bws.n_cols; r++)
    {
        QString name = indepVars[r].name;
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwCVValue->setItem(r, 0, nameItem);
        for (int c = 0; c < 2; c++)
        {
            QTableWidgetItem* Item = new QTableWidgetItem(QString("%1").arg(bws(c,r), 0, 'f', 3));
            Item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            Item->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwCVValue->setItem(r, c + 1, Item);
        }
    }
}
