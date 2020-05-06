#include "gwmcsvtodatthread.h"
#include <qdebug.h>

GwmCsvToDatThread::GwmCsvToDatThread(QString csvFielname,QString datFileName,QString seperator)
    :mCsvFilename(csvFielname),
     mDatFilename(datFileName),
     mSeperator(seperator),
     mRowCount(0),
     mColCount(0),
     mIsColumnStore(false)
{
    mCsvLines = QList<QStringList>();
}


QList<QStringList> GwmCsvToDatThread::read(){
    QFile file(mCsvFilename);
    QString line;
    if (file.open(QFile::QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line = stream.readLine();
            if (mColCount == 0)
            {
                mColCount = line.split(mSeperator).size();
            }
            mCsvLines.append(line.split(mSeperator));
        }
        file.close();
    }
    mRowCount = mCsvLines.size();
    return mCsvLines;
}

void GwmCsvToDatThread::readSize()
{
    QFile file(mCsvFilename);
    QString line;
    mRowCount = 0;
    if (file.open(QFile::QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line = stream.readLine();
            if (mColCount == 0)
            {
                mColCount = line.split(mSeperator).size();
            }
            mRowCount++;
        }
        file.close();
    }
}

void GwmCsvToDatThread::run()
{
    if(mIsColumnStore){
        readSize();
        int progress = 0;
        int total = mRowCount;
        QFile datFile(mDatFilename);
        QFile csvFile(mCsvFilename);
        if (csvFile.open(QFile::QIODevice::ReadOnly))
        {
            if (datFile.open(QFile::QIODevice::WriteOnly))
            {
                unsigned long long basePos = 3 * sizeof (int);
                QByteArray byte;
                byte.resize(basePos);
                int metaData[3] = {mRowCount, mColCount, 1};
                memcpy(byte.data(), metaData, basePos);
                datFile.write(byte);
                QStringList lines;
                QTextStream fin(&csvFile);
                for (int c = 0; c < mColCount; c++)
                {
                    byte.resize(mRowCount * sizeof (double));
                    memset(byte.data(), 0, mRowCount * sizeof (double));
                    datFile.write(byte);
                }
                int r = 0, c = 0;
                byte.resize(sizeof (double));
                while (!fin.atEnd())
                {
                    c = 0;
                    lines = fin.readLine().split(mSeperator);
                    for (QString line : lines)
                    {
                        datFile.seek(basePos + (c * mRowCount + r) * sizeof (double));
                        double data = line.toDouble();
                        memcpy(byte.data(), &data, sizeof (double));
                        datFile.write(byte);
                        c++;
                    }
                    progress++;
                    emit tick(progress,total);
                    r++;
                }
            }
        }
        datFile.close();
        emit success();
    }
    else{
        readSize();
        int progress = 0;
        int total = mRowCount;
        QFile targetFile(mDatFilename);
        QFile sourceFile(mCsvFilename);
        if (targetFile.open(QFile::QIODevice::WriteOnly))
        {
            if (sourceFile.open(QFile::QIODevice::ReadOnly))
            {
                unsigned long long basePos = 3 * sizeof (int);
                QDataStream fout(&targetFile);
                fout.setByteOrder(QDataStream::BigEndian);
                fout << mRowCount << mColCount << 0;
                // 文本文件转二进制文件
                QStringList lines;
                QTextStream fin(&sourceFile);
                while (!fin.atEnd())
                {
                    lines = fin.readLine().split(mSeperator);
                    for (QString line : lines)
                    {
                        double data = line.toDouble();
                        fout << data;
                    }
                    progress++;
                    emit tick(progress,total);
                }
                sourceFile.close();
                targetFile.close();
            }
        }
        emit success();
    }
}

QString GwmCsvToDatThread::name() const
{
    return tr("Csv To Dmat");
}

void GwmCsvToDatThread::setIsColumnStore(bool flag)
{
    mIsColumnStore = flag;
}



