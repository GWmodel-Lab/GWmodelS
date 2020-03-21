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
class GwmCoordTransSettingDialog;
}

class GwmCoordTransSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmCoordTransSettingDialog(QWidget *parent = nullptr);
    ~GwmCoordTransSettingDialog();

public:
    QgsCoordinateReferenceSystem desCrs() const;
    void setDesCrs(const QgsCoordinateReferenceSystem &desCrs);

    QgsCoordinateReferenceSystem srcCrs() const;
    void setSrcCrs(const QgsCoordinateReferenceSystem &srcCrs);

    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::GwmCoordTransSettingDialog *ui;
    QgsCoordinateReferenceSystem mDesCrs;
    QgsCoordinateReferenceSystem mSrcCrs;
};

#endif // GWMCOORDINATE_H
