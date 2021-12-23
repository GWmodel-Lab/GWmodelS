#include "gwmapp.h"
#include "ui_gwmapp.h"

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

#include <QtWidgets>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDebug>

#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>
#include <qgsreport.h>

#include <qgsapplication.h>
#include <qgsattributetableview.h>
#include <qgsattributetablemodel.h>
#include <qgsvectorlayercache.h>
#include <qgsattributetablefiltermodel.h>
#include <qgseditorwidgetregistry.h>
#include <qgsfeatureselectionmodel.h>
#include <qgsapplication.h>
#include <qgsstyle.h>
#include <qgsstylemodel.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsprojectionselectionwidget.h>
#include <qgsproviderregistry.h>
#include <qgsdatumtransformdialog.h>
#include <qgslayoutguiutils.h>
#include <qgsprintlayout.h>
#include <qgslayoutmanager.h>

#include "gwmopenxyeventlayerdialog.h"
#include "gwmprogressdialog.h"
#include "gwmcoordtranssettingdialog.h"
#include "TaskThread/gwmcoordtransthread.h"
#include "gwmsaveascsvdialog.h"
#include "qgsvectorfilewriter.h"
#include "TaskThread/gwmcsvtodatthread.h"
#include "gwmggwroptionsdialog.h"

#include "gwmscalablegwroptionsdialog.h"
#include "TaskThread/gwmscalablegwralgorithm.h"
#include "Model/gwmlayerscalablegwritem.h"

#include "gwmmultiscalegwroptionsdialog.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"
#include "PropertyPanelTabs/gwmpropertymultiscalegwrtab.h"

#include "gwmattributetabledialog.h"
#include "Model/gwmlayerggwritem.h"

#include "TaskThread/gwmbasicgwralgorithm.h"
#include "TaskThread/gwmgeneralizedgwralgorithm.h"
#include "Model/gwmlayergwssitem.h"
#include "gwmgwssoptionsdialog.h"
#include "SpatialWeight/gwmcrsdistance.h"

#include "Model/gwmlayerbasicgwritem.h"
#include "Model/gwmlayercollinearitygwritem.h"

#include "TaskThread/gwmgwpcataskthread.h"

#include "gwmgwpcaoptionsdialog.h"
#include "Model/gwmlayergwpcaitem.h"

#include "gwmcoordtranssettingdialog.h"
#include "gwmgwroptionsdialog.h"
#include "gwmcsvtodatdialog.h"

#include "gwmrobustgwroptionsdialog.h"
#include "gwmlcrgwroptionsdialog.h"
#include "gwmgwpcaoptionsdialog.h"

#include "Layout/gwmlayoutdesigner.h"
#include "Layout/qgslayoutmanagerdialog.h"
#include "Layout/gwmlayoutbatchdialog.h"

#include "TaskThread/gwmgtwralgorithm.h"
#include "gwmgtwroptionsdialog.h"
#include "Model/gwmlayergtwritem.h"

#include "aboutdevelopteam.h"
#include "aboutdevelopers.h"

#include "TaskThread/gwmgwaveragetaskthread.h"
#include "gwmgwaverageoptionsdialog.h"

#include "gwmprojcrssettingdialog.h"

static bool cmpByText_(QAction *a, QAction *b)
{
	return QString::localeAwareCompare(a->text(), b->text()) < 0;
}

GwmApp * GwmApp::Instance()
{
	return mInstance;
}

GwmApp* GwmApp::mInstance = nullptr;

GwmApp::GwmApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	if (mInstance)
	{
		QMessageBox::critical(
			this,
			tr("Multiple Instances of GwmApp"),
            tr("Multiple instances of GWmodelS application object detected.\nPlease contact the developers.\n"));
		abort();
	}
	mInstance = this;

    mMapModel = GwmProject::instance()->layerItemModel();

    ui->setupUi(this);

    connect(GwmProject::instance(), &GwmProject::nameChanged, this, &GwmApp::updateWindowTitle);
    connect(GwmProject::instance(), &GwmProject::dirtyChanged, this, &GwmApp::updateWindowTitle);
    updateWindowTitle();

    setupMenus();
    setAttribute(Qt::WA_QuitOnClose);
    setupMapPanel();
    setupToolbar();
    setupFeaturePanel();
    setupPropertyPanel();
    QgsGui::editorWidgetRegistry()->initEditors(mMapCanvas);
//    ui->actionGWR->setIcon(QIcon("images/icons/res/GWR.png"));
//    ui->actionGWSS->setIcon(QIcon("C:/Users/GWmodelS/Documents/gwmodeldesktop/images/icons/GWSS.svg"));
//    ui->actionGWPCA->setIcon(QIcon("C:/Users/GWmodelS/Documents/gwmodeldesktop/images/icons/GWPCA.svg"));

#ifdef Q_OS_MAC
    // Window Menu Items

    mActionWindowMinimize = new QAction( tr( "Minimize" ), this );
    mActionWindowMinimize->setShortcut( tr( "Ctrl+M", "Minimize Window" ) );
    mActionWindowMinimize->setStatusTip( tr( "Minimizes the active window to the dock" ) );
    connect( mActionWindowMinimize, SIGNAL( triggered() ), this, SLOT( showActiveWindowMinimized() ) );

    mActionWindowZoom = new QAction( tr( "Zoom" ), this );
    mActionWindowZoom->setStatusTip( tr( "Toggles between a predefined size and the window size set by the user" ) );
    connect( mActionWindowZoom, SIGNAL( triggered() ), this, SLOT( toggleActiveWindowMaximized() ) );

    mActionWindowAllToFront = new QAction( tr( "Bring All to Front" ), this );
    mActionWindowAllToFront->setStatusTip( tr( "Bring forward all open windows" ) );
    connect( mActionWindowAllToFront, SIGNAL( triggered() ), this, SLOT( bringAllToFront() ) );

    // list of open windows
    mWindowActions = new QActionGroup( this );
#endif

	functionProfile(&GwmApp::initLayouts, this, QStringLiteral("Initialize layouts support"));

}

GwmApp::~GwmApp()
{
    delete ui;
}

void GwmApp::setupMenus()
{
    connect(ui->action_ESRI_Shapefile, &QAction::triggered, this, &GwmApp::onOpenFileImportShapefile);
    connect(ui->action_Exit,&QAction::triggered, this, [&](){
        this->close();
    });
    connect(ui->actionGeo_Json, &QAction::triggered, this, &GwmApp::onOpenFileImportJson);
    connect(ui->action_GPKG,&QAction::triggered,this,&GwmApp::onOpenFileImportGPKG);
    connect(ui->action_CSV, &QAction::triggered, this, &GwmApp::onOpenFileImportCsv);
    connect(ui->action_CsvToDat, &QAction::triggered, this, &GwmApp::onCsvToDat);
    connect(ui->actionRobustGWR,&QAction::triggered,this,&GwmApp::onRobustGWR);
    connect(ui->actionScalable_GWR, &QAction::triggered,this,&GwmApp::onScalableGWRBtnClicked);
    connect(ui->action_GGWR,&QAction::triggered, this, &GwmApp::onGGWRBtnClicked);
    connect(ui->actionLocal_collinearity_GWR,&QAction::triggered, this, &GwmApp::onLcrGWRBtnClicked);
    connect(ui->actionMultiscale_GWR,&QAction::triggered, this, &GwmApp::onMultiscaleGWRBtnClicked);
    connect(ui->actionBasic_GWR,&QAction::triggered, this, &GwmApp::onGWRBtnClicked);
    connect(ui->actionGTWR,&QAction::triggered, this, &GwmApp::onGTWRBtnClicked);
    connect(ui->action_Open_Project, &QAction::triggered, this, &GwmApp::onOpenProject);
    connect(ui->action_Save_Project, &QAction::triggered, this, &GwmApp::onSaveProject);
    connect(ui->action_Save_NowProject, &QAction::triggered, this, &GwmApp::onSaveNowProject);
    connect(ui->actionBasic_GWPCA, &QAction::triggered,this,&GwmApp::onGWPCABtnClicked);
    connect(ui->actionNew, &QAction::triggered,this,&GwmApp::onNewProject);
<<<<<<< HEAD
    connect(ui->action_SetProjCRS,&QAction::triggered,this,&GwmApp::onSetProjCRS);
=======
    connect(ui->actionRobust_GWPCA, &QAction::triggered, this, &GwmApp::onGWPCABtnClicked);
>>>>>>> 9835d6e (robustGWPCA)
    //以下信号为暂未实现的功能
    connect(ui->actionGW_Averages, &QAction::triggered, this, &GwmApp::onGWaverageBtnClicked);
    connect(ui->actionGW_Covariance, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionGW_Correlations, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionGlyph_Plot, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionFlow_data, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionFlow_distance, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionSWIM, &QAction::triggered, this, &GwmApp::developingMessageBox);
    connect(ui->actionFlow_Visualization, &QAction::triggered, this, &GwmApp::developingMessageBox);
    //about信号槽连接
//    connect(ui->actionInformation, &QAction::triggered, this, [&]()
//    {
//        QString title = tr("Infomation");
//        QString message = tr("GWmodelS\n\n") + tr("* Version: v1.0\n") + tr("* Developer: GWmodel Lab\n") +
//                tr("* Contact us: binbinlu@whu.edu.cn\n") + tr("* Website: http://gwmodel.whu.edu.cn/");
//        myMessageBox(message, title,0);
//    });

    connect(ui->actionInformation, &QAction::triggered, this, &GwmApp::aboutInformation);
    connect(ui->actionGWmodel_Team, &QAction::triggered, this, [&]()
    {
        QString message = "GWmodelS";
        gotoPages(QString("http://gwmodel.whu.edu.cn/"));
    });
//    connect(ui->actionDevelopment_Team, &QAction::triggered, this, [&]()
//    {
//        QString title = tr("Develop Team");
//        QString message = tr("@GWModel Contributors:\n") +
//                          tr(" \t                                                                 _________________\n") +
//                          tr(" \t Binbin Lu, Yigong Hu, Paul Harris, |Martin Charlton|, Chris Brunsdon, \n") +
//                          tr(" \t                                                ￣￣￣￣￣￣￣￣￣￣￣ \n") +
//                          tr(" \t  Tomoki Nakaya, Daisuke Murakami\n\n") +
//                          tr("@GWmodelS Developers:\n \t Binbin Lu, Yigong Hu, Haokun Tang, Tongyao Zhang, \n") +
//                          tr(" \t Linyi Zhang, liuqi Liao, Tianyang Xia, Zuoyao Yin, Zheyi Dong, Jintao Dong\n") +
//                          tr(" \t and ALL MEMBERS of GWmodel Lab!\n\n");
//        myMessageBox(message, title, 1);
//    });
    connect(ui->actionDevelopment_Team, &QAction::triggered, this, &GwmApp::aboutDeveloperTeam);

}

void GwmApp::aboutInformation()
{
    aboutdevelopteam *messageBox = new aboutdevelopteam(this);
    messageBox->setWindowTitle("Infomation");
    messageBox->show();

}

void GwmApp::aboutDeveloperTeam()
{
    aboutDevelopers *messageBox = new aboutDevelopers(this);
    messageBox->setWindowTitle("Develop Team");
    messageBox->show();
}

void GwmApp::onSetProjCRS()
{
    GwmProjCRSSettingDialog *mProjCRSSettingDlg = new GwmProjCRSSettingDialog(this);
    if(mProjCRSSettingDlg->exec()==QDialog::Accepted)
    {
        QgsCoordinateReferenceSystem desProjCrs = mProjCRSSettingDlg->desProjCrs();
        QgsProject::instance()->setCrs(desProjCrs);
        mMapCanvas = ui->mapCanvas;
        mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());
        mMapCanvas->setExtent(mMapCanvas->fullExtent());
        mMapCanvas->refresh();
    }
}

void GwmApp::gotoPages(QString URL)
{
    QDesktopServices::openUrl(QUrl(URL));
}

void GwmApp::myMessageBox(QString message, QString title, int styleCode)
{
    QMessageBox msgBox(QMessageBox::NoIcon, "Title", "");
    msgBox.setWindowTitle(title);
    msgBox.setInformativeText(message);
    switch (styleCode)
    {
        case 0:
            msgBox.setStyleSheet("QLabel{""min-width:300px;" "min-height:0px;""}");
        break;
        case 1:
            msgBox.setStyleSheet("QLabel{""min-width:800px;" "min-height:0px;""}");
            msgBox.setIconPixmap(QPixmap("/images/icons/qbrowser-icon.png"));
        break;
        default: break;
    }
    msgBox.exec();
}

void GwmApp::developingMessageBox()
{
    QMessageBox::information(NULL, "warning", tr("敬请期待!"));//, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}

void GwmApp::toggleToolbarGeneral(bool flag)
{
    ui->actionZoom_to_Layer->setEnabled(flag);
    ui->actionZoom_to_Area->setEnabled(flag);
    ui->actionSave_Layer->setEnabled(flag);
    ui->actionSave_Layer_As->setEnabled(flag);
    ui->featureSortUpBtn->setEnabled(flag);
    ui->featureSortDownBtn->setEnabled(flag);
    ui->featureRemoveBtn->setEnabled(flag);
    ui->featureSymbolBtn->setEnabled(flag);
}

void GwmApp::onOpenFileImportShapefile(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open ESRI Shapefile"), tr(""), tr("ESRI Shapefile (*.shp)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        createLayerToModel(filePath, fileName, "ogr");
    }
}

void GwmApp::onOpenFileImportJson()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        createLayerToModel(filePath, fileName, "ogr");
    }
}

void GwmApp::onOpenFileImportGPKG()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open GPKG"), tr(""), tr("GPKG (*.gpkg)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        createLayerToModel(filePath, fileName, "ogr");
    }
}

void GwmApp::onOpenFileImportCsv()
{
    GwmOpenXYEventLayerDialog* dialog = new GwmOpenXYEventLayerDialog(this);
    connect(dialog, &GwmOpenXYEventLayerDialog::addVectorLayerSignal, this, &GwmApp::createLayerToModel);
    dialog->show();
}

void GwmApp::onCsvToDat()
{
    GwmCsvToDatDialog* csvtodatDlg = new GwmCsvToDatDialog();
    if(csvtodatDlg->exec() == QDialog::Accepted){
        QString csvFileName = csvtodatDlg->csvFileName();
        QString datFileName = csvtodatDlg->datFileName();
        if(csvFileName != "" && datFileName != ""){
            GwmCsvToDatThread* thread = new GwmCsvToDatThread(csvFileName,datFileName);
            thread->setIsColumnStore(csvtodatDlg->isColumnStore());
            GwmProgressDialog* progressDlg = new GwmProgressDialog(thread, this);
            if (progressDlg->exec() == QDialog::Accepted)
            {
                qDebug() << "[MainWindow::onCsvToDat]"
                         << "Finished";
            }
        }
    }
}

void GwmApp::onEditMode()
{
//    QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerList[0];
//    layer->selectAll();
    if ( QMessageBox::question( this,tr( "Warning" ),
                                          tr( "Are you sure to delete the features?" ) ) == QMessageBox::Yes ){
        for(int i = 0; i < mMapLayerList.size(); i++){
            ((QgsVectorLayer *)mMapLayerList[i])->startEditing();
            qDebug() << ((QgsVectorLayer *)mMapLayerList[i])->deleteSelectedFeatures();
            qDebug() << ((QgsVectorLayer *)mMapLayerList[i])->commitChanges();
            onMapSelectionChanged((QgsVectorLayer *)mMapLayerList[i]);
        }
        mMapCanvas->refresh();
    }
}

void GwmApp::setupToolbar()
{
    // 连接信号槽
    connect(ui->actionOpen, &QAction::triggered, this, &GwmApp::onOpenFileImportShapefile);
    connect(ui->actionOpen_XY_Coordinate_Layer, &QAction::triggered, this, &GwmApp::onOpenFileImportCsv);
//    connect(ui->action, &QAction::triggered, this, &MainWindow::openFileImportJson);
//    connect(ui-, &QAction::triggered, this, &MainWindow::openFileImportCsv);
    connect(ui->actionSelect_Feature, &QAction::triggered, this, [&]()
    {
        mMapCanvas->setMapTool(mMapIdentifyTool);
    });
    connect(ui->actionPan, &QAction::triggered, this, [&]()
    {
        mMapCanvas->setMapTool(mMapPanTool);
    });
    connect(ui->actionEdit, &QAction::triggered, this, &GwmApp::onEditMode);
//    connect(ui->actionSave_Layer, &QAction::triggered, this, &GwmApp::onSaveLayer);
//    connect(ui->actionSave_Layer_As, &QAction::triggered, this, [&]()
//    {
//        onExportLayer(tr("ESRI Shapefile (*.shp);;Geo Package (*.gpkg)"));

//    });
    connect(ui->actionSave_Layer, &QAction::triggered, this, &GwmApp::onSaveNowProject);
    connect(ui->actionSave_Layer_As, &QAction::triggered, this, &GwmApp::onSaveProject);
    connect(ui->actionZoom_to_Area, &QAction::triggered,this,&GwmApp::onZoomToSelection);
    connect(ui->actionZoom_to_Layer, &QAction::triggered,this,&GwmApp::onZoomToLayerBtn);
    connect(ui->actionZoom_Full_Extent, &QAction::triggered, this, [&]()
    {
        mMapCanvas->setExtent(mMapCanvas->fullExtent());
        mMapCanvas->refresh();
    });

    connect(ui->actionGWR, &QAction::triggered,this,&GwmApp::onGWRBtnClicked);
    connect(ui->actionGWSS, &QAction::triggered,this,&GwmApp::onGWSSBtnClicked);
    connect(ui->actionGWPCA, &QAction::triggered,this,&GwmApp::onGWPCABtnClicked);

	connect(ui->actionNew_Layout, &QAction::triggered, this, [&]()
	{
		QString title;
		if (!uniqueLayoutTitle(this, title, true, QgsMasterLayoutInterface::PrintLayout))
		{
			return;
		}
		createPrintLayout(title);
	});
	connect(ui->actionLayout_Manager, &QAction::triggered, this, &GwmApp::showLayoutManager);
	connect(ui->actionLayout_Batch, &QAction::triggered, this, [&]()
	{
		GwmLayoutBatchDialog* dlg = new GwmLayoutBatchDialog(this);
        dlg->setModal(false);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->exec();
	});
}

void GwmApp::setupFeaturePanel()
{
    mFeaturePanel = ui->featurePanel;
    mFeaturePanel->setMapModel(mMapModel);
    // 连接信号槽
    connect(mFeaturePanel, &GwmFeaturePanel::showAttributeTableSignal,this, &GwmApp::onShowAttributeTable);
    connect(mFeaturePanel, &GwmFeaturePanel::zoomToLayerSignal, this, &GwmApp::onZoomToLayer);
    connect(mFeaturePanel, &GwmFeaturePanel::showLayerPropertySignal, this, &GwmApp::onShowLayerProperty);
    connect(mFeaturePanel, &GwmFeaturePanel::rowOrderChangedSignal, this, &GwmApp::onFeaturePanelRowOrderChanged);
    connect(mFeaturePanel, &GwmFeaturePanel::showSymbolSettingSignal, this, &GwmApp::onShowSymbolSetting);
    connect(mFeaturePanel, &GwmFeaturePanel::showCoordinateTransDlg,this,&GwmApp::onShowCoordinateTransDlg);
    connect(mFeaturePanel, &GwmFeaturePanel::sendSetProjCrsFromLayer,this,&GwmApp::setProjCrsFromLayer);
    connect(mFeaturePanel, &GwmFeaturePanel::currentChanged,this,&GwmApp::onFeaturePanelCurrentChanged);
    connect(mFeaturePanel,&GwmFeaturePanel::sendDataSigEsriShp,this, [&]()
    {
        onExportLayer(tr("ESRI Shapefile (*.shp)"));
    });
    connect(mFeaturePanel,&GwmFeaturePanel::sendDataSigGeoJson,this, [&]()
    {
        onExportLayer(tr("GeoJson (*.json *.geojson)"));
    });
    connect(mFeaturePanel,&GwmFeaturePanel::sendDataSigGPKG,this, [&]()
    {
        onExportLayer(tr("Geo Package (*.gpkg)"));
    });

    connect(mFeaturePanel,&GwmFeaturePanel::sendDataSigCsv,this,&GwmApp::onExportLayerAsCsv);
    connect(ui->featureSortUpBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::onSortUpBtnClicked);
    connect(ui->featureSortDownBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::onSortDownBtnClicked);
    connect(ui->featureRemoveBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::removeLayer);
    connect(ui->featureSymbolBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::symbol);
}

void GwmApp::setupPropertyPanel()
{
    mPropertyPanel = ui->propertyPanel;
    mPropertyPanel->setMapModel(mMapModel);
}

void GwmApp::setupMapPanel()
{
    mMapCanvas = ui->mapCanvas;
    mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());
    // 工具
    mMapPanTool = new QgsMapToolPan(mMapCanvas);
    mMapIdentifyTool = new GwmMapToolIdentifyFeature(mMapCanvas);
    mMapCanvas->setMapTool(mMapIdentifyTool);

    // 连接信号槽
    connect(mMapModel, &GwmLayerItemModel::layerAddedSignal, this, QOverload<>::of(&GwmApp::onMapModelChanged));
    connect(mMapModel,&GwmLayerItemModel::layerRemovedSignal, this, QOverload<>::of(&GwmApp::onMapModelChanged));
    connect(mMapModel, &GwmLayerItemModel::layerItemChangedSignal, this, QOverload<GwmLayerItem *>::of(&GwmApp::onMapModelChanged));
    connect(mMapModel, &GwmLayerItemModel::layerItemMovedSignal, this, QOverload<>::of(&GwmApp::onMapModelChanged));
    connect(mMapCanvas, &QgsMapCanvas::selectionChanged, this, &GwmApp::onMapSelectionChanged);
}

GwmLayoutDesigner* GwmApp::createPrintLayout(const QString& t)
{
	QString title = t;
	if (title.isEmpty())
	{
		title = QgsProject::instance()->layoutManager()->generateUniqueTitle(QgsMasterLayoutInterface::PrintLayout);
	}

    QgsPrintLayout* layout = new QgsPrintLayout(QgsProject::instance());
	layout->setName(title);
	layout->initializeDefaults();

	if (QgsProject::instance()->layoutManager()->addLayout(layout))
		return openLayoutDesignerDialog(layout);
	else
		return nullptr;

}

GwmLayoutDesigner * GwmApp::createNewReport(QString title)
{
	if (title.isEmpty())
	{
		title = QgsProject::instance()->layoutManager()->generateUniqueTitle(QgsMasterLayoutInterface::Report);
	}
	//create new report
	std::unique_ptr< QgsReport > report = qgis::make_unique< QgsReport >(QgsProject::instance());
	report->setName(title);
	QgsMasterLayoutInterface *layout = report.get();
	QgsProject::instance()->layoutManager()->addLayout(report.release());
	return openLayoutDesignerDialog(layout);
}

GwmLayoutDesigner * GwmApp::openLayoutDesignerDialog(QgsMasterLayoutInterface * layout)
{
	GwmLayoutDesigner* designer = new GwmLayoutDesigner();
	designer->setMasterLayout(layout);
	//connect(desinger, &GwmLayoutDesigner::abou)
	designer->open();
	return designer;
}

GwmLayoutDesigner * GwmApp::duplicateLayout(QgsMasterLayoutInterface * layout, const QString & t)
{
	QString title = t;
	if (title.isEmpty())
	{
		// TODO: inject a bit of randomness in auto-titles?
		title = tr("%1 copy").arg(layout->name());
	}

	QgsMasterLayoutInterface *newLayout = QgsProject::instance()->layoutManager()->duplicateLayout(layout, title);
	GwmLayoutDesigner *dlg = openLayoutDesignerDialog(newLayout);
	dlg->activate();
	return dlg;
}

void GwmApp::showLayoutManager()
{
	QgsLayoutManagerDialog* manager = new QgsLayoutManagerDialog(this, Qt::Window);
	manager->setAttribute(Qt::WA_DeleteOnClose);
	manager->show();
	manager->activate();
}

bool GwmApp::uniqueLayoutTitle(QWidget * parent, QString & title, bool acceptEmpty, QgsMasterLayoutInterface::Type type, const QString & currentTitle)
{
	if (!parent)
	{
		parent = this;
	}
	bool ok = false;
	bool titleValid = false;
	QString newTitle = QString(currentTitle);

	QString typeString;
	switch (type)
	{
		case QgsMasterLayoutInterface::PrintLayout:
			typeString = tr("print layout");
			break;
		case QgsMasterLayoutInterface::Report:
			typeString = tr("report");
			break;
	}

	QString chooseMsg = tr("Enter a unique %1 title").arg(typeString);
	if (acceptEmpty)
	{
		chooseMsg += '\n' + tr("(a title will be automatically generated if left empty)");
	}
	QString titleMsg = chooseMsg;

	QStringList layoutNames;
	const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
	layoutNames.reserve(layouts.size() + 1);
	layoutNames << newTitle;
	for (QgsMasterLayoutInterface *l : layouts)
	{
		layoutNames << l->name();
	}
	while (!titleValid)
	{
		newTitle = QInputDialog::getText(parent,
			tr("Create %1 Title").arg(typeString),
			titleMsg,
			QLineEdit::Normal,
			newTitle,
			&ok);
		if (!ok)
		{
			return false;
		}

		if (newTitle.isEmpty())
		{
			if (!acceptEmpty)
			{
				titleMsg = chooseMsg + "\n\n" + tr("Title can not be empty!");
			}
			else
			{
				titleValid = true;
				newTitle = QgsProject::instance()->layoutManager()->generateUniqueTitle(type);
			}
		}
		else if (layoutNames.indexOf(newTitle, 1) >= 0)
		{
			layoutNames[0] = QString(); // clear non-unique name
			titleMsg = chooseMsg + "\n\n" + tr("Title already exists!");
		}
		else
		{
			titleValid = true;
		}
	}

	title = newTitle;

    return true;
}

void GwmApp::onNewProject(){
    if (GwmProject::instance()->dirty())
    {
        QString title = tr("This project has been changed");
        QString body = tr("Do you want to save it before open a new project?");
//        QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Yes
//                | QMessageBox::StandardButton::No
//                | QMessageBox::StandardButton::Cancel;
        int confirm = QMessageBox::question(GwmApp::Instance(), title, body);
        if (confirm == QMessageBox::StandardButton::Yes)
        {
            onSaveProject();
        }
    }
    QString filePath = QFileDialog::getSaveFileName(this, tr("New Project"), tr(""), tr("GWmodel Desktop Project (*.gwm)"));

    if(filePath!="")
    {
        QFileInfo fileInfo(filePath);
        GwmProject::instance()->setFilePath(filePath);
        GwmProject::instance()->setName(fileInfo.completeBaseName());
        GwmProject::instance()->newProject(fileInfo);}
}

void GwmApp::onSaveProject()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Project"), tr(""), tr("GWmodelS Project (*.gwm)"));
    if(filePath !="")
    {
    QFileInfo fileInfo(filePath);
    GwmProject::instance()->setFilePath(filePath);
    GwmProject::instance()->setName(fileInfo.completeBaseName());
    GwmProject::instance()->save(fileInfo);
    }
}

void GwmApp::onSaveNowProject()
{
    if (GwmProject::instance()->filePath()==""){
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save Project"), tr(""), tr("GWmodelS Project (*.gwm)"));
        if(filePath !="")
        {
            QFileInfo fileInfo(filePath);
            GwmProject::instance()->setFilePath(filePath);
            GwmProject::instance()->setName(fileInfo.completeBaseName());
            GwmProject::instance()->save(fileInfo);
        }
    }
    else{
    QString filePath = GwmProject::instance()->filePath();
    QFileInfo fileInfo(filePath);
    GwmProject::instance()->save(fileInfo);}
}

void GwmApp::onOpenProject()
{
    if (GwmProject::instance()->dirty())
    {
        QString title = tr("This project has been changed");
        QString body = tr("Do you want to save it before open a new project?");
//        QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Yes
//                | QMessageBox::StandardButton::No
//                | QMessageBox::StandardButton::Cancel;
        int confirm = QMessageBox::question(GwmApp::Instance(), title, body);
        if (confirm == QMessageBox::StandardButton::Yes)
        {
            onSaveProject();
        }
    }
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Project"), tr(""), tr("GWmodelS Project (*.gwm)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists() && fileInfo.isFile() && GwmProject::instance()->read(fileInfo))
    {
        onMapModelChanged();
        GwmProject::instance()->setFilePath(filePath);
    }
}

void GwmApp::addWindow( QAction *action )
{
#ifdef Q_OS_MAC
  mWindowActions->addAction( action );
  mWindowMenu->addAction( action );
  action->setCheckable( true );
  action->setChecked( true );
#else
  Q_UNUSED( action )
#endif
}

void GwmApp::removeWindow( QAction *action )
{
#ifdef Q_OS_MAC
  mWindowActions->removeAction( action );
  mWindowMenu->removeAction( action );
#else
  Q_UNUSED( action )
#endif
}

void GwmApp::addLayerToModel(QgsVectorLayer *layer)
{
    if (layer->isValid())
    {
        mMapModel->appendItem(layer, layer->dataProvider()->dataSourceUri(), layer->providerType());
    }
}

void GwmApp::createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey)
{
    qDebug() << "[MainWindow::addLayerToModel]"
             << uri << layerName << providerKey;
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(uri, layerName, providerKey);
    if (vectorLayer->isValid())
    {
        mMapModel->appendItem(vectorLayer,uri,providerKey);
        QgsProject::instance()->setCrs(vectorLayer->crs());
        mMapCanvas = ui->mapCanvas;
        mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());
    }
    else delete vectorLayer;
}

void GwmApp::onFeaturePanelCurrentChanged(const QModelIndex &current,const QModelIndex &previous){
//    qDebug() << current;
//    qDebug() << previous;
    if(current.isValid()){
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(current);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GTWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
            layerItem = ((GwmLayerVectorItem*)item);
            break;
        default:
            layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
            this->toggleToolbarGeneral(true);
        }
        else{
            this->toggleToolbarGeneral(false);
        }
        // 设置图层组件工具按钮的状态
        ui->featureSortUpBtn->setEnabled(mMapModel->canMoveUp(current));
        ui->featureSortDownBtn->setEnabled(mMapModel->canMoveDown(current));
        ui->featureRemoveBtn->setEnabled(mMapModel->canRemove(current));
        ui->featureSymbolBtn->setEnabled(mMapModel->canSetSymbol(current));
    }
    else{
        this->toggleToolbarGeneral(false);
    }
}

void GwmApp::onZoomToLayer(const QModelIndex &index)
{
    qDebug() << "[MainWindow::onZoomToLayer]"
             << "index:" << index;
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mMapModel->layerFromItem(item);
    if (layer)
    {
        QgsCoordinateTransform transform;
        transform.setSourceCrs(layer->crs());
        transform.setDestinationCrs(QgsProject::instance()->crs());
        QgsRectangle extent = transform.transformBoundingBox(layer->extent());
        mMapCanvas->setExtent(extent);
        mMapCanvas->refresh();
    }
}

void GwmApp::onSaveLayer()
{
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GTWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
            layerItem = ((GwmLayerVectorItem*)item);
            break;
        default:
            layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
            if(layerItem->provider() != "ogr"){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),tr("ESRI Shapefile (*.shp)"));
                if(filePath != ""){
                    QFileInfo fileInfo(filePath);
                    QString fileName = fileInfo.baseName();
                    QString file_suffix = fileInfo.suffix();
                    if (layerItem->save(filePath,fileName,file_suffix))
                    {
                        layerItem->setDataSource(filePath, "ogr");
                    }
                }
            }
            else
            {
                 mMapModel->layerFromItem(item)->commitChanges();
            }
        }
    }
}

void GwmApp::onExportLayerAsCsv(const QModelIndex &index)
{
    //    onExportLayer(tr("CSV (*.csv)"));
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    GwmLayerVectorItem* layerItem;
    switch (item->itemType()) {
    case GwmLayerItem::GwmLayerItemType::Group:
        layerItem = ((GwmLayerGroupItem*)item)->originChild();
        break;
    case GwmLayerItem::GwmLayerItemType::Vector:
    case GwmLayerItem::GwmLayerItemType::Origin:
    case GwmLayerItem::GwmLayerItemType::GWR:
    case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
    case GwmLayerItem::GwmLayerItemType::ScalableGWR:
    case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
    case GwmLayerItem::GwmLayerItemType::GWSS:
    case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
    case GwmLayerItem::GwmLayerItemType::GTWR:
    case GwmLayerItem::GwmLayerItemType::GWPCA:
        layerItem = ((GwmLayerVectorItem*)item);
        break;
    default:
        layerItem = nullptr;
    }
    if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
        GwmSaveAsCSVDialog* saveAsCSVDlg = new GwmSaveAsCSVDialog();
        if(saveAsCSVDlg->exec() == QDialog::Accepted){
            QString filePath = saveAsCSVDlg->filepath();
            if(filePath != ""){
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.baseName();
                QString file_suffix = fileInfo.suffix();
                QgsVectorFileWriter::SaveVectorOptions& options = *(new QgsVectorFileWriter::SaveVectorOptions());
                if(saveAsCSVDlg->isAddXY()){
                    QStringList layerOptions;
                    if(layerItem->layer()->geometryType() == QgsWkbTypes::Type::Point){
                        layerOptions << QStringLiteral( "%1=%2" ).arg( "GEOMETRY", "AS_XY" );
                    }
                    else{
                        layerOptions << QStringLiteral( "%1=%2" ).arg( "GEOMETRY", "AS_WKT" );
                    }
                    options.layerOptions = layerOptions;
                }
                layerItem->save(filePath,fileName,file_suffix,options);
            }
        }
    }
}

void GwmApp::onExportLayer(QString filetype)
{
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GTWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
            layerItem = ((GwmLayerVectorItem*)item);
            break;
        default:
            layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),filetype);
                if(filePath != ""){
                    QFileInfo fileInfo(filePath);
                    QString fileName = fileInfo.baseName();
                    QString file_suffix = fileInfo.suffix();
                    layerItem->save(filePath,fileName,file_suffix);
                }
        }
    }
}

void GwmApp::onZoomToLayerBtn(){
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        onZoomToLayer(index);
    }
}

void GwmApp::onZoomToSelection(){
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        QgsVectorLayer* layer = mMapModel->layerFromItem(item);
        mMapCanvas->zoomToSelected(layer);
        mMapCanvas->refresh();
    }
}

bool GwmApp::askUserForDatumTransfrom(const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsMapLayer *layer)
{
    Q_ASSERT( qApp->thread() == QThread::currentThread() );

    QString title;
    if ( layer )
    {
        // try to make a user-friendly (short!) identifier for the layer
        QString layerIdentifier;
        if ( !layer->name().isEmpty() )
        {
            layerIdentifier = layer->name();
        }
        else
        {
            const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
            if ( parts.contains( QStringLiteral( "path" ) ) )
            {
                const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
                layerIdentifier = fi.fileName();
            }
            else if ( layer->dataProvider() )
            {
                const QgsDataSourceUri uri( layer->source() );
                layerIdentifier = uri.table();
            }
        }
        if ( !layerIdentifier.isEmpty() )
            title = tr( "Select Transformation for %1" ).arg( layerIdentifier );
    }

    return QgsDatumTransformDialog::run( sourceCrs, destinationCrs, this, mMapCanvas, title );
}

void GwmApp::onMapSelectionChanged(QgsVectorLayer *layer)
{
    qDebug() << "[MainWindow]" << "Map Selection Changed: (layer " << layer->name() << ")";
    // 移除旧的橡皮条
    QList<QgsRubberBand*> rubbers0 = mMapLayerRubberDict[layer];
    if (rubbers0.size() > 0)
    {
        for (QgsRubberBand* rubber : rubbers0)
        {
            rubber->reset();
        }
    }
    rubbers0.clear();

     //添加新的橡皮条
    QgsFeatureList selectedFeatures = layer->selectedFeatures();
    for (QgsFeature feature : selectedFeatures)
    {
        QgsGeometry geometry = feature.geometry();
        QgsRubberBand* rubber = new QgsRubberBand(mMapCanvas, geometry.type());
        rubber->addGeometry(geometry, layer);
        rubber->setStrokeColor(QColor(255, 0, 0));
        rubber->setFillColor(QColor(255, 0, 0, 144));
        rubber->show();
        mMapLayerRubberDict[layer] += rubber;
        qDebug() << "[MainWindow::onMapSelectionChanged]"
                 << "selected" << geometry.asWkt();
    }
}

void GwmApp::onMapModelChanged(GwmLayerItem *item)
{
    mMapLayerList = mMapModel->toMapLayerList();
    mMapCanvas->setLayers(mMapLayerList);
    if (mMapLayerList.size() == 1)
    {
        QgsMapLayer* firstLayer = mMapLayerList.first();
        QgsRectangle extent = mMapCanvas->mapSettings().layerExtentToOutputExtent(firstLayer, firstLayer->extent());
        qDebug() << "[MainWindow::onMapModelChanged]"
                 << "origin extent: " << firstLayer->extent()
                 << "trans extent: " << extent;
        mMapCanvas->setExtent(extent);
    }
    mMapCanvas->refresh();

    QgsVectorLayer *layer = mMapModel->layerFromItem(item);
    QList<QgsRubberBand*> rubbers0 = mMapLayerRubberDict[layer];
    if(item->checkState()!=Qt::CheckState::Checked){
    if (rubbers0.size() > 0)
    {
        for (QgsRubberBand* rubber : rubbers0)
        {
            rubber->hide();
        }
    }
    }
    else{
        if (rubbers0.size() > 0)
        {
            for (QgsRubberBand* rubber : rubbers0)
            {
                rubber->show();
            }
        }

    }
}

void GwmApp::onMapModelChanged()
{
    mMapLayerList = mMapModel->toMapLayerList();
    mMapCanvas->setLayers(mMapLayerList);
    QList<QgsMapLayer*> maplist=mMapLayerRubberDict.keys();
    for(QgsMapLayer* formlayer:mMapLayerRubberDict.keys()){
        for(QgsMapLayer* nowlayer:mMapLayerList){
            if(formlayer==nowlayer){
                maplist.removeAll(nowlayer);
            }
        }
    }

    for(QgsMapLayer* layer:maplist){
        QList<QgsRubberBand*> rubbers0 = mMapLayerRubberDict[layer];
        if (rubbers0.size() > 0)
        {
            for (QgsRubberBand* rubber : rubbers0)
            {
                rubber->reset();
            }
        }
        rubbers0.clear();
    }

    if (mMapLayerList.size() == 1)
    {
        QgsMapLayer* firstLayer = mMapLayerList.first();
        QgsRectangle extent = mMapCanvas->mapSettings().layerExtentToOutputExtent(firstLayer, firstLayer->extent());
        qDebug() << "[MainWindow::onMapModelChanged]"
                 << "origin extent: " << firstLayer->extent()
                 << "trans extent: " << extent;
        mMapCanvas->setExtent(extent);
    }
    mMapCanvas->refresh();
}

void GwmApp::onShowLayerProperty(const QModelIndex &index)
{
    mPropertyPanel->addPropertyTab(index);
}


void GwmApp::onFeaturePanelRowOrderChanged(int from, int dest)
{
    qDebug() << "[MainWindow::onFeaturePanelRowOrderChanged]"
             << "layer list size" << mMapLayerList.size();
    mMapLayerList = mMapModel->toMapLayerList();
    mMapCanvas->setLayers(mMapLayerList);
    mMapCanvas->refresh();
}

// 属性表
void GwmApp::onShowAttributeTable(const QModelIndex &index)
{
    // qDebug() << 123;
    qDebug() << "[MainWindow::onShowAttributeTable]"
             << "index:" << index;
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mMapModel->layerFromItem(item);
    if (currentLayer)
    {
        GwmAttributeTableDialog *d = new GwmAttributeTableDialog(currentLayer,mMapCanvas,this,Qt::Dialog);
        d->show();
    }
}

void GwmApp::onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list)
{
    for (QgsFeatureId id : list)
    {
        qDebug() << "[MainWindow::receiveSigAttriToMap]"
                 << "id:" << id;
    }
}
void GwmApp::onShowSymbolSetting(const QModelIndex &index)
{
    createSymbolWindow(index);
    connect(mSymbolWindow,&GwmSymbolWindow::canvasRefreshSingal,this,[&]()
    {
        mMapCanvas->refresh();
    });
    mSymbolWindow->show();
}

bool GwmApp::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == mMapCanvas)
    {
        switch (e->type()) {
        case QEvent::MouseMove:
            qDebug() << "[MainWindow::eventFilter]"
                     << "Cur map position: (" << mMapCanvas->getCoordinateTransform()->toMapCoordinates(((QMouseEvent*)e)->pos()).asWkt();
            return true;
        default:
            break;
        }
    }
    return false;
}

void GwmApp::createSymbolWindow(const QModelIndex &index)
{
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mMapModel->layerFromItem(item);
    if (layer)
    {
        mSymbolWindow = new GwmSymbolWindow(layer);
        mSymbolWindow->setAttribute(Qt::WA_DeleteOnClose);
    }
}

// 投影到坐标系
void GwmApp::onShowCoordinateTransDlg(const QModelIndex &index)
{
    // qDebug() << index;
    // 获取当前矢量图层
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mMapModel->layerFromItem(item);
    // 原始图层的投影坐标系
    if (currentLayer)
    {
        GwmCoordTransSettingDialog *mCoordinateTransDlg = new GwmCoordTransSettingDialog(this);
        // 连接投影坐标系窗口和主窗口
        mCoordinateTransDlg->setSrcCrs(currentLayer->crs());
        if (mCoordinateTransDlg->exec() == QDialog::Accepted)
        {
            QgsCoordinateReferenceSystem desCrs = mCoordinateTransDlg->desCrs();
            GwmCoordTransThread* thread = new GwmCoordTransThread(currentLayer, desCrs);
            GwmProgressDialog* progressDlg = new GwmProgressDialog(thread, this);
            if (progressDlg->exec() == QDialog::Accepted)
            {
                qDebug() << "[MainWindow::onShowCoordinateTransDlg]"
                         << "Finished";
                addLayerToModel(thread->getWorkLayer());
            }
        }
    }
}

//设置图层CRS为工程CRS
void GwmApp::setProjCrsFromLayer(const QModelIndex &index)
{
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mMapModel->layerFromItem(item);
    // 原始图层的投影坐标系
    if (currentLayer){
        QgsCoordinateReferenceSystem desProjCrs = currentLayer->crs();
        QgsProject::instance()->setCrs(desProjCrs);
        mMapCanvas = ui->mapCanvas;
        mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());
        mMapCanvas->setExtent(mMapCanvas->fullExtent());
        mMapCanvas->refresh();
    }
}

void GwmApp::onGWRBtnClicked()
{
    GwmBasicGWRAlgorithm* gwrTaskThread = new GwmBasicGWRAlgorithm();
    GwmGWROptionsDialog* gwrOptionDialog = new GwmGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->hasRegressionLayer() ? gwrOptionDialog->selectedRegressionLayer() : gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer0, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }     
    }
    delete gwrOptionDialog;
    delete gwrTaskThread;
}

void GwmApp::onGWRNewBtnClicked()
{
    GwmBasicGWRAlgorithm* algorithm = new GwmBasicGWRAlgorithm();
    QgsVectorLayer* dataLayer = static_cast<GwmLayerGroupItem*>(mMapModel->rootChildren()[0])->originChild()->layer();
    algorithm->setDataLayer(dataLayer);
    QgsFields fields = dataLayer->fields();
    GwmVariable depVar = {0, fields[0].name(), fields[0].type(), fields[0].isNumeric()};
    QList<GwmVariable> indepVars;
    for (int i : {1, 10, 12, 13, 15})
    {
        indepVars.append({ i, fields[i].name(), fields[i].type(), fields[i].isNumeric()});
    }
    algorithm->setDependentVariable(depVar);
    algorithm->setIndependentVariables(indepVars);
    algorithm->setIsAutoselectIndepVars(true);
    algorithm->setIndepVarSelectionThreshold(150.0);
    GwmSpatialWeight spatialWeight;
    spatialWeight.setDistance(GwmCRSDistance(dataLayer->featureCount(), false));
    spatialWeight.setWeight(GwmBandwidthWeight(36, true, GwmBandwidthWeight::Gaussian));
    algorithm->setSpatialWeight(spatialWeight);
    algorithm->setIsAutoselectBandwidth(true);
    algorithm->setBandwidthSelectionCriterionType(GwmBasicGWRAlgorithm::CV);
    algorithm->setHasHatMatrix(true);
    algorithm->setHasFTest(true);


    GwmProgressDialog* progressDlg = new GwmProgressDialog(algorithm);
    if (progressDlg->exec() == QDialog::Accepted)
    {
        QgsVectorLayer* resultLayer = algorithm->resultLayer();
        addLayerToModel(resultLayer);

        GwmDiagnostic diagnostic = algorithm->diagnostic();
        GwmBasicGWRAlgorithm::FTestResultPack fTestResult = algorithm->fTestResult();
    }

}

void GwmApp::onGWSSBtnClicked()
{
    GwmGWSSTaskThread* gwssTaskThread = new GwmGWSSTaskThread();
    GwmGWSSOptionsDialog* gwssOptionDialog = new GwmGWSSOptionsDialog(mMapModel->rootChildren(), gwssTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwssOptionDialog->exec() == QDialog::Accepted)
    {
        gwssOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwssOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwssTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwssTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerGWSSItem* gwssItem = new GwmLayerGWSSItem(selectedItem, resultLayer0, gwssTaskThread);
            mMapModel->appentItem(gwssItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwssItem));
        }
    }
    delete gwssOptionDialog;
    delete gwssTaskThread;
}

void GwmApp::onGWaverageBtnClicked()
{
    GwmGWaverageTaskThread* gwssTaskThread = new GwmGWaverageTaskThread();
    GwmGWaverageOptionsDialog* gwssOptionDialog = new GwmGWaverageOptionsDialog(mMapModel->rootChildren(), gwssTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwssOptionDialog->exec() == QDialog::Accepted)
    {
        gwssOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwssOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwssTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwssTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerGWSSItem* gwssItem = new GwmLayerGWSSItem(selectedItem, resultLayer0, gwssTaskThread);
            mMapModel->appentItem(gwssItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwssItem));
        }
    }
    delete gwssOptionDialog;
    delete gwssTaskThread;
}

void GwmApp::onScalableGWRBtnClicked()
{
    GwmScalableGWRAlgorithm* gwrTaskThread = new GwmScalableGWRAlgorithm();
    GwmScalableGWROptionsDialog* gwrOptionDialog = new GwmScalableGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->hasRegressionLayer() ? gwrOptionDialog->selectedRegressionLayer() : gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerScalableGWRItem* gwrItem = new GwmLayerScalableGWRItem(selectedItem, resultLayer0, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwrOptionDialog;
    delete gwrTaskThread;
}

void GwmApp::onMultiscaleGWRBtnClicked()
{
    GwmMultiscaleGWRAlgorithm* gwrTaskThread = new GwmMultiscaleGWRAlgorithm();
    GwmMultiscaleGWROptionsDialog* gwrOptionDialog = new GwmMultiscaleGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerMultiscaleGWRItem* gwrItem = new GwmLayerMultiscaleGWRItem(selectedItem, resultLayer0, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwrOptionDialog;
    delete gwrTaskThread;
}

void GwmApp::onRobustGWR()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mMapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrRobustOptionDialog->exec() == QDialog::Accepted)
    {
        gwrRobustOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrRobustOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer0, gwrRobustTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwrRobustOptionDialog;
    delete gwrRobustTaskThread;
}

void GwmApp::onRobustGWRBtnClicked()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mMapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrRobustOptionDialog->exec() == QDialog::Accepted)
    {
        gwrRobustOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrRobustOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer0, gwrRobustTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwrRobustOptionDialog;
    delete gwrRobustTaskThread;
}

void GwmApp::onLcrGWRBtnClicked()
{
    GwmLocalCollinearityGWRAlgorithm * lcrGWRTaskThread = new GwmLocalCollinearityGWRAlgorithm();
    GwmLcrGWROptionsDialog* gwrLcrOptionDialog = new GwmLcrGWROptionsDialog(mMapModel->rootChildren(), lcrGWRTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrLcrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrLcrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrLcrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrLcrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrLcrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(lcrGWRTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = lcrGWRTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerCollinearityGWRItem* gwrItem = new GwmLayerCollinearityGWRItem(selectedItem, resultLayer0, lcrGWRTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwrLcrOptionDialog;
    delete lcrGWRTaskThread;
}

void GwmApp::onGGWRBtnClicked(){
    GwmGeneralizedGWRAlgorithm* ggwrTaskThread = new GwmGeneralizedGWRAlgorithm();
    GwmGGWROptionsDialog* ggwrOptionDialog = new GwmGGWROptionsDialog(this->mMapModel->rootChildren(), ggwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (ggwrOptionDialog->exec() == QDialog::Accepted)
    {
        ggwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = ggwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(ggwrTaskThread); //
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = ggwrTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerGGWRItem* ggwrItem = new GwmLayerGGWRItem(selectedItem, resultLayer0, ggwrTaskThread);
            mMapModel->appentItem(ggwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(ggwrItem));
        }
    }
    delete ggwrOptionDialog;
    delete ggwrTaskThread;
}

void GwmApp::onGTWRBtnClicked()
{
    GwmGTWRAlgorithm* gtwrTaskThread = new GwmGTWRAlgorithm();
    GwmGTWROptionsDialog* ggwrOptionDialog = new GwmGTWROptionsDialog(this->mMapModel->rootChildren(), gtwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (ggwrOptionDialog->exec() == QDialog::Accepted)
    {
        ggwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = ggwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gtwrTaskThread); //
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gtwrTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerGTWRItem* gtwrItem = new GwmLayerGTWRItem(selectedItem, resultLayer0, gtwrTaskThread);
            mMapModel->appentItem(gtwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gtwrItem));
        }
    }
    delete ggwrOptionDialog;
    delete gtwrTaskThread;
}

void GwmApp::onGWPCABtnClicked()
{
    GwmGWPCATaskThread* gwpcaTaskThread = new GwmGWPCATaskThread();
    GwmGWPCAOptionsDialog* gwpcaOptionDialog = new GwmGWPCAOptionsDialog(mMapModel->rootChildren(), gwpcaTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwpcaOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwpcaOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwpcaOptionDialog->exec() == QDialog::Accepted)
    {
        gwpcaOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwpcaOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwpcaTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwpcaTaskThread->resultLayer();
            QgsVectorLayer* resultLayer0 = new QgsVectorLayer();
            resultLayer0 = resultLayer->clone();
            GwmLayerGWPCAItem * gwrItem = new GwmLayerGWPCAItem(selectedItem, resultLayer0, gwpcaTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
            onShowLayerProperty(mMapModel->indexFromItem(gwrItem));
        }
    }
    delete gwpcaOptionDialog;
    delete gwpcaTaskThread;
}

void GwmApp::populateLayoutsMenu(QMenu * menu)
{
	menu->clear();
	QList<QAction *> acts;
	const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
	acts.reserve(layouts.size());
	for (QgsMasterLayoutInterface *layout : layouts)
	{
		QAction *a = new QAction(layout->name(), menu);
		connect(a, &QAction::triggered, this, [this, layout]
		{
			openLayoutDesignerDialog(layout);
		});
		acts << a;
	}
	if (acts.size() > 1)
	{
		// sort actions by text
		std::sort(acts.begin(), acts.end(), cmpByText_);
	}
	menu->addActions(acts);
}

void GwmApp::initLayouts()
{
    QgsLayoutGuiUtils::registerGuiForKnownItemTypes(mMapCanvas);

	//mLayoutQptDropHandler = new QgsLayoutQptDropHandler(this);
	//registerCustomLayoutDropHandler(mLayoutQptDropHandler);
	//mLayoutImageDropHandler = new QgsLayoutImageDropHandler(this);
	//registerCustomLayoutDropHandler(mLayoutImageDropHandler);
}

void GwmApp::updateWindowTitle()
{
    QString projectName = GwmProject::instance()->name();
    bool projectDirty = GwmProject::instance()->dirty();
    QString title = QString("%1%2 - GWmodelS").arg(projectName).arg((projectDirty ? " *" : ""));
    setWindowTitle(title);
}

void GwmApp::closeEvent( QCloseEvent * event )
{
    if(GwmProject::instance()->dirty()){
       QMessageBox::StandardButton result=QMessageBox::question(this, "GWmodelS", "You have unsaved changes,do you want to save before exiting",
                         QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                         QMessageBox::Yes);
       if (result==QMessageBox::Yes){
           if (GwmProject::instance()->filePath()==""){
               QString filePath = QFileDialog::getSaveFileName(this, tr("Save Project"), tr(""), tr("GWmodelS Project (*.gwm)"));
               if(filePath !="")
               {
                   QFileInfo fileInfo(filePath);
                   GwmProject::instance()->setFilePath(filePath);
                   GwmProject::instance()->setName(fileInfo.completeBaseName());
                   GwmProject::instance()->save(fileInfo);
                   event->accept();
               }
               else
                   event->ignore();
           }
           else{
                QString filePath = GwmProject::instance()->filePath();
                QFileInfo fileInfo(filePath);
                GwmProject::instance()->save(fileInfo);
                event->accept();
           }
       }
       if (result==QMessageBox::No)
           event->accept();
       if(result==QMessageBox::Cancel)
           event->ignore();
    }
}
