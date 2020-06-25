#include "gwmpropertymultiscalegwrtab.h"
#include "ui_gwmpropertymultiscalegwrtab.h"

#include <armadillo>

#include "TaskThread/gwmgwrmodelselectionthread.h"
#include "TaskThread/gwmbandwidthselecttaskthread.h"

#include <QStandardItemModel>

using namespace arma;

GwmPropertyMultiscaleGWRTab::GwmPropertyMultiscaleGWRTab(QWidget *parent, GwmLayerMultiscaleGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyMultiscaleGWRTab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {
        mParameterSpecifiedModel = new GwmPropertyMultiscaleParameterSpecifiedItemModel(item, this);
        if (!item->hasHatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
    }
}

GwmPropertyMultiscaleGWRTab::~GwmPropertyMultiscaleGWRTab()
{
    delete ui;
}

void GwmPropertyMultiscaleGWRTab::updateUI()
{
    if (!mLayerItem)
        return;
    if (true)
    {
        ui->lblRegressionPoints->setText(tr("the same location as observations are used."));
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    // 带宽参数
    ui->trvBandwdithDistance->setModel(mParameterSpecifiedModel);

    if (mLayerItem->hasHatmatrix())
    {
        GwmDiagnostic diagnostic = mLayerItem->diagnostic();
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

}
