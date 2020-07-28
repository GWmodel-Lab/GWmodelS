#include "gwmlayervectoritem.h"
#include "gwmlayersymbolitem.h"
#include <qgsrenderer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgscategorizedsymbolrenderer.h>
#include <qgsvectorfilewritertask.h>
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgsmaplayerlegend.h>
#include "TaskThread/gwmsavelayerthread.h"
#include "gwmprogressdialog.h"
#include "Model/gwmlayergroupitem.h"

GwmEnumValueNameMapper<GwmLayerVectorItem::SymbolType> GwmLayerVectorItem::SymbolTypeNameMapper = {
    std::make_pair(GwmLayerVectorItem::SymbolType::singleSymbol, "singleSymbol"),
    std::make_pair(GwmLayerVectorItem::SymbolType::categorizedSymbol, "categorizedSymbol"),
    std::make_pair(GwmLayerVectorItem::SymbolType::graduatedSymbol, "graduatedSymbol"),
    std::make_pair(GwmLayerVectorItem::SymbolType::RuleRenderer, "RuleRenderer"),
    std::make_pair(GwmLayerVectorItem::SymbolType::heatmapRenderer, "heatmapRenderer"),
    std::make_pair(GwmLayerVectorItem::SymbolType::invertedPolygonRenderer, "invertedPolygonRenderer"),
    std::make_pair(GwmLayerVectorItem::SymbolType::pointCluster, "pointCluster"),
    std::make_pair(GwmLayerVectorItem::SymbolType::pointDisplacement, "pointDisplacement"),
    std::make_pair(GwmLayerVectorItem::SymbolType::nullSymbol, "nullSymbol")
};

GwmEnumValueNameMapper<QVariant::Type> GwmLayerVectorItem::FieldTypeNameMapper = {
    std::make_pair(QVariant::Type::Int, "Int"),
    std::make_pair(QVariant::Type::Double, "Double")
};

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
    if (row >= 0 && row < childCount())
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
    GwmLayerGroupItem* parentItem = dynamic_cast<GwmLayerGroupItem*>(mParentItem);
    if (parentItem)
    {
        return (parentItem->originChild() == this) ? 0 : (parentItem->analyseChildren().indexOf(this) + 1);
    }
    else return -1;
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
        case Qt::EditRole:
            return col == 0 ? text() : QString();
        default:
            break;
        }
    }
    return QVariant();
}

bool GwmLayerVectorItem::setData(int col, int role, QVariant value)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::CheckStateRole:
            mCheckState = Qt::CheckState(value.toInt());
            if (mCheckState == Qt::CheckState::Checked)
            {
                QgsProject::instance()->addMapLayer(mLayer);
            }
            else
            {
                QgsProject::instance()->takeMapLayer(mLayer);
            }
            break;
        case Qt::EditRole:
            if (mLayer) mLayer->setName(value.toString());
            break;
        default:
            return false;
        }
    }
    return true;
}

Qt::ItemFlags GwmLayerVectorItem::flags()
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable | GwmLayerItem::flags();
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
#ifndef _DEBUG
    QgsDefaultVectorLayerLegend* legend = static_cast<QgsDefaultVectorLayerLegend*>(QgsMapLayerLegend::defaultVectorLegend(mLayer));
    mLayer->setLegend(legend);
#endif
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

bool GwmLayerVectorItem::moveChildren(int position, int count, int destination)
{
    QList<GwmLayerItem*> removedChildren = takeChildren(position, count);
    if (removedChildren.size() > 0)
        return insertChildren(destination, removedChildren);
    else return false;
}

bool GwmLayerVectorItem::readXml(QDomNode &node)
{
    QDomElement vector = node.toElement();
    if (vector.hasAttribute("provider") && vector.hasAttribute("path"))
    {
        mProvider = vector.attribute("provider");
        mPath = vector.attribute("path");
        QString layerName;
        if (vector.hasAttribute("name"))
            layerName = vector.attribute("name");
        else
        {
            QFileInfo fileInfo(vector.attribute("path"));
            layerName = fileInfo.baseName();
        }
        mLayer = new QgsVectorLayer(vector.attribute("path"), layerName, vector.attribute("provider"));
        if (mLayer->isValid())
        {
            connect(mLayer, &QgsVectorLayer::rendererChanged, this, &GwmLayerVectorItem::onLayerRendererChanged);
            // 符号
            mSymbolType = GwmLayerVectorItem::renderTypeToSymbolType(mLayer->renderer()->type());
            createSymbolChildren();
        }
        else
        {
            delete mLayer;
            return false;
        }

        return true;
    }
    else return false;
}

bool GwmLayerVectorItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    QDomElement vector = node.toElement();
    vector.setAttribute("provider", mProvider);
    vector.setAttribute("path", mPath);
    vector.setAttribute("type", LayerItemTypeNameMapper.name(itemType()));

//    QDomElement vector_symbol = doc.createElement("symbol");
//    vector_symbol.setAttribute("type", SymbolTypeNameMapper.name(mSymbolType));

    return true;
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

bool GwmLayerVectorItem::save(QString filePath, QString fileName, QString fileType, QgsVectorFileWriter::SaveVectorOptions& options)
{
//    qDebug() << filePath;
//    qDebug() << fileName;
//    qDebug() << fileType;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile;
    mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
    options.actionOnExistingFile = mActionOnExistingFile;
    //    options.layerName = fileType;
    if (fileType == "shp"){
        options.driverName = "ESRI Shapefile";
    }
    else if(fileType == "csv"){
        qDebug() << fileType;
        const QList< QgsVectorFileWriter::DriverDetails > drivers = QgsVectorFileWriter::ogrDriverList();
        for ( const QgsVectorFileWriter::DriverDetails &driver : drivers )
        {
            //          mFormatComboBox->addItem( driver.longName, driver.driverName );
            if(driver.longName == "Comma Separated Value [CSV]"){
                qDebug() << driver.driverName;
                options.driverName = driver.driverName;
            }
        }
    }
    else{
        options.driverName = "ESRI Shapefile";
    }
    GwmSaveLayerThread* thread = new GwmSaveLayerThread(mLayer,filePath,options);
    GwmProgressDialog* progressDialog = new GwmProgressDialog(thread);
    if (progressDialog->exec() == QDialog::Accepted)
    {
        return true;
    }
    else return false;
    //    QgsVectorFileWriterTask *writerTask = new QgsVectorFileWriterTask( mLayer, filePath, options );
    //    // when writer is successful:

    //    QgsApplication::taskManager()->addTask( writerTask );
}

void GwmLayerVectorItem::setDataSource(const QString &path, const QString &provider)
{
    mProvider = provider;
    mPath = path;
    mLayer->setDataSource(path, mLayer->name(), provider, QgsDataProvider::ProviderOptions());
    mLayer->reload();
}


