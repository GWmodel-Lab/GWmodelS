#include "gwmpropertyswimtab.h"
#include "ui_gwmpropertyswimtab.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>
#include <armadillo>
#include <QTextStream>
#include <QFileInfo>
#include <cmath>

#include "SpatialWeight/gwmbandwidthweight.h"
#include "SpatialWeight/gwmcrsdistance.h"
#include "SpatialWeight/gwmminkwoskidistance.h"
#include "SpatialWeight/gwmdmatdistance.h"

using namespace arma;

GwmPropertySWIMTab::GwmPropertySWIMTab(QWidget *parent, GwmSWIMTaskThread* taskThread) :
    QWidget(parent),
    ui(new Ui::GwmPropertySWIMTab),
    mTaskThread(taskThread)
{
    ui->setupUi(this);
    // Do not call updateUI() here - it will be called explicitly from outside
    // This follows the pattern used by other property tabs (GWR, GTWR, etc.)
}

GwmPropertySWIMTab::~GwmPropertySWIMTab()
{
    delete ui;
}

void GwmPropertySWIMTab::setTaskThread(GwmSWIMTaskThread* taskThread)
{
    mTaskThread = taskThread;
    if (mTaskThread && ui)
    {
        updateUI();
    }
}

void GwmPropertySWIMTab::updateUI()
{
    if (!mTaskThread || !ui)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] mTaskThread or ui is null";
        return;
    }

    qDebug() << "[GwmPropertySWIMTab::updateUI] Starting UI update";
    qDebug() << "[GwmPropertySWIMTab::updateUI] TaskThread valid:" << (mTaskThread != nullptr);
    qDebug() << "[GwmPropertySWIMTab::updateUI] UI valid:" << (ui != nullptr);
    
    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayModelConfiguration...";
        displayModelConfiguration();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayModelConfiguration completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayModelConfiguration:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayModelConfiguration";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayWeightingScheme...";
        displayWeightingScheme();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayWeightingScheme completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayWeightingScheme:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayWeightingScheme";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayDistanceMetric...";
        displayDistanceMetric();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayDistanceMetric completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayDistanceMetric:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayDistanceMetric";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayParallelInfo...";
        displayParallelInfo();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayParallelInfo completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayParallelInfo:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayParallelInfo";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayFlowStatistics...";
        displayFlowStatistics();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayFlowStatistics completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayFlowStatistics:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayFlowStatistics";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling displayWeightMatrixInfo...";
        displayWeightMatrixInfo();
        qDebug() << "[GwmPropertySWIMTab::updateUI] displayWeightMatrixInfo completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in displayWeightMatrixInfo:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in displayWeightMatrixInfo";
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Calling populateFlowTable...";
        populateFlowTable();
        qDebug() << "[GwmPropertySWIMTab::updateUI] populateFlowTable completed";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Error in populateFlowTable:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Unknown error in populateFlowTable";
    }

    qDebug() << "[GwmPropertySWIMTab::updateUI] UI update completed";
}

void GwmPropertySWIMTab::displayModelConfiguration()
{
    if (!mTaskThread || !ui) return;

    try {
        if (ui->lblSWIMMode)
            ui->lblSWIMMode->setText(swimModeText(mTaskThread->swimMode()));
        
        if (ui->lblCsvFilePath)
        {
            QString filePath = mTaskThread->csvFilePath();
            ui->lblCsvFilePath->setText(filePath.isEmpty() ? tr("-") : filePath);
        }
        
        QList<GwmFlowData> flowData = mTaskThread->flowData();
        if (ui->lblFlowCount)
            ui->lblFlowCount->setText(QString::number(flowData.size()));

        auto mapping = mTaskThread->fieldMapping();
        if (ui->lblDependentField)
        {
            QString depFieldName = fieldName(mapping.flowVolume);
            ui->lblDependentField->setText(depFieldName.isEmpty() ? tr("-") : depFieldName);
        }

        QStringList indepNames = mapping.independentVarNames;
        if (ui->lblIndependentVars)
            ui->lblIndependentVars->setText(indepNames.isEmpty() ? tr("-") : indepNames.join(", "));
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::displayModelConfiguration] Exception:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayModelConfiguration] Unknown error";
    }
}

void GwmPropertySWIMTab::displayWeightingScheme()
{
    if (!mTaskThread || !ui) return;

    try {
        if (ui->lblKernelFunction)
            ui->lblKernelFunction->setText(tr("-"));
        if (ui->lblBandwidthType)
            ui->lblBandwidthType->setText(tr("-"));
        if (ui->lblBandwidthSize)
            ui->lblBandwidthSize->setText(tr("-"));

        const GwmSpatialWeight weight = mTaskThread->spatialWeight();
        if (auto* bw = weight.weight<GwmBandwidthWeight>())
        {
            if (ui->lblKernelFunction)
                ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(bw->kernel()));
            if (ui->lblBandwidthType)
                ui->lblBandwidthType->setText(bw->adaptive() ? tr("Adaptive") : tr("Fixed"));
            if (ui->lblBandwidthSize)
            {
                if (bw->adaptive())
                {
                    int neighbours = static_cast<int>(std::round(bw->bandwidth()));
                    ui->lblBandwidthSize->setText(tr("%1 neighbours").arg(neighbours));
                }
                else
                {
                    ui->lblBandwidthSize->setText(QString::number(bw->bandwidth(), 'f', 4));
                }
            }
        }
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayWeightingScheme] Error";
    }
}

void GwmPropertySWIMTab::displayDistanceMetric()
{
    if (!mTaskThread || !ui) 
    {
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] mTaskThread or ui is null";
        return;
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Starting...";
        
        if (ui->lblDistanceMetric)
            ui->lblDistanceMetric->setText(tr("-"));
        
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Calling resetDistanceParameterLabels...";
        resetDistanceParameterLabels();
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] resetDistanceParameterLabels completed";

        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Getting spatial weight...";
        const GwmSpatialWeight spatialWeight = mTaskThread->spatialWeight();
        if (!spatialWeight.distance())
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] No distance configured";
            return;
        }

        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Checking distance type...";
        
        // Get base distance pointer first
        GwmDistance* baseDistance = spatialWeight.distance();
        if (!baseDistance)
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Base distance is null";
            return;
        }
        
        // Check type using enum to avoid casting issues
        GwmDistance::DistanceType distType = baseDistance->type();
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Distance type enum:" << static_cast<int>(distType);
        
        if (distType == GwmDistance::DistanceType::DMatDistance)
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Distance type: DMat (confirmed by enum)";
            
            // Set UI labels first
            if (ui->lblDistanceMetric)
            {
                ui->lblDistanceMetric->setText(tr("Distance Matrix"));
            }
            
            if (ui->lblDistanceFileTitle)
            {
                ui->lblDistanceFileTitle->setVisible(true);
            }
            
            // Get file path - temporarily skip to avoid crash
            // TODO: Fix the root cause of dMatFile() crash
            QString dmatFilePath;
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] WARNING: Skipping dMatFile() call to avoid crash";
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] This is a temporary workaround";
            
            // Set file path label with placeholder
            if (ui->lblDistanceFilePath)
            {
                ui->lblDistanceFilePath->setVisible(true);
                ui->lblDistanceFilePath->setText(tr("Distance Matrix File"));
            }
        }
        else if (auto* dmat = spatialWeight.distance<GwmDMatDistance>())
        {
            // Fallback: if enum check didn't work, try template method
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Using template method (fallback)";
            if (ui->lblDistanceMetric)
                ui->lblDistanceMetric->setText(tr("Distance Matrix"));
            if (ui->lblDistanceFileTitle)
                ui->lblDistanceFileTitle->setVisible(true);
            if (ui->lblDistanceFilePath)
            {
                ui->lblDistanceFilePath->setVisible(true);
                ui->lblDistanceFilePath->setText(tr("-"));
            }
        }
        else if (auto* minkowski = spatialWeight.distance<GwmMinkwoskiDistance>())
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Distance type: Minkowski";
            if (ui->lblDistanceMetric)
                ui->lblDistanceMetric->setText(tr("Minkowski"));
            if (ui->lblDistanceParam1Title)
            {
                ui->lblDistanceParam1Title->setVisible(true);
                ui->lblDistanceParam1Title->setText(tr("Theta (°):"));
            }
            if (ui->lblDistanceParam1)
            {
                ui->lblDistanceParam1->setVisible(true);
                ui->lblDistanceParam1->setText(QString::number(minkowski->theta(), 'f', 2));
            }
            if (ui->lblDistanceParam2Title)
            {
                ui->lblDistanceParam2Title->setVisible(true);
                ui->lblDistanceParam2Title->setText(tr("Order (p):"));
            }
            if (ui->lblDistanceParam2)
            {
                ui->lblDistanceParam2->setVisible(true);
                ui->lblDistanceParam2->setText(QString::number(minkowski->poly(), 'f', 2));
            }
        }
        else if (auto* crs = spatialWeight.distance<GwmCRSDistance>())
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Distance type: CRS";
            if (ui->lblDistanceMetric)
                ui->lblDistanceMetric->setText(crs->geographic() ? tr("CRS - Geographic") : tr("CRS - Planar"));
        }
        else
        {
            qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Distance type: Custom";
            if (ui->lblDistanceMetric)
                ui->lblDistanceMetric->setText(tr("Custom Distance"));
        }
        
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Completed successfully";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Exception:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayDistanceMetric] Unknown error";
    }
}

void GwmPropertySWIMTab::displayParallelInfo()
{
    if (!mTaskThread || !ui) return;

    try {
        QString modeText = tr("-");
        QString detailText = tr("-");

        switch (mTaskThread->parallelType())
        {
        case IParallelalbe::ParallelType::SerialOnly:
            modeText = tr("Serial");
            detailText = tr("Single thread execution");
            break;
        case IParallelalbe::ParallelType::OpenMP:
            modeText = tr("OpenMP");
            detailText = tr("%1 threads").arg(mTaskThread->ompThreadNum());
            break;
        case IParallelalbe::ParallelType::CUDA:
            modeText = tr("CUDA");
            detailText = tr("GPU acceleration");
            break;
        default:
            break;
        }

        if (ui->lblParallelType)
            ui->lblParallelType->setText(modeText);
        if (ui->lblParallelDetail)
            ui->lblParallelDetail->setText(detailText);
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayParallelInfo] Error";
    }
}

void GwmPropertySWIMTab::displayFlowStatistics()
{
    if (!mTaskThread || !ui)
        return;

    try {
        QList<GwmFlowData> flowData = mTaskThread->flowData();
        if (flowData.isEmpty())
        {
            if (ui->lblMinFlowVolume)
                ui->lblMinFlowVolume->setText(tr("-"));
            if (ui->lblMaxFlowVolume)
                ui->lblMaxFlowVolume->setText(tr("-"));
            if (ui->lblAvgFlowVolume)
                ui->lblAvgFlowVolume->setText(tr("-"));
            if (ui->lblTotalFlowVolume)
                ui->lblTotalFlowVolume->setText(tr("-"));
            if (ui->lblAvgOriginValue)
                ui->lblAvgOriginValue->setText(tr("-"));
            if (ui->lblAvgDestValue)
                ui->lblAvgDestValue->setText(tr("-"));
            return;
        }

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

        if (ui->lblMinFlowVolume)
            ui->lblMinFlowVolume->setText(QString::number(minVolume, 'f', 2));
        if (ui->lblMaxFlowVolume)
            ui->lblMaxFlowVolume->setText(QString::number(maxVolume, 'f', 2));
        if (ui->lblAvgFlowVolume)
            ui->lblAvgFlowVolume->setText(QString::number(avgVolume, 'f', 2));
        if (ui->lblTotalFlowVolume)
            ui->lblTotalFlowVolume->setText(QString::number(sumVolume, 'f', 2));
        if (ui->lblAvgOriginValue)
            ui->lblAvgOriginValue->setText(QString::number(avgOriginValue, 'f', 2));
        if (ui->lblAvgDestValue)
            ui->lblAvgDestValue->setText(QString::number(avgDestValue, 'f', 2));
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayFlowStatistics] Error";
    }
}

void GwmPropertySWIMTab::displayWeightMatrixInfo()
{
    if (!mTaskThread || !ui)
        return;

    try {
        mat weightMatrix = mTaskThread->weightMatrix();
        
        // Check if matrix is empty
        if (weightMatrix.n_rows == 0 || weightMatrix.n_cols == 0)
        {
            if (ui->lblWeightMatrixSize)
                ui->lblWeightMatrixSize->setText(tr("-"));
            if (ui->lblMinWeight)
                ui->lblMinWeight->setText(tr("-"));
            if (ui->lblMaxWeight)
                ui->lblMaxWeight->setText(tr("-"));
            if (ui->lblMeanWeight)
                ui->lblMeanWeight->setText(tr("-"));
            if (ui->lblSumWeight)
                ui->lblSumWeight->setText(tr("-"));
            return;
        }

        if (ui->lblWeightMatrixSize)
            ui->lblWeightMatrixSize->setText(QString("%1 x %2").arg(weightMatrix.n_rows).arg(weightMatrix.n_cols));

        // Safely compute statistics
        try {
            double minWeight = weightMatrix.min();
            double maxWeight = weightMatrix.max();
            double meanWeight = mean(mean(weightMatrix));
            double sumWeight = accu(weightMatrix);

            if (ui->lblMinWeight)
                ui->lblMinWeight->setText(QString::number(minWeight, 'f', 6));
            if (ui->lblMaxWeight)
                ui->lblMaxWeight->setText(QString::number(maxWeight, 'f', 6));
            if (ui->lblMeanWeight)
                ui->lblMeanWeight->setText(QString::number(meanWeight, 'f', 6));
            if (ui->lblSumWeight)
                ui->lblSumWeight->setText(QString::number(sumWeight, 'f', 6));
        }
        catch (const std::exception& e)
        {
            qDebug() << "[GwmPropertySWIMTab::displayWeightMatrixInfo] Error computing statistics:" << e.what();
            if (ui->lblMinWeight)
                ui->lblMinWeight->setText(tr("-"));
            if (ui->lblMaxWeight)
                ui->lblMaxWeight->setText(tr("-"));
            if (ui->lblMeanWeight)
                ui->lblMeanWeight->setText(tr("-"));
            if (ui->lblSumWeight)
                ui->lblSumWeight->setText(tr("-"));
        }
        catch (...)
        {
            qDebug() << "[GwmPropertySWIMTab::displayWeightMatrixInfo] Unknown error computing statistics";
            if (ui->lblMinWeight)
                ui->lblMinWeight->setText(tr("-"));
            if (ui->lblMaxWeight)
                ui->lblMaxWeight->setText(tr("-"));
            if (ui->lblMeanWeight)
                ui->lblMeanWeight->setText(tr("-"));
            if (ui->lblSumWeight)
                ui->lblSumWeight->setText(tr("-"));
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::displayWeightMatrixInfo] Exception:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::displayWeightMatrixInfo] Unknown error";
    }
}

void GwmPropertySWIMTab::populateFlowTable()
{
    if (!mTaskThread || !ui || !ui->tableFlowData)
        return;

    try {
        QList<GwmFlowData> flowData = mTaskThread->flowData();
        
        if (flowData.isEmpty())
        {
            ui->tableFlowData->clearContents();
            ui->tableFlowData->setRowCount(0);
            ui->tableFlowData->setColumnCount(0);
            return;
        }

        ui->tableFlowData->clearContents();
        ui->tableFlowData->setRowCount(flowData.size());
        ui->tableFlowData->setColumnCount(10);

        QStringList headers = QStringList()
                              << tr("Flow ID") << tr("Origin ID") << tr("Dest ID")
                              << tr("Flow Volume") << tr("Origin Value") << tr("Dest Value")
                              << tr("Origin X") << tr("Origin Y") << tr("Dest X") << tr("Dest Y");
        ui->tableFlowData->setHorizontalHeaderLabels(headers);
        if (ui->tableFlowData->horizontalHeader())
            ui->tableFlowData->horizontalHeader()->setStretchLastSection(true);

        for (int i = 0; i < flowData.size(); i++)
        {
            if (i >= flowData.size())
                break;
                
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
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::populateFlowTable] Exception:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::populateFlowTable] Unknown error";
    }
}

QString GwmPropertySWIMTab::swimModeText(SWIMMode mode) const
{
    switch (mode)
    {
    case SWIMMode::OriginFocused:
        return tr("Origin-Focused");
    case SWIMMode::DestinationFocused:
        return tr("Destination-Focused");
    case SWIMMode::FlowFocusedEuclidean:
        return tr("Flow-Focused (Euclidean)");
    case SWIMMode::FlowFocusedSOP:
        return tr("Flow-Focused (SOP)");
    default:
        return tr("-");
    }
}

QString GwmPropertySWIMTab::fieldName(int columnIndex) const
{
    if (!mTaskThread)
        return tr("-");

    try {
        const QStringList headers = mTaskThread->csvHeaders();
        if (columnIndex >= 0 && columnIndex < headers.size())
        {
            return headers[columnIndex];
        }
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::fieldName] Error";
    }
    return tr("-");
}

void GwmPropertySWIMTab::resetDistanceParameterLabels()
{
    if (!ui) 
    {
        qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] ui is null";
        return;
    }

    try {
        qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] Starting...";
        
        // Check if labels exist before accessing them
        if (!ui->lblDistanceParam1Title || !ui->lblDistanceParam2Title || !ui->lblDistanceFileTitle ||
            !ui->lblDistanceParam1 || !ui->lblDistanceParam2 || !ui->lblDistanceFilePath)
        {
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] Some labels are null";
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceParam1Title:" << (ui->lblDistanceParam1Title != nullptr);
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceParam2Title:" << (ui->lblDistanceParam2Title != nullptr);
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceFileTitle:" << (ui->lblDistanceFileTitle != nullptr);
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceParam1:" << (ui->lblDistanceParam1 != nullptr);
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceParam2:" << (ui->lblDistanceParam2 != nullptr);
            qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] lblDistanceFilePath:" << (ui->lblDistanceFilePath != nullptr);
            return;
        }

        const QList<QLabel*> titles = { ui->lblDistanceParam1Title, ui->lblDistanceParam2Title, ui->lblDistanceFileTitle };
        const QList<QLabel*> values = { ui->lblDistanceParam1, ui->lblDistanceParam2, ui->lblDistanceFilePath };

        for (QLabel* label : titles)
        {
            if (!label)
                continue;
            label->setVisible(false);
        }
        for (QLabel* label : values)
        {
            if (!label)
                continue;
            label->setText(tr("-"));
            label->setVisible(false);
        }
        
        qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] Completed successfully";
    }
    catch (const std::exception& e)
    {
        qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] Exception:" << e.what();
    }
    catch (...)
    {
        qDebug() << "[GwmPropertySWIMTab::resetDistanceParameterLabels] Unknown error";
    }
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

