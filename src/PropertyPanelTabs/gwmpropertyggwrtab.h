#ifndef GWMPROPERTYGGWRTAB_H
#define GWMPROPERTYGGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "gwmpropertygwrtab.h"
#include "Model/gwmlayerggwritem.h"

namespace Ui {
class GwmPropertyGGWRTab;
}

class GwmPropertyGGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGeneralizedGWRAlgorithm::Family, QString> familyTypeNameDict;

public:
    explicit GwmPropertyGGWRTab(QWidget *parent = nullptr, GwmLayerGGWRItem* item = nullptr);
    ~GwmPropertyGGWRTab();

private:
    Ui::GwmPropertyGGWRTab *ui;

private:
    GwmLayerGGWRItem* mLayerItem;

    GwmPlot* mBandwidthSelPlot;

    QString FilePath;

private:
    void setQuartiles(const int row, QString name, const GwmQuartiles& quartiles);

public:
    void updateUI();
    bool openSelectFile();

private slots:
    void on_btnSaveRes_clicked();
};

#endif // GWMPROPERTYGGWRTAB_H
