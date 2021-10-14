#include "aboutdevelopers.h"
#include "ui_aboutdevelopers.h"

aboutDevelopers::aboutDevelopers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDevelopers)
{
    ui->setupUi( this );
}

aboutDevelopers::~aboutDevelopers()
{
    delete ui;
}
