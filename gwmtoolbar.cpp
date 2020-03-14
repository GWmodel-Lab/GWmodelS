#include "gwmtoolbar.h"

GwmToolbar::GwmToolbar(QWidget *parent) :
    QWidget(parent)
{
    this->setupUi();
}

GwmToolbar::~GwmToolbar()
{

}

void GwmToolbar::setupUi()
{
    createButtons();
    QHBoxLayout* widgetLayout = new QHBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(openLayerBtn);
    widgetLayout->addWidget(openByXYBtn);
    widgetLayout->addWidget(saveLayerBtn);
    widgetLayout->addWidget(exportLayerBtn);
    widgetLayout->addWidget(selectBtn);
    QFrame* seperator1 =  new QFrame(this);
    seperator1->setLineWidth(1);
    seperator1->setFrameStyle(QFrame::Shape::VLine);
    widgetLayout->addWidget(seperator1);
    widgetLayout->addWidget(moveBtn);
    widgetLayout->addWidget(editBtn);
    widgetLayout->addWidget(fullScreenBtn);
    widgetLayout->addWidget(showPositionBtn);
    QFrame* seperator2 =  new QFrame(this);
    seperator2->setLineWidth(1);
    seperator2->setFrameStyle(QFrame::Shape::VLine);
    widgetLayout->addWidget(seperator2);
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
    emit editBtnSignal();
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

QPushButton* createToolbarButton(QString icon, QString tooltip)
{
    QPushButton* button = new QPushButton();
    button->setIcon(QPixmap(icon));
    button->setIconSize(QSize(24, 24));
    button->setFlat(true);
    button->setToolTip(tooltip);
    return button;
}

void GwmToolbar::createButtons()
{
    openLayerBtn = createToolbarButton(
                QStringLiteral("res/icon/folder.png"),
                QStringLiteral("Open"));
    openByXYBtn = createToolbarButton(
                QStringLiteral("res/icon/csv.png"),
                QStringLiteral("Open By XY Coordinate"));
    saveLayerBtn = createToolbarButton(
                QStringLiteral("res/icon/save.png"),
                QStringLiteral("Save"));
    exportLayerBtn = createToolbarButton(
                QStringLiteral("res/icon/download.png"),
                QStringLiteral("Export"));
    editBtn = createToolbarButton(
                QStringLiteral("res/icon/edit.png"),
                QStringLiteral("Edit"));
    selectBtn = createToolbarButton(
                QStringLiteral("res/icon/select.png"),
                QStringLiteral("Select"));
    moveBtn = createToolbarButton(
                QStringLiteral("res/icon/move.png"),
                QStringLiteral("Move"));
    fullScreenBtn = createToolbarButton(
                QStringLiteral("res/icon/view larger.png"),
                QStringLiteral("Full"));
    showPositionBtn = createToolbarButton(
                QStringLiteral("res/icon/map.png"),
                QStringLiteral("Set center position"));

    gwmodelGWRBtn = new QPushButton(tr("GWR"));

    gwmodelGWSSBtn = new QPushButton(tr("GWSS"));

    gwmodelGWPCABtn = new QPushButton(tr("GWPCA"));
}
