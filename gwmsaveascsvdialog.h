#ifndef GWMSAVEASCSVDIALOG_H
#define GWMSAVEASCSVDIALOG_H

#include <QDialog>
#include <Qt>

namespace Ui {
class GwmSaveAsCSVDialog;
}

class GwmSaveAsCSVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmSaveAsCSVDialog(QWidget *parent = nullptr);
    ~GwmSaveAsCSVDialog();

private:
    Ui::GwmSaveAsCSVDialog *ui;

private:
    void onOpenFileDialog();

public:
    QString filepath();
    QString xcolumnname();
    QString ycolumnname();
    bool isAddXY();
};

#endif // GWMSAVEASCSVDIALOG_H
