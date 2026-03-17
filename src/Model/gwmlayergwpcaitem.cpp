#include "gwmlayergwpcaitem.h"
#include "gwmlayergroupitem.h"

#include <QDir>
#include <algorithm>

GwmLayerGWPCAItem::GwmLayerGWPCAItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGWPCATaskThread *taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        try
        {
            mDResult1 = taskThread->variance();
            mLocalPV = taskThread->localPV();
            mK = taskThread->k();
            
            gwm::SpatialWeight sw = taskThread->spatialWeight();
            gwm::BandwidthWeight* gwmBw = sw.weight<gwm::BandwidthWeight>();
            if (gwmBw)
            {
                if(gwmBw->bandwidth() == 0)
                {
                    qDebug() << "[GwmLayerGWPCAItem] WARNING: Bandwidth is 0! This will cause display issues in property panel.";
                }
                mWeight = gwm::BandwidthWeight(*gwmBw);
            }
            else
            {
                qDebug() << "[GwmLayerGWPCAItem] ERROR: Cannot get gwm::BandwidthWeight from spatialWeight!";
                mWeight = gwm::BandwidthWeight();
            }
            
            mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
            isBandwidthOptimized = taskThread->isAutoselectBandwidth();
            mLoadings = taskThread->loadings();
            mScores = taskThread->scores();
            mVariance = taskThread->variance();
        }
        catch (const std::exception& e)
        {
            qDebug() << "[GwmLayerGWPCAItem] EXCEPTION during data copy:" << e.what();
        }
        catch (...)
        {
            qDebug() << "[GwmLayerGWPCAItem] UNKNOWN EXCEPTION during data copy";
        }
    }
}

bool GwmLayerGWPCAItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        QDomElement analyse = node.toElement();
        mK = analyse.attribute("k").toInt();

        QDomElement weightNode = analyse.firstChildElement("weight");
        if (weightNode.hasAttribute("bandwidth") && weightNode.hasAttribute("kernel")
                && weightNode.hasAttribute("adaptive"))
        {
            double bandwidth = weightNode.attribute("bandwidth").toDouble();
            bool adaptive = weightNode.attribute("adaptive").toInt();
            auto it = std::find_if(
                gwm::BandwidthWeight::KernelFunctionTypeNameMapper.begin(),
                gwm::BandwidthWeight::KernelFunctionTypeNameMapper.end(),
                [&](const auto& kv){ return kv.second == weightNode.attribute("kernel").toStdString(); }
                );
            gwm::BandwidthWeight::KernelFunctionType kernel =
                it != gwm::BandwidthWeight::KernelFunctionTypeNameMapper.end() ? it->first : gwm::BandwidthWeight::Gaussian;
            mWeight = gwm::BandwidthWeight(bandwidth, adaptive, kernel);
        }
        else return false;

        if (isBandwidthOptimized)
        {
            QDomElement bandwidthCriterionsNode = node.firstChildElement("bandwidthCriterions");
            if (!bandwidthCriterionsNode.isNull())
            {
                QDomElement bandwidthNode = bandwidthCriterionsNode.firstChildElement("bandwidth");
                while (!bandwidthNode.isNull())
                {
                    if (bandwidthNode.hasAttribute("size") && bandwidthNode.hasAttribute("criterion"))
                    {
                        double size = bandwidthNode.attribute("size").toDouble();
                        double criterion = bandwidthNode.attribute("criterion").toDouble();
                        mBandwidthSelScores.push_back(std::make_pair(size, criterion));
                    }
                    bandwidthNode = bandwidthNode.nextSiblingElement("bandwidth");
                }
            }
            else
            {
                isBandwidthOptimized = false;
            }
        }

        if (analyse.hasAttribute("dResult") && analyse.hasAttribute("localPV")
                && analyse.hasAttribute("loadings")
                && analyse.hasAttribute("variance"))
        {
            QString fileDResult = analyse.attribute("dResult");
            QString fileLocalPV = analyse.attribute("localPV");
            QString fileLoadings = analyse.attribute("loadings");
            QString fileVariance = analyse.attribute("variance");

            if (!mDResult1.load(fileDResult.toStdString()))
                return false;

            if (!mLocalPV.load(fileLocalPV.toStdString()))
                return false;

            if (!mLoadings.load(fileLoadings.toStdString()))
                return false;

            if (!mVariance.load(fileVariance.toStdString()))
                return false;
        }
        else return false;

        if (analyse.hasAttribute("scores"))
        {
            QString fileScores = analyse.attribute("scores");
            mScores.load(fileScores.toStdString());
        }

        return true;
    }
    else return false;
}

bool GwmLayerGWPCAItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("k", mK);

        QDomElement nodeBandwidth = doc.createElement("weight");
        nodeBandwidth.setAttribute(
            "kernel",
            QString::fromStdString(gwm::BandwidthWeight::KernelFunctionTypeNameMapper.at(mWeight.kernel()))
            );
        nodeBandwidth.setAttribute("bandwidth", mWeight.bandwidth());
        nodeBandwidth.setAttribute("adaptive", mWeight.adaptive());
        nodeAnalyse.appendChild(nodeBandwidth);

        QFileInfo layerFileInfo(mPath);
        if (layerFileInfo.isFile())
        {
            bool flag = true;

            QFileInfo fileDResult(layerFileInfo.dir(), QString("%1_dResult.bin").arg(mLayer->name()));
            flag = flag && mDResult1.save(fileDResult.filePath().toStdString());

            QFileInfo fileLocalPV(layerFileInfo.dir(), QString("%1_localPV.bin").arg(mLayer->name()));
            flag = flag && mLocalPV.save(fileLocalPV.filePath().toStdString());

            QFileInfo fileLoadings(layerFileInfo.dir(), QString("%1_loadings.bin").arg(mLayer->name()));
            flag = flag && mLoadings.save(fileLoadings.filePath().toStdString());

            QFileInfo fileVariance(layerFileInfo.dir(), QString("%1_variance.bin").arg(mLayer->name()));
            flag = flag && mVariance.save(fileVariance.filePath().toStdString());

            if (flag)
            {
                nodeAnalyse.setAttribute("dResult", fileDResult.filePath());
                nodeAnalyse.setAttribute("localPV", fileLocalPV.filePath());
                nodeAnalyse.setAttribute("loadings", fileLoadings.filePath());
                nodeAnalyse.setAttribute("variance", fileVariance.filePath());
            }
            else return false;

            QFileInfo fileScores(layerFileInfo.path(), QString("%1_scores.bin").arg(mLayer->name()));
            bool savedScore = mScores.save(fileScores.filePath().toStdString());
            if (savedScore)
            {
                nodeAnalyse.setAttribute("scores", fileScores.filePath());
            }
            else return false;
        }
        else return false;

        if (isBandwidthOptimized)
        {
            QDomElement nodeBandwidthCriterion = doc.createElement("bandwidthCriterions");
            for (auto bandwidth : mBandwidthSelScores)
            {
                QDomElement nodeBandwidth = doc.createElement("bandwidth");
                nodeBandwidth.setAttribute("size", bandwidth.first);
                nodeBandwidth.setAttribute("criterion", bandwidth.second);
                nodeBandwidthCriterion.appendChild(nodeBandwidth);
            }
            nodeAnalyse.appendChild(nodeBandwidthCriterion);
        }

        return true;
    }
    else return false;
}

gwm::BandwidthWeight GwmLayerGWPCAItem::weight() const
{
    return mWeight;
}

gwm::BandwidthCriterionList GwmLayerGWPCAItem::bandwidthSelScores() const
{
    return mBandwidthSelScores;
}
