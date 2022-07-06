#ifndef GWMSAVELAYERTHREAD_H
#define GWMSAVELAYERTHREAD_H

#include "gwmtaskthread.h"
#include <qgsvectorfilewriter.h>
#include <QThread>

class GwmSaveLayerThread: public GwmTaskThread
{
public:
    GwmSaveLayerThread(QgsVectorLayer* layer,QString filepath,QgsVectorFileWriter::SaveVectorOptions& mOptions);
    int cancelFlag;
    virtual QString name() const override;

protected:
    void run() override; //新线程入口

public slots:
    void onCancelTrans(int canceledFlag);
private:
    QgsVectorLayer* mLayer;
    QString mFilePath;
    QgsVectorFileWriter::SaveVectorOptions& mOptions;
};

#endif // GWMSAVELAYERTHREAD_H
