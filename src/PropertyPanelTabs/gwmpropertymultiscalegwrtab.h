#ifndef GWMPROPERTYMULTISCALEGWRTAB_H
#define GWMPROPERTYMULTISCALEGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmplot.h"
#include "Model/gwmlayermultiscalegwritem.h"
#include "Model/gwmpropertymultiscaleparameterspecifieditemmodel.h"

namespace Ui {
class GwmPropertyMultiscaleGWRTab;
}

class GwmPropertyMultiscaleGWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyMultiscaleGWRTab(QWidget *parent = nullptr, GwmLayerMultiscaleGWRItem* item = nullptr);
    ~GwmPropertyMultiscaleGWRTab();

private:
    Ui::GwmPropertyMultiscaleGWRTab *ui;
    GwmLayerMultiscaleGWRItem* mLayerItem;
    GwmPropertyMultiscaleParameterSpecifiedItemModel* mParameterSpecifiedModel = nullptr;

    QString FilePath;
public:
    void updateUI();
    bool openSelectFile();
    void GetNode(GwmPropertyMultiscaleParameterSpecifiedItemModel* model);
    void GetItem();
private slots:
    void on_btnSaveRes_clicked();

};


#endif // GWMPROPERTYGWRTA.B_H
