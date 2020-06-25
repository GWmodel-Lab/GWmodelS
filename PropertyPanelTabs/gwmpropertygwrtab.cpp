#include "gwmpropertygwrtab.h"
#include "ui_gwmpropertygwrtab.h"

#include <armadillo>

#include "TaskThread/gwmgwrmodelselectionthread.h"
#include "TaskThread/gwmbandwidthselecttaskthread.h"

#include <QStandardItemModel>

using namespace arma;

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

GwmPropertyGWRTab::GwmPropertyGWRTab(QWidget *parent, GwmLayerBasicGWRItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWRTab),
    mLayerItem(item),
    mModelSelVarsPlot(nullptr),
    mModelSelAICsPlot(nullptr),
    mBandwidthSelPlot(nullptr)
{
    ui->setupUi(this);
    if (item)
    {
        if (item->modelOptimized())
        {
            mModelSelVarsPlot = new GwmPlot();
            mModelSelAICsPlot = new GwmPlot();
            ui->grpModelSelView->layout()->addWidget(mModelSelVarsPlot);
            ui->grpModelSelView->layout()->addWidget(mModelSelAICsPlot);
        }
        else
        {
            ui->grpModelSelView->hide();
        }
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
        if (!item->hatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
        if (!item->fTest())
        {
            ui->grpFTest->hide();
        }
    }
}

GwmPropertyGWRTab::~GwmPropertyGWRTab()
{
    delete ui;
}

void GwmPropertyGWRTab::updateUI()
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

    // 绘制可视化图标
    if (mLayerItem->modelOptimized())
    {
        IndepVarsCriterionList models = mLayerItem->modelSelModels();
        QVariant data = QVariant::fromValue(models);
        GwmIndependentVariableSelector::PlotModelOrder(data, mModelSelVarsPlot);
        GwmIndependentVariableSelector::PlotModelAICcs(data, mModelSelAICsPlot);
    }

    if(mLayerItem->bandwidthOptimized())
    {
        BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        QVariant data = QVariant::fromValue(bwScores);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }

    // F检验结果
    GwmBasicGWRAlgorithm::FTestResultPack fTestResults = mLayerItem->fTestResults();
    if (mLayerItem->fTest())
    {
        QStandardItemModel* model = new QStandardItemModel(4, 5);
        model->setHorizontalHeaderLabels(
                    QStringList() << ""
                    << tr("Statistics")
                    << tr("Numerator DF")
                    << tr("Denominator DF")
                    << QStringLiteral("Pr(>)"));
        ui->trvFTest->setModel(model);
        GwmFTestResult f1 = fTestResults.f1,
                f2 = fTestResults.f2,
                f4 = fTestResults.f4;
        QList<GwmFTestResult> f3 = fTestResults.f3;
        // F1
        model->setItem(0, 0, new QStandardItem(tr("F1 test")));
        model->setItem(0, 1, new QStandardItem(QString("%1").arg(f1.s, 0, 'f', 4)));
        model->setItem(0, 2, new QStandardItem(QString("%1").arg(f1.df1, 0, 'f', 4)));
        model->setItem(0, 3, new QStandardItem(QString("%1").arg(f1.df2, 0, 'f', 4)));
        model->setItem(0, 4, new QStandardItem(QString("%1").arg(f1.p, 0, 'f', 4)));
        // F2
        model->setItem(1, 0, new QStandardItem(tr("F2 test")));
        model->setItem(1, 1, new QStandardItem(QString("%1").arg(f2.s, 0, 'f', 4)));
        model->setItem(1, 2, new QStandardItem(QString("%1").arg(f2.df1, 0, 'f', 4)));
        model->setItem(1, 3, new QStandardItem(QString("%1").arg(f2.df2, 0, 'f', 4)));
        model->setItem(1, 4, new QStandardItem(QString("%1").arg(f2.p, 0, 'f', 4)));
        // F4
        model->setItem(3, 0, new QStandardItem(tr("F4 test")));
        model->setItem(3, 1, new QStandardItem(QString("%1").arg(f4.s, 0, 'f', 4)));
        model->setItem(3, 2, new QStandardItem(QString("%1").arg(f4.df1, 0, 'f', 4)));
        model->setItem(3, 3, new QStandardItem(QString("%1").arg(f4.df2, 0, 'f', 4)));
        model->setItem(3, 4, new QStandardItem(QString("%1").arg(f4.p, 0, 'f', 4)));
        // F3
        QStandardItem* f3Item = new QStandardItem(tr("F3 test"));
        model->setItem(2, 0, f3Item);
        for (int i = 0; i < f3.size(); i++)
        {
            QString name = i == 0 ? tr("Intercept") : indepVars[i - 1].name;
            f3Item->appendRow(new QStandardItem(name));
            f3Item->setChild(i, 1, new QStandardItem(QString("%1").arg(f3[i].s, 0, 'f', 4)));
            f3Item->setChild(i, 2, new QStandardItem(QString("%1").arg(f3[i].df1, 0, 'f', 4)));
            f3Item->setChild(i, 3, new QStandardItem(QString("%1").arg(f3[i].df2, 0, 'f', 4)));
            f3Item->setChild(i, 4, new QStandardItem(QString("%1").arg(f3[i].p, 0, 'f', 4)));
        }
    }
}
