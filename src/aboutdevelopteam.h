#ifndef ABOUTDEVELOPTEAM_H
#define ABOUTDEVELOPTEAM_H

#include <QDialog>

namespace Ui {
class aboutdevelopteam;
}

class aboutdevelopteam : public QDialog
{
    Q_OBJECT

public:
    explicit aboutdevelopteam(QWidget *parent = nullptr);
    ~aboutdevelopteam();

private slots:
    void on_btnOK_clicked();

private:
    Ui::aboutdevelopteam *ui;
};


#endif // ABOUTDEVELOPTEAM_H
