#include "gwmproject.h"
#include <QDomDocument>

#include <QMessageBox>
#include "gwmapp.h"

GwmProject* GwmProject::sProject = nullptr;

GwmProject *GwmProject::instance()
{
    if (!sProject)
    {
        sProject = new GwmProject();
    }
    return sProject;
}

GwmProject::GwmProject(QObject *parent) : QObject(parent)
{
    mLayerItemModel = new GwmLayerItemModel(parent);
    connect(mLayerItemModel, &GwmLayerItemModel::layerAddedSignal, this, &GwmProject::onLayerModelChanged);
    connect(mLayerItemModel, &GwmLayerItemModel::layerRemovedSignal, this, &GwmProject::onLayerModelChanged);
    connect(mLayerItemModel, &GwmLayerItemModel::layerItemChangedSignal, this, &GwmProject::onLayerModelChanged);
    connect(mLayerItemModel, &GwmLayerItemModel::layerItemMovedSignal, this, &GwmProject::onLayerModelChanged);
}

bool GwmProject::read(const QFileInfo &projectFile)
{
    if (!mLayerItemModel->clear())
    {
        QMessageBox::warning(GwmApp::Instance(), tr("Close Error!"), tr("Cannot close old project."));
        return false;
    }

    QDomDocument doc;
    QFile file(projectFile.filePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(GwmApp::Instance(), tr("Open File Error!"), tr("Cannot open project file."));
        return false;
    }
    if (!doc.setContent(&file))
    {
        QMessageBox::warning(GwmApp::Instance(), tr("Read File Error!"), tr("Cannot read project file."));
        file.close();
        return false;
    }
    file.close();

    QDomElement docElement = doc.documentElement();

    auto nodeProject = doc.firstChildElement("gwmproject");
    if (nodeProject.isNull())
    {
        QMessageBox::warning(GwmApp::Instance(), tr("Read File Error!"), tr("Cannot find any project definition."));
        return false;
    }
    setName(nodeProject.attribute("name"));

    auto nodeCrs = nodeProject.firstChildElement("crs");
    if (nodeCrs.isNull())
    {
        QMessageBox::warning(GwmApp::Instance(), tr("Read File Error!"), tr("No crs defined."));
        return false;
    }

    QgsCoordinateReferenceSystem crs;
    crs.readXml(nodeCrs.firstChild());
    if (crs.isValid())
    {
        QgsProject::instance()->setCrs(crs);
    }
    else
    {
        QMessageBox::information(GwmApp::Instance(), tr("Crs is Invalid!"), tr("Use WGS84 as default."));
        QgsProject::instance()->setCrs(QgsCoordinateReferenceSystem::fromEpsgId(4326));
    }

    auto nodeGroupList = nodeProject.firstChildElement("groupList");
    if (!nodeGroupList.isNull())
    {
        auto nodeGroup = nodeGroupList.firstChildElement("group");
        while (!nodeGroup.isNull()) {
            GwmLayerGroupItem* groupItem = new GwmLayerGroupItem();
            if (groupItem->readXml(nodeGroup))
            {
                mLayerItemModel->appentItem(groupItem);
            }
            else
            {
                setDirty(false);
                delete groupItem;
            }
            nodeGroup = nodeGroup.nextSiblingElement("group");
        }
        setDirty(false);
    }
    else
    {
        setDirty(false);
        QMessageBox::information(GwmApp::Instance(), tr("Read File Warning"), tr("No layer item in this project file."));
    }

    return true;
}

void GwmProject::save(const QFileInfo &projectFile)
{
    // 保存临时图层
    for (auto groupItem : mLayerItemModel->rootChildren())
    {
        for (auto analyseItem : groupItem->analyseChildren())
        {
            QFileInfo analyseFile(projectFile.path(), QString("%1.shp").arg(analyseItem->text()));
            if (analyseItem->save(analyseFile.filePath(), analyseFile.fileName(), QStringLiteral("shp")))
            {
                analyseItem->setDataSource(analyseFile.filePath(), QStringLiteral("ogr"));
            }
        }
    }

    // 保存工程文件
    QDomDocument doc;
    QDomProcessingInstruction xmlInstruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
    doc.appendChild(xmlInstruction);

    QDomElement project = doc.createElement("gwmproject");
    project.setAttribute("name", mName);
    doc.appendChild(project);

    QDomElement project_crs = doc.createElement("crs");
    QgsProject::instance()->crs().writeXml(project_crs, doc);
    project.appendChild(project_crs);

    QDomElement project_groupList = doc.createElement("groupList");
    project.appendChild(project_groupList);

    auto groupList = mLayerItemModel->rootChildren();
    for (auto item : groupList)
    {
        QDomElement group = doc.createElement("group");
        group.setAttribute("name", item->text());
        project_groupList.appendChild(group);
        item->writeXml(group, doc);
    }

    QFile file(projectFile.filePath());
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        doc.save(stream, 2);
    }
    file.close();

    setDirty(false);
}

void GwmProject::onLayerModelChanged()
{
    setDirty(true);
}

void GwmProject::setName(const QString &name)
{
    QString oldName = name;
    mName = name;
    emit nameChanged(name, oldName);
}

void GwmProject::setDirty(bool dirty)
{
    mDirty = dirty;
    emit dirtyChanged(mDirty);
}
