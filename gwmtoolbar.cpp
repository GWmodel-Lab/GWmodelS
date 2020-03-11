#include "gwmtoolbar.h"

GwmToolbar::GwmToolbar(QWidget *parent) :
    QWidget(parent)
{
    createButtons();
    QHBoxLayout* widgetLayout = new QHBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(openLayerBtn);
    widgetLayout->addWidget(openByXYBtn);
    widgetLayout->addWidget(saveLayerBtn);
    widgetLayout->addWidget(exportLayerBtn);
    widgetLayout->addWidget(selectBtn);
    widgetLayout->addWidget(moveBtn);
    widgetLayout->addWidget(editBtn);
    widgetLayout->addWidget(fullScreenBtn);
    widgetLayout->addWidget(showPositionBtn);
    widgetLayout->addWidget(gwmodelGWRBtn);
    widgetLayout->addWidget(gwmodelGWSSBtn);
    widgetLayout->addWidget(gwmodelGWPCABtn);
    widgetLayout->addStretch();
    this->setLayout(widgetLayout);
    widgetLayout->setSpacing(5);
    connect(openLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportShapefile);
    connect(saveLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportJson);
    connect(exportLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportCsv);
    connect(openByXYBtn,&QPushButton::clicked,this,&GwmToolbar::openByXYBtnSlot);
    connect(selectBtn, &QPushButton::clicked, this, &GwmToolbar::selectBtnSignal);
    connect(moveBtn,&QPushButton::clicked,this,&GwmToolbar::moveBtnSlot);
    connect(editBtn,&QPushButton::clicked,this,&GwmToolbar::editBtnSlot);
    connect(fullScreenBtn,&QPushButton::clicked,this,&GwmToolbar::fullScreenBtnSlot);
    connect(showPositionBtn,&QPushButton::clicked,this,&GwmToolbar::showPositionBtnSlot);
    connect(gwmodelGWRBtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWRBtnSlot);
    connect(gwmodelGWSSBtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWSSBtnSlot);
    connect(gwmodelGWPCABtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWPCABtnSlot);
}

GwmToolbar::~GwmToolbar()
{

}

void GwmToolbar::openFileImportShapefile(){
    emit openFileImportShapefileSignal();
}

void GwmToolbar::openFileImportJson(){
    emit openFileImportJsonSignal();
}

void GwmToolbar::openFileImportCsv(){
    emit openFileImportCsvSignal();
}

void GwmToolbar::openByXYBtnSlot(){
    emit openByXYBtnSingnal();
}

void GwmToolbar::editBtnSlot(){
    emit openFileImportJsonSignal();
}

void GwmToolbar::moveBtnSlot(){
    emit moveBtnSignal();
}
void GwmToolbar::fullScreenBtnSlot(){
    emit fullScreenBtnSignal();
}

void GwmToolbar::showPositionBtnSlot(){
    emit showPositionBtnSignal();
}

void GwmToolbar::gwmodelGWRBtnSlot(){
    emit gwmodelGWRBtnSignal();
}

void GwmToolbar::gwmodelGWSSBtnSlot(){
    emit gwmodelGWSSBtnSignal();
}

void GwmToolbar::gwmodelGWPCABtnSlot(){
    emit gwmodelGWPCABtnSignal();
}


void GwmToolbar::createButtons()
{
    openLayerBtn = new QPushButton();
    openLayerBtn->setFixedSize(50,50);
    openLayerBtn->setIcon(QPixmap(tr("./release/icon/folder.png")));
    openBtnInfo = new QLabel(this);
    openBtnInfo->setText(tr("Open"));
    openBtnInfo->setContentsMargins(3,3,3,3);
    openBtnInfo->adjustSize();
    openBtnInfo->setStyleSheet("background-color:#FFFFFF");
    openBtnInfo->hide();
    openLayerBtn->installEventFilter(this);
    openBtnInfo->installEventFilter(this);

    openByXYBtn = new QPushButton();
    openByXYBtn->setFixedSize(50,50);
    openByXYBtn->setIcon(QPixmap("./release/icon/csv.png"));
    openByXYBtnInfo = new QLabel(this);
    openByXYBtnInfo->setText(tr("Open By XY Coordinate"));
    openByXYBtnInfo->setContentsMargins(3,3,3,3);
    openByXYBtnInfo->adjustSize();
    openByXYBtnInfo->setStyleSheet("background-color:#FFFFFF");
    openByXYBtnInfo->hide();
    openByXYBtn->installEventFilter(this);
    openByXYBtnInfo->installEventFilter(this);

    saveLayerBtn = new QPushButton();
    saveLayerBtn->setFixedSize(50,50);
    saveLayerBtn->setIcon(QPixmap("./release/icon/save.png"));
    saveBtnInfo = new QLabel(this);
    saveBtnInfo->setText(tr("Save"));
    saveBtnInfo->setContentsMargins(3,3,3,3);
    saveBtnInfo->adjustSize();
    saveBtnInfo->setStyleSheet("background-color:#FFFFFF");
    saveBtnInfo->hide();
    saveLayerBtn->installEventFilter(this);
    saveBtnInfo->installEventFilter(this);

    exportLayerBtn = new QPushButton();
    exportLayerBtn->setFixedSize(50,50);
    exportLayerBtn->setIcon(QPixmap("./release/icon/download.png"));
    exportBtnInfo = new QLabel(this);
    exportBtnInfo->setText(tr("Export"));
    exportBtnInfo->setContentsMargins(3,3,3,3);
    exportBtnInfo->adjustSize();
    exportBtnInfo->setStyleSheet("background-color:#FFFFFF");
    exportBtnInfo->hide();
    exportLayerBtn->installEventFilter(this);
    exportBtnInfo->installEventFilter(this);


    editBtn = new QPushButton();
    editBtn->setFixedSize(50,50);
    editBtn->setIcon(QPixmap("./release/icon/edit.png"));
    editBtnInfo = new QLabel(this);
    editBtnInfo->setText(tr("Edit"));
    editBtnInfo->setContentsMargins(3,3,3,3);
    editBtnInfo->adjustSize();
    editBtnInfo->setStyleSheet("background-color:#FFFFFF");
    editBtnInfo->hide();
    editBtn->installEventFilter(this);
    editBtnInfo->installEventFilter(this);


    selectBtn = new QPushButton();
    selectBtn->setFixedSize(50,50);
    selectBtn->setText("Select");
    selectBtn->installEventFilter(this);


    moveBtn = new QPushButton();
    moveBtn->setFixedSize(50,50);
    moveBtn->setIcon(QPixmap("./release/icon/move.png"));
    moveBtnInfo = new QLabel(this);
    moveBtnInfo->setText(tr("Move"));
    moveBtnInfo->setContentsMargins(3,3,3,3);
    moveBtnInfo->adjustSize();
    moveBtnInfo->setStyleSheet("background-color:#FFFFFF");
    moveBtnInfo->hide();
    moveBtn->installEventFilter(this);
    moveBtnInfo->installEventFilter(this);

    fullScreenBtn = new QPushButton();
    fullScreenBtn->setFixedSize(50,50);
    fullScreenBtn->setIcon(QPixmap("./release/icon/view larger.png"));
    fullScreenBtnInfo = new QLabel(this);
    fullScreenBtnInfo->setText(tr("Full"));
    fullScreenBtnInfo->setContentsMargins(3,3,3,3);
    fullScreenBtnInfo->adjustSize();
    fullScreenBtnInfo->setStyleSheet("background-color:#FFFFFF");
    fullScreenBtnInfo->hide();
    fullScreenBtn->installEventFilter(this);
    fullScreenBtnInfo->installEventFilter(this);

    showPositionBtn = new QPushButton();
    showPositionBtn->setFixedSize(50,50);
    showPositionBtn->setIcon(QPixmap("./release/icon/map.png"));
    showPositionBtnInfo = new QLabel(this);
    showPositionBtnInfo->setText(tr("Pos"));
    showPositionBtnInfo->setContentsMargins(3,3,3,3);
    showPositionBtnInfo->adjustSize();
    showPositionBtnInfo->setStyleSheet("background-color:#FFFFFF");
    showPositionBtnInfo->hide();
    showPositionBtn->installEventFilter(this);
    showPositionBtnInfo->installEventFilter(this);

    gwmodelGWRBtn = new QPushButton(tr("GWR"));
    gwmodelGWRBtn->setFixedSize(50,50);

    gwmodelGWSSBtn = new QPushButton(tr("GWSS"));
    gwmodelGWSSBtn->setFixedSize(50,50);

    gwmodelGWPCABtn = new QPushButton(tr("GWPCA"));
    gwmodelGWPCABtn->setFixedSize(50,50);
}

bool GwmToolbar::eventFilter(QObject *watched, QEvent *event)
{
    if(openLayerBtn == watched || openBtnInfo == watched){
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
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!openBtnInfo->isHidden()){
                openBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(openByXYBtn == watched || openByXYBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(openByXYBtnInfo->isHidden()){
                openByXYBtnInfo->show();
                QPoint point = openByXYBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                openByXYBtnInfo->move(point);
                openByXYBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!openByXYBtnInfo->isHidden()){
                if(!openByXYBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!openByXYBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       openByXYBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!openByXYBtnInfo->isHidden()){
                openByXYBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(saveLayerBtn == watched || saveBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(saveBtnInfo->isHidden()){
                saveBtnInfo->show();
                QPoint point = saveLayerBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                saveBtnInfo->move(point);
                saveBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!saveBtnInfo->isHidden()){
                if(!saveLayerBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!saveBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       saveBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!saveBtnInfo->isHidden()){
                saveBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(exportLayerBtn == watched || exportBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(exportBtnInfo->isHidden()){
                exportBtnInfo->show();
                QPoint point = exportLayerBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                exportBtnInfo->move(point);
                exportBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!exportBtnInfo->isHidden()){
                if(!exportLayerBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!exportBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       exportBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!exportBtnInfo->isHidden()){
                exportBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(editBtn == watched || editBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(editBtnInfo->isHidden()){
                editBtnInfo->show();
                QPoint point = editBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                editBtnInfo->move(point);
                editBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!editBtnInfo->isHidden()){
                if(!editBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!editBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       editBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!editBtnInfo->isHidden()){
                editBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(moveBtn == watched || moveBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(moveBtnInfo->isHidden()){
                moveBtnInfo->show();
                QPoint point = moveBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                moveBtnInfo->move(point);
                moveBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!moveBtnInfo->isHidden()){
                if(!moveBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!moveBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       moveBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!moveBtnInfo->isHidden()){
                moveBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(fullScreenBtn == watched || fullScreenBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(fullScreenBtnInfo->isHidden()){
                fullScreenBtnInfo->show();
                QPoint point = fullScreenBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                fullScreenBtnInfo->move(point);
                fullScreenBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!fullScreenBtnInfo->isHidden()){
                if(!fullScreenBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!fullScreenBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       fullScreenBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!fullScreenBtnInfo->isHidden()){
                fullScreenBtnInfo->hide();
//                return true;
            }
        }
    }
    else if(showPositionBtn == watched || showPositionBtnInfo == watched){
        if(QEvent::Enter ==  event->type()){
            if(showPositionBtnInfo->isHidden()){
                showPositionBtnInfo->show();
                QPoint point = showPositionBtn->pos();
                point.rx() = point.x() + 40 ;
                point.ry() = point.y() + 20;
                showPositionBtnInfo->move(point);
                showPositionBtnInfo->raise();
                return true;
            }
        }
        else if(QEvent::Leave == event->type()){
            if(!showPositionBtnInfo->isHidden()){
                if(!showPositionBtn->geometry().contains(this->mapFromGlobal(QCursor::pos())) //判断鼠标是否在控件上
                        &&!showPositionBtnInfo->geometry().contains(this->mapFromGlobal(QCursor::pos())) )
                   {
                       showPositionBtnInfo->hide();
                       return true;
                   }
            }
        }
        else if(QEvent::MouseButtonRelease == event->type()){
            if(!showPositionBtnInfo->isHidden()){
                showPositionBtnInfo->hide();
//                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
