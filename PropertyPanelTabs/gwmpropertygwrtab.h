#ifndef GWMPROPERTYGWRTAB_H
#define GWMPROPERTYGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayerbasicgwritem.h"

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
    explicit GwmPropertyGWRTab(QWidget *parent = nullptr, GwmLayerBasicGWRItem* item = nullptr);
    ~GwmPropertyGWRTab();

private:
    Ui::GwmPropertyGWRTab *ui;
    GwmLayerBasicGWRItem* mLayerItem;

    GwmPlot* mModelSelVarsPlot;
    GwmPlot* mModelSelAICsPlot;

    GwmPlot* mBandwidthSelPlot;

public:
    void updateUI();
};


#endif // GWMPROPERTYGWRTAB_H
