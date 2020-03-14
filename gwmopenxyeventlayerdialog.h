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

private slots:
    void onFormatCSVRadioToggled(bool checked);

    void onFormatRegRadioToggled(bool checked);

    void onFormatCustomRadioToggled(bool checked);

private:
    Ui::GwmOpenXYEventLayerDialog *ui;
};

#endif // GWMOPENXYEVENTLAYERDIALOG_H
