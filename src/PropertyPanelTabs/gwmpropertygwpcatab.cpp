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
    {
        qDebug() << "[GwmPropertyGWPCATab::updateUI] ERROR: mLayerItem is NULL!";
        return;
    }

    gwm::BandwidthWeight weight = mLayerItem->weight();
    ui->lblKernelFunction->setText(QString::fromStdString(gwm::BandwidthWeight::KernelFunctionTypeNameMapper.at(weight.kernel())));
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
    ui->lblPCCount->setText(QString("%1").arg(mLayerItem->mK));

    qDebug() << "[GwmPropertyGWPCATab::updateUI] Accessing data matrices...";
    const mat& betas = mLayerItem->mLocalPV;
    qDebug() << "[GwmPropertyGWPCATab::updateUI] mLocalPV accessed:" << betas.n_rows << "x" << betas.n_cols << ", empty:" << betas.is_empty();
    
    const mat& betas2 = mLayerItem->mDResult1;
    qDebug() << "[GwmPropertyGWPCATab::updateUI] mDResult1 accessed:" << betas2.n_rows << "x" << betas2.n_cols << ", empty:" << betas2.is_empty();
    
    if (betas.is_empty() || betas.n_cols == 0)
    {
        qDebug() << "[GwmPropertyGWPCATab::updateUI] ERROR: mLocalPV is empty or has no columns";
        return;
    }
    
    if (betas2.is_empty() || betas2.n_cols == 0)
    {
        qDebug() << "[GwmPropertyGWPCATab::updateUI] ERROR: mDResult1 is empty or has no columns";
        return;
    }
    
    qDebug() << "[GwmPropertyGWPCATab::updateUI] Data matrices are valid, proceeding...";

    // 计算四分位数 - Local Proportion of Variance
    ui->tbwProp->setRowCount(mLayerItem->mK+1);
    ui->tbwProp->setColumnCount(6);
    ui->tbwProp->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwProp->setHorizontalHeaderLabels(headers);
    
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    uword nCols = std::min((uword)mLayerItem->mK, betas.n_cols);
    for (uword r = 0; r < nCols; r++)
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
    ui->tbwProp->setItem(nCols, 0, nameItem);
    for (int c = 0; c < 5; c++)
    {
        QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
        quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        ui->tbwProp->setItem(nCols, c + 1, quantileItem);
    }

    uword nVarCols = betas2.n_cols;
    uword nDisplayCols = std::min((uword)mLayerItem->mK, nVarCols);
    ui->tbwLocalvariance->setRowCount(nDisplayCols);
    ui->tbwLocalvariance->setColumnCount(6);
    ui->tbwLocalvariance->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QStringList headers2 = QStringList() << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
    ui->tbwLocalvariance->setHorizontalHeaderLabels(headers2);
    
    for (uword r = 0; r < nDisplayCols; r++)
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
    if(mLayerItem->bandwidthOptimized())
    {
        gwm::BandwidthCriterionList bwScores = mLayerItem->bandwidthSelScores();
        qDebug() << "bwScores size:" << bwScores.size();
        QVector<QPair<double,double>> qlist;
        for (const auto &item : bwScores)
            qlist.append(qMakePair(item.first, item.second));
        QVariant data = QVariant::fromValue(qlist);
        GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthSelPlot);
    }


    if (mLayerItem->mScores.is_empty() || mLayerItem->mScores.n_rows == 0 || mLayerItem->mScores.n_cols == 0)
    {
        ui->tbwScores->setRowCount(0);
        ui->tbwScores->setColumnCount(0);
        ui->label_5->hide();
        ui->tbwScores->hide();
    }
    else
    {
        try
        {
            const uword nRows = mLayerItem->mScores.n_rows;  // nDp
            const uword nCols = mLayerItem->mScores.n_cols;  // mK
            const uword nSlices = mLayerItem->mScores.n_slices;  // nDp

            if (nRows == 0 || nCols == 0 || nSlices == 0)
            {
                ui->tbwScores->setRowCount(0);
                ui->tbwScores->setColumnCount(0);
                ui->label_5->hide();
                ui->tbwScores->hide();
                return;
            }

            // Local Scores应该是每个数据点在自己的局部PCA空间中的scores
            // 即提取对角线上的值：mScores.slice(i)(i, j) for each component j
            mat localScores(nRows, nCols, fill::zeros);
            for (uword i = 0; i < nRows && i < nSlices; i++)
            {
                for (uword j = 0; j < nCols; j++)
                {
                    localScores(i, j) = mLayerItem->mScores.slice(i)(i, j);
                }
            }

            ui->tbwScores->setRowCount(nCols);
            ui->tbwScores->setColumnCount(6);
            ui->tbwScores->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

            QStringList scoreHeaders = QStringList()
                << tr("Name") << tr("Min") << tr("1st Qu") << tr("Median") << tr("3rd Qu") << tr("Max");
            ui->tbwScores->setHorizontalHeaderLabels(scoreHeaders);

            const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
            for (uword r = 0; r < localScores.n_cols; r++)
            {
                vec q = quantile(localScores.col(r), p);

                QString name = QString("Comp.%1").arg(r + 1);
                QTableWidgetItem* nameItem = new QTableWidgetItem(name);
                nameItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                ui->tbwScores->setItem(r, 0, nameItem);

                for (int c = 0; c < 5; c++)
                {
                    QTableWidgetItem* quantileItem = new QTableWidgetItem(QString("%1").arg(q(c), 0, 'f', 3));
                    quantileItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    quantileItem->setFlags(Qt::ItemFlag::NoItemFlags | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                    ui->tbwScores->setItem(r, c + 1, quantileItem);
                }
            }
        }
        catch (const std::exception& e)
        {
            qDebug() << "[GwmPropertyGWPCATab::updateUI] EXCEPTION when processing scores:" << e.what();
            ui->tbwScores->setRowCount(0);
            ui->tbwScores->setColumnCount(0);
            ui->label_5->hide();
            ui->tbwScores->hide();
        }
    }
}

bool GwmPropertyGWPCATab::openSelectFile()
{
  QString strPath = QFileDialog::getSaveFileName(NULL,QString::fromUtf8("选择文件"),"",QObject::tr("txt(*.txt)"));
  if(strPath == "")
  {
     QMessageBox::information(this,QString::fromUtf8("提示"),QString::fromUtf8("选择文件失败，无路径"),"OK");
    return false;
  }
  FilePath = strPath;
  return true;
}

void GwmPropertyGWPCATab::on_btnSaveRes_clicked()
{
    if(false == openSelectFile())
      {
        return;
      }
      if(FilePath == "")
      {
        return;
      }

      QFile myfile(FilePath);
          if (myfile.open(QFile::WriteOnly|QFile::Text))
          {
              QTextStream out(&myfile);

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

