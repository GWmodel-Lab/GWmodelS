#include "aboutdevelopteam.h"
#include "ui_aboutdevelopteam.h"

aboutdevelopteam::aboutdevelopteam(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutdevelopteam)
{
    ui->setupUi(this);
    ui->label_6->setText(tr("<a href = 'http://gwmodel.whu.edu.cn/'>http://gwmodel.whu.edu.cn/</a>"));
    ui->label_6->setOpenExternalLinks( true );
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
}

aboutdevelopteam::~aboutdevelopteam()
{
    delete ui;
}

void aboutdevelopteam::on_btnOK_clicked()
{

}

