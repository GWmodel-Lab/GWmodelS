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
    widgetLayout->setSpacing(0);
    widgetLayout->addWidget(openLayerBtn);
    widgetLayout->addWidget(openByXYBtn);
    widgetLayout->addWidget(saveLayerBtn);
    widgetLayout->addWidget(exportLayerBtn);
    QFrame* seperator1 =  new QFrame(this);
    seperator1->setLineWidth(1);
    seperator1->setFrameStyle(QFrame::Shape::VLine);
    widgetLayout->addWidget(seperator1);
    widgetLayout->addWidget(selectBtn);
    widgetLayout->addWidget(moveBtn);
    widgetLayout->addWidget(editBtn);
    widgetLayout->addWidget(zoomFull);
    widgetLayout->addWidget(zoomToLayer);
    widgetLayout->addWidget(zoomToSelection);
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
    connect(openLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportShapefileSignal);
    connect(saveLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportJsonSignal);
    connect(exportLayerBtn,&QPushButton::clicked,this,&GwmToolbar::openFileImportCsvSignal);
    connect(openByXYBtn,&QPushButton::clicked,this,&GwmToolbar::openByXYBtnSingnal);
    connect(selectBtn, &QPushButton::clicked, this, &GwmToolbar::selectBtnSignal);
    connect(moveBtn,&QPushButton::clicked,this,&GwmToolbar::moveBtnSignal);
    connect(editBtn,&QPushButton::clicked,this,&GwmToolbar::editBtnSignal);
    connect(zoomFull,&QPushButton::clicked,this,&GwmToolbar::zoomFullBtnSignal);
    connect(zoomToLayer,&QPushButton::clicked,this,&GwmToolbar::zoomToLayerBtnSignal);
    connect(zoomToSelection,&QPushButton::clicked,this,&GwmToolbar::zoomToSelectionBtnSignal);
    connect(gwmodelGWRBtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWRBtnSignal);
    connect(gwmodelGWSSBtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWSSBtnSignal);
    connect(gwmodelGWPCABtn,&QPushButton::clicked,this,&GwmToolbar::gwmodelGWPCABtnSignal);
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
    openLayerBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionAddOgrLayer.svg"), tr("Open"));
    openByXYBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionAddDelimitedTextLayer.svg"), tr("Open By XY Coordinate"));
    saveLayerBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionFileSave.svg"), tr("Save"));
    exportLayerBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionFileSaveAs.svg"), tr("Export"));
    editBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionToggleEditing.svg"), tr("Edit"));
    selectBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionIdentify.svg"), tr("Select"));
    moveBtn = createToolbarButton(QStringLiteral(":/images/themes/default/mActionPan.svg"), tr("Move"));
    zoomFull = createToolbarButton(QStringLiteral(":/images/themes/default/mActionZoomFullExtent.svg"), tr("Zoom Full"));
    zoomToLayer = createToolbarButton(QStringLiteral(":/images/themes/default/mActionZoomToLayer.svg"), tr("Zoom to Layer"));
    zoomToSelection = createToolbarButton(QStringLiteral(":/images/themes/default/mActionZoomToArea.svg"), tr("zoom to Selection"));

    gwmodelGWRBtn = new QPushButton(tr("GWR"));

    gwmodelGWSSBtn = new QPushButton(tr("GWSS"));

    gwmodelGWPCABtn = new QPushButton(tr("GWPCA"));
}
