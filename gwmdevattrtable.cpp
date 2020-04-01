#include "gwmdevattrtable.h"

#include "mainwidget.h"

#include "qgsfieldcalculator.h"

#include <QMessageBox>
#include <QGridLayout>
#include <QDialogButtonBox>

//#include "qgsattributetabledialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"
#include "qgsexpressioncontextutils.h"

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsexpressionbuilderwidget.h"
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgsdockwidget.h"
#include "qgsfeatureiterator.h"
#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsfieldcalculator.h"
#include "qgsfeatureaction.h"
#include "qgsactionmanager.h"
#include "qgsmessagebar.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsrubberband.h"
#include "qgsfields.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfieldproxymodel.h"
#include "qgsgui.h"
//#include "qgsclipboard.h"
#include "qgsfeaturestore.h"
#include "qgsguiutils.h"
#include "qgsproxyprogresstask.h"
//#include "qgisapp.h"

//GwmDevAttrTable::GwmDevAttrTable(QgsVectorLayer *theVecLayer, QgsMapCanvas* myMapCanvas,QWidget *parent)
GwmDevAttrTable::GwmDevAttrTable(QgsVectorLayer *theVecLayer,QgsMapCanvas* myMapCanvas, QWidget *parent, Qt::WindowFlags flags)
{
    setupUi(this);
    mMainView->init(theVecLayer,myMapCanvas);
    this->currentLayer = theVecLayer;
    this->mymapCanvas = myMapCanvas;

    QgsAttributeTableConfig config = this->currentLayer->attributeTableConfig();
    mMainView->setAttributeTableConfig( config );

    //编辑功能
    connect(mActionToggleEditing,&QAction::toggled,this,&GwmDevAttrTable::mActionToggleEditing_toggled);
    //全选功能
    connect( mActionSelectAll, &QAction::triggered, this, &GwmDevAttrTable::mActionSelectAll_triggered );
    //反选功能
    connect( mActionInvertSelection, &QAction::triggered, this, &GwmDevAttrTable::mActionInvertSelection_triggered );
    //删除所选
    connect( mActionRemoveSelection, &QAction::triggered, this, &GwmDevAttrTable::mActionRemoveSelection_triggered );
    //让所选的要素置顶属性表
    connect( mActionSelectedToTop, &QAction::toggled, this, &GwmDevAttrTable::mActionSelectedToTop_toggled );
    //缩放所选要素
    connect( mActionPanMapToSelectedRows, &QAction::triggered, this, &GwmDevAttrTable::mActionPanMapToSelectedRows_triggered );
    //缩放所选要素(更大)
    connect( mActionZoomMapToSelectedRows, &QAction::triggered, this, &GwmDevAttrTable::mActionZoomMapToSelectedRows_triggered );
    //打开要素计算器
    connect( mActionOpenFieldCalculator, &QAction::triggered, this, &GwmDevAttrTable::mActionOpenFieldCalculator_triggered );

    connect( mActionAddAttribute, &QAction::triggered, this, &GwmDevAttrTable::mActionAddAttribute_triggered );
    connect( mActionRemoveAttribute, &QAction::triggered, this, &GwmDevAttrTable::mActionRemoveAttribute_triggered );

    connect( mActionSaveEdits, &QAction::triggered, this, &GwmDevAttrTable::mActionSaveEdits_triggered );

    //添加要素
    //connect( mActionAddFeature, &QAction::triggered, this, &GwmDevAttrTable::mActionAddFeature_triggered );
    //删除要素
    //connect( mActionDeleteSelected, &QAction::triggered, this, &GwmDevAttrTable::mActionDeleteSelected_triggered );

    //QGIS源码
    connect( this->currentLayer, &QgsVectorLayer::editingStarted, this, &GwmDevAttrTable::editingToggled );
    connect( this->currentLayer, &QgsVectorLayer::editingStopped, this, &GwmDevAttrTable::editingToggled );

    connect( currentLayer, &QgsVectorLayer::selectionChanged, this, &GwmDevAttrTable::updateTitle );
    connect( currentLayer, &QgsVectorLayer::featureAdded, this, &GwmDevAttrTable::updateTitle );
    connect( currentLayer, &QgsVectorLayer::featuresDeleted, this, &GwmDevAttrTable::updateTitle );
    connect( currentLayer, &QgsVectorLayer::editingStopped, this, &GwmDevAttrTable::updateTitle );
    connect( mMainView, &QgsDualView::filterChanged, this, &GwmDevAttrTable::updateTitle );

    mActionFeatureActions = new QToolButton();
    mActionFeatureActions->setAutoRaise( false );
    mActionFeatureActions->setPopupMode( QToolButton::InstantPopup );
    mActionFeatureActions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
    mActionFeatureActions->setText( tr( "Actions" ) );
    mActionFeatureActions->setToolTip( tr( "Actions" ) );
    mToolbar->addWidget( mActionFeatureActions );

    installEventFilter( this );
    updateTitle();
    // toggle editing
    bool canChangeAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
    bool canDeleteFeatures = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
    bool canAddAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
    bool canDeleteAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
    bool canAddFeatures = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;

    mActionToggleEditing->blockSignals( true );
    mActionToggleEditing->setCheckable( true );
    mActionToggleEditing->setChecked( this->currentLayer->isEditable() );
    mActionToggleEditing->blockSignals( false );

    mActionSaveEdits->setEnabled( mActionToggleEditing->isEnabled() && this->currentLayer->isEditable() );
    mActionReload->setEnabled( ! this->currentLayer->isEditable() );
    mActionAddAttribute->setEnabled( ( canChangeAttributes || canAddAttributes ) && this->currentLayer->isEditable() );
    mActionRemoveAttribute->setEnabled( canDeleteAttributes && this->currentLayer->isEditable() );
    if ( !canDeleteFeatures )
    {
      mToolbar->removeAction( mActionDeleteSelected );
      mToolbar->removeAction( mActionCutSelectedRows );
    }
    mActionAddFeature->setEnabled( canAddFeatures && this->currentLayer->isEditable() );
    mActionPasteFeatures->setEnabled( canAddFeatures && this->currentLayer->isEditable() );
    if ( !canAddFeatures )
    {
      mToolbar->removeAction( mActionAddFeature );
      mToolbar->removeAction( mActionPasteFeatures );
    }

    mMainViewButtonGroup->setId( mTableViewButton, QgsDualView::AttributeTable );
    mMainViewButtonGroup->setId( mAttributeViewButton, QgsDualView::AttributeEditor );

//    switch ( initialMode )
//    {
//      case QgsAttributeTableFilterModel::ShowVisible:
//        mFeatureFilterWidget->filterVisible();
//        break;

//      case QgsAttributeTableFilterModel::ShowSelected:
//        mFeatureFilterWidget->filterSelected();
//        break;

//      case QgsAttributeTableFilterModel::ShowAll:
//      default:
//        mFeatureFilterWidget->filterShowAll();
//        break;
//    }
    mFieldCombo->setFilters( QgsFieldProxyModel::AllTypes | QgsFieldProxyModel::HideReadOnly );
    mFieldCombo->setLayer( this->currentLayer );

    connect( mRunFieldCalc, &QAbstractButton::clicked, this, &GwmDevAttrTable::updateFieldFromExpression );
    connect( mRunFieldCalcSelected, &QAbstractButton::clicked, this, &GwmDevAttrTable::updateFieldFromExpressionSelected );
    connect( mUpdateExpressionText, static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &GwmDevAttrTable::updateButtonStatus );
    mUpdateExpressionText->setLayer( this->currentLayer );
    mUpdateExpressionText->setLeftHandButtonStyle( true );

//    int initialView = settings.value( QStringLiteral( "qgis/attributeTableView" ), -1 ).toInt();
//    if ( initialView < 0 )
//    {
//      initialView = settings.value( QStringLiteral( "qgis/attributeTableLastView" ), QgsDualView::AttributeTable ).toInt();
//    }
//    mMainView->setView( static_cast< QgsDualView::ViewMode >( initialView ) );
//    mMainViewButtonGroup->button( initialView )->setChecked( true );

    connect( mActionToggleMultiEdit, &QAction::toggled, mMainView, &QgsDualView::setMultiEditEnabled );
    connect( mActionSearchForm, &QAction::toggled, mMainView, &QgsDualView::toggleSearchMode );
    updateMultiEditButtonState();
    editingToggled();
}

GwmDevAttrTable::~GwmDevAttrTable()
{

}

// 启动编辑
void GwmDevAttrTable::mActionToggleEditing_toggled(bool)
{
    //this->currentLayer->startEditing();
    if(!this->currentLayer){
        return;
    }
    if(this->currentLayer->isEditable() && mMainView->tableView()->indexWidget(mMainView->tableView()->currentIndex())){
    //if(!this->currentLayer->isEditable()){
        mMainView->tableView()->indexWidget( mMainView->tableView()->currentIndex() )->setEnabled( false );
        //qDebug() << 3;
    }
//    if(!this->currentLayer->isEditable())
//    {
//        mActionToggleEditing->setChecked(false);
//        mActionToggleEditing->setEnabled( false );
//        this->currentLayer->startEditing();
//        editingToggled();
//    }
    if(!toggleEditing2(this->currentLayer))
    {
        editingToggled();
    }
    //this->currentLayer->commitChanges();
}

void GwmDevAttrTable::editingToggled()
{
    //this->currentLayer->startEditing();
    mActionToggleEditing->blockSignals(true);
    mActionToggleEditing->setChecked(this->currentLayer->isEditable());
    mActionSaveEdits->setEnabled(this->currentLayer->isEditable());
    mActionReload->setEnabled( ! this->currentLayer->isEditable() );
    updateMultiEditButtonState();
    if ( this->currentLayer->isEditable() )
    {
      mActionSearchForm->setChecked( false );
    }
    mActionToggleEditing->blockSignals( false );

    bool canChangeAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
    bool canDeleteFeatures = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
    bool canAddAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
    bool canDeleteAttributes = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
    bool canAddFeatures = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
    mActionAddAttribute->setEnabled( ( canChangeAttributes || canAddAttributes ) && this->currentLayer->isEditable() );
    mActionRemoveAttribute->setEnabled( canDeleteAttributes && this->currentLayer->isEditable() );
    mActionDeleteSelected->setEnabled( canDeleteFeatures && this->currentLayer->isEditable() && this->currentLayer->selectedFeatureCount() > 0 );
    mActionCutSelectedRows->setEnabled( canDeleteFeatures && this->currentLayer->isEditable() && this->currentLayer->selectedFeatureCount() > 0 );
    mActionAddFeature->setEnabled( canAddFeatures && this->currentLayer->isEditable() );
    mActionPasteFeatures->setEnabled( canAddFeatures && this->currentLayer->isEditable() );
    mActionToggleEditing->setEnabled( ( canChangeAttributes || canDeleteFeatures || canAddAttributes || canDeleteAttributes || canAddFeatures ) && !this->currentLayer->readOnly() );

    mUpdateExpressionBox->setVisible( this->currentLayer->isEditable() );
    if ( this->currentLayer->isEditable() && mFieldCombo->currentIndex() == -1 )
    {
      mFieldCombo->setCurrentIndex( 0 );
    }
    // not necessary to set table read only if layer is not editable
    // because model always reflects actual state when returning item flags
    QList<QgsAction> actions = this->currentLayer->actions()->actions( QStringLiteral( "Layer" ) );
    if ( actions.isEmpty() )
    {
      //qDebug() << 1;
      mActionFeatureActions->setVisible( false );
    }
    else
    {
      QMenu *actionMenu = new QMenu();
      const auto constActions = actions;
      for ( const QgsAction &action : constActions )
      {
        if ( !this->currentLayer->isEditable() && action.isEnabledOnlyWhenEditable() )
          continue;

        QAction *qAction = actionMenu->addAction( action.icon(), action.shortTitle() );
        qAction->setToolTip( action.name() );
        qAction->setData( QVariant::fromValue<QgsAction>( action ) );
        connect( qAction, &QAction::triggered, this, &GwmDevAttrTable::layerActionTriggered );
      }
      mActionFeatureActions->setMenu( actionMenu );
    }
}

// 全选功能
void GwmDevAttrTable::mActionSelectAll_triggered()
{
    this->currentLayer->selectAll();
}

// 反选功能
void GwmDevAttrTable::mActionInvertSelection_triggered()
{
  this->currentLayer->invertSelection();
}

// 删除所选
void GwmDevAttrTable::mActionRemoveSelection_triggered()
{
  this->currentLayer->removeSelection();
}

// 置顶属性表所选要素
void GwmDevAttrTable::mActionSelectedToTop_toggled( bool checked )
{
  if ( checked )
  {
    mMainView->setSelectedOnTop( true );
  }
  else
  {
    mMainView->setSelectedOnTop( false );
  }
}

// 缩放所选要素
void GwmDevAttrTable::mActionPanMapToSelectedRows_triggered()
{
  this->mymapCanvas->panToSelected( this->currentLayer );
}

// 缩放所选要素-更大
void GwmDevAttrTable::mActionZoomMapToSelectedRows_triggered()
{
  this->mymapCanvas->zoomToSelected( this->currentLayer );
}

// 打开要素计算器
void GwmDevAttrTable::mActionOpenFieldCalculator_triggered()
{
  QgsAttributeTableModel *masterModel = mMainView->masterModel();
  QgsFieldCalculator calc( this->currentLayer, this );
  if ( calc.exec() == QDialog::Accepted )
  {
    int col = masterModel->fieldCol( calc.changedAttributeId() );

    if ( col >= 0 )
    {
      masterModel->reload( masterModel->index( 0, col ), masterModel->index( masterModel->rowCount() - 1, col ) );
    }
  }
}

//
void GwmDevAttrTable::layerActionTriggered()
{
    QAction *qAction = qobject_cast<QAction *>( sender() );
    Q_ASSERT( qAction );

    QgsAction action = qAction->data().value<QgsAction>();

    QgsExpressionContext context = this->currentLayer->createExpressionContext();
    QgsExpressionContextScope *scope = new QgsExpressionContextScope();
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "action_scope" ), "AttributeTable" ) );
    context.appendScope( scope );
    action.run( context );
}

void GwmDevAttrTable::updateMultiEditButtonState()
{
  if ( ! this->currentLayer || ( this->currentLayer->editFormConfig().layout() == QgsEditFormConfig::EditorLayout::UiFileLayout ) )
    return;

  mActionToggleMultiEdit->setEnabled( this->currentLayer->isEditable() );

  if ( !this->currentLayer->isEditable() || ( this->currentLayer->isEditable() && mMainView->view() != QgsDualView::AttributeEditor ) )
  {
    mActionToggleMultiEdit->setChecked( false );
  }
}

void GwmDevAttrTable::updateFieldFromExpression()
{
  bool filtered = mMainView->filterMode() != QgsAttributeTableFilterModel::ShowAll;
  QgsFeatureIds filteredIds = filtered ? mMainView->filteredFeatures() : QgsFeatureIds();
  runFieldCalculation( this->currentLayer, mFieldCombo->currentField(), mUpdateExpressionText->asExpression(), filteredIds );
}

void GwmDevAttrTable::updateFieldFromExpressionSelected()
{
  QgsFeatureIds filteredIds = this->currentLayer->selectedFeatureIds();
  runFieldCalculation( this->currentLayer, mFieldCombo->currentField(), mUpdateExpressionText->asExpression(), filteredIds );
}

void GwmDevAttrTable::runFieldCalculation( QgsVectorLayer *layer, const QString &fieldName, const QString &expression, const QgsFeatureIds &filteredIds )
{
  int fieldindex = layer->fields().indexFromName( fieldName );
  if ( fieldindex < 0 )
  {
    // this shouldn't happen... but it did. There's probably some deeper underlying issue
    // but we may as well play it safe here.
    QMessageBox::critical( nullptr, tr( "Update Attributes" ), tr( "An error occurred while trying to update the field %1" ).arg( fieldName ) );
    return;
  }

  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );
  this->currentLayer->beginEditCommand( QStringLiteral( "Field calculator" ) );

  bool calculationSuccess = true;
  QString error;

  QgsExpression exp( expression );
  QgsDistanceArea da;
  da.setSourceCrs( this->currentLayer->crs(), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );
  exp.setGeomCalculator( &da );
  exp.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp.setAreaUnits( QgsProject::instance()->areaUnits() );
  bool useGeometry = exp.needsGeometry();

  QgsFeatureRequest request( mMainView->masterModel()->request() );
  useGeometry |= !request.filterRect().isNull();
  request.setFlags( useGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );

  int rownum = 1;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exp.prepare( &context );

  QgsField fld = layer->fields().at( fieldindex );

  QSet< QString >referencedColumns = exp.referencedColumns();
  referencedColumns.insert( fld.name() ); // need existing column value to store old attribute when changing field values
  request.setSubsetOfAttributes( referencedColumns, layer->fields() );

  //go through all the features and change the new attributes
  QgsFeatureIterator fit = layer->getFeatures( request );

  std::unique_ptr< QgsScopedProxyProgressTask > task = qgis::make_unique< QgsScopedProxyProgressTask >( tr( "Calculating field" ) );

  long long count = !filteredIds.isEmpty() ? filteredIds.size() : layer->featureCount();
  long long i = 0;

  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    if ( !filteredIds.isEmpty() && !filteredIds.contains( feature.id() ) )
    {
      continue;
    }

    i++;
    task->setProgress( i / static_cast< double >( count ) * 100 );

    context.setFeature( feature );
    context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), rownum, true ) );

    QVariant value = exp.evaluate( &context );
    ( void )fld.convertCompatible( value );
    // Bail if we have a update error
    if ( exp.hasEvalError() )
    {
      calculationSuccess = false;
      error = exp.evalErrorString();
      break;
    }
    else
    {
      QVariant oldvalue = feature.attributes().value( fieldindex );
      this->currentLayer->changeAttributeValue( feature.id(), fieldindex, value, oldvalue );
    }

    rownum++;
  }

  cursorOverride.release();
  task.reset();

  if ( !calculationSuccess )
  {
    QMessageBox::critical( nullptr, tr( "Update Attributes" ), tr( "An error occurred while evaluating the calculation string:\n%1" ).arg( error ) );
    this->currentLayer->destroyEditCommand();
  }
  else
  {
    this->currentLayer->endEditCommand();

    // refresh table with updated values
    // fixes https://github.com/qgis/QGIS/issues/25210
    QgsAttributeTableModel *masterModel = mMainView->masterModel();
    int modelColumn = masterModel->fieldCol( fieldindex );
    masterModel->reload( masterModel->index( 0, modelColumn ), masterModel->index( masterModel->rowCount() - 1, modelColumn ) );
  }
}

void GwmDevAttrTable::updateButtonStatus( const QString &fieldName, bool isValid )
{
  Q_UNUSED( fieldName )
  mRunFieldCalc->setEnabled( isValid );
}

void GwmDevAttrTable::updateTitle()
{
  if ( ! this->currentLayer )
  {
    return;
  }
//  QWidget *w = mDock ? qobject_cast<QWidget *>( mDock )
//               : mDialog ? qobject_cast<QWidget *>( mDialog )
//               : qobject_cast<QWidget *>( this );
//  w->setWindowTitle( tr( " %1 :: Features Total: %2, Filtered: %3, Selected: %4" )
//                     .arg( mLayer->name() )
//                     .arg( std::max( static_cast< long >( mMainView->featureCount() ), mLayer->featureCount() ) ) // layer count may be estimated, so use larger of the two
//                     .arg( mMainView->filteredFeatureCount() )
//                     .arg( mLayer->selectedFeatureCount() )
//                   );

  if ( mMainView->filterMode() == QgsAttributeTableFilterModel::ShowAll )
    mRunFieldCalc->setText( tr( "Update All" ) );
  else
    mRunFieldCalc->setText( tr( "Update Filtered" ) );

  bool canDeleteFeatures = this->currentLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool enabled = this->currentLayer->selectedFeatureCount() > 0;
  mRunFieldCalcSelected->setEnabled( enabled );
  mActionDeleteSelected->setEnabled( canDeleteFeatures && this->currentLayer->isEditable() && enabled );
  mActionCutSelectedRows->setEnabled( canDeleteFeatures && this->currentLayer->isEditable() && enabled );
  mActionCopySelectedRows->setEnabled( enabled );
}

void GwmDevAttrTable::mActionAddAttribute_triggered()
{
  if ( !this->currentLayer )
  {
    return;
  }

  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  QgsAddAttrDialog dialog( this->currentLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    this->currentLayer->beginEditCommand( tr( "Attribute added" ) );
    if ( this->currentLayer->addAttribute( dialog.field() ) )
    {
      this->currentLayer->endEditCommand();
    }
    else
    {
      this->currentLayer->destroyEditCommand();
      QMessageBox::critical( this, tr( "Add Field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( dialog.field().name(), dialog.field().typeName() ) );
    }


    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

void GwmDevAttrTable::mActionRemoveAttribute_triggered()
{
  if ( !this->currentLayer )
  {
    return;
  }

  QgsDelAttrDialog dialog( this->currentLayer );
  if ( dialog.exec() == QDialog::Accepted )
  {
    QList<int> attributes = dialog.selectedAttributes();
    if ( attributes.empty() )
    {
      return;
    }

    QgsAttributeTableModel *masterModel = mMainView->masterModel();

    this->currentLayer->beginEditCommand( tr( "Deleted attribute" ) );
    if ( this->currentLayer->deleteAttributes( attributes ) )
    {
      this->currentLayer->endEditCommand();
    }
    else
    {
      //QgisApp::instance()->messageBar()->pushMessage( tr( "Attribute error" ), tr( "The attribute(s) could not be deleted" ), Qgis::Warning, QgisApp::instance()->messageTimeout() );
      this->currentLayer->destroyEditCommand();
    }
    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

bool GwmDevAttrTable::toggleEditing2( QgsMapLayer *layer, bool allowCancel )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return false;
  }

  bool res = true;

  QString connString = QgsDataSourceUri( vlayer->source() ).connectionInfo();
  QString key = vlayer->providerType();

  QMap< QPair< QString, QString>, QgsTransactionGroup *> transactionGroups = QgsProject::instance()->transactionGroups();
  QMap< QPair< QString, QString>, QgsTransactionGroup *>::iterator tIt = transactionGroups .find( qMakePair( key, connString ) );
  QgsTransactionGroup *tg = ( tIt != transactionGroups.end() ? tIt.value() : nullptr );

  bool isModified = false;

  // Assume changes if: a) the layer reports modifications or b) its transaction group was modified
  if ( vlayer->isModified() )
    isModified  = true;

  if ( !vlayer->isEditable() && !vlayer->readOnly() )
  {
    if ( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
    {
      mActionToggleEditing->setChecked( false );
      mActionToggleEditing->setEnabled( false );
//      visibleMessageBar()->pushMessage( tr( "Start editing failed" ),
//                                        tr( "Provider cannot be opened for editing" ),
//                                        Qgis::Info, messageTimeout() );
      return false;
    }

    vlayer->startEditing();

    QgsSettings settings;
    QString markerType = settings.value( QStringLiteral( "qgis/digitizing/marker_style" ), "Cross" ).toString();
    bool markSelectedOnly = settings.value( QStringLiteral( "qgis/digitizing/marker_only_for_selected" ), true ).toBool();

    // redraw only if markers will be drawn
    if ( ( !markSelectedOnly || vlayer->selectedFeatureCount() > 0 ) &&
         ( markerType == QLatin1String( "Cross" ) || markerType == QLatin1String( "SemiTransparentCircle" ) ) )
    {
      vlayer->triggerRepaint();
    }
  }
  else if ( isModified )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if ( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch ( QMessageBox::question( nullptr,
                                    tr( "Stop Editing" ),
                                    tr( "Do you want to save the changes to layer %1?" ).arg( vlayer->name() ),
                                    buttons ) )
    {
      case QMessageBox::Cancel:
        res = false;
        break;

      case QMessageBox::Save:
        QApplication::setOverrideCursor( Qt::WaitCursor );

        if ( !vlayer->commitChanges() )
        {
          //commitError( vlayer );
          // Leave the in-memory editing state alone,
          // to give the user a chance to enter different values
          // and try the commit again later
          res = false;
        }

        vlayer->triggerRepaint();

        QApplication::restoreOverrideCursor();
        break;

      case QMessageBox::Discard:
      {
        QApplication::setOverrideCursor( Qt::WaitCursor );

        //QgsCanvasRefreshBlocker refreshBlocker;
        if ( !vlayer->rollBack() )
        {
//          visibleMessageBar()->pushMessage( tr( "Error" ),
//                                            tr( "Problems during roll back" ),
//                                            Qgis::Critical );
          res = false;
        }

        vlayer->triggerRepaint();

        QApplication::restoreOverrideCursor();
        break;
      }

      default:
        break;
    }
  }
  else //layer not modified
  {
    //QgsCanvasRefreshBlocker refreshBlocker;
    vlayer->rollBack();
    res = true;
    vlayer->triggerRepaint();
  }

  return res;
}

void GwmDevAttrTable::mActionSaveEdits_triggered()
{
    saveEdits(this->currentLayer,true,true);
}

void GwmDevAttrTable::saveEdits( QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isEditable() || !vlayer->isModified() )
    return;

//  if ( vlayer == activeLayer() )
//    mSaveRollbackInProgress = true;

  if ( !vlayer->commitChanges() )
  {
//    mSaveRollbackInProgress = false;
//    commitError( vlayer );
  }

  if ( leaveEditable )
  {
    vlayer->startEditing();
  }
  if ( triggerRepaint )
  {
    vlayer->triggerRepaint();
  }
}
