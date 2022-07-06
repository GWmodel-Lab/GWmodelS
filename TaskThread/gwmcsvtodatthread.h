#ifndef GWMCSVTODATTHREAD_H
#define GWMCSVTODATTHREAD_H

#include <QObject>
#include <QThread>
#include "gwmtaskthread.h"
#include <QTextStream>
#include <qstringlist.h>
#include <QList>
#include <qfile.h>

class GwmCsvToDatThread : public GwmTaskThread
{
public:
    GwmCsvToDatThread(QString csvFielname,QString datFileName,QString seperator = ",");

    QList<QStringList> read();
    virtual QString name() const override;
    void setIsColumnStore(bool flag);
    void readSize();

protected:
    void run() override; //新线程入口

private:
    QString mCsvFilename;
    QString mDatFilename;
    QString mSeperator;
    QList<QStringList> mCsvLines;
    int mRowCount;
    int mColCount;
    bool mIsColumnStore;

};

#endif // GWMCSVTODATTHREAD_H
