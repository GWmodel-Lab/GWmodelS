#include "gwmsavelayerthread.h"
#include <qgsvectorfilewriter.h>
#include <qgsvectorfilewritertask.h>
#include <qgsapplication.h>

GwmSaveLayerThread::GwmSaveLayerThread(QgsVectorLayer* layer,QString filepath,QgsVectorFileWriter::SaveVectorOptions& options):options(options)
{
    this->mLayer = layer;
    this->filePath = filepath;
//    this->options = *(new QgsVectorFileWriter::SaveVectorOptions());
}

void GwmSaveLayerThread::run()
{
    QgsVectorFileWriterTask *writerTask = new QgsVectorFileWriterTask( mLayer, filePath, options );
    // when writer is successful:
    this->cancelFlag = 0;
    connect( writerTask, &QgsVectorFileWriterTask::completed, this, [=]( const QString & newFilename, const QString & newLayer )
    {
        qDebug() << "[GwmSaveLayerThread::run] QgsVectorFileWriterTask::completed" << newFilename;
        emit success();
    });
    connect( writerTask, &QgsVectorFileWriterTask::taskCompleted, this, [=]( )
    {
        qDebug() << "[GwmSaveLayerThread::run] QgsVectorFileWriterTask::taskCompleted" << "newFilename";
        emit success();
    });
    connect( writerTask, &QgsVectorFileWriterTask::errorOccurred, this, [=](int err, const QString & errorMessage)
    {
        qDebug() << "[GwmSaveLayerThread::run] QgsVectorFileWriterTask::errorOccurred" << errorMessage;
        emit error(errorMessage);
    });
    QgsApplication::taskManager()->addTask( writerTask );
    emit tick(0, 0);
}

void GwmSaveLayerThread::onCancelTrans(int flag){
    this->cancelFlag = flag;
}

QString GwmSaveLayerThread::name() const
{
    return tr("Export Layer");
}
