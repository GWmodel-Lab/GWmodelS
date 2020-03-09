#ifndef GWMFEATURESTATISTICSTAB_H
#define GWMFEATURESTATISTICSTAB_H

#include <QWidget>

namespace Ui {
class GwmPropertyStatisticsTab;
}

class GwmPropertyStatisticsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyStatisticsTab(QWidget *parent = nullptr);
    ~GwmPropertyStatisticsTab();

private:
    Ui::GwmPropertyStatisticsTab *ui;
};

#endif // GWMFEATURESTATISTICSTAB_H
