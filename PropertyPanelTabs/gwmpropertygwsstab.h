#ifndef GWMPROPERTYGWSSTAB_H
#define GWMPROPERTYGWSSTAB_H

#include <QWidget>

#include "Model/gwmlayergwssitem.h"


namespace Ui {
class GwmPropertyGWSSTab;
}

class GwmPropertyGWSSTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmBandwidthWeight::KernelFunctionType, QString> kernelFunctionNameDict;
    static QMap<bool, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyGWSSTab(QWidget *parent = nullptr);
    ~GwmPropertyGWSSTab();

private:
    Ui::GwmPropertyGWSSTab *ui;
    GwmLayerGWSSItem* mLayerItem;



};

#endif // GWMPROPERTYGWSSTAB_H
