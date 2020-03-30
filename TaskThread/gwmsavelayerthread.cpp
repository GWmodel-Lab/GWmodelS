#include "gwmsavelayerthread.h"
#include <qgsvectorfilewriter.h>
#include <qgsvectorfilewritertask.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>

GwmSaveLayerThread::GwmSaveLayerThread(QgsVectorLayer* layer,QString filepath,QgsVectorFileWriter::SaveVectorOptions& options)
    : GwmTaskThread()
    , mOptions(options)
{
    this->mLayer = layer;
    this->mFilePath = filepath;
}

void GwmSaveLayerThread::run()
{
    emit tick(0, 0);
    QString newFileName, errorMessage, newLayerName;
    QgsVectorFileWriter::writeAsVectorFormat(mLayer, mFilePath, mOptions, &newFileName, &errorMessage, &newLayerName);
    if (errorMessage.isEmpty())
    {
        emit success();
    }
    else
    {
        emit error(errorMessage);
    }

}

void GwmSaveLayerThread::onCancelTrans(int flag){
    this->cancelFlag = flag;
}

QString GwmSaveLayerThread::name() const
{
    return tr("Export Layer");
}
