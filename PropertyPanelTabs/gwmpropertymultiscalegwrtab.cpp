#include "gwmpropertymultiscalegwrtab.h"
#include "ui_gwmpropertymultiscalegwrtab.h"

#include <armadillo>

#include <QMessageBox>
#include <QtableWidget>
#include <QListWidget>
#include <QDebug>
#include <QVariant>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QModelIndex>
#include <QHeaderView>
#include <QStandardItemModel>

using namespace arma;

GwmPropertyMultiscaleGWRTab::GwmPropertyMultiscaleGWRTab(QWidget *parent, GwmLayerMultiscaleGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyMultiscaleGWRTab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {
        mParameterSpecifiedModel = new GwmPropertyMultiscaleParameterSpecifiedItemModel(item, this);
        if (!item->hasHatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
    }
}

GwmPropertyMultiscaleGWRTab::~GwmPropertyMultiscaleGWRTab()
{
    delete ui;
}

void GwmPropertyMultiscaleGWRTab::updateUI()
{
    if (!mLayerItem)
        return;
    if (true)
    {
        ui->lblRegressionPoints->setText(tr("the same location as observations are used."));
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    // 带宽参数
    ui->trvBandwdithDistance->setModel(mParameterSpecifiedModel);

    if (mLayerItem->hasHatmatrix())
    {
        GwmDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblRSquare->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
        ui->lblRSquareAdjusted->setText(QString("%1").arg(diagnostic.RSquareAdjust, 0, 'f', 6));
    }

    // 计算四分位数
    QList<GwmVariable> indepVars = mLayerItem->indepVars();
    ui->tbwCoefficient->setRowCount(indepVars.size() + 1);
    ui->tbwCoefficient->setColumnCount(6);
    ui->tbwCoefficient->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwCoefficient->setHorizontalHeaderLabels(headers);
    const mat& betas = mLayerItem->betas();
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = (r == 0) ? QStringLiteral("Intercept") : indepVars[r - 1].name;
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwCoefficient->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwCoefficient->setItem(r, c + 1, quantileItem);
        }
    }

}

bool GwmPropertyMultiscaleGWRTab::openSelectFile()//弹出选择文件对话框
{
  QString strPath = QFileDialog::getSaveFileName(NULL,QString::fromUtf8("选择文件"),"",QObject::tr("txt(*.txt)"));
  if(strPath == "")
  {
     QMessageBox::information(this,QString::fromUtf8("提示"),QString::fromUtf8("选择文件失败，无路径"),"OK");
    return false;//用户点击的取消按钮
  }
  FilePath = strPath;
  return true;
}

////遍历树形结构
//void GwmPropertyMultiscaleGWRTab::GetNode(GwmPropertyMultiscaleParameterSpecifiedItemModel* model)
//{
//    for(int i = 0;i < model->rowCount() ;i++)
//    {
//        QStandardItem *item = model->item(i);

//        qDebug() <<"item = " << item->text();
//        GetItem(model->item(i) );
//    }
//}
////递归获取所有节点
//void GwmPropertyMultiscaleGWRTab::GetItem(QStandardItem *item)
//{
//    Q_ASSERT(item);
//    if(item->hasChildren())
//    {
//        for(int i = 0;i < item->rowCount() ;i++)
//        {
//            QStandardItem * childitem = item->child(i);
//            qDebug() << "childitem = " << childitem->text();
//            GetItem(childitem);
//        }
//    }
//}

void GwmPropertyMultiscaleGWRTab::on_btnSaveRes_clicked()
{
    if(false == openSelectFile())//弹出选择文件对话框 如果成功选择文件，主线程myWidget类就有对象存储了文件路径
      {
        return;
      }
      if(FilePath == "")//如果没有选择文件，即文件路径为空
      {
        return;
      }

      QFile myfile(FilePath);//创建一个输出文件的文档
          if (myfile.open(QFile::WriteOnly|QFile::Text))//注意WriteOnly是往文本中写入的时候用，ReadOnly是在读文本中内容的时候用，Truncate表示将原来文件中的内容清空
          {
              QTextStream out(&myfile);
//              使用组件赋值
              out << "  Model Calibration Information"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Regression points:  "; out << ui->lblRegressionPoints->text() <<endl;
              out << "" << endl;
              out << "**********************************************" << endl;
              out << "" << endl;
              out << "  Diagnostic Information"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Number of data points: "; out << ui->lblNumberDataPoints->text() << endl;
              out << "AICc:  "; out << ui->lblAICc->text() <<endl;
              out << "Residual sum of squares:   "; out << ui->lblRSS->text() <<endl;
              out << "R-square value:  "; out << ui->lblRSquare->text() <<endl;
              out << "Adjusted R-square value:   "; out << ui->lblRSquareAdjusted->text() <<endl;
              out << "" <<endl;
              out << "**********************************************" << endl;
              out << "" << endl;
              out << "  Summary of GWR Coefficient Estimates"<<endl;
              out << "----------------------------------------------"<<endl;
              for(int i = 0 ; i < 6 ; i++){
                  out << ui->tbwCoefficient->horizontalHeaderItem(i)->text();
                  out << (i == 0 ? "\t\t" : "\t");
              }
              out << "" <<endl;

              for(int i = 0 ; i < ui->tbwCoefficient->rowCount() ; i++){
                  for (int j = 0 ; j < 6; j++){
                       out << ui->tbwCoefficient->item(i, j)->text();

                       out << ((j == 0 && i != 0) ? "\t\t" : "\t");
                  }
                  out << "" << endl;
              }
              out << "" <<endl;
              out << "**********************************************" << endl;
              out << "" << endl;

              out << "  Bandwidth and Distance"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Name\t\t"; out << "Adaptive\t"; out << "Size\t"; out << "Type\t";
              out << "Kernel\t"; out << "Seled\t";out << "Approach\t";out << "Distance\t"<<endl;
              out << "------------------------------------------------------------------------------------------------------------"<<endl;
              for(int i = 0 ; i < mParameterSpecifiedModel->rowCount() ; i++){
                  out << mParameterSpecifiedModel->data(ui->trvBandwdithDistance->model()->index(i,0)).toString();
                  i == 0 ? out << "\t" : out << "\t\t";
                  out << mParameterSpecifiedModel->data(ui->trvBandwdithDistance->model()->index(i,1)).toString();
                  out <<"\t";
                  for(int j = 0 ; j < 6; j++){
                      out << mParameterSpecifiedModel->data(ui->trvBandwdithDistance->model()->index(i,0).child(j, 1)).toString();
                      out << "\t";
                  }
                  out <<"" <<endl;
              }


              out << "" <<endl;
              out << "**********************************************" << endl;
              out << "" << endl;
              out << "GWmodel Lab\t"; out << "http://gwmodel.whu.edu.cn/"<<endl;
              out << "Contact us\t"; out << "binbinlu@whu.edu.cn";
              myfile.close();
          }
}

