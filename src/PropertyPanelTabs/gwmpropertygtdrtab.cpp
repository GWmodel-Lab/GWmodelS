#include "gwmpropertygtdrtab.h"
#include "ui_gwmpropertygtdrtab.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>

#include <armadillo>

#include <QStandardItemModel>
#include "TaskThread/gwmgtdrtaskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"

QMap<GwmBandwidthWeight::KernelFunctionType, QString> GwmPropertyGTDRTab::kernelFunctionNameDict = {
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Gaussian, QStringLiteral("Gaussian")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Exponential, QStringLiteral("Exponential")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Bisquare, QStringLiteral("Bisquare")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Tricube, QStringLiteral("Tricube")),
    std::make_pair(GwmBandwidthWeight::KernelFunctionType::Boxcar, QStringLiteral("Boxcar"))
};

QMap<bool, QString> GwmPropertyGTDRTab::bandwidthTypeNameDict = {
    std::make_pair(true, QStringLiteral("Adaptive")),
    std::make_pair(false, QStringLiteral("Fixed:"))
};

GwmPropertyGTDRTab::GwmPropertyGTDRTab(QWidget *parent, GwmLayerGTDRItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGTDRTab),
    mLayerItem(item)
{
    ui->setupUi(this);
}

GwmPropertyGTDRTab::~GwmPropertyGTDRTab()
{
    delete ui;
}

void GwmPropertyGTDRTab::updateUI()
{
    if (!mLayerItem)
        return;
    GwmBandwidthWeight weight = mLayerItem->bandwidth();
    QList<GwmBandwidthWeight*> weights = mLayerItem->bandwidths();
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));

    ui->label->hide();
    ui->lblBandwidthType->hide();
    ui->lblKernelFunction->hide();
    ui->lblBandwidthSize->hide();

    if (mLayerItem->isBandwidthOptimizationSuccessful())
    {
        ui->label_5->hide();
        ui->lblBandwidthOptFail->hide();  // 优化成功，隐藏警告
    }
    else
    {
        // 只有在启用自动优化但优化失败时才显示
        if (mLayerItem->bandwidthOptimized())
        {
            ui->lblBandwidthOptFail->setText(tr("Bandwidth optimization failed, using initial bandwidths."));
            ui->lblBandwidthOptFail->show();  // 优化失败，显示警告
        }
        else
        {
            ui->label_5->hide();
            ui->lblBandwidthOptFail->hide();  // 没有优化，隐藏标签
        }
    }

    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Euclidean distance metric is used."));
    }

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
    }else{
        ui->grpDiagnostic->hide();
    }

    // set bandwidth parameters
    // QList<GwmBandwidthWeight*> weights = mLayerItem->bandwidths();
    QList<GwmVariable> indepVars = mLayerItem->indepVar();
    QList<GwmVariable> weightingVars = mLayerItem->weightingVar();
    int nDims = weights.size();

    if (nDims > 0)
    {
        // 设置表格行数和列数
        ui->tbwBandwidthParameters->setRowCount(nDims);
        ui->tbwBandwidthParameters->setColumnCount(3);
        
        // 设置水平滚动模式
        ui->tbwBandwidthParameters->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        
        // 设置表头
        QStringList headers = QStringList() 
            << tr("Dimen.") 
            << tr("Bandwidth") 
            << tr("Kernel");
        ui->tbwBandwidthParameters->setHorizontalHeaderLabels(headers);
        
        // 获取带宽类型（所有维度应该使用相同的类型）
        bool isAdaptive = weights[0] ? weights[0]->adaptive() : false;
        
        // 填充表格数据
        for (int i = 0; i < nDims; ++i)
        {
            auto* bw = weights[i];
            if (!bw)
                continue;
            
            // 第1列：权重维度
            QString varName;
            if (i < weightingVars.size())
            {
                varName = weightingVars[i].name;
            }
            else
            {
                varName = QString("Dim_%1").arg(i);
            }
            QTableWidgetItem* nameItem = new QTableWidgetItem(varName);
            nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwBandwidthParameters->setItem(i, 0, nameItem);
            
            // 第2列：带宽值
            QString bandwidthText;
            if (isAdaptive)
            {
                // Adaptive 模式：显示为整数（邻居数量）
                bandwidthText = QString("%1").arg(int(bw->bandwidth()));
            }
            else
            {
                // Fixed 模式：显示为浮点数，保留2位小数
                bandwidthText = QString("%1").arg(bw->bandwidth(), 0, 'f', 2);
            }
            QTableWidgetItem* bandwidthItem = new QTableWidgetItem(bandwidthText);
            bandwidthItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            bandwidthItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwBandwidthParameters->setItem(i, 1, bandwidthItem);
            
            // 第3列：核函数
            QString kernelName = GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(bw->kernel());
            QTableWidgetItem* kernelItem = new QTableWidgetItem(kernelName);
            kernelItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwBandwidthParameters->setItem(i, 2, kernelItem);
        }
        
        // 调整列宽以适应内容
        ui->tbwBandwidthParameters->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        
        // 可选：设置列宽策略，确保表格美观
        // 如果内容太长，可以设置最小宽度
        ui->tbwBandwidthParameters->setColumnWidth(0, 150);  // 自变量名称列
        ui->tbwBandwidthParameters->setColumnWidth(1, 100);  // 带宽值列
        ui->tbwBandwidthParameters->setColumnWidth(2, 120); // 核函数列
    }
    else
    {
        // 如果没有带宽数据，清空表格
        ui->tbwBandwidthParameters->setRowCount(0);
        ui->tbwBandwidthParameters->setColumnCount(3);
        QStringList headers = QStringList() 
            << tr("Dimen.") 
            << tr("Bandwidth") 
            << tr("Kernel");
        ui->tbwBandwidthParameters->setHorizontalHeaderLabels(headers);
    }

    // 计算四分位数
    //QList<GwmVariable> indepVars = mLayerItem->indepVar();
    const mat& betas = mLayerItem->betas();

    ui->tbwCoefficient->setRowCount(int(betas.n_cols));  // 行数 = 系数列数（Intercept + 各自变量）
    ui->tbwCoefficient->setColumnCount(6);
    ui->tbwCoefficient->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwCoefficient->setHorizontalHeaderLabels(headers);

    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = (r == 0) ? QStringLiteral("Intercept") : indepVars[int(r - 1)].name;
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwCoefficient->setItem(int(r), 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwCoefficient->setItem(int(r), c + 1, quantileItem);
        }
    }
    ui->tbwCoefficient->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    GwmGTDRTaskThread::CreateResultLayerData data = mLayerItem->resultlist();
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
