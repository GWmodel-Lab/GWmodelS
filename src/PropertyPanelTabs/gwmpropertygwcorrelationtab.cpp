#include "gwmpropertygwcorrelationtab.h"
#include "ui_gwmpropertygwcorrelationtab.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>

#include <armadillo>

#include <QStandardItemModel>


GwmPropertyGWCorrelationTab::GwmPropertyGWCorrelationTab(QWidget *parent,GwmLayerGWCorrelationItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWCorrelationTab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {
        mParameterSpecifiedModel = new GwmPropertyGWCorrelationsParameterSpecifiedItemModel(item, this);
    }
}

GwmPropertyGWCorrelationTab::~GwmPropertyGWCorrelationTab()
{
    delete ui;
}

void GwmPropertyGWCorrelationTab::updateUI()
{
    if (!mLayerItem)
        return;

    // 带宽参数
    ui->trvBandwidthDistance->setModel(mParameterSpecifiedModel);


    // 计算四分位数
    QList<GwmVariable> var = mLayerItem->variables();
    QList<GwmVariable> varY = mLayerItem->variablesY();
    GwmGWCorrelationTaskThread::CreateResultLayerData data = mLayerItem->resultlist();
    int nVar = var.size()*varY.size();
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
            int r = 0;
            for(uword j = 0; j < nVar; j++)
            {
                    vec q = quantile(value.col(r), p);
                    QString name = var[j/varY.size()].name + "*" + varY[(j+varY.size())%varY.size()].name ;
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
        layout->addWidget(tablewidget);
        groupBox->setLayout(layout);
        ui->verticalLayout_2->addWidget(groupBox);
        tablewidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        }

}
