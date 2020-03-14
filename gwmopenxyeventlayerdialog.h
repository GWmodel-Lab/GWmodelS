#ifndef GWMOPENXYEVENTLAYERDIALOG_H
#define GWMOPENXYEVENTLAYERDIALOG_H

#include <QDialog>

namespace Ui {
class GwmOpenXYEventLayerDialog;
}

class GwmOpenXYEventLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmOpenXYEventLayerDialog(QWidget *parent = nullptr);
    ~GwmOpenXYEventLayerDialog();

private:
    Ui::GwmOpenXYEventLayerDialog *ui;
};

#endif // GWMOPENXYEVENTLAYERDIALOG_H
