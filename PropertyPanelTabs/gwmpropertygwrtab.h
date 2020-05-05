#ifndef GWMPROPERTYGWRTAB_H
#define GWMPROPERTYGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayergwritem.h"

namespace Ui {
class GwmPropertyGWRTab;
}

struct GwmQuartiles
{
    double min = 0.0;
    double first = 0.0;
    double median = 0.0;
    double third = 0.0;
    double max = 0.0;
};

class GwmPropertyGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGWRTaskThread::KernelFunction, QString> kernelFunctionNameDict;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyGWRTab(QWidget *parent = nullptr, GwmLayerGWRItem* item = nullptr);
    ~GwmPropertyGWRTab();

private:
    Ui::GwmPropertyGWRTab *ui;
    GwmLayerGWRItem* mLayerItem;

    GwmPlot* mModelSelVarsPlot;
    GwmPlot* mModelSelAICsPlot;

    GwmPlot* mBandwidthSelPlot;

public:
    void updateUI();

private:
    void setQuartiles(const int row, QString name, const GwmQuartiles& quartiles);
};


class GwmPropertyGWRTabCalcTread : public QThread
{
    Q_OBJECT

public:
    explicit GwmPropertyGWRTabCalcTread(GwmLayerGWRItem* item);

    void run() override;

    inline double median(vec column)
    {
        int nrow = column.n_rows;
        return (column[nrow / 2] + column[(nrow - 1) / 2]) / 2;
    }

    inline double quartile(vec x, double p)
    {
        double p0 = (x.n_elem - 1) * p;
        double w = p0 - int(p0);
        return x.n_elem == 0 ? 0 : (x[(int)p0] * (1.0 - w) + x[(int)p0 + 1] * w);
    }

    QList<GwmQuartiles> quartiles() const;

private:
    GwmLayerGWRItem* mLayerItem;
    QList<GwmQuartiles> mQuartiles;
};

#endif // GWMPROPERTYGWRTAB_H
