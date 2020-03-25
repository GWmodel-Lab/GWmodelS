#ifndef GWMCOORDINATE_H
#define GWMCOORDINATE_H

#include "prefix.h"

#include <QDialog>
#include <QtWidgets>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

#include "QProgressDialog"

namespace Ui {
class GwmCoordTransSettingDialog;
}

class GwmCoordTransSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmCoordTransSettingDialog(QWidget *parent = nullptr);
    ~GwmCoordTransSettingDialog();

signals:
    void cancelTransSignal(int flag);
public:
    QgsCoordinateReferenceSystem desCrs() const;
    void setDesCrs(const QgsCoordinateReferenceSystem &desCrs);

    QgsCoordinateReferenceSystem srcCrs() const;
    void setSrcCrs(const QgsCoordinateReferenceSystem &srcCrs);

    virtual void accept() override;
    virtual void reject() override;

    QProgressDialog *progressDialog;

private:
    Ui::GwmCoordTransSettingDialog *ui;

};

#endif // GWMCOORDINATE_H
