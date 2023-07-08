#include "gwmcsvtodatdialog.h"
#include "ui_gwmcsvtodatdialog.h"
#include <qfiledialog.h>

GwmCsvToDatDialog::GwmCsvToDatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmCsvToDatDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Csv To Dmat"));

    connect(ui->mCsvOpenButton,&QPushButton::clicked,this,&GwmCsvToDatDialog::onCsvOpenButton);
    connect(ui->mDatOpenButton,&QPushButton::clicked,this,&GwmCsvToDatDialog::onDatOpenButton);
}

GwmCsvToDatDialog::~GwmCsvToDatDialog()
{
    delete ui;
}

QString GwmCsvToDatDialog::csvFileName()
{
    return ui->mCsvFileName->text();
}

QString GwmCsvToDatDialog::datFileName()
{
    return ui->mDatFileName->text();
}

void GwmCsvToDatDialog::onCsvOpenButton()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open CSV File"),tr(""),tr("CSV (*.csv)"));
    ui->mCsvFileName->setText(filePath);
}

void GwmCsvToDatDialog::onDatOpenButton()
{
    QString filePath = QFileDialog::getSaveFileName(this,tr("Save As Dmat File"),tr(""),tr("Dmat (*.dmat)"));
    ui->mDatFileName->setText(filePath);
}

bool GwmCsvToDatDialog::isColumnStore()
{
    if(ui->mIsColumnStore->checkState() == Qt::Checked)
    {
        return true;
    }
    return false;
}

