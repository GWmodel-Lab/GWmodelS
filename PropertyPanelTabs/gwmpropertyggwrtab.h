#ifndef GWMPROPERTYGGWRTAB_H
#define GWMPROPERTYGGWRTAB_H

#include <QWidget>
#include "gwmpropertygwrtab.h"
#include "Model/gwmlayerggwritem.h"

namespace Ui {
class GwmPropertyGGWRTab;
}

class GwmPropertyGGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGWRTaskThread::KernelFunction, QString> kernelFunctionNameDict;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> bandwidthTypeNameDict;
    static QMap<GwmGGWRAlgorithm::Family, QString> GwmPropertyGGWRTab::familyTypeNameDict;

public:
    explicit GwmPropertyGGWRTab(QWidget *parent = nullptr, GwmLayerGGWRItem* item = nullptr);
    ~GwmPropertyGGWRTab();

private:
    Ui::GwmPropertyGGWRTab *ui;

private:
    GwmLayerGGWRItem* mLayerItem;

    GwmPlot* mBandwidthSelPlot;

private:
    void setQuartiles(const int row, QString name, const GwmQuartiles& quartiles);

public:
    void updateUI();


};

#endif // GWMPROPERTYGGWRTAB_H
