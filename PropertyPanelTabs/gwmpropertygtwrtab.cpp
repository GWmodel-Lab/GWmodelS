#include "gwmpropertygtwrtab.h"
#include "ui_gwmpropertygtwrtab.h"

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

GwmPropertyGTWRTab::GwmPropertyGTWRTab(QWidget *parent, GwmLayerGTWRItem *item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGTWRTab),
    mLayerItem(item),
    mBandwidthSelPlot(nullptr)
{
    ui->setupUi(this);
    if (item)
    {
        if (item->bandwidthOptimized())
        {
            mBandwidthSelPlot = new GwmPlot();
            ui->grpBwSelView->layout()->addWidget(mBandwidthSelPlot);
        }
        else
        {
            ui->grpBwSelView->hide();
        }
        if (!item->hatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
    }
}

GwmPropertyGTWRTab::~GwmPropertyGTWRTab()
{
    delete ui;
}

void GwmPropertyGTWRTab::updateUI()
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
    if (mLayerItem->regressionPointGiven())
    {
        ui->lblRegressionPoints->setText(tr("A seperate set of regression points is used."));
    }
    else
    {
        ui->lblRegressionPoints->setText(tr("The same location as observations are used."));
    }
    if (true)
    {
        ui->lblDistanceMetric->setText(tr("Edclidean distance metric is used."));
    }
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    if (mLayerItem->hatmatrix())
    {
        GwmDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblENP->setText(QString("%1").arg(diagnostic.ENP, 0, 'f', 6));
        ui->lblEDF->setText(QString("%1").arg(diagnostic.EDF, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
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

    if(mLayerItem->bandwidthOptimized())
    {
        BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        QVariant data = QVariant::fromValue(bwScores);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }
}

bool GwmPropertyGTWRTab::openSelectFile()//弹出选择文件对话框
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

void GwmPropertyGTWRTab::on_btnSaveRes_clicked()
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
              out << "Kernel function:  "; out << ui->lblKernelFunction->text() <<endl;
              out << "Bandwidth Type:   "; out << ui->lblBandwidthType->text() <<endl;
              out << "BandWidth: "; out << ui->lblBandwidthSize->text() << endl;
              out << "Regression points:  "; out << ui->lblRegressionPoints->text() <<endl;
              out << "Distance metric:   "; out << ui->lblDistanceMetric->text() <<endl;
              out << "" << endl;
              out << "**********************************************" << endl;
              out << "" << endl;
              out << "  Diagnostic Information"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Number of data points: "; out << ui->lblNumberDataPoints->text() << endl;
              out << "Effective number of parameters:  "; out << ui->lblENP->text() <<endl;
              out << "Effective degrees of freedom:   "; out << ui->lblEDF->text() <<endl;
              out << "AIC: "; out << ui->lblAIC->text() << endl;
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
              out << "GWmodel Lab\t"; out << "http://gwmodel.whu.edu.cn/"<<endl;
              out << "Contact us\t"; out << "binbinlu@whu.edu.cn";
              myfile.close();
          }
}


