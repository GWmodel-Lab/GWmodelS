#ifndef GWMODELMAPPANEL_H
#define GWMODELMAPPANEL_H

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include <QWidget>
#include <QStandardItemModel>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>

class GWmodelMapPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GWmodelMapPanel(QWidget *parent = nullptr, QStandardItemModel* model = new QStandardItemModel);
    ~GWmodelMapPanel();

private:
    QgsMapCanvas* mapCanvas;
    QStandardItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerSet;

    void onMapItemInserted(const QModelIndex &parent, int first, int last);
};

#endif // GWMODELMAPPANEL_H
