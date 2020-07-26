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

void GwmProject::read(const QFileInfo &projectFile)
{

}

void GwmProject::save(const QFileInfo &projectFile)
{
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
