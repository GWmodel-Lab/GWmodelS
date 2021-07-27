#ifndef GWMPROPERTYCOLLINEARITYGWRTAB_H
#define GWMPROPERTYCOLLINEARITYGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmplot.h"
#include "Model/gwmlayercollinearitygwritem.h"

namespace Ui {
class GwmPropertyCollinearityGWRTab;
}

class GwmPropertyCollinearityGWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyCollinearityGWRTab(QWidget *parent = nullptr, GwmLayerCollinearityGWRItem* item = nullptr);
    ~GwmPropertyCollinearityGWRTab();

private:
    Ui::GwmPropertyCollinearityGWRTab *ui;
    GwmLayerCollinearityGWRItem* mLayerItem;

    GwmPlot* mModelSelVarsPlot;
    GwmPlot* mModelSelAICsPlot;

    GwmPlot* mBandwidthSelPlot;

    QString FilePath;


public:
    void updateUI();
    bool openSelectFile();
private slots:
    void on_btnSaveRes_clicked();
};

#endif // GWMPROPERTYGWRTAB_H
