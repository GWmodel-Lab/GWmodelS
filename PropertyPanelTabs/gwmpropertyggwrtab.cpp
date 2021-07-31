#include "gwmpropertyggwrtab.h"
#include "ui_gwmpropertyggwrtab.h"

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

QMap<GwmGeneralizedGWRAlgorithm::Family, QString> GwmPropertyGGWRTab::familyTypeNameDict = {
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Poisson, QStringLiteral("Poisson")),
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Binomial, QStringLiteral("Binomial"))
};

GwmPropertyGGWRTab::GwmPropertyGGWRTab(QWidget *parent,GwmLayerGGWRItem* item) :
    QWidget(parent),
    ui(new Ui::GwmPropertyGGWRTab),
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
//            ui->grpBwSelView->hide();
        }
        else
        {
            ui->grpBwSelView->hide();
        }
        if (!item->hatmatrix())
        {
            ui->grpDiagnostic->hide();
        }
        ui->grpFTest->hide();
    }
}

GwmPropertyGGWRTab::~GwmPropertyGGWRTab()
{
    delete ui;
}

void GwmPropertyGGWRTab::updateUI()
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
    ui->lblFamily->setText(familyTypeNameDict[mLayerItem->family()]);
    ui->lblNumberDataPoints->setText(QString("%1").arg(mLayerItem->dataPointsSize()));

    //GLM计算结果
    GwmGLMDiagnostic GLMdiagnostic = mLayerItem->GLMdiagnostic();
    ui->lblNullDev->setText(QString("%1").arg(GLMdiagnostic.NullDev, 0, 'f', 6));
    ui->lblGLMDev->setText(QString("%1").arg(GLMdiagnostic.Dev, 0, 'f', 6));
    ui->lblGLMAIC->setText(QString("%1").arg(GLMdiagnostic.AIC, 0, 'f', 6));
    ui->lblGLMAICc->setText(QString("%1").arg(GLMdiagnostic.AICc, 0, 'f', 6));
    ui->lblGLMRSS->setText(QString("%1").arg(GLMdiagnostic.RSquare, 0, 'f', 6));

    if (mLayerItem->hatmatrix())
    {
        GwmGGWRDiagnostic diagnostic = mLayerItem->diagnostic();
        ui->lblGwDev->setText(QString("%1").arg(diagnostic.RSS, 0, 'f', 6));
        ui->lblAIC->setText(QString("%1").arg(diagnostic.AIC, 0, 'f', 6));
        ui->lblAICc->setText(QString("%1").arg(diagnostic.AICc, 0, 'f', 6));
        ui->lblRSS->setText(QString("%1").arg(diagnostic.RSquare, 0, 'f', 6));
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

void GwmPropertyGGWRTab::setQuartiles(const int row, QString name, const GwmQuartiles &quartiles)
{
    ui->tbwCoefficient->setItem(row, 0, new QTableWidgetItem(name));
    QTableWidgetItem* minItem = new QTableWidgetItem(QString("%1").arg(quartiles.min, 0, 'f', 3));
    QTableWidgetItem* firstItem = new QTableWidgetItem(QString("%1").arg(quartiles.first, 0, 'f', 3));
    QTableWidgetItem* medianItem = new QTableWidgetItem(QString("%1").arg(quartiles.median, 0, 'f', 3));
    QTableWidgetItem* thirdItem = new QTableWidgetItem(QString("%1").arg(quartiles.third, 0, 'f', 3));
    QTableWidgetItem* maxItem = new QTableWidgetItem(QString("%1").arg(quartiles.max, 0, 'f', 3));
    minItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    firstItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    medianItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    thirdItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tbwCoefficient->setItem(row, 1, minItem);
    ui->tbwCoefficient->setItem(row, 2, firstItem);
    ui->tbwCoefficient->setItem(row, 3, medianItem);
    ui->tbwCoefficient->setItem(row, 4, thirdItem);
    ui->tbwCoefficient->setItem(row, 5, maxItem);

}

bool GwmPropertyGGWRTab::openSelectFile()//弹出选择文件对话框
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


void GwmPropertyGGWRTab::on_btnSaveRes_clicked()
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
              out << "Used family:   "; out << ui->lblFamily->text() <<endl;
              out << "" << endl;
              out << "**********************************************" << endl;
              out << "" << endl;

              out << "  GLM Result"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Null deviance: "; out << ui->lblNullDev->text() << endl;
              out << "AIC:  "; out << ui->lblGLMAIC->text() <<endl;
              out << "AICc:   "; out << ui->lblGLMAICc->text() <<endl;
              out << "Preudo R-square value: "; out << ui->lblGLMRSS->text() << endl;
              out << "" <<endl;
              out << "**********************************************" << endl;
              out << "" << endl;

              out << " GGWR Diagnostic Information"<<endl;
              out << "----------------------------------------------"<<endl;
              out << "Number of data points: "; out << ui->lblNumberDataPoints->text() << endl;
              out << "GW Deviance:  "; out << ui->lblGwDev->text() <<endl;
              out << "AIC: "; out << ui->lblAIC->text() << endl;
              out << "AICc:  "; out << ui->lblAICc->text() <<endl;
              out << "Preudo R-square value:  "; out << ui->lblRSS->text() <<endl;
              out << "" <<endl;
              out << "**********************************************" << endl;
              out << "" << endl;

              out << "  Summary of GGWR Coefficient Estimates"<<endl;
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

