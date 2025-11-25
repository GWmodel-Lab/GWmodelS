#ifndef GWMPROPERTYSWIMTAB_H
#define GWMPROPERTYSWIMTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "TaskThread/gwmswimtaskthread.h"

namespace Ui {
class GwmPropertySWIMTab;
}

class GwmPropertySWIMTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertySWIMTab(QWidget *parent = nullptr, GwmSWIMTaskThread* taskThread = nullptr);
    ~GwmPropertySWIMTab();

public:
    void updateUI();
    void setTaskThread(GwmSWIMTaskThread* taskThread);

private slots:
    void on_btnSaveRes_clicked();
    void on_btnExportWeightMatrix_clicked();

private:
    Ui::GwmPropertySWIMTab *ui;
    GwmSWIMTaskThread* mTaskThread = nullptr;

    QString mFilePath;

    void displayModelConfiguration();
    void displayWeightingScheme();
    void displayDistanceMetric();
    void displayParallelInfo();
    void displayFlowStatistics();
    void displayWeightMatrixInfo();
    void populateFlowTable();

    QString swimModeText(SWIMMode mode) const;
    QString fieldName(int columnIndex) const;
    void resetDistanceParameterLabels();
};

#endif // GWMPROPERTYSWIMTAB_H
