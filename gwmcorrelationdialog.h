#ifndef GWMCORRELATIONDIALOG_H
#define GWMCORRELATIONDIALOG_H

#include <QWidget>

namespace Ui {
class gwmcorrelationdialog;
}

class gwmcorrelationdialog : public QWidget
{
    Q_OBJECT

public:
    explicit gwmcorrelationdialog(QWidget *parent = nullptr);
    ~gwmcorrelationdialog();

private:
    Ui::gwmcorrelationdialog *ui;
};

#endif // GWMCORRELATIONDIALOG_H
