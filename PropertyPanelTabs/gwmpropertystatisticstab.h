#ifndef GWMFEATURESTATISTICSTAB_H
#define GWMFEATURESTATISTICSTAB_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class GwmPropertyStatisticsTab;
}

class GwmPropertyStatisticsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyStatisticsTab(QWidget *parent = nullptr, QStandardItemModel* model = new QStandardItemModel);
    ~GwmPropertyStatisticsTab();

private:
    Ui::GwmPropertyStatisticsTab *ui;
    QStandardItemModel* propertyModel;
};

#endif // GWMFEATURESTATISTICSTAB_H
