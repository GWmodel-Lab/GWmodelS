#include "gwmodeltoolbar.h"

GWmodelToolbar::GWmodelToolbar(QWidget *parent) :
    QWidget(parent)
{
    createButtons();
    QHBoxLayout* widgetLayout = new QHBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(openLayerBtn);
    widgetLayout->addWidget(saveLayerBtn);
    widgetLayout->addWidget(exportLayerBtn);
    widgetLayout->addWidget(editBtn);
    widgetLayout->addWidget(moveBtn);
    widgetLayout->addWidget(fullScreenBtn);
    widgetLayout->addWidget(showPositionBtn);
    widgetLayout->addWidget(gwmodelGWRBtn);
    widgetLayout->addWidget(gwmodelGWSSBtn);
    widgetLayout->addWidget(gwmodelGWPCABtn);
    widgetLayout->addStretch();
    this->setLayout(widgetLayout);
    widgetLayout->setSpacing(5);
    connect(openLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportShapefile);
    connect(saveLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportJson);
    connect(exportLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportCsv);
}

GWmodelToolbar::~GWmodelToolbar()
{

}

void GWmodelToolbar::openFileImportShapefile(){
    emit openFileImportShapefileSignal();
}

void GWmodelToolbar::openFileImportJson(){
    emit openFileImportJsonSignal();
}

void GWmodelToolbar::openFileImportCsv(){
    emit openFileImportCsvSignal();
}

void GWmodelToolbar::createButtons()
{
    openLayerBtn = new QPushButton();
    openLayerBtn->setFixedSize(50,50);
    openLayerBtn->setIcon(QPixmap(":/icon/res/icon/folder.png"));
    openBtnInfo = new QLabel(this);
    openBtnInfo->setText(tr("Open"));
    openBtnInfo->setContentsMargins(3,3,3,3);
    openBtnInfo->adjustSize();
    openBtnInfo->setStyleSheet("background-color:#FFFFFF");
    openBtnInfo->hide();
    openLayerBtn->installEventFilter(this);
    openBtnInfo->installEventFilter(this);

    saveLayerBtn = new QPushButton(tr("Save"));
    saveLayerBtn->setFixedSize(50,50);
    exportLayerBtn = new QPushButton(tr("Export"));
    exportLayerBtn->setFixedSize(50,50);
    editBtn = new QPushButton(tr("Edit"));
    editBtn->setFixedSize(50,50);
    moveBtn = new QPushButton(tr("Move"));
    moveBtn->setFixedSize(50,50);
    fullScreenBtn = new QPushButton(tr("Full"));
    fullScreenBtn->setFixedSize(50,50);
    showPositionBtn = new QPushButton(tr("Pos"));
    showPositionBtn->setFixedSize(50,50);
    gwmodelGWRBtn = new QPushButton(tr("GWR"));
    gwmodelGWRBtn->setFixedSize(50,50);
    gwmodelGWSSBtn = new QPushButton(tr("GWSS"));
    gwmodelGWSSBtn->setFixedSize(50,50);
    gwmodelGWPCABtn = new QPushButton(tr("GWPCA"));
    gwmodelGWPCABtn->setFixedSize(50,50);
}

bool GWmodelToolbar::eventFilter(QObject *watched, QEvent *event)
{
    if( openLayerBtn == watched || openBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(openBtnInfo->isHidden()){
                openBtnInfo->show();
                QPoint point = openLayerBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                openBtnInfo->move(point);
                openBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!openBtnInfo->isHidden()){
                if(!openLayerBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!openBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       openBtnInfo->hide();
                       return true;
                   }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
