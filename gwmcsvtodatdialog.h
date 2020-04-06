#ifndef GWMCSVTODATDIALOG_H
#define GWMCSVTODATDIALOG_H

#include <QDialog>

namespace Ui {
class GwmCsvToDatDialog;
}

class GwmCsvToDatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmCsvToDatDialog(QWidget *parent = nullptr);
    ~GwmCsvToDatDialog();

private:
    Ui::GwmCsvToDatDialog *ui;

private:
    void onCsvOpenButton();
    void onDatOpenButton();

public:
    QString csvFileName();
    QString datFileName();
};

#endif // GWMCSVTODATDIALOG_H
