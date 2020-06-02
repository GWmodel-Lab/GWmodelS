#include "gwmpropertyggwrtab.h"
#include "ui_gwmpropertyggwrtab.h"

#include <QStandardItemModel>
#include "TaskThread/gwmbandwidthselecttaskthread.h"

QMap<GwmGWRTaskThread::KernelFunction, QString> GwmPropertyGGWRTab::kernelFunctionNameDict = {
    std::make_pair(GwmGWRTaskThread::KernelFunction::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Boxcar, QStringLiteral("Boxcar"))
};

QMap<GwmGWRTaskThread::BandwidthType, QString> GwmPropertyGGWRTab::bandwidthTypeNameDict = {
    std::make_pair(GwmGWRTaskThread::BandwidthType::Adaptive, QStringLiteral("Adaptive bandwidth:")),
    std::make_pair(GwmGWRTaskThread::BandwidthType::Fixed, QStringLiteral("Fixed bandwidth:"))
};

GwmPropertyGGWRTab::GwmPropertyGGWRTab(QWidget *parent,GwmLayerGGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGGWRTab),
    mLayerItem(item),
    mBandwidthSelPlot(nullptr)
{
    ui->setupUi(this);
    if (item)
    {
        if (item->getIsBandwidthOptimized())
        {
            mBandwidthSelPlot = new GwmPlot();
            ui->grpBwSelView->layout()->addWidget(mBandwidthSelPlot);
//            ui->grpBwSelView->hide();
        }
        else
        {
            ui->grpBwSelView->hide();
        }
        if (!item->getHasHatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
        if (!item->getHasFTest())
        {
            ui->grpFTest->hide();
        }
    }
}

GwmPropertyGGWRTab::~GwmPropertyGGWRTab()
{
    delete ui;
}

void GwmPropertyGGWRTab::updateUI()
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
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    if (mLayerItem->getHasHatmatrix())
    {
        GwmGGWRDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblGwDev->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
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
    GwmPropertyGWRTabCalcTread* thread = new GwmPropertyGWRTabCalcTread(mLayerItem);
    connect(thread, &QThread::finished, this, [=]() {
        QList<GwmQuartiles> quartiles = thread->quartiles();
        setQuartiles(0, QStringLiteral("Intercept"), quartiles[0]);
        QgsVectorLayer* layer = mLayerItem->layer();
        for (int i = 0; i < indepVarsIndex.size(); i++)
        {
            int r = i + 1;
            setQuartiles(r, indepVars[i]->attributeName(), quartiles[r]);
        }
        delete thread;
    });
    thread->start();

    if(mLayerItem->getIsBandwidthOptimized())
    {
        QMap<double, double> bwScores = mLayerItem->getBandwidthSelScores();
        QList<QVariant> plotData;
        for (auto i = bwScores.constBegin(); i != bwScores.constEnd(); i++)
        {
            plotData.append(QVariant(QPointF(i.key(), i.value())));
        }
        GwmBandwidthSelectTaskThread::plotBandwidthResult(plotData, mBandwidthSelPlot);
    }

    // F检验结果
    QList<GwmFTestResult> fTestResults = mLayerItem->getFTestResults();
    if (mLayerItem->getHasFTest() && fTestResults.size() > 0)
    {
        QStandardItemModel* model = new QStandardItemModel(4, 5);
        model->setHorizontalHeaderLabels(
                    QStringList() << ""
                    << tr("Statistics")
                    << tr("Numerator DF")
                    << tr("Denominator DF")
                    << QStringLiteral("Pr(>)"));
        ui->trvFTest->setModel(model);
        GwmFTestResult f1 = fTestResults.takeFirst(),
                f2 = fTestResults.takeFirst(),
                f4 = fTestResults.takeFirst();
        QList<GwmFTestResult> f3 = fTestResults;
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
            QString name = i == 0 ? tr("Intercept") : indepVars[i - 1]->attributeName();
            f3Item->appendRow(new QStandardItem(name));
            f3Item->setChild(i, 1, new QStandardItem(QString("%1").arg(f3[i].s, 0, 'f', 4)));
            f3Item->setChild(i, 2, new QStandardItem(QString("%1").arg(f3[i].df1, 0, 'f', 4)));
            f3Item->setChild(i, 3, new QStandardItem(QString("%1").arg(f3[i].df2, 0, 'f', 4)));
            f3Item->setChild(i, 4, new QStandardItem(QString("%1").arg(f3[i].p, 0, 'f', 4)));
        }
    }
}

void GwmPropertyGGWRTab::setQuartiles(const int row, QString name, const GwmQuartiles &quartiles)
{
    ui->tbwCoefficient->setItem(row, 0, new QTableWidgetItem(name));
    QTableWidgetItem* minItem = new QTableWidgetItem(QString("%1").arg(quartiles.min, 0, 'f', 3));
    QTableWidgetItem* firstItem = new QTableWidgetItem(QString("%1").arg(quartiles.first, 0, 'f', 3));
    QTableWidgetItem* medianItem = new QTableWidgetItem(QString("%1").arg(quartiles.median, 0, 'f', 3));
    QTableWidgetItem* thirdItem = new QTableWidgetItem(QString("%1").arg(quartiles.third, 0, 'f', 3));
    QTableWidgetItem* maxItem = new QTableWidgetItem(QString("%1").arg(quartiles.max, 0, 'f', 3));
    minItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    firstItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    medianItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    thirdItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tbwCoefficient->setItem(row, 1, minItem);
    ui->tbwCoefficient->setItem(row, 2, firstItem);
    ui->tbwCoefficient->setItem(row, 3, medianItem);
    ui->tbwCoefficient->setItem(row, 4, thirdItem);
    ui->tbwCoefficient->setItem(row, 5, maxItem);

}
