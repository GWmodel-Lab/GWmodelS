#include "gwmpropertygwpcatab.h"
#include "ui_gwmpropertygwpcatab.h"

#include <armadillo>

#include <QMessageBox>
#include <QTableWidget>
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

GwmPropertyGWPCATab::GwmPropertyGWPCATab(QWidget *parent, GwmLayerGWPCAItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGWPCATab),
    mLayerItem(item)
{
    ui->setupUi(this);
    if (item)
    {

        if (item->bandwidthOptimized())
        {
            mBandwidthSelPlot = new GwmPlot();
            ui->grpBwSelView->layout()->addWidget(mBandwidthSelPlot);
//            ui->grpBwSelView->hide();
        }
        else
        {
            ui->grpBwSelView->hide();
        }
    }
}

GwmPropertyGWPCATab::~GwmPropertyGWPCATab()
{
    delete ui;
}

void GwmPropertyGWPCATab::updateUI()
{
    if (!mLayerItem)
        return;

    GwmBandwidthWeight weight = mLayerItem->weight();
    ui->lblKernelFunction->setText(GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(weight.kernel()));
    ui->lblBandwidthType->setText(weight.adaptive() ? tr("Adaptive") : tr("Fixed"));
    if (weight.adaptive())
    {
        QString bwSizeString = QString("%1 (number of nearest neighbours)").arg(int(weight.bandwidth()));
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    else
    {
        QString bwSizeString = QString("%1 %2")
                .arg(weight.bandwidth(), 0, 'f', 12)
                .arg(weight.bandwidth());
        ui->lblBandwidthSize->setText(bwSizeString);
    }
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }
    //ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));
    ui->lblPCCount->setText(QString("%1").arg(mLayerItem->mK));

    // 计算四分位数
    //QList<GwmVariable> indepVars = mLayerItem->indepVars();
    ui->tbwProp->setRowCount(mLayerItem->mK+1);
    ui->tbwProp->setColumnCount(6);
    ui->tbwProp->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwProp->setHorizontalHeaderLabels(headers);
    //const mat& betas = mLayerItem->betas();
    const mat& betas2 = mLayerItem->mDResult1;
    const mat& betas = mLayerItem->mLocalPV;
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas.n_cols; r++)
    {
        vec q = quantile(betas.col(r), p);
        QString name = QString("Comp.%1").arg(r+1);
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwProp->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwProp->setItem(r, c + 1, quantileItem);
        }
    }
    //quantile(sum(betas,1),p)
    vec q = quantile(sum(betas,1),p);
    QString name = QString("Cumulative");
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
    ui->tbwProp->setItem(betas.n_cols, 0, nameItem);
    for (int c = 0; c < 5; c++)
    {
        QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
        quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwProp->setItem(betas.n_cols, c + 1, quantileItem);
    }

    ui->tbwLocalvariance->setRowCount(mLayerItem->mK);
    ui->tbwLocalvariance->setColumnCount(6);
    ui->tbwLocalvariance->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers2 = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwLocalvariance->setHorizontalHeaderLabels(headers);
    //const mat& betas = mLayerItem->betas();
    //const mat&betas2 = mLayerItem->mdResult1;
    //const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for (uword r = 0; r < betas2.n_cols; r++)
    {
        vec q = quantile(betas2.col(r), p);
        QString name = QString("Comp.%1").arg(r+1);
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwLocalvariance->setItem(r, 0, nameItem);
        for (int c = 0; c < 5; c++)
        {
            QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
            quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
            ui->tbwLocalvariance->setItem(r, c + 1, quantileItem);
        }
    }
    // 绘制可视化图标
    if(mLayerItem->bandwidthOptimized())
    {
        BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        QVariant data = QVariant::fromValue(bwScores);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }
}

bool GwmPropertyGWPCATab::openSelectFile()//弹出选择文件对话框
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

void GwmPropertyGWPCATab::on_btnSaveRes_clicked()
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
              out << "  Model Calibration Information"<< Qt::endl;
              out << "----------------------------------------------"<< Qt::endl;
              out << "Kernel function:  "; out << ui->lblKernelFunction->text() << Qt::endl;
              out << "Bandwidth Type:   "; out << ui->lblBandwidthType->text() << Qt::endl;
              out << "BandWidth: "; out << ui->lblBandwidthSize->text() << Qt::endl;
              out << "Distance metric:   "; out << ui->lblDistanceMetric->text() << Qt::endl;
              out << "Principle components:  "; out << ui->lblPCCount->text() << Qt::endl;
              out << "" << Qt::endl;
              out << "**********************************************" << Qt::endl;
              out << "" << Qt::endl;
              out << "  Local variance"<< Qt::endl;
              out << "----------------------------------------------"<< Qt::endl;
              for(int i = 0 ; i < 6 ; i++){
                  out << ui->tbwLocalvariance->horizontalHeaderItem(i)->text();
                  out << "\t";
              }
              out << "" << Qt::endl;

              for(int i = 0 ; i < ui->tbwLocalvariance->rowCount() ; i++){
                  for (int j = 0 ; j < 6; j++){
                       out << ui->tbwLocalvariance->item(i, j)->text();

                       out << "\t";
                  }
                  out << "" << Qt::endl;
              }
              out << "" << Qt::endl;
              out << "**********************************************" << Qt::endl;
              out << "" << Qt::endl;
              out << "  Local Proportion of Variance"<< Qt::endl;
              out << "----------------------------------------------"<< Qt::endl;
              for(int i = 0 ; i < 6 ; i++){
                  out << ui->tbwProp->horizontalHeaderItem(i)->text();
                  out << (i == 0 ? "\t\t" : "\t");
              }
              out << "" << Qt::endl;

              for(int i = 0 ; i < ui->tbwProp->rowCount() ; i++){
                  for (int j = 0 ; j < 6; j++){
                       out << ui->tbwProp->item(i, j)->text();

                       out << ((j == 0 && i == ui->tbwProp->rowCount() - 1) ? "\t" : "\t\t");
                  }
                  out << "" << Qt::endl;
              }
              out << "" << Qt::endl;
              out << "**********************************************" << Qt::endl;
              out << "" << Qt::endl;
              out << "GWmodel Lab\t"; out << "http://gwmodel.whu.edu.cn/"<< Qt::endl;
              out << "Contact us\t"; out << "binbinlu@whu.edu.cn";
              myfile.close();
          }
}

