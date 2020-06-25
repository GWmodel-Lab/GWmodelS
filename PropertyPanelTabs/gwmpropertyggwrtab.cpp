#include "gwmpropertyggwrtab.h"
#include "ui_gwmpropertyggwrtab.h"

#include <QStandardItemModel>

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

QMap<GwmGGWRAlgorithm::Family, QString> GwmPropertyGGWRTab::familyTypeNameDict = {
    std::make_pair(GwmGGWRAlgorithm::Family::Poisson, QStringLiteral("Poisson")),
    std::make_pair(GwmGGWRAlgorithm::Family::Binomial, QStringLiteral("Binomial"))
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
        ui->grpFTest->hide();
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
    ui->lblFamily->setText(familyTypeNameDict[mLayerItem->family()]);
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    //GLM计算结果
    GwmGLMDiagnostic GLMdiagnostic = mLayerItem->GLMdiagnostic();
    ui->lblNullDev->setText(QString("%1").arg(GLMdiagnostic.NullDev, 0, 'f', 6));
    ui->lblGLMDev->setText(QString("%1").arg(GLMdiagnostic.Dev, 0, 'f', 6));
    ui->lblGLMAIC->setText(QString("%1").arg(GLMdiagnostic.AIC, 0, 'f', 6));
    ui->lblGLMAICc->setText(QString("%1").arg(GLMdiagnostic.AICc, 0, 'f', 6));
    ui->lblGLMRSS->setText(QString("%1").arg(GLMdiagnostic.RSquare, 0, 'f', 6));

    if (mLayerItem->hatmatrix())
    {
        GwmGGWRDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblGwDev->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
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
