#include "gwmpropertyswimtab.h"
#include "ui_gwmpropertyswimtab.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
#include <armadillo>
#include <QTextStream>

using namespace arma;

GwmPropertySWIMTab::GwmPropertySWIMTab(QWidget *parent, GwmSWIMTaskThread* taskThread) :
    QWidget(parent),
    ui(new Ui::GwmPropertySWIMTab),
    mTaskThread(taskThread)
{
    ui->setupUi(this);
    updateUI();
}

GwmPropertySWIMTab::~GwmPropertySWIMTab()
{
    delete ui;
}

void GwmPropertySWIMTab::setTaskThread(GwmSWIMTaskThread* taskThread)
{
    mTaskThread = taskThread;
    updateUI();
}

void GwmPropertySWIMTab::updateUI()
{
    if (!mTaskThread)
        return;

    // 显示SWIM模式信息
    displaySWIMModeInfo();
    
    // 显示流数据统计
    displayFlowStatistics();
    
    // 显示权重矩阵信息
    displayWeightMatrixInfo();
}

void GwmPropertySWIMTab::displaySWIMModeInfo()
{
    if (!mTaskThread)
        return;

    SWIMMode mode = mTaskThread->swimMode();
    QString modeText;
    switch (mode)
    {
    case SWIMMode::OriginFocused:
        modeText = tr("Origin-Focused SWIM (以起点为中心)");
        break;
    case SWIMMode::DestinationFocused:
        modeText = tr("Destination-Focused SWIM (以终点为中心)");
        break;
    case SWIMMode::FlowFocusedEuclidean:
        modeText = tr("Flow-Focused SWIM - Euclidean (四维欧氏距离)");
        break;
    case SWIMMode::FlowFocusedSOP:
        modeText = tr("Flow-Focused SWIM - SOP (轨迹距离)");
        break;
    }
    ui->lblSWIMMode->setText(modeText);
    
    // 显示CSV文件路径
    ui->lblCsvFilePath->setText(mTaskThread->csvFilePath());
    
    // 显示流数据数量
    QList<GwmFlowData> flowData = mTaskThread->flowData();
    ui->lblFlowCount->setText(QString::number(flowData.size()));
}

void GwmPropertySWIMTab::displayFlowStatistics()
{
    if (!mTaskThread)
        return;

    QList<GwmFlowData> flowData = mTaskThread->flowData();
    if (flowData.isEmpty())
        return;

    // 计算流量的统计信息
    double minVolume = flowData[0].flow_volume;
    double maxVolume = flowData[0].flow_volume;
    double sumVolume = 0.0;
    double sumOriginValue = 0.0;
    double sumDestValue = 0.0;

    for (const GwmFlowData& flow : flowData)
    {
        if (flow.flow_volume < minVolume) minVolume = flow.flow_volume;
        if (flow.flow_volume > maxVolume) maxVolume = flow.flow_volume;
        sumVolume += flow.flow_volume;
        sumOriginValue += flow.origin_value;
        sumDestValue += flow.dest_value;
    }

    double avgVolume = sumVolume / flowData.size();
    double avgOriginValue = sumOriginValue / flowData.size();
    double avgDestValue = sumDestValue / flowData.size();

    // 显示统计信息
    ui->lblMinFlowVolume->setText(QString::number(minVolume, 'f', 2));
    ui->lblMaxFlowVolume->setText(QString::number(maxVolume, 'f', 2));
    ui->lblAvgFlowVolume->setText(QString::number(avgVolume, 'f', 2));
    ui->lblTotalFlowVolume->setText(QString::number(sumVolume, 'f', 2));
    ui->lblAvgOriginValue->setText(QString::number(avgOriginValue, 'f', 2));
    ui->lblAvgDestValue->setText(QString::number(avgDestValue, 'f', 2));

    // 填充流数据表格
    ui->tableFlowData->setRowCount(flowData.size());
    ui->tableFlowData->setColumnCount(10);
    QStringList headers = QStringList() 
        << tr("Flow ID") << tr("Origin ID") << tr("Dest ID") 
        << tr("Flow Volume") << tr("Origin Value") << tr("Dest Value")
        << tr("Origin X") << tr("Origin Y") << tr("Dest X") << tr("Dest Y");
    ui->tableFlowData->setHorizontalHeaderLabels(headers);
    ui->tableFlowData->horizontalHeader()->setStretchLastSection(true);

    for (int i = 0; i < flowData.size(); i++)
    {
        const GwmFlowData& flow = flowData[i];
        ui->tableFlowData->setItem(i, 0, new QTableWidgetItem(QString::number(flow.flow_id)));
        ui->tableFlowData->setItem(i, 1, new QTableWidgetItem(QString::number(flow.origin_id)));
        ui->tableFlowData->setItem(i, 2, new QTableWidgetItem(QString::number(flow.dest_id)));
        ui->tableFlowData->setItem(i, 3, new QTableWidgetItem(QString::number(flow.flow_volume, 'f', 2)));
        ui->tableFlowData->setItem(i, 4, new QTableWidgetItem(QString::number(flow.origin_value, 'f', 2)));
        ui->tableFlowData->setItem(i, 5, new QTableWidgetItem(QString::number(flow.dest_value, 'f', 2)));
        ui->tableFlowData->setItem(i, 6, new QTableWidgetItem(QString::number(flow.origin_x, 'f', 6)));
        ui->tableFlowData->setItem(i, 7, new QTableWidgetItem(QString::number(flow.origin_y, 'f', 6)));
        ui->tableFlowData->setItem(i, 8, new QTableWidgetItem(QString::number(flow.dest_x, 'f', 6)));
        ui->tableFlowData->setItem(i, 9, new QTableWidgetItem(QString::number(flow.dest_y, 'f', 6)));
    }
}

void GwmPropertySWIMTab::displayWeightMatrixInfo()
{
    if (!mTaskThread)
        return;

    mat weightMatrix = mTaskThread->weightMatrix();
    if (weightMatrix.n_rows == 0 || weightMatrix.n_cols == 0)
        return;

    // 显示权重矩阵基本信息
    ui->lblWeightMatrixSize->setText(QString("%1 x %2").arg(weightMatrix.n_rows).arg(weightMatrix.n_cols));
    
    // 计算权重矩阵的统计信息
    double minWeight = weightMatrix.min();
    double maxWeight = weightMatrix.max();
    double meanWeight = mean(mean(weightMatrix));
    double sumWeight = accu(weightMatrix);

    ui->lblMinWeight->setText(QString::number(minWeight, 'f', 6));
    ui->lblMaxWeight->setText(QString::number(maxWeight, 'f', 6));
    ui->lblMeanWeight->setText(QString::number(meanWeight, 'f', 6));
    ui->lblSumWeight->setText(QString::number(sumWeight, 'f', 6));
}

void GwmPropertySWIMTab::on_btnSaveRes_clicked()
{
    if (!mTaskThread)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("保存结果"), "", tr("CSV文件 (*.csv)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    
    // 写入流数据
    QList<GwmFlowData> flowData = mTaskThread->flowData();
    out << "flow_id,origin_id,dest_id,flow_volume,origin_value,dest_value,origin_x,origin_y,dest_x,dest_y\n";
    for (const GwmFlowData& flow : flowData)
    {
        out << flow.flow_id << ","
            << flow.origin_id << ","
            << flow.dest_id << ","
            << flow.flow_volume << ","
            << flow.origin_value << ","
            << flow.dest_value << ","
            << flow.origin_x << ","
            << flow.origin_y << ","
            << flow.dest_x << ","
            << flow.dest_y << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Results saved to: %1").arg(fileName));
}

void GwmPropertySWIMTab::on_btnExportWeightMatrix_clicked()
{
    if (!mTaskThread)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("导出权重矩阵"), "", tr("CSV文件 (*.csv)"));
    if (fileName.isEmpty())
        return;

    mat weightMatrix = mTaskThread->weightMatrix();
    if (weightMatrix.n_rows == 0 || weightMatrix.n_cols == 0)
    {
        QMessageBox::warning(this, tr("Error"), tr("Weight matrix is empty."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    
    // 写入权重矩阵（CSV格式）
    for (uword i = 0; i < weightMatrix.n_rows; i++)
    {
        for (uword j = 0; j < weightMatrix.n_cols; j++)
        {
            if (j > 0) out << ",";
            out << weightMatrix(i, j);
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Weight matrix exported to: %1").arg(fileName));
}

