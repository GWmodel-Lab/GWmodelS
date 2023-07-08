#ifndef GWMPROJCRSSETTINGDIALOG_H
#define GWMPROJCRSSETTINGDIALOG_H

#include "prefix.h"

#include <QDialog>
#include <QtWidgets>
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsrenderer.h>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

namespace Ui {
class GwmProjCRSSettingDialog;
}

class GwmProjCRSSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmProjCRSSettingDialog(QWidget *parent = nullptr);
    ~GwmProjCRSSettingDialog();

signals:
    void cancelProjTransSignal(int flag);
public:
    QgsCoordinateReferenceSystem desProjCrs() const;
    void setDesProjCrs(const QgsCoordinateReferenceSystem &desProjCrs);

    virtual void accept() override;
    virtual void reject() override;
private:
    Ui::GwmProjCRSSettingDialog *ui;
};

#endif // GWMPROJCRSSETTINGDIALOG_H
