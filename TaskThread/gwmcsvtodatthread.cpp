#include "gwmcsvtodatthread.h"

GwmCsvToDatThread::GwmCsvToDatThread(QString csvFielname,QString datFileName,QString seperator)
    :mCsvFielname(csvFielname),
     mDatFielname(datFileName),
     mSeperator(seperator),
     mRowCount(0),
     mColCount(0)
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

void GwmCsvToDatThread::run()
{
    read();
    int progress = 0;
    int total = mRowCount * mColCount;
    QFile file(mDatFielname);
//    QStringList line;
    if(file.open(QFile::QIODevice::WriteOnly)){
//      QTextStream stream(&file);
//      for(int i=0;i<lines.size();i++){
//          line = lines[i];
//          stream << line.join(this->seperator)<<"\n";
//      }
      QTextStream stream(&file);
      QByteArray byte;
      memcpy(byte.data(),&mRowCount,sizeof(mRowCount));
      stream << byte;
      memcpy(byte.data(),&mColCount,sizeof(mColCount));
      stream << byte;
      for(int col = 0; col < mColCount; col++){
          for(int row = 0; row < mRowCount; row++){
              double data = mCsvLines[row][col].toDouble();
              memcpy(byte.data(),&data,sizeof(data));
              stream << byte;
              progress++;
              emit tick(progress,total);
          }
      }
      file.close();
    }
    emit success();
}

QString GwmCsvToDatThread::name() const
{
    return tr("Csv To Dat");
}



