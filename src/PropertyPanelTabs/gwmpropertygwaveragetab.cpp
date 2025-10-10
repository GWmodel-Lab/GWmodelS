#include "gwmpropertygwaveragetab.h"
#include "ui_gwmpropertygwaveragetab.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>

#include <armadillo>

#include <QStandardItemModel>
#include "TaskThread/gwmgwaveragetaskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"

QMap<GwmBandwidthWeight::KernelFunctionType, QString> GwmPropertyGWAverageTab::kernelFunctionNameDict = {
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Boxcar, QStringLiteral("Boxcar"))
};

QMap<bool, QString> GwmPropertyGWAverageTab::bandwidthTypeNameDict = {
    std::make_pair(true, QStringLiteral("Adaptive")),
    std::make_pair(false, QStringLiteral("Fixed:"))
};

GwmPropertyGWAverageTab::GwmPropertyGWAverageTab(QWidget *parent, GwmLayerGWAverageItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWAverageTab),
    mLayerItem(item)
{
    ui->setupUi(this);
}

GwmPropertyGWAverageTab::~GwmPropertyGWAverageTab()
{
    delete ui;
}

void GwmPropertyGWAverageTab::updateUI()
{
    if (!mLayerItem)
        return;
    GwmBandwidthWeight weight = mLayerItem->bandwidth();
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));
    if (weight.adaptive())
    {
        QString bwSizeString = QString("%1 (number of nearest neighbours)").arg(int(weight.bandwidth()));
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }

    // 计算四分位数
    QList<GwmVariable> indepVars = mLayerItem->variables();
    GwmGWAverageTaskThread::CreateResultLayerData data = mLayerItem->resultlist();
    int nVar = indepVars.size();
    for (QPair<QString, const mat&> item : data)
    {
        QString title = item.first;
        const mat& value = item.second;
        QGroupBox* groupBox = new QGroupBox(this);
        groupBox->setTitle(  "Summary information for " + title);
        groupBox->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));
        QVBoxLayout* layout = new QVBoxLayout();
        QTableWidget* tablewidget = new QTableWidget(this);
        tablewidget->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed));
        tablewidget->setRowCount(nVar);
        tablewidget->setColumnCount(6);
        tablewidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
        tablewidget->setHorizontalHeaderLabels(headers);
        const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
        if (value.n_cols > nVar){
            int r = 0;
            for(uword j = 0; j < nVar-1; j++)
            {
                for (uword k = j+1; k < nVar; k++)
                {
                    vec q = quantile(value.col(r), p);
                    QString name =  indepVars[j].name + "*" + indepVars[k].name ;
                    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
                    nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                    tablewidget->setItem(r, 0, nameItem);
                    for (int c = 0; c < 5; c++)
                    {
                        QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
                        quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                        tablewidget->setItem(r, c + 1, quantileItem);
                    }
                    r++;
                }
            }
        }
        else
        {
            for (uword r = 0; r < value.n_cols; r++)
            {
                vec q = quantile(value.col(r), p);
                QString name = indepVars[r].name;
                QTableWidgetItem* nameItem = new QTableWidgetItem(name);
                nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                tablewidget->setItem(r, 0, nameItem);
                for (int c = 0; c < 5; c++)
                {
                    QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
                    quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                    tablewidget->setItem(r, c + 1, quantileItem);
                }
            }
        }
        layout->addWidget(tablewidget);
        groupBox->setLayout(layout);
        ui->verticalLayout->addWidget(groupBox);
        tablewidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    }

}
