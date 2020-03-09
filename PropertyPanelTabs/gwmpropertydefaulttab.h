#ifndef GWMPROPERTYDEFAULTTAB_H
#define GWMPROPERTYDEFAULTTAB_H

#include <QWidget>

namespace Ui {
class GwmPropertyDefaultTab;
}

class GwmPropertyDefaultTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyDefaultTab(QWidget *parent = nullptr);
    ~GwmPropertyDefaultTab();

private:
    Ui::GwmPropertyDefaultTab *ui;
};

#endif // GWMPROPERTYDEFAULTTAB_H
