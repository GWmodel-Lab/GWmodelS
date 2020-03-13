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

static const int SYMBOL_LAYER_ITEM_TYPE = QStandardItem::UserType + 1;

DataDefinedRestorer::DataDefinedRestorer( QgsSymbol *symbol, const QgsSymbolLayer *symbolLayer )

{
  if ( symbolLayer->type() == QgsSymbol::Marker && symbol->type() == QgsSymbol::Marker )
  {
    Q_ASSERT( symbol->type() == QgsSymbol::Marker );
    mMarker = static_cast<QgsMarkerSymbol *>( symbol );
    mMarkerSymbolLayer = static_cast<const QgsMarkerSymbolLayer *>( symbolLayer );
    mDDSize = mMarker->dataDefinedSize();
    mDDAngle = mMarker->dataDefinedAngle();
    // check if restore is actually needed
    if ( !mDDSize && !mDDAngle )
      mMarker = nullptr;
  }
  else if ( symbolLayer->type() == QgsSymbol::Line && symbol->type() == QgsSymbol::Line )
  {
    mLine = static_cast<QgsLineSymbol *>( symbol );
    mLineSymbolLayer = static_cast<const QgsLineSymbolLayer *>( symbolLayer );
    mDDWidth = mLine->dataDefinedWidth();
    // check if restore is actually needed
    if ( !mDDWidth )
      mLine = nullptr;
  }
  save();
}

void DataDefinedRestorer::save()
{
  if ( mMarker )
  {
    mSize = mMarkerSymbolLayer->size();
    mAngle = mMarkerSymbolLayer->angle();
    mMarkerOffset = mMarkerSymbolLayer->offset();
  }
  else if ( mLine )
  {
    mWidth = mLineSymbolLayer->width();
    mLineOffset = mLineSymbolLayer->offset();
  }
}

void DataDefinedRestorer::restore()
{
  if ( mMarker )
  {
    if ( mDDSize &&
         ( mSize != mMarkerSymbolLayer->size() || mMarkerOffset != mMarkerSymbolLayer->offset() ) )
      mMarker->setDataDefinedSize( mDDSize );
    if ( mDDAngle &&
         mAngle != mMarkerSymbolLayer->angle() )
      mMarker->setDataDefinedAngle( mDDAngle );
  }
  else if ( mLine )
  {
    if ( mDDWidth &&
         ( mWidth != mLineSymbolLayer->width() || mLineOffset != mLineSymbolLayer->offset() ) )
      mLine->setDataDefinedWidth( mDDWidth );
  }
  save();
}

class SymbolLayerItem : public QStandardItem
{
  public:
    explicit SymbolLayerItem( QgsSymbolLayer *layer )
    {
      setLayer( layer );
    }

    explicit SymbolLayerItem( QgsSymbol *symbol )
    {
      setSymbol( symbol );
    }

    void setLayer( QgsSymbolLayer *layer )
    {
      mLayer = layer;
      mIsLayer = true;
      mSymbol = nullptr;
      updatePreview();
    }

    void setSymbol( QgsSymbol *symbol )
    {
      mSymbol = symbol;
      mIsLayer = false;
      mLayer = nullptr;
      updatePreview();
    }

    void updatePreview()
    {
      if ( !mSize.isValid() )
      {
        const int size = QgsGuiUtils::scaleIconSize( 16 );
        mSize = QSize( size, size );
      }
      QIcon icon;
      if ( mIsLayer )
        icon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( mLayer, QgsUnitTypes::RenderMillimeters, mSize ); //todo: make unit a parameter
      else
        icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSymbol, mSize );
      setIcon( icon );

      if ( parent() )
        static_cast<SymbolLayerItem *>( parent() )->updatePreview();
    }

    int type() const override { return SYMBOL_LAYER_ITEM_TYPE; }
    bool isLayer() { return mIsLayer; }

    // returns the symbol pointer; helpful in determining a layer's parent symbol
    QgsSymbol *symbol()
    {
      return mSymbol;
    }

    QgsSymbolLayer *layer()
    {
      return mLayer;
    }

    QVariant data( int role ) const override
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        if ( mIsLayer )
          return QgsApplication::symbolLayerRegistry()->symbolLayerMetadata( mLayer->layerType() )->visibleName();
        else
        {
          switch ( mSymbol->type() )
          {
            case QgsSymbol::Marker :
              return QCoreApplication::translate( "SymbolLayerItem", "Marker" );
            case QgsSymbol::Fill   :
              return QCoreApplication::translate( "SymbolLayerItem", "Fill" );
            case QgsSymbol::Line   :
              return QCoreApplication::translate( "SymbolLayerItem", "Line" );
            default:
              return "Symbol";
          }
        }
      }
      else if ( role == Qt::ForegroundRole && mIsLayer )
      {
        QBrush brush( Qt::black, Qt::SolidPattern );
        if ( !mLayer->enabled() )
        {
          brush.setColor( Qt::lightGray );
        }
        return brush;
      }

//      if ( role == Qt::SizeHintRole )
//        return QVariant( QSize( 32, 32 ) );
      if ( role == Qt::CheckStateRole )
        return QVariant(); // could be true/false
      return QStandardItem::data( role );
    }

  protected:
    QgsSymbolLayer *mLayer = nullptr;
    QgsSymbol *mSymbol = nullptr;
    bool mIsLayer;
    QSize mSize;
};


GwmSymbolWindow::GwmSymbolWindow(QgsVectorLayer* vectorLayer,QWidget *parent) : QWidget(parent)
{
    if(vectorLayer){
        mLayer = vectorLayer;
    }
    setMinimumWidth(880);
    setMinimumHeight(520);
    setWindowTitle("Symbol");
    QgsSingleSymbolRenderer* render = (QgsSingleSymbolRenderer*)mLayer->renderer();
    mSymbol = render->symbol();
    createComboBox();
    createStackWidget();
    createButtons();
    createLayout();
    getLayerSymbol();
}

void GwmSymbolWindow::createsingleSymbolWidget()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    singleSymbolWidget = new QWidget(this);
    singleSymbolWidget->setLayout(mainLayout);
    mainLayout->setMargin(0);

    QFrame* unitWidget = new QFrame(stack);
    QHBoxLayout* unitLayout = new QHBoxLayout(stack);
    unitLayout->setMargin(1);
    unitWidget->setLayout(unitLayout);
    QLabel* unitLabel = new QLabel("Unit");
    unitLayout->setContentsMargins(5,0,5,0);
    unitLabel->adjustSize();
    unitLayout->addWidget(unitLabel,1);
    QComboBox* unitCombobox = new QComboBox(stack);
    unitCombobox->addItem("Millmeter");
    unitCombobox->addItem("Kilometer");
    unitCombobox->addItem("Meter");
    unitLayout->addWidget(unitCombobox,15);
    mainLayout->addWidget(unitWidget);

    QFrame* opacityWidget = new QFrame(stack);
    QHBoxLayout* opacityLayout = new QHBoxLayout(stack);
    opacityLayout->setMargin(1);
    opacityWidget->setLayout(opacityLayout);
    opacityLayout->setContentsMargins(5,0,5,0);
    QLabel* opacityLabel = new QLabel("Opacity");
    opacityLabel->adjustSize();
    opacityLayout->addWidget(opacityLabel,1);
    QSlider* opacitySlider = new QSlider(Qt::Horizontal,stack);
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(100);
    opacityLayout->addWidget(opacitySlider,14);
    QDoubleSpinBox* opacitySpinBox = new QDoubleSpinBox(stack);
    opacitySpinBox->setSuffix("%");
    opacitySpinBox->setMinimum(0);
    opacitySpinBox->setMaximum(100.00);
    opacityLayout->addWidget(opacitySpinBox,1);
    mainLayout->addWidget(opacityWidget);

    QFrame* colorWidget = new QFrame(stack);
    QHBoxLayout* colorLayout = new QHBoxLayout(stack);
    colorLayout->setMargin(1);
    colorWidget->setLayout(colorLayout);
    QLabel* colorLabel = new QLabel("Color");
    colorLayout->setContentsMargins(5,0,5,0);
    colorLabel->adjustSize();
    colorLayout->addWidget(colorLabel,1);
    colorCombobox = new QComboBox(stack);
    QStringList colorList = QColor::colorNames();
    QString color;
    foreach( color, colorList )
    {
       QPixmap pix( QSize( 70, 20 ) );
       pix.fill( QColor( color ) );
       colorCombobox->addItem( QIcon( pix ), NULL );
       colorCombobox->setIconSize( QSize( 70, 20 ) );
       colorCombobox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    }

    colorLayout->addWidget(colorCombobox,15);
    mainLayout->addWidget(colorWidget);

    QFrame* sizeWidget = new QFrame(stack);
    QHBoxLayout* sizeLayout = new QHBoxLayout(stack);
    sizeLayout->setMargin(1);
    sizeWidget->setLayout(sizeLayout);
    QLabel* sizeLabel = new QLabel("Size");
    sizeLayout->setContentsMargins(5,0,5,0);
    sizeLabel->adjustSize();
    sizeLayout->addWidget(sizeLabel,1);
    QDoubleSpinBox* sizeSpinBox = new QDoubleSpinBox(stack);
    sizeSpinBox->setMinimum(0.00);
    sizeLayout->addWidget(sizeSpinBox,15);
    mainLayout->addWidget(sizeWidget);

    QFrame* rotationWidget = new QFrame(stack);
    QHBoxLayout* rotationLayout = new QHBoxLayout(stack);
    rotationLayout->setMargin(1);
    rotationWidget->setLayout(rotationLayout);
    QLabel* rotationLabel = new QLabel("Rotation");
    rotationLayout->setContentsMargins(5,0,5,0);
    rotationLabel->adjustSize();
    rotationLayout->addWidget(rotationLabel,1);
    QDoubleSpinBox* rotationSpinBox = new QDoubleSpinBox(stack);
    rotationSpinBox->setSuffix("°");
    rotationSpinBox->setMinimum(0.00);
    rotationLayout->addWidget(rotationSpinBox,15);
    mainLayout->addWidget(rotationWidget);

    layersTree = new QTreeView(stack);
    layersTree->setMaximumHeight( static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 7 ) );
    layersTree->setMinimumHeight( layersTree->maximumHeight() );
    model = new QStandardItemModel( layersTree );
    //设置符号模型
    layersTree->setModel( model );
    layersTree->setHeaderHidden( true );

    QItemSelectionModel *selModel = layersTree->selectionModel();
//      connect( selModel, &QItemSelectionModel::currentChanged, this, &GwmSymbolWindow::layerChanged );
    //加载符号样式
    loadSymbol( mSymbol, static_cast<SymbolLayerItem *>( model->invisibleRootItem() ) );
    QModelIndex newIndex = layersTree->model()->index( 0, 0 );
    layersTree->setCurrentIndex( newIndex );

    mainLayout->addWidget(layersTree);
    mainLayout->addWidget(stackedWidget);
}

void GwmSymbolWindow::createcategorizedWidget()
{
    categorizedWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    mainLayout->setMargin(0);
    categorizedWidget->setLayout(mainLayout);
    QLabel* label1 = new QLabel("categorizedWidget");
    label1->adjustSize();
    mainLayout->addWidget(label1);
}

void GwmSymbolWindow::creategraduatedWidget()
{
    graduatedWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(stack);
    graduatedWidget->setLayout(mainLayout);

    mainLayout->addWidget(new QLabel(tr("graduatedWidget")));

}

void GwmSymbolWindow::createLayout()
{
    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->addWidget(symbolTypeSelect);
    windowLayout->setMargin(1);
    windowLayout->addWidget(stack);
    windowLayout->addStretch(1);
    windowLayout->addWidget(btnContainer);
    setLayout(windowLayout);
}

void GwmSymbolWindow::createComboBox()
{
    symbolTypeSelect = new QComboBox(this);
//    symbolTypeSelect->setMaximumWidth(this->width()-10);
//    symbolTypeSelect->setStyleSheet("border");
    symbolTypeSelect->setStyle(QStyleFactory::create("Fusion"));
    symbolTypeSelect->addItem("Single Symbol");
    symbolTypeSelect->addItem("Categorized");
    symbolTypeSelect->addItem("Graduated");
    connect(symbolTypeSelect,SIGNAL(currentIndexChanged(int)),this,SLOT(toggleWidget(int)));
}

void GwmSymbolWindow::createStackWidget()
{
    stack = new QStackedWidget(this);
    stack->setFrameStyle(QFrame::Panel | QFrame::Raised );
    createsingleSymbolWidget();
    createcategorizedWidget();
    creategraduatedWidget();
    stack->addWidget(singleSymbolWidget);
    stack->addWidget(categorizedWidget);
    stack->addWidget(graduatedWidget);
}

//combobox值改变 切换窗口槽函数
void GwmSymbolWindow::toggleWidget(int index)
{
    stack->setCurrentIndex(index);
}

void GwmSymbolWindow::loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent )
{
  if ( !symbol )
    return;

  if ( !parent )
  {
    mSymbol = symbol;
    model->clear();
    parent = static_cast<SymbolLayerItem *>( model->invisibleRootItem() );
  }

  SymbolLayerItem *symbolItem = new SymbolLayerItem( symbol );
  QFont boldFont = symbolItem->font();
  boldFont.setBold( true );
  symbolItem->setFont( boldFont );
  parent->appendRow( symbolItem );

  int count = symbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    SymbolLayerItem *layerItem = new SymbolLayerItem( symbol->symbolLayer( i ) );
    layerItem->setEditable( false );
    symbolItem->appendRow( layerItem );
    if ( symbol->symbolLayer( i )->subSymbol() )
    {
      loadSymbol( symbol->symbolLayer( i )->subSymbol(), layerItem );
    }
    layersTree->setExpanded( layerItem->index(), true );
  }
  layersTree->setExpanded( symbolItem->index(), true );
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

    btnContainer->setLayout(btnLayout);
}

void GwmSymbolWindow::applyLayerSymbol()
{
    QStringList colorList = QColor::colorNames();
    QColor color = QColor( colorList[ colorCombobox->currentIndex() ] );
    QgsSimpleMarkerSymbolLayer* newMarkerSymbol = new QgsSimpleMarkerSymbolLayer(
                QgsSimpleMarkerSymbolLayerBase::Star,
                3,
                0.0,
                QgsSymbol::ScaleDiameter,
                color
                );
    QgsSymbolLayerList symList;
    symList.append(newMarkerSymbol);
    QgsMarkerSymbol* markSym = new QgsMarkerSymbol(symList);
    QgsSingleSymbolRenderer* symRenderer = new QgsSingleSymbolRenderer(markSym);
    mLayer->setRenderer(symRenderer);
    emit canvasRefreshSingal();
}




void GwmSymbolWindow::getLayerSymbol()
{
//    QgsSingleSymbolRenderer* render = (QgsSingleSymbolRenderer*)mLayer->renderer();
//    QgsSymbol* layerSymbol = render->symbol();
//    QgsMarkerSymbolLayer* svgMarker = (QgsMarkerSymbolLayer*)layerSymbol->symbolLayers().at(0);
//    svgMarker->size();
//    qDebug() << svgMarker->color();

//    qDebug() << svgMarker;
}
