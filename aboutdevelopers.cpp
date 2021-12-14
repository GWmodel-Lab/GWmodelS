#include "aboutdevelopers.h"
#include "ui_aboutdevelopers.h"

aboutDevelopers::aboutDevelopers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDevelopers)
{
    ui->setupUi( this );
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
}

aboutDevelopers::~aboutDevelopers()
{
    delete ui;
}
