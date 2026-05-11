#ifndef GWMPROPERTYSWIMTAB_H
#define GWMPROPERTYSWIMTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>
#include <QLabel>
#include <limits>

#include "TaskThread/gwmswimtaskthread.h"

namespace Ui {
class GwmPropertySWIMTab;
}

class GwmPlot;

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
    struct OlsResult
    {
        QStringList names;
        QVector<double> estimates;
        QVector<double> stdErrors;
        QVector<double> tValues;
        double rss = std::numeric_limits<double>::quiet_NaN();
        double aic = std::numeric_limits<double>::quiet_NaN();
        double aicc = std::numeric_limits<double>::quiet_NaN();
        double rSquared = std::numeric_limits<double>::quiet_NaN();
        double adjRSquared = std::numeric_limits<double>::quiet_NaN();
        bool valid = false;
    };

    Ui::GwmPropertySWIMTab *ui;
    GwmSWIMTaskThread* mTaskThread = nullptr;

    QString mFilePath;
    OlsResult mCachedOlsResult;
    bool mHasCachedOls = false;
    GwmPlot* mBandwidthPlot = nullptr;

    void displayGlobalDiagnostics();
    void populateGlobalCoefficients();
    void displaySwimCalibrationInfo();
    void populateSwimCoefficients();
    void displaySwimDiagnostics();
    void updateBandwidthSelectionView();

    void setLabelText(QLabel* label, const QString& text);
    void setupTableHeaders(QTableWidget* table,
                           const QStringList& headers);
    void populateEmptyTableMessage(QTableWidget* table,
                                   const QString& message);
    void adjustTableToContents(QTableWidget* table) const;
    QString kernelDescription() const;
    QString distanceDescription() const;
    QString focusTypeDescription() const;
    OlsResult computeGlobalOls(bool& ok) const;
    QString formatNumber(double value, int precision = 4) const;
    double calculateQuantile(const QVector<double>& sortedValues, double quantile) const;
};

#endif // GWMPROPERTYSWIMTAB_H
