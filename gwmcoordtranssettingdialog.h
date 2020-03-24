#ifndef GWMCOORDINATE_H
#define GWMCOORDINATE_H

#include <QDialog>
#include <QtWidgets>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

#include "gwmcoordtransthread.h"

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
private slots:
    void updateTransProgress(int progress,int total);
public:
    QgsCoordinateReferenceSystem desCrs() const;
    void setDesCrs(const QgsCoordinateReferenceSystem &desCrs);

    QgsCoordinateReferenceSystem srcCrs() const;
    void setSrcCrs(const QgsCoordinateReferenceSystem &srcCrs);

    virtual void accept() override;
    virtual void reject() override;

    // 投影转换
    void transformCoordinate(QgsCoordinateReferenceSystem desCrs, QgsVectorLayer *handleLayer);

    QProgressDialog *progressDialog;

private:
    Ui::GwmCoordTransSettingDialog *ui;

    GwmCoordTransThread *m_transThread; //用于投影转换的子线程

};

#endif // GWMCOORDINATE_H
