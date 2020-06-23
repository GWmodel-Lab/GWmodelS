#ifndef GWMPROPERTYCOLLINEARITYGWRTAB_H
#define GWMPROPERTYCOLLINEARITYGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayercollinearitygwritem.h"

namespace Ui {
class GwmPropertyCollinearityGWRTab;
}

class GwmPropertyCollinearityGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGWRTaskThread::KernelFunction, QString> kernelFunctionNameDict;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyCollinearityGWRTab(QWidget *parent = nullptr, GwmLayerCollinearityGWRItem* item = nullptr);
    ~GwmPropertyCollinearityGWRTab();

private:
    Ui::GwmPropertyCollinearityGWRTab *ui;
    GwmLayerCollinearityGWRItem* mLayerItem;

    GwmPlot* mModelSelVarsPlot;
    GwmPlot* mModelSelAICsPlot;

    GwmPlot* mBandwidthSelPlot;

public:
    void updateUI();
};

#endif // GWMPROPERTYGWRTAB_H
