#ifndef GWMPROJECT_H
#define GWMPROJECT_H

#include <qgsproject.h>
#include <QString>
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

signals:
    void nameChanged(QString name, QString oldName);
    void dirtyChanged(bool isDirty);

public:
    bool read(const QFileInfo& projectFile);
    void save(const QFileInfo& projectFile);

public:
    void onLayerModelChanged();

public:
    QFileInfo projectFile() const;
    void setProjectFile(const QFileInfo &projectFile);

    GwmLayerItemModel *layerItemModel() const;
    void setLayerItemModel(GwmLayerItemModel *layerItemModel);

    QString name() const;
    void setName(const QString &name);

    bool dirty() const;
    void setDirty(bool dirty);

    QString filePath() const;
    void setFilePath(const QString &filepath);
private:
    GwmLayerItemModel* mLayerItemModel = nullptr;

    QFileInfo mProjectFile;

    QString mName = "Untitled Project";
    bool mDirty = false;
    QString mFilePath ;
};

inline QString GwmProject::name() const
{
    return mName;
}

inline QString GwmProject::filePath() const
{
    return mFilePath;
}

inline bool GwmProject::dirty() const
{
    return mDirty;
}

inline GwmLayerItemModel *GwmProject::layerItemModel() const
{
    return mLayerItemModel;
}

inline void GwmProject::setLayerItemModel(GwmLayerItemModel *layerItemModel)
{
    mLayerItemModel = layerItemModel;
}

inline QFileInfo GwmProject::projectFile() const
{
    return mProjectFile;
}

inline void GwmProject::setProjectFile(const QFileInfo &projectFile)
{
    mProjectFile = projectFile;
}

#endif // GWMPROJECT_H
