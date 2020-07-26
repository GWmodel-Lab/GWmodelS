#ifndef GWMPROJECT_H
#define GWMPROJECT_H

#include <qgsproject.h>

#include "Model/gwmlayeritemmodel.h"

class GwmProject : public QObject
{
    Q_OBJECT

public:
    static GwmProject* instance();

private:
    static GwmProject* sProject;

public:
    GwmProject(QObject* parent = nullptr);

    QFileInfo projectFile() const;
    void setProjectFile(const QFileInfo &projectFile);

    GwmLayerItemModel *layerItemModel() const;
    void setLayerItemModel(GwmLayerItemModel *layerItemModel);

public:
    void read(const QFileInfo& projectFile);
    void save(const QFileInfo& projectFile);

private:
    GwmLayerItemModel* mLayerItemModel = nullptr;

    QFileInfo mProjectFile;
};

#endif // GWMPROJECT_H
