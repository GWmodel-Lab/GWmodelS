#include <QWidget>
#include "gwmsymbolwindow.h"
#include <QLabel>
#include <QStyleFactory>
#include <QFile>
#include <QSlider>
#include <QDoubleSpinBox>

GwmSymbolWindow::GwmSymbolWindow(QWidget *parent) : QWidget(parent)
{
    setMinimumWidth(880);
    setMinimumHeight(520);
    setWindowTitle("Symbol");

    createComboBox();
    createStackWidget();
    createButtons();
    createLayout();

}

void GwmSymbolWindow::createsingleSymbolWidget()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    singleSymbolWidget = new QWidget(this);
    singleSymbolWidget->setLayout(mainLayout);
    mainLayout->setMargin(0);

    QFrame* unitWidget = new QFrame(stack);
    QHBoxLayout* unitLayout = new QHBoxLayout(stack);
    unitLayout->setMargin(1);
    unitWidget->setLayout(unitLayout);
    QLabel* unitLabel = new QLabel("Unit");
    unitLayout->setContentsMargins(5,0,5,0);
    unitLabel->adjustSize();
    unitLayout->addWidget(unitLabel,1);
    QComboBox* unitCombobox = new QComboBox(stack);
    unitCombobox->addItem("Millmeter");
    unitCombobox->addItem("Kilometer");
    unitCombobox->addItem("Meter");
    unitLayout->addWidget(unitCombobox,15);
    mainLayout->addWidget(unitWidget);

    QFrame* opacityWidget = new QFrame(stack);
    QHBoxLayout* opacityLayout = new QHBoxLayout(stack);
    opacityLayout->setMargin(1);
    opacityWidget->setLayout(opacityLayout);
    opacityLayout->setContentsMargins(5,0,5,0);
    QLabel* opacityLabel = new QLabel("Opacity");
    opacityLabel->adjustSize();
    opacityLayout->addWidget(opacityLabel,1);
    QSlider* opacitySlider = new QSlider(Qt::Horizontal,stack);
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(100);
    opacityLayout->addWidget(opacitySlider,14);
    QDoubleSpinBox* opacitySpinBox = new QDoubleSpinBox(stack);
    opacitySpinBox->setSuffix("%");
    opacitySpinBox->setMinimum(0);
    opacitySpinBox->setMaximum(100.00);
    opacityLayout->addWidget(opacitySpinBox,1);
    mainLayout->addWidget(opacityWidget);

    QFrame* colorWidget = new QFrame(stack);
    QHBoxLayout* colorLayout = new QHBoxLayout(stack);
    colorLayout->setMargin(1);
    colorWidget->setLayout(colorLayout);
    QLabel* colorLabel = new QLabel("Color");
    colorLayout->setContentsMargins(5,0,5,0);
    colorLabel->adjustSize();
    colorLayout->addWidget(colorLabel,1);
    QComboBox* colorCombobox = new QComboBox(stack);
    colorCombobox->addItem("Millmeter");
    colorCombobox->addItem("Kilometer");
    colorCombobox->addItem("Meter");
    colorLayout->addWidget(colorCombobox,15);
    mainLayout->addWidget(colorWidget);

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

    mainLayout->addWidget(new QLabel("graduatedWidget"));

}

void GwmSymbolWindow::createLayout()
{
    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->addWidget(symbolTypeSelect);
    windowLayout->setMargin(1);
    windowLayout->addWidget(stack);
    windowLayout->addStretch(1);
    windowLayout->addWidget(btnContainer);
    setLayout(windowLayout);
}

void GwmSymbolWindow::createComboBox()
{
    symbolTypeSelect = new QComboBox(this);
//    symbolTypeSelect->setMaximumWidth(this->width()-10);
//    symbolTypeSelect->setStyleSheet("border");
    symbolTypeSelect->setStyle(QStyleFactory::create("Fusion"));
    symbolTypeSelect->addItem("Single Symbol");
    symbolTypeSelect->addItem("Categorized");
    symbolTypeSelect->addItem("Graduated");
//    connect(symbolTypeSelect,&QComboBox::currentIndexChanged,this,&GwmSymbolWindow::toggleWidget);
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

    btnContainer->setLayout(btnLayout);
}
