#include "gwmcorrelationdialog.h"
#include "ui_gwmcorrelationdialog.h"

gwmcorrelationdialog::gwmcorrelationdialog(QWidget *parent) :
//    QWidget(parent),
    ui(new Ui::gwmcorrelationdialog)
{
    ui->setupUi(this);
}

gwmcorrelationdialog::~gwmcorrelationdialog()
{
    delete ui;
}
