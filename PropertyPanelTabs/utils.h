#ifndef UTILS_H
#define UTILS_H

#include <QStandardItem>

void addModelItem(QStandardItem* item, QString property, QString value)
{
    QStandardItem* subitem = new QStandardItem(property);
    item->appendRow(subitem);
    item->setChild(subitem->index().row(), 1, new QStandardItem(value));
}

#endif // UTILS_H
