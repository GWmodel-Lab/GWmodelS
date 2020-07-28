#include "gwmproject.h"
#include <QDomDocument>

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
}

bool GwmProject::read(const QFileInfo &projectFile)
{
    QDomDocument doc;
    QFile file(projectFile.filePath());
    if (!file.open(QIODevice::ReadOnly))
        return false;
    if (!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElement = doc.documentElement();

    auto project = doc.firstChildElement("gwmproject");
    if (project.isNull())
        return false;

    auto crsNode = project.firstChildElement("crs");
    if (crsNode.isNull())
        return false;

    QgsCoordinateReferenceSystem crs;
    crs.readXml(crsNode.firstChild());
    QgsProject::instance()->setCrs(crs);

    auto groupListNode = project.firstChildElement("groupList");
    if (!groupListNode.isNull())
    {
        auto groupNode = groupListNode.firstChildElement("group");
        while (!groupNode.isNull()) {
            GwmLayerGroupItem* groupItem = new GwmLayerGroupItem();
            if (groupItem->readXml(groupNode))
            {
                mLayerItemModel->appentItem(groupItem);
            }
            else
            {
                delete groupItem;
            }
            groupNode = groupNode.nextSiblingElement("group");
        }
    }
    else return false;
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
}

GwmLayerItemModel *GwmProject::layerItemModel() const
{
    return mLayerItemModel;
}

void GwmProject::setLayerItemModel(GwmLayerItemModel *layerItemModel)
{
    mLayerItemModel = layerItemModel;
}

QFileInfo GwmProject::projectFile() const
{
    return mProjectFile;
}

void GwmProject::setProjectFile(const QFileInfo &projectFile)
{
    mProjectFile = projectFile;
}
