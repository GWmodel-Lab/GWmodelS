#include <QWidget>
#include "gwmsymbolwindow.h"
#include <QLabel>
#include <QStyleFactory>
#include <QFile>
#include <QSlider>
#include <QDoubleSpinBox>
#include <qgsvectorlayer.h>
#include <QDebug>
#include <qgssinglesymbolrenderer.h>
#include <qgsmarkersymbollayer.h>
#include <qgssymbol.h>
#include <QListWidget>
#include <qgssymbollayerregistry.h>
#include <qgsapplication.h>
#include "qgsguiutils.h"
#include "qgsgui.h"
#include <qgssymbollayerutils.h>
#include "qgsrendererregistry.h"


static bool _initRenderer( const QString &name, QgsRendererWidgetFunc f, const QString &iconName = QString() )
{
  QgsRendererRegistry *reg = QgsApplication::rendererRegistry();
  QgsRendererAbstractMetadata *am = reg->rendererMetadata( name );
  if ( !am )
    return false;
  QgsRendererMetadata *m = dynamic_cast<QgsRendererMetadata *>( am );
  if ( !m )
    return false;

  m->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    m->setIcon( QgsApplication::getThemeIcon( iconName ) );
  }

  QgsDebugMsgLevel( "Set for " + name, 2 );
  return true;
}

static void _initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  _initRenderer( QStringLiteral( "singleSymbol" ), QgsSingleSymbolRendererWidget::create, QStringLiteral( "rendererSingleSymbol.svg" ) );
  _initRenderer( QStringLiteral( "categorizedSymbol" ), QgsCategorizedSymbolRendererWidget::create, QStringLiteral( "rendererCategorizedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "graduatedSymbol" ), QgsGraduatedSymbolRendererWidget::create, QStringLiteral( "rendererGraduatedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "RuleRenderer" ), QgsRuleBasedRendererWidget::create, QStringLiteral( "rendererRuleBasedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "pointDisplacement" ), QgsPointDisplacementRendererWidget::create, QStringLiteral( "rendererPointDisplacementSymbol.svg" ) );
  _initRenderer( QStringLiteral( "pointCluster" ), QgsPointClusterRendererWidget::create, QStringLiteral( "rendererPointClusterSymbol.svg" ) );
  _initRenderer( QStringLiteral( "invertedPolygonRenderer" ), QgsInvertedPolygonRendererWidget::create, QStringLiteral( "rendererInvertedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "heatmapRenderer" ), QgsHeatmapRendererWidget::create, QStringLiteral( "rendererHeatmapSymbol.svg" ) );
//  _initRenderer( QStringLiteral( "25dRenderer" ), Qgs25DRendererWidget::create, QStringLiteral( "renderer25dSymbol.svg" ) );
  _initRenderer( QStringLiteral( "nullSymbol" ), QgsNullSymbolRendererWidget::create, QStringLiteral( "rendererNullSymbol.svg" ) );
  sInitialized = true;
}

GwmSymbolWindow::GwmSymbolWindow(QgsVectorLayer* vectorLayer,QWidget *parent) : QWidget(parent)
{
    if(vectorLayer){
        mLayer = vectorLayer;
    }
    setMinimumWidth(880);
    setMinimumHeight(520);
    setWindowTitle("Symbol");
    _initRendererWidgetFunctions();

    QgsRendererRegistry *reg = QgsApplication::rendererRegistry();
    QStringList renderers = reg->renderersList( mLayer );
    const auto constRenderers = renderers;
    symbolTypeSelect = new QComboBox(this);
    stack = new QStackedWidget(this);
    stack->setFrameStyle(QFrame::Panel | QFrame::Raised );
    symbolTypeSelect->setStyle(QStyleFactory::create("Fusion"));
    for ( const QString &name : constRenderers )
    {
      QgsRendererAbstractMetadata *m = reg->rendererMetadata( name );
//      if(name != "RuleRenderer")
      symbolTypeSelect->addItem( m->icon(), m->visibleName(), name );
    }
//    symbolTypeSelect->setCurrentIndex( -1 );
    symbolTypeSelect->setCurrentIndex( 1 ); // set no current renderer
    rendererChanged(1);
    connect( symbolTypeSelect, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &GwmSymbolWindow::rendererChanged );
    createButtons();
    createLayout();
}



void GwmSymbolWindow::createLayout()
{
    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->addWidget(symbolTypeSelect);
    windowLayout->setMargin(6);
    windowLayout->addWidget(stack);
    windowLayout->addWidget(btnContainer);
    setLayout(windowLayout);
}


//combobox值改变 切换窗口槽函数
void GwmSymbolWindow::rendererChanged(int index)
{
    if ( symbolTypeSelect->currentIndex() == -1 )
    {
      QgsDebugMsg( QStringLiteral( "No current item -- this should never happen!" ) );
      return;
    }

    QString rendererName = symbolTypeSelect->currentData().toString();

    //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
    QgsFeatureRenderer *oldRenderer = nullptr;
//    if ( mActiveWidget && mActiveWidget->renderer() )
//    {
//      qDebug() << "true";
//      oldRenderer = mActiveWidget->renderer()->clone();
//    }
//    else
//    {
//      qDebug() << "false";
//      oldRenderer = mLayer->renderer()->clone();
//    }
    oldRenderer = mLayer->renderer()->clone();
    // get rid of old active widget (if any)
    if ( mActiveWidget )
    {
      stack->removeWidget( mActiveWidget );

      delete mActiveWidget;
      mActiveWidget = nullptr;
    }

    QgsRendererWidget *w = nullptr;
    QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererName );
    if ( m )
      w = m->createRendererWidget( mLayer, mStyle, oldRenderer );
    delete oldRenderer;
    if ( w )
    {
      // instantiate the widget and set as active
      mActiveWidget = w;
      stack->addWidget( mActiveWidget );
      stack->setCurrentWidget( mActiveWidget );
      if ( mActiveWidget->renderer() )
      {
//        if ( mMapCanvas || mMessageBar )
//        {
//          QgsSymbolWidgetContext context;
//          context.setMapCanvas( mMapCanvas );
//          context.setMessageBar( mMessageBar );
//          mActiveWidget->setContext( context );
//        }
//        changeOrderBy( mActiveWidget->renderer()->orderBy(), mActiveWidget->renderer()->orderByEnabled() );
//        connect( mActiveWidget, &QgsRendererWidget::layerVariablesChanged, this, &GwmSymbolWindow::layerVariablesChanged );
      }
//      connect( mActiveWidget, &QgsPanelWidget::widgetChanged, this, &QgsRendererPropertiesDialog::widgetChanged );
//     connect( mActiveWidget, &QgsPanelWidget::showPanel, this, &QgsRendererPropertiesDialog::openPanel );
//      w->setDockMode( mDockMode );
    }
    else
    {
      // set default "no edit widget available" page
      stack->setCurrentWidget( pageNoWidget );
    }
}

void GwmSymbolWindow::createButtons()
{
    btnContainer = new QWidget(this);
    QHBoxLayout* btnLayout = new QHBoxLayout(this);

    okBtn = new QPushButton(tr("OK"));
    okBtn->setFixedSize(80,25);

    cancelBtn = new QPushButton(tr("Cancel"));
    cancelBtn->setFixedSize(80,25);

    applyBtn = new QPushButton(tr("Apply"));
    applyBtn->setFixedSize(80,25);

    helpBtn = new QPushButton(tr("Help"));
    helpBtn->setFixedSize(80,25);

    btnLayout->addStretch(1);
    btnLayout->setSpacing(5);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(helpBtn);

    connect(applyBtn,&QPushButton::clicked,this,&GwmSymbolWindow::applyLayerSymbol);
    connect(cancelBtn,&QPushButton::clicked,this,&GwmSymbolWindow::close);
    connect(okBtn,&QPushButton::clicked,this,&GwmSymbolWindow::onOkBtnClicked);
    btnContainer->setLayout(btnLayout);
}

void GwmSymbolWindow::applyLayerSymbol()
{
    if ( !mActiveWidget || !mLayer )
    {
        return;
    }

    mActiveWidget->applyChanges();

    QgsFeatureRenderer *renderer = mActiveWidget->renderer();
    if ( renderer )
    {
       // renderer->setPaintEffect( mPaintEffect->clone() );
        // set the order by
       // renderer->setOrderBy( mOrderBy );
       // renderer->setOrderByEnabled( checkboxEnableOrderBy->isChecked() );

        mLayer->setRenderer( renderer->clone() );
        emit canvasRefreshSingal();
    }
}

void GwmSymbolWindow::onOkBtnClicked()
{
    applyLayerSymbol();
    this->close();
}

