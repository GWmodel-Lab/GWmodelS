#include "gwmindepvarselectorwidget.h"
#include "ui_gwmindepvarselectorwidget.h"
#include "qgsfields.h"

GwmIndepVarSelectorWidget::GwmIndepVarSelectorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GwmIndepVarSelectorWidget)
{
    mIndepVarModel = new GwmLayerAttributeItemModel(this);
    mSelectedIndepVarModel= new GwmLayerAttributeItemModel(this);
    ui->setupUi(this);
    ui->mIndepVarView->setModel(mIndepVarModel);
    ui->mSelectedIndepVarView->setModel(mSelectedIndepVarModel);
    connect(ui->mAddIndepVarBtn,&QPushButton::clicked,this,&GwmIndepVarSelectorWidget::onAddIndepVarBtn);
    connect(ui->mDelIndepVarBtn,&QPushButton::clicked,this,&GwmIndepVarSelectorWidget::onDelIndepVarBtn);

}

GwmIndepVarSelectorWidget::~GwmIndepVarSelectorWidget()
{
    delete ui;
}

GwmLayerAttributeItemModel *GwmIndepVarSelectorWidget::selectedIndepVarModel() const
{
    return mSelectedIndepVarModel;
}

GwmLayerAttributeItemModel *GwmIndepVarSelectorWidget::indepVarModel() const
{
    return mIndepVarModel;
}

void GwmIndepVarSelectorWidget::layerChanged(QgsVectorLayer* layer)
{
    if (mLayer)
    {
        mLayer = nullptr;
    }
    mLayer =  layer;
//    QList<int> attributeList = mLayer->attributeList();
    QgsFields fields = mLayer->fields();
    if (mIndepVarModel)
    {
        mIndepVarModel->clear();
    }
    if (mSelectedIndepVarModel)
    {
        mSelectedIndepVarModel->clear();
    }
    qDebug() << "layerChanged";
//    ui->mDepVarComboBox->clear();
}

void GwmIndepVarSelectorWidget::onDepVarChanged(QString depVarName)
{
//    QString depVarName = ui->mDepVarComboBox->itemText(index);
    if (mIndepVarModel)
    {
        mIndepVarModel->clear();
    }
    else{
        mIndepVarModel = new GwmLayerAttributeItemModel(this);
    }
//    QList<int> attributeList = mLayer->attributeList();
    QgsFields fields = mLayer->fields();
    QList<QString> fieldNameList = fields.names();
    for(QString fieldName : fieldNameList)
    {
        if (fieldName != depVarName)
        {
            int index = fields.indexFromName(fieldName);
            GwmLayerAttributeItem *item = new GwmLayerAttributeItem(index,fieldName,fields.field(index).type());
//            item->setData(index);
            mIndepVarModel->appendRow(item);
//            qDebug() << mIndepVarModel->indexFromItem(item).internalPointer();
        }
    }
//    ui->mIndepVarView->setModel(mIndepVarModel);
    if (mSelectedIndepVarModel)
    {
//        QList<GwmLayerAttributeItem*> removeItems = mSelectedIndepVarModel->findItems(depVarName);
//        for (GwmLayerAttributeItem* item : removeItems)
//        {
//            mSelectedIndepVarModel->removeRows(mSelectedIndepVarModel->indexFromItem(item).row(),1);
//        }
        mSelectedIndepVarModel->clear();
    }

}

void GwmIndepVarSelectorWidget::onAddIndepVarBtn()
{
    if(!mSelectedIndepVarModel){
        mSelectedIndepVarModel = new GwmLayerAttributeItemModel(this);
//        qDebug() << "mSelectedAttributeModel";
    }
    QModelIndexList selected = ui->mIndepVarView->selectionModel()->selectedIndexes();
    for(QModelIndex index : selected){
        if(index.isValid()){
            GwmLayerAttributeItem *item = mIndepVarModel->itemFromIndex(index)->clone();
            mSelectedIndepVarModel->appendRow(item);
//            qDebug() << index;
        }
    }
    for(QModelIndex index : selected)
    {
        if(index.isValid()){
            mIndepVarModel->removeRows(index.row(),1);
        }
    }
    ui->mSelectedIndepVarView->setModel(mSelectedIndepVarModel);
    emit selectedIndepVarChangedSignal();
}

void GwmIndepVarSelectorWidget::onDelIndepVarBtn()
{
    QModelIndexList selected = ui->mSelectedIndepVarView->selectionModel()->selectedIndexes();
    for(QModelIndex index:selected){
        if(index.isValid()){
            GwmLayerAttributeItem *item = mSelectedIndepVarModel->itemFromIndex(index)->clone();
            mIndepVarModel->appendRow(item);
        }
    }
    for(QModelIndex index:selected)
    {
        if(index.isValid()){
            mSelectedIndepVarModel->removeRows(index.row(),1);
        }
    }
    emit selectedIndepVarChangedSignal();
}

