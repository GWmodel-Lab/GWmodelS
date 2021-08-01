#ifndef GWMPROPERTYGTWRTAB_H
#define GWMPROPERTYGTWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmplot.h"
#include "Model/gwmlayergtwritem.h"

namespace Ui {
class GwmPropertyGTWRTab;
}

class GwmPropertyGTWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyGTWRTab(QWidget *parent = nullptr, GwmLayerGTWRItem* item = nullptr);
    ~GwmPropertyGTWRTab();

private:
    Ui::GwmPropertyGTWRTab *ui;
    GwmLayerGTWRItem* mLayerItem;

    GwmPlot* mBandwidthSelPlot;
    QString FilePath;

public:
    void updateUI();
    bool openSelectFile();
private slots:
    void on_btnSaveRes_clicked();
};


#endif // GWMPROPERTYGTWRTAB_H
