#ifndef GWMDEVATTRTABLE_H
#define GWMDEVATTRTABLE_H

#include "prefix.h"
#include <QDialog>
#include "ui_qgsattributetabledialog.h"

#include <qgsvectorlayer.h>

class GwmDevAttrTable: public QDialog, public Ui::QgsAttributeTableDialog
{
    Q_OBJECT
public:
    GwmDevAttrTable(QgsVectorLayer* theVecLayer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window);
    ~GwmDevAttrTable();
};

#endif // GWMDEVATTRTABLE_H
