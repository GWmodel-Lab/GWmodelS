#include "gwmpropertygwdatab.h"
#include "ui_gwmpropertygwdatab.h"
#include "Model/gwmlayergwdaitem.h"  // 添加头文件
#include "SpatialWeight/gwmbandwidthweight.h"  // 如果需要显示带宽信息

#include <QMessageBox>
#include <QTableWidget>
#include <QListWidget>
#include <QDebug>
#include <QVariant>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QModelIndex>
#include <QHeaderView>
#include <QMap>

#include <QStandardItemModel>

GwmPropertyGWDATab::GwmPropertyGWDATab(QWidget *parent, GwmLayerGWDAItem *item) :
    QWidget(parent),  // 改为 QWidget
    ui(new Ui::GwmPropertyGWDATab),
    mLayerItem(item)  // 初始化成员变量
{
    ui->setupUi(this);
}

GwmPropertyGWDATab::~GwmPropertyGWDATab()
{
    delete ui;
}

void GwmPropertyGWDATab::updateUI()
{
    if (!mLayerItem)
        return;
    
    // 在这里添加更新 UI 的代码
    // 可以参考 gwmpropertygtdrtab.cpp 的实现方式
    // 例如显示：
    // - 数据点数量
    // - 带宽信息
    // - 正确率
    // - 分组变量和独立变量信息
    // - 算法参数（isWqda, hasCov, hasMean, hasPrior）
    
    // 示例代码（根据你的 UI 设计调整）：
    // ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    // ui->lblCorrectRate->setText(QString("%1").arg(mLayerItem->correctRate(), 0, 'f', 6));
    // GwmBandwidthWeight weight = mLayerItem->weight();
    // ui->lblBandwidthSize->setText(QString("%1").arg(weight.bandwidth()));
    // 等等...

    // 获取带宽权重信息
    GwmBandwidthWeight weight = mLayerItem->weight();

    // 1. 填充 Kernel function
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));

    // 2. 填充 Bandwidth Type (Adaptive/Fixed)
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));

    // 3. 填充 Bandwidth Size
    if (weight.adaptive())
    {
        QString bwSizeString = QString("%1 (number of nearest neighbours)").arg(int(weight.bandwidth()));
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    else
    {
        QString bwSizeString = QString("%1").arg(weight.bandwidth(), 0, 'f', 12);
        ui->lblBandwidthSize->setText(bwSizeString);
    }

    // 4. 填充 Distance metric
    QString distanceText = tr("Euclidean distance metric is used.");
    ui->lblDistanceMetric->setText(distanceText);

    // 5. 填充 Number of data points
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    // 6. 填充 Correct Rate
    ui->lblCorrectRate->setText(QString("%1").arg(mLayerItem->correctRate(), 0, 'f', 6));

    // 7. 填充 Classification Summary (groupBox_2)
    QgsVectorLayer* resultLayer = mLayerItem->layer();
    if (resultLayer)
    {
        // 统计每个类别的样本数
        QMap<QString, int> classCount;
        QgsFeatureIterator iterator = resultLayer->getFeatures();
        QgsFeature feature;
        int fieldIndex = resultLayer->fields().indexFromName("group_pred");

        if (fieldIndex >= 0)
        {
            while (iterator.nextFeature(feature))
            {
                QString className = feature.attribute(fieldIndex).toString();
                classCount[className]++;
            }

            // 设置表格
            int nClasses = classCount.size();
            ui->tableWidget->setRowCount(nClasses);
            ui->tableWidget->setColumnCount(2);
            ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            QStringList headers = QStringList() << tr("Class Name") << tr("Count");
            ui->tableWidget->setHorizontalHeaderLabels(headers);

            // 填充数据
            int row = 0;
            for (auto it = classCount.begin(); it != classCount.end(); ++it, ++row)
            {
                // 第一列：类名
                QTableWidgetItem* classNameItem = new QTableWidgetItem(it.key());
                classNameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                ui->tableWidget->setItem(row, 0, classNameItem);

                // 第二列：样本数
                QTableWidgetItem* countItem = new QTableWidgetItem(QString("%1").arg(it.value()));
                countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                countItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                ui->tableWidget->setItem(row, 1, countItem);
            }

            // 调整列宽
            ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

            // 8. 填充 Log Probability Summary (groupBox_3)
            ui->tableWidget_2->setRowCount(nClasses);
            ui->tableWidget_2->setColumnCount(6);
            ui->tableWidget_2->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            QStringList logpHeaders = QStringList() << tr("Class Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
            ui->tableWidget_2->setHorizontalHeaderLabels(logpHeaders);

            // 9. 填充 Probability Summary (groupBox_4)
            ui->tableWidget_3->setRowCount(nClasses);
            ui->tableWidget_3->setColumnCount(6);
            ui->tableWidget_3->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            QStringList probsHeaders = QStringList() << tr("Class Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
            ui->tableWidget_3->setHorizontalHeaderLabels(probsHeaders);

            // 使用 Armadillo 计算四分位数
            using namespace arma;
            const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };

            row = 0;
            for (auto it = classCount.begin(); it != classCount.end(); ++it, ++row)
            {
                QString className = it.key();

                // 读取 logp 字段
                QString logpFieldName = QString("logp_%1").arg(className);
                int logpFieldIndex = resultLayer->fields().indexFromName(logpFieldName);

                // 读取 probs 字段
                QString probsFieldName = QString("probs_%1").arg(className);
                int probsFieldIndex = resultLayer->fields().indexFromName(probsFieldName);

                if (logpFieldIndex >= 0)
                {
                    // 收集 logp 值
                    QgsFeatureIterator logpIterator = resultLayer->getFeatures();
                    QgsFeature logpFeature;
                    QList<double> logpValues;

                    while (logpIterator.nextFeature(logpFeature))
                    {
                        QVariant value = logpFeature.attribute(logpFieldIndex);
                        if (value.isValid() && value.canConvert<double>())
                        {
                            logpValues.append(value.toDouble());
                        }
                    }

                    // 转换为 arma::vec 并计算四分位数
                    if (!logpValues.isEmpty())
                    {
                        vec logpVec(logpValues.size());
                        for (int i = 0; i < logpValues.size(); i++)
                        {
                            logpVec(i) = logpValues[i];
                        }

                        vec q = quantile(logpVec, p);

                        // 填充类名
                        QTableWidgetItem* nameItem = new QTableWidgetItem(className);
                        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                        ui->tableWidget_2->setItem(row, 0, nameItem);

                        // 填充统计值
                        for (int c = 0; c < 5; c++)
                        {
                            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
                            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                            ui->tableWidget_2->setItem(row, c + 1, quantileItem);
                        }
                    }
                }

                if (probsFieldIndex >= 0)
                {
                    // 收集 probs 值
                    QgsFeatureIterator probsIterator = resultLayer->getFeatures();
                    QgsFeature probsFeature;
                    QList<double> probsValues;

                    while (probsIterator.nextFeature(probsFeature))
                    {
                        QVariant value = probsFeature.attribute(probsFieldIndex);
                        if (value.isValid() && value.canConvert<double>())
                        {
                            probsValues.append(value.toDouble());
                        }
                    }

                    // 转换为 arma::vec 并计算四分位数
                    if (!probsValues.isEmpty())
                    {
                        vec probsVec(probsValues.size());
                        for (int i = 0; i < probsValues.size(); i++)
                        {
                            probsVec(i) = probsValues[i];
                        }

                        vec q = quantile(probsVec, p);

                        // 填充类名
                        QTableWidgetItem* nameItem = new QTableWidgetItem(className);
                        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                        ui->tableWidget_3->setItem(row, 0, nameItem);

                        // 填充统计值
                        for (int c = 0; c < 5; c++)
                        {
                            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
                            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                            ui->tableWidget_3->setItem(row, c + 1, quantileItem);
                        }
                    }
                }
            }

            // 调整列宽
            ui->tableWidget_2->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
            ui->tableWidget_3->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        }
    }
}
