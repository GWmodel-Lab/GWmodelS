/***************************************************************************
    Gwmfeaturefilterwidget.cpp
     --------------------------------------
    Date                 : 20.9.2019
    Copyright            : (C) 2019 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "gwmfeaturefilterwidget_p.h"

#include "qgsapplication.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsdualview.h"
#include "qgsstoredexpressionmanager.h"
#include "qgseditorwidgetregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsgui.h"
#include "qgsdialog.h"
#include "qgsexpressionlineedit.h"
#include "qgsmessagebar.h"

#include <QMenu>

GwmFeatureFilterWidget::GwmFeatureFilterWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  // Initialize filter gui elements
  mFilterColumnsMenu = new QMenu( this );
  mActionFilterColumnsMenu->setMenu( mFilterColumnsMenu );
  mStoredFilterExpressionMenu = new QMenu( this );
  mActionStoredFilterExpressions->setMenu( mStoredFilterExpressionMenu );

  // Set filter icon in a couple of places
  mActionShowAllFilter->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTable.svg" ) );
  mActionAdvancedFilter->setIcon( QgsApplication::getThemeIcon( "/mActionFilter2.svg" ) );
  mActionSelectedFilter->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTableSelected.svg" ) );
  mActionVisibleFilter->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTableVisible.svg" ) );
  mActionEditedFilter->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTableEdited.svg" ) );


  // Set button to store or delete stored filter expressions
  mStoreFilterExpressionButton->setDefaultAction( mActionHandleStoreFilterExpression );
  connect( mActionSaveAsStoredFilterExpression, &QAction::triggered, this, &GwmFeatureFilterWidget::saveAsStoredFilterExpression );
  connect( mActionEditStoredFilterExpression, &QAction::triggered, this, &GwmFeatureFilterWidget::editStoredFilterExpression );
  connect( mActionHandleStoreFilterExpression, &QAction::triggered, this, &GwmFeatureFilterWidget::handleStoreFilterExpression );
  mApplyFilterButton->setDefaultAction( mActionApplyFilter );

  // Connect filter signals
  connect( mActionAdvancedFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterExpressionBuilder );
  connect( mActionShowAllFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterShowAll );
  connect( mActionSelectedFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterSelected );
  connect( mActionVisibleFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterVisible );
  connect( mActionEditedFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterEdited );
  connect( mFilterQuery, &QLineEdit::returnPressed, this, &GwmFeatureFilterWidget::filterQueryAccepted );
  connect( mActionApplyFilter, &QAction::triggered, this, &GwmFeatureFilterWidget::filterQueryAccepted );
  connect( mFilterQuery, &QLineEdit::textChanged, this, &GwmFeatureFilterWidget::onFilterQueryTextChanged );
}

void GwmFeatureFilterWidget::init( QgsVectorLayer *layer, const QgsAttributeEditorContext &context, QgsDualView *mainView)
{
  mMainView = mainView;
  mLayer = layer;
  mEditorContext = context;
//  mMessageBar = messageBar;
//  mMessageBarTimeout = messageBarTimeout;

  connect( mLayer, &QgsVectorLayer::attributeAdded, this, &GwmFeatureFilterWidget::columnBoxInit );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &GwmFeatureFilterWidget::columnBoxInit );

  //set delay on entering text
  mFilterQueryTimer.setSingleShot( true );
  connect( &mFilterQueryTimer, &QTimer::timeout, this, &GwmFeatureFilterWidget::updateCurrentStoredFilterExpression );

  columnBoxInit();
  storedFilterExpressionBoxInit();
  storeExpressionButtonInit();
}

void GwmFeatureFilterWidget::filterShowAll()
{
  mFilterButton->setDefaultAction( mActionShowAllFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mFilterQuery->setText( QString() );
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
  }
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowAll );
}

void GwmFeatureFilterWidget::filterSelected()
{
  mFilterButton->setDefaultAction( mActionSelectedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowSelected );
}

void GwmFeatureFilterWidget::filterVisible()
{
  if ( !mLayer->isSpatial() )
  {
    filterShowAll();
    return;
  }

  mFilterButton->setDefaultAction( mActionVisibleFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
}

void GwmFeatureFilterWidget::filterEdited()
{
  mFilterButton->setDefaultAction( mActionEditedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowEdited );
}


void GwmFeatureFilterWidget::filterQueryAccepted()
{
  if ( ( mFilterQuery->isVisible() && mFilterQuery->text().isEmpty() ) ||
       ( mCurrentSearchWidgetWrapper && mCurrentSearchWidgetWrapper->widget()->isVisible()
         && mCurrentSearchWidgetWrapper->expression().isEmpty() ) )
  {
    filterShowAll();
    return;
  }
  filterQueryChanged( mFilterQuery->text() );
}

void GwmFeatureFilterWidget::filterQueryChanged( const QString &query )
{
  QString str;
  if ( mFilterButton->defaultAction() == mActionAdvancedFilter )
  {
    str = query;
  }
  else if ( mCurrentSearchWidgetWrapper )
  {
    str = mCurrentSearchWidgetWrapper->expression();
  }

  setFilterExpression( str );
}

void GwmFeatureFilterWidget::columnBoxInit()
{
  const auto constActions = mFilterColumnsMenu->actions();
  for ( QAction *a : constActions )
  {
    mFilterColumnsMenu->removeAction( a );
    mFilterButton->removeAction( a );
    delete a;
  }

  mFilterButton->addAction( mActionShowAllFilter );
  mFilterButton->addAction( mActionSelectedFilter );
  if ( mLayer->isSpatial() )
  {
    mFilterButton->addAction( mActionVisibleFilter );
  }
  mFilterButton->addAction( mActionEditedFilter );
  mFilterButton->addAction( mActionFilterColumnsMenu );
  mFilterButton->addAction( mActionAdvancedFilter );
  mFilterButton->addAction( mActionStoredFilterExpressions );

  const QList<QgsField> fields = mLayer->fields().toList();

  const auto constFields = fields;
  for ( const QgsField &field : constFields )
  {
    const int idx = mLayer->fields().lookupField( field.name() );
    if ( idx < 0 )
      continue;

    if ( QgsGui::editorWidgetRegistry()->findBest( mLayer, field.name() ).type() != QLatin1String( "Hidden" ) )
    {
      const QIcon icon = mLayer->fields().iconForField( idx );
      const QString alias = mLayer->attributeDisplayName( idx );

      // Generate action for the filter popup button
      QAction *filterAction = new QAction( icon, alias, mFilterButton );
      filterAction->setData( field.name() );

      connect( filterAction, &QAction::triggered, this, [ = ] { filterColumnChanged( filterAction ); } );
      mFilterColumnsMenu->addAction( filterAction );
    }
  }
}

void GwmFeatureFilterWidget::handleStoreFilterExpression()
{
  if ( !mActionHandleStoreFilterExpression->isChecked() )
  {
    mLayer->storedExpressionManager()->removeStoredExpression( mActionHandleStoreFilterExpression->data().toString() );
  }
  else
  {
    mLayer->storedExpressionManager()->addStoredExpression( mFilterQuery->text(), mFilterQuery->text() );
  }

  updateCurrentStoredFilterExpression();
  storedFilterExpressionBoxInit();
}

void GwmFeatureFilterWidget::storedFilterExpressionBoxInit()
{
  const auto constActions = mStoredFilterExpressionMenu->actions();
  for ( QAction *a : constActions )
  {
    mStoredFilterExpressionMenu->removeAction( a );
    delete a;
  }

  const QList< QgsStoredExpression > storedExpressions = mLayer->storedExpressionManager()->storedExpressions();
  for ( const QgsStoredExpression &storedExpression : storedExpressions )
  {
    QAction *storedExpressionAction = new QAction( storedExpression.name, mFilterButton );
    connect( storedExpressionAction, &QAction::triggered, this, [ = ]()
    {
      setFilterExpression( storedExpression.expression, QgsAttributeForm::ReplaceFilter, true );
    } );
    mStoredFilterExpressionMenu->addAction( storedExpressionAction );
  }
}

void GwmFeatureFilterWidget::storeExpressionButtonInit()
{
  if ( mActionHandleStoreFilterExpression->isChecked() )
  {
    mActionHandleStoreFilterExpression->setToolTip( tr( "Delete stored expression" ) );
    mActionHandleStoreFilterExpression->setText( tr( "Delete Stored Expression" ) );
    mActionHandleStoreFilterExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionHandleStoreFilterExpressionChecked.svg" ) ) );
    mStoreFilterExpressionButton->removeAction( mActionSaveAsStoredFilterExpression );
    mStoreFilterExpressionButton->addAction( mActionEditStoredFilterExpression );
  }
  else
  {
    mActionHandleStoreFilterExpression->setToolTip( tr( "Save expression with the text as name" ) );
    mActionHandleStoreFilterExpression->setText( tr( "Save Expression" ) );
    mActionHandleStoreFilterExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionHandleStoreFilterExpressionUnchecked.svg" ) ) );
    mStoreFilterExpressionButton->addAction( mActionSaveAsStoredFilterExpression );
    mStoreFilterExpressionButton->removeAction( mActionEditStoredFilterExpression );
  }
}


void GwmFeatureFilterWidget::filterColumnChanged( QAction *filterAction )
{
  mFilterButton->setDefaultAction( filterAction );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  // replace the search line edit with a search widget that is suited to the selected field
  // delete previous widget
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
    delete mCurrentSearchWidgetWrapper;
  }
  const QString fieldName = mFilterButton->defaultAction()->data().toString();
  // get the search widget
  const int fldIdx = mLayer->fields().lookupField( fieldName );
  if ( fldIdx < 0 )
    return;
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldName );
  mCurrentSearchWidgetWrapper = QgsGui::editorWidgetRegistry()->
                                createSearchWidget( setup.type(), mLayer, fldIdx, setup.config(), mFilterContainer, mEditorContext );
  if ( mCurrentSearchWidgetWrapper->applyDirectly() )
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &GwmFeatureFilterWidget::filterQueryChanged );
    mApplyFilterButton->setVisible( false );
    mStoreFilterExpressionButton->setVisible( false );
  }
  else
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &GwmFeatureFilterWidget::filterQueryAccepted );
    mApplyFilterButton->setVisible( true );
    mStoreFilterExpressionButton->setVisible( true );
  }

  replaceSearchWidget( mFilterQuery, mCurrentSearchWidgetWrapper->widget() );
}

void GwmFeatureFilterWidget::filterExpressionBuilder()
{
  // Show expression builder
  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, mFilterQuery->text(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Filter" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    setFilterExpression( dlg.expressionText(), QgsAttributeForm::ReplaceFilter, true );
  }
}

void GwmFeatureFilterWidget::saveAsStoredFilterExpression()
{
  QgsDialog *dlg = new QgsDialog( this, Qt::WindowFlags(), QDialogButtonBox::Save | QDialogButtonBox::Cancel );
  dlg->setWindowTitle( tr( "Save Expression As" ) );
  QVBoxLayout *layout = dlg->layout();
  dlg->resize( std::max( 400, this->width() / 2 ), dlg->height() );

  QLabel *nameLabel = new QLabel( tr( "Name" ), dlg );
  QLineEdit *nameEdit = new QLineEdit( dlg );
  layout->addWidget( nameLabel );
  layout->addWidget( nameEdit );
  nameEdit->setFocus();

  if ( dlg->exec() == QDialog::Accepted )
  {
    mLayer->storedExpressionManager()->addStoredExpression( nameEdit->text(), mFilterQuery->text() );

    updateCurrentStoredFilterExpression();
    storedFilterExpressionBoxInit();
  }
}

void GwmFeatureFilterWidget::editStoredFilterExpression()
{
  QgsDialog *dlg = new QgsDialog( this, Qt::WindowFlags(), QDialogButtonBox::Save | QDialogButtonBox::Cancel );
  dlg->setWindowTitle( tr( "Edit expression" ) );
  QVBoxLayout *layout = dlg->layout();
  dlg->resize( std::max( 400, this->width() / 2 ), dlg->height() );

  QLabel *nameLabel = new QLabel( tr( "Name" ), dlg );
  QLineEdit *nameEdit = new QLineEdit( mLayer->storedExpressionManager()->storedExpression( mActionHandleStoreFilterExpression->data().toString() ).name, dlg );
  QLabel *expressionLabel = new QLabel( tr( "Expression" ), dlg );
  QgsExpressionLineEdit *expressionEdit = new QgsExpressionLineEdit( dlg );
  expressionEdit->setExpression( mLayer->storedExpressionManager()->storedExpression( mActionHandleStoreFilterExpression->data().toString() ).expression );

  layout->addWidget( nameLabel );
  layout->addWidget( nameEdit );
  layout->addWidget( expressionLabel );
  layout->addWidget( expressionEdit );
  nameEdit->setFocus();

  if ( dlg->exec() == QDialog::Accepted )
  {
    //update stored expression
    mLayer->storedExpressionManager()->updateStoredExpression( mActionHandleStoreFilterExpression->data().toString(), nameEdit->text(), expressionEdit->expression(), QgsStoredExpression::Category::FilterExpression );

    //update text
    mFilterQuery->setValue( expressionEdit->expression() );

    storedFilterExpressionBoxInit();
  }
}

void GwmFeatureFilterWidget::updateCurrentStoredFilterExpression()
{
  const QgsStoredExpression currentStoredExpression = mLayer->storedExpressionManager()->findStoredExpressionByExpression( mFilterQuery->value() );

  //set checked when it's an existing stored expression
  mActionHandleStoreFilterExpression->setChecked( !currentStoredExpression.id.isNull() );

  mActionHandleStoreFilterExpression->setData( currentStoredExpression.id );
  mActionEditStoredFilterExpression->setData( currentStoredExpression.id );

  //update bookmark button
  storeExpressionButtonInit();
}

void GwmFeatureFilterWidget::setFilterExpression( const QString &filterString, QgsAttributeForm::FilterType type, bool alwaysShowFilter )
{
  QString filter;
  if ( !mFilterQuery->text().isEmpty() && !filterString.isEmpty() )
  {
    switch ( type )
    {
      case QgsAttributeForm::ReplaceFilter:
        filter = filterString;
        break;

      case QgsAttributeForm::FilterAnd:
        filter = QStringLiteral( "(%1) AND (%2)" ).arg( mFilterQuery->text(), filterString );
        break;

      case QgsAttributeForm::FilterOr:
        filter = QStringLiteral( "(%1) OR (%2)" ).arg( mFilterQuery->text(), filterString );
        break;
    }
  }
  else if ( !filterString.isEmpty() )
  {
    filter = filterString;
  }
  else
  {
    filterShowAll();
    return;
  }

  mFilterQuery->setText( filter );

  if ( alwaysShowFilter || !mCurrentSearchWidgetWrapper || !mCurrentSearchWidgetWrapper->applyDirectly() )
  {

    mFilterButton->setDefaultAction( mActionAdvancedFilter );
    mFilterButton->setPopupMode( QToolButton::MenuButtonPopup );
    mFilterQuery->setVisible( true );
    mApplyFilterButton->setVisible( true );
    mStoreFilterExpressionButton->setVisible( true );
    if ( mCurrentSearchWidgetWrapper )
    {
      // replace search widget widget with the normal filter query line edit
      replaceSearchWidget( mCurrentSearchWidgetWrapper->widget(), mFilterQuery );
    }
  }

  // parse search string and build parsed tree
  QgsExpression filterExpression( filter );
  if ( filterExpression.hasParserError() )
  {
    // mMessageBar->pushMessage( tr( "Parsing error" ), filterExpression.parserErrorString(), Qgis::MessageLevel::Warning );
    return;
  }

  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  if ( !filterExpression.prepare( &context ) )
  {
    // mMessageBar->pushMessage( tr( "Evaluation error" ), filterExpression.evalErrorString(), Qgis::MessageLevel::Warning );
  }

  mMainView->filterFeatures( filterExpression, context );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowFilteredList );
}

void GwmFeatureFilterWidget::replaceSearchWidget( QWidget *oldw, QWidget *neww )
{
  mFilterLayout->removeWidget( oldw );
  oldw->setVisible( false );
  mFilterLayout->addWidget( neww, 0, 0 );
  neww->setVisible( true );
  neww->setFocus();
}

void GwmFeatureFilterWidget::onFilterQueryTextChanged( const QString &value )
{
  Q_UNUSED( value );
  mFilterQueryTimer.start( 300 );
}

/// @endcond
