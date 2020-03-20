#ifndef GWMCOORDINATE_H
#define GWMCOORDINATE_H

#include <QDialog>
#include <QtWidgets>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

namespace Ui {
class GwmCoordinate;
}

class GwmCoordinate : public QDialog
{
    Q_OBJECT

public:
    explicit GwmCoordinate(QWidget *parent = nullptr);
    ~GwmCoordinate();
signals:
    void sendSigCoordinate(QString,const QModelIndex &index);
    void sendSigSetCoordinate(QgsCoordinateReferenceSystem,QString,const QModelIndex& index);

private slots:
    void on_btnOK_clicked();

    void on_btnCancel_clicked();

    void on_BtnCrs_clicked();

    void on_BtnResPrj_clicked();

private:
    Ui::GwmCoordinate *ui;
    QgsProjectionSelectionWidget* mCRSSelector;
    QgsCoordinateReferenceSystem desCrs;
};

#endif // GWMCOORDINATE_H
