#include "gwmcsvtodatthread.h"
#include <qdebug.h>

GwmCsvToDatThread::GwmCsvToDatThread(QString csvFielname,QString datFileName,QString seperator)
    :mCsvFielname(csvFielname),
     mDatFielname(datFileName),
     mSeperator(seperator),
     mRowCount(0),
     mColCount(0),
     mIsColumnStore(false)
{
    mCsvLines = QList<QStringList>();
}


QList<QStringList> GwmCsvToDatThread::read(){
    QFile file(mCsvFielname);
    QString line;
    if(file.open(QFile::QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            line = stream.readLine();
            if(mColCount == 0){
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
    QFile file(mCsvFielname);
    QString line;
    mRowCount = 0;
    if(file.open(QFile::QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            line = stream.readLine();
            if(mColCount == 0){
                mColCount = line.split(mSeperator).size();
            };
            mRowCount++;
        }
        file.close();
    }
}

void GwmCsvToDatThread::run()
{
    if(mIsColumnStore){
        read();
        int progress = 0;
        int total = mRowCount * mColCount;
        QFile file(mDatFielname);
        if(file.open(QFile::QIODevice::WriteOnly)){
          QTextStream stream(&file);
          QByteArray byte;
          byte.clear();
          memcpy(byte.data(),&mRowCount,sizeof(mRowCount));
          stream << byte.data();
          byte.clear();
          memcpy(byte.data(),&mColCount,sizeof(mColCount));
          stream << byte.data();
          int type = 1;
          memcpy(byte.data(),&type,sizeof(type));
          stream << byte.data();
          for(int col = 0; col < mColCount; col++){
              for(int row = 0; row < mRowCount; row++){
                  byte.clear();
                  double data = mCsvLines[row][col].toDouble();
                  memcpy(byte.data(),&data,sizeof(data));
                  stream << byte.data();
                  progress++;
                  emit tick(progress,total);
              }
          }
          file.close();
        }
        emit success();
    }
    else{
        readSize();
        int progress = 0;
        int total = mRowCount * mColCount;
        QFile targetFile(mDatFielname);
        QFile sourceFile(mCsvFielname);
        if(targetFile.open(QFile::QIODevice::WriteOnly)){
            if(sourceFile.open(QFile::QIODevice::ReadOnly)){
                QTextStream stream1(&targetFile);
                QByteArray byte;
                byte.clear();
                memcpy(byte.data(),&mRowCount,sizeof(mRowCount));
                stream1 << byte.data();
                byte.clear();
                memcpy(byte.data(),&mColCount,sizeof(mColCount));
                stream1 << byte.data();
                byte.clear();
                int type = 0;
                memcpy(byte.data(),&type,sizeof(type));
                stream1 << byte.data();
                QStringList lines;
                QTextStream stream2(&sourceFile);
                while(!stream2.atEnd()){
                    lines = stream2.readLine().split(mSeperator);
                    for(QString line : lines){
                        byte.clear();
                        double data = line.toDouble();
                        memcpy(byte.data(),&data,sizeof(data));
                        stream1 << byte.data();
                        progress++;
                        emit tick(progress,total);
                    }
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



