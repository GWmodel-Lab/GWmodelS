#ifndef GWMPROPERTYSCALABLEGWRTAB_H
#define GWMPROPERTYSCALABLEGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmplot.h"
#include "Model/gwmlayerscalablegwritem.h"

namespace Ui {
class GwmPropertyScalableGWRTab;
}

class GwmPropertyScalableGWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyScalableGWRTab(QWidget *parent = nullptr, GwmLayerScalableGWRItem* item = nullptr);
    ~GwmPropertyScalableGWRTab();

private:
    Ui::GwmPropertyScalableGWRTab *ui;
    GwmLayerScalableGWRItem* mLayerItem;
    QString FilePath;

public:
    void updateUI();
    bool openSelectFile();
private slots:
    void on_btnSaveRes_clicked();
};
#endif // GWMPROPERTYSCALABLEGWRTAB_H
