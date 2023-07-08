#include "gwmsaveascsvdialog.h"
#include "ui_gwmsaveascsvdialog.h"
#include <qfiledialog.h>

GwmSaveAsCSVDialog::GwmSaveAsCSVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmSaveAsCSVDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Save As CSV"));
    connect(ui->mOpenFileDialogButton,&QPushButton::clicked,this,&GwmSaveAsCSVDialog::onOpenFileDialog);
    connect(ui->mAddXYCheckBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->mAddXYCheckBox->checkState() == Qt::Checked){
            ui->mXColumnName->setEnabled(true);
            ui->mYColumnName->setEnabled(true);
        }
        else{
            ui->mXColumnName->setEnabled(false);
            ui->mYColumnName->setEnabled(false);
        }
    });
}

GwmSaveAsCSVDialog::~GwmSaveAsCSVDialog()
{
    delete ui;
}

void GwmSaveAsCSVDialog::onOpenFileDialog()
{
    QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),tr("CSV (*.csv)"));
    ui->mFilePath->setText(filePath);
}

QString GwmSaveAsCSVDialog::filepath()
{
    return ui->mFilePath->text();
}

QString GwmSaveAsCSVDialog::xcolumnname()
{
    return  ui->mXColumnName->text();
}

QString GwmSaveAsCSVDialog::ycolumnname()
{
    return  ui->mYColumnName->text();
}

bool GwmSaveAsCSVDialog::isAddXY()
{
    if(ui->mAddXYCheckBox->checkState() == Qt::Checked){
        return true;
    }
    return false;
}

