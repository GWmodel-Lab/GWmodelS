#include "gwmlayervectoritem.h"
#include "gwmlayersymbolitem.h"
#include <qgsrenderer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgscategorizedsymbolrenderer.h>
#include <qgsvectorfilewritertask.h>
#include <qgsapplication.h>

GwmLayerVectorItem::SymbolType GwmLayerVectorItem::renderTypeToSymbolType(QString type)
{
    if ( type == QStringLiteral( "singleSymbol" ) ) return SymbolType::singleSymbol;
    else if ( type == QStringLiteral( "categorizedSymbol" ) ) return SymbolType::categorizedSymbol;
    else if ( type == QStringLiteral( "graduatedSymbol" ) ) return SymbolType::graduatedSymbol;
    else if ( type == QStringLiteral( "RuleRenderer" ) ) return SymbolType::RuleRenderer;
    else if ( type == QStringLiteral( "heatmapRenderer" ) ) return SymbolType::heatmapRenderer;
    else if ( type == QStringLiteral( "invertedPolygonRenderer" ) ) return SymbolType::invertedPolygonRenderer;
    else if ( type == QStringLiteral( "pointCluster" ) ) return SymbolType::pointCluster;
    else if ( type == QStringLiteral( "pointDisplacement" ) ) return SymbolType::pointDisplacement;
    else return SymbolType::nullSymbol;
}

GwmLayerVectorItem::GwmLayerVectorItem(GwmLayerItem* parent, QgsVectorLayer* vector)
    : GwmLayerItem(parent)
    , mLayer(vector)
{
    if (vector)
    {
        createSymbolChildren();
        mSymbolType = GwmLayerVectorItem::renderTypeToSymbolType(vector->renderer()->type());
        connect(mLayer, &QgsVectorLayer::rendererChanged, this, &GwmLayerVectorItem::onLayerRendererChanged);
    }
}

GwmLayerVectorItem::~GwmLayerVectorItem()
{
    for (GwmLayerSymbolItem* item : mSymbolChildren)
    {
        delete item;
    }
    mSymbolChildren.clear();
}


QString GwmLayerVectorItem::text()
{
    return mLayer ? mLayer->name() : QStringLiteral("");
}

GwmLayerItem* GwmLayerVectorItem::child(int row)
{
    if (childCount() > 0)
        return mSymbolChildren.at(row);
    else return nullptr;
}

int GwmLayerVectorItem::childCount()
{
    switch (mSymbolType) {
    case SymbolType::singleSymbol:
    case SymbolType::nullSymbol:
        return 0;
    default:
        return mSymbolChildren.size();
    }
    return mSymbolType == SymbolType::singleSymbol ? 0 : mSymbolChildren.size();
}

int GwmLayerVectorItem::childNumber()
{
    return 0;
}

bool GwmLayerVectorItem::insertChildren(int position, int count)
{
    return false;
}

bool GwmLayerVectorItem::removeChildren(int position, int count)
{
    return false;
}

QVariant GwmLayerVectorItem::data(int col, int role)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            return col == 0 ? text() : QString();
        case Qt::DecorationRole:
        {
            if (mSymbolType == singleSymbol)
            {
                return mSymbolChildren.first()->symbol();
            }
            else
            {
                switch (mLayer->geometryType())
                {
                case QgsWkbTypes::PointGeometry:
                    return  QIcon(QStringLiteral(":/images/themes/default/mIconPointLayer.svg"));
                    break;
                case QgsWkbTypes::LineGeometry:
                    return  QIcon(QStringLiteral(":/images/themes/default/mIconLineLayer.svg"));
                    break;
                case QgsWkbTypes::PolygonGeometry:
                    return  QIcon(QStringLiteral(":/images/themes/default/mIconPolygonLayer.svg"));
                    break;
                default:
                    break;
                }
            }
        }
        case Qt::CheckStateRole:
            return mCheckState;
        default:
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags GwmLayerVectorItem::flags()
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | GwmLayerItem::flags();
}


void GwmLayerVectorItem::createSymbolChildren()
{
    mSymbolChildren.clear();
    QList<GwmLayerSymbolItem*> symbols;
    auto symbolItemList = mLayer->renderer()->legendSymbolItems();
    for (auto symbolItem : symbolItemList)
    {
        auto symbol = symbolItem.symbol();
        QSize iconSize(12, 12);
        QPixmap pixmap(iconSize);
        pixmap.fill(Qt::GlobalColor::transparent);
        QPainter painter(&pixmap);
        symbol->drawPreviewIcon(&painter, iconSize);
        auto label = symbolItem.label();
        auto item = new GwmLayerSymbolItem(this, pixmap, label);
        symbols.append(item);
    }
    mSymbolChildren = symbols;
}

void GwmLayerVectorItem::onLayerRendererChanged()
{
    this->createSymbolChildren();
    this->mSymbolType = GwmLayerVectorItem::renderTypeToSymbolType(mLayer->renderer()->type());
    emit itemSymbolChangedSignal();
}

bool GwmLayerVectorItem::insertChildren(int position, QList<GwmLayerItem *> items)
{
    return false;
}

bool GwmLayerVectorItem::appendChildren(QList<GwmLayerItem *> items)
{
    return false;
}

QList<GwmLayerItem*> GwmLayerVectorItem::takeChildren(int position, int count)
{
    return QList<GwmLayerItem*>();
}

QString GwmLayerVectorItem::provider() const
{
    return mProvider;
}

void GwmLayerVectorItem::setProvider(const QString &provider)
{
    mProvider = provider;
}

QString GwmLayerVectorItem::path() const
{
    return mPath;
}

void GwmLayerVectorItem::setPath(const QString &path)
{
    mPath = path;
}

void GwmLayerVectorItem::save(QString filePath, QString fileName, QString fileType, QgsVectorFileWriter::SaveVectorOptions& options)
{
    qDebug() << filePath;
    qDebug() << fileName;
    qDebug() << fileType;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
    mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
    options.actionOnExistingFile = mActionOnExistingFile;
//    options.layerName = fileType;
    if (fileType == "shp"){
        options.driverName = "ESRI Shapefile";
    }
    else{
        options.driverName = "ESRI Shapefile";
    }

    QgsVectorFileWriterTask *writerTask = new QgsVectorFileWriterTask( mLayer, filePath, options );
    // when writer is successful:

    QgsApplication::taskManager()->addTask( writerTask );
}


