#ifndef ABOUTDEVELOPERS_H
#define ABOUTDEVELOPERS_H

#include <QDialog>

namespace Ui {
class aboutDevelopers;
}

class aboutDevelopers : public QDialog
{
    Q_OBJECT

public:
    explicit aboutDevelopers(QWidget *parent = nullptr);
    ~aboutDevelopers();

private:
    Ui::aboutDevelopers *ui;
};

#endif // ABOUTDEVELOPERS_H
