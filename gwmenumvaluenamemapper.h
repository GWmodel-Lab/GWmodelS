#ifndef GWMENUMVALUENAMEMAPPER_H
#define GWMENUMVALUENAMEMAPPER_H

#include <QHash>
#include <QString>

using namespace std;

template <class EnumT>
class GwmEnumValueNameMapper
{
public:
    GwmEnumValueNameMapper(initializer_list<pair<EnumT, QString> > values);

    QString name(const EnumT& value)
    {
        return mValueToNameMap[value];
    }

    EnumT value(const QString& name)
    {
        return mNameToValueMap[name];
    }

    QString operator[](const EnumT& value)
    {
        return mValueToNameMap[value];
    }

    EnumT operator[](const QString& name)
    {
        return mNameToValueMap[name];
    }

private:
    QHash<EnumT, QString> mValueToNameMap;
    QHash<QString, EnumT> mNameToValueMap;
};

template<class EnumT>
GwmEnumValueNameMapper<EnumT>::GwmEnumValueNameMapper(initializer_list<pair<EnumT, QString> > values)
{
    for (auto i = values.begin(); i != values.end(); i++)
    {
        mValueToNameMap[i->first] = i->second;
        mNameToValueMap[i->second] = i->first;
    }
}

#endif // GWMENUMVALUENAMEMAPPER_H
