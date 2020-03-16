#include <QWidget>
#include "gwmsymbolwindow.h"
#include <QLabel>
#include <QStyleFactory>
#include <QFile>
#include <QSlider>
#include <QDoubleSpinBox>
#include <qgsvectorlayer.h>
#include <QDebug>
#include <qgssinglesymbolrenderer.h>
#include <qgsmarkersymbollayer.h>
#include <qgssymbol.h>
#include <QListWidget>
#include <qgssymbollayerregistry.h>
#include <qgsapplication.h>
#include "qgsguiutils.h"
#include "qgsgui.h"
#include <qgssymbollayerutils.h>



GwmSymbolWindow::GwmSymbolWindow(QgsVectorLayer* vectorLayer,QWidget *parent) : QWidget(parent)
{
    if(vectorLayer){
        mLayer = vectorLayer;
    }
    setMinimumWidth(880);
    setMinimumHeight(520);
    setWindowTitle("Symbol");
    getLayerSymbol();
    createComboBox();
    createStackWidget();
    createButtons();
    createLayout();
}

void GwmSymbolWindow::createsingleSymbolWidget()
{
    singleSymbolWidget = new QgsSymbolSelectorWidget( mSymbol, QgsStyle::defaultStyle(), mLayer, this);
}

void GwmSymbolWindow::createcategorizedWidget()
{
    categorizedWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    mainLayout->setMargin(0);
    categorizedWidget->setLayout(mainLayout);
    QLabel* label1 = new QLabel("categorizedWidget");
    label1->adjustSize();
    mainLayout->addWidget(label1);
}

void GwmSymbolWindow::creategraduatedWidget()
{
    graduatedWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    graduatedWidget->setLayout(mainLayout);

    mainLayout->addWidget(new QLabel(tr("graduatedWidget")));

}

void GwmSymbolWindow::createLayout()
{
    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->addWidget(symbolTypeSelect);
    windowLayout->setMargin(6);
    windowLayout->addWidget(stack);
    windowLayout->addWidget(btnContainer);
    setLayout(windowLayout);
}

void GwmSymbolWindow::createComboBox()
{
    symbolTypeSelect = new QComboBox(this);
    symbolTypeSelect->setStyle(QStyleFactory::create("Fusion"));
    symbolTypeSelect->addItem("Single Symbol");
    symbolTypeSelect->addItem("Categorized");
    symbolTypeSelect->addItem("Graduated");
    connect(symbolTypeSelect,SIGNAL(currentIndexChanged(int)),this,SLOT(toggleWidget(int)));
}

void GwmSymbolWindow::createStackWidget()
{
    stack = new QStackedWidget(this);
    stack->setFrameStyle(QFrame::Panel | QFrame::Raised );
    createsingleSymbolWidget();
    createcategorizedWidget();
    creategraduatedWidget();
    stack->addWidget(singleSymbolWidget);
    stack->addWidget(categorizedWidget);
    stack->addWidget(graduatedWidget);
}

//combobox值改变 切换窗口槽函数
void GwmSymbolWindow::toggleWidget(int index)
{
    stack->setCurrentIndex(index);
}

void GwmSymbolWindow::createButtons()
{
    btnContainer = new QWidget(this);
    QHBoxLayout* btnLayout = new QHBoxLayout(this);

    okBtn = new QPushButton(tr("OK"));
    okBtn->setFixedSize(80,25);

    cancelBtn = new QPushButton(tr("Cancel"));
    cancelBtn->setFixedSize(80,25);

    applyBtn = new QPushButton(tr("Apply"));
    applyBtn->setFixedSize(80,25);

    helpBtn = new QPushButton(tr("Help"));
    helpBtn->setFixedSize(80,25);

    btnLayout->addStretch(1);
    btnLayout->setSpacing(5);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(helpBtn);

    connect(applyBtn,&QPushButton::clicked,this,&GwmSymbolWindow::applyLayerSymbol);
    connect(cancelBtn,&QPushButton::clicked,this,&GwmSymbolWindow::close);
    connect(okBtn,&QPushButton::clicked,this,&GwmSymbolWindow::onOkBtnClicked);
    btnContainer->setLayout(btnLayout);
}

void GwmSymbolWindow::applyLayerSymbol()
{
//    mSingleRenderer->setSymbol( mSymbol );
    mLayer->triggerRepaint();
    mLayer->emitStyleChanged();
    mLastSymbol = mSingleRenderer->symbol()->clone();

}

void GwmSymbolWindow::onOkBtnClicked()
{
    applyLayerSymbol();
    this->close();
}

void GwmSymbolWindow::closeEvent(QCloseEvent *ev)
{
    mSingleRenderer->setSymbol(mLastSymbol);
    ev->accept();
}

void GwmSymbolWindow::getLayerSymbol()
{
    mSingleRenderer = (QgsSingleSymbolRenderer*)mLayer->renderer();
    mSymbol =  mSingleRenderer->symbol();
    mLastSymbol = mSingleRenderer->symbol()->clone();
    std::unique_ptr< QgsSymbol > symbol( mSingleRenderer->symbol() ? mSingleRenderer->symbol()->clone() : nullptr );

}
