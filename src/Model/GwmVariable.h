#ifndef GWMVARIABLE_H
#define GWMVARIABLE_H

#include <QString>
#include <QVariant>

struct GwmVariable
{
    int index;
    QString name;
    QVariant::Type type;
};

#endif // GWMVARIABLE_H
