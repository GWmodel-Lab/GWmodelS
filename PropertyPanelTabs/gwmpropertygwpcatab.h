#ifndef GWMPROPERTYGWPCATAB_H
#define GWMPROPERTYGWPCATAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmplot.h"
#include "Model/gwmlayergwpcaitem.h"

namespace Ui {
class GwmPropertyGWPCATab;
}


class GwmPropertyGWPCATab : public QWidget
{
    Q_OBJECT

public:

public:
    explicit GwmPropertyGWPCATab(QWidget *parent = nullptr, GwmLayerGWPCAItem* item = nullptr);
    ~GwmPropertyGWPCATab();

private:
    Ui::GwmPropertyGWPCATab *ui;
    GwmLayerGWPCAItem* mLayerItem;

    QString FilePath;
public:
    void updateUI();

    GwmPlot* mBandwidthSelPlot;
private slots:
    void on_btnSaveRes_clicked();
    bool openSelectFile();
};


#endif // GWMPROPERTYGWRTAB_H
