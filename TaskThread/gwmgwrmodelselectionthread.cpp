#include "gwmgwrmodelselectionthread.h"
#include "GWmodel/GWmodel.h"
#include <QDebug>
#include <exception>
#include <prefix.h>
#include <qwt_plot_marker.h>


GwmGWRModelSelectionThread::GwmGWRModelSelectionThread()
{
    hasHatMatrix = false;
}

GwmGWRModelSelectionThread::GwmGWRModelSelectionThread(const GwmGWRTaskThread &gwrTaskThread)
    : GwmGWRTaskThread(gwrTaskThread)
    , createdFromGWRTaskThread(true)
{
    hasHatMatrix = false;
}

void GwmGWRModelSelectionThread::run()
{
//    emit message("Setting matrices.");
    QgsFeatureIterator it = mLayer->getFeatures();
    QgsFeature f;
    while (it.nextFeature(f))
    {
        mFeatureList.append(f);
    }
    it.rewind();
    int process = 0;
    int total = (mIndepVars.size()+1)*mIndepVars.size()/2;
    mDataPoints = mat(mFeatureList.size(), 2);
    for (int row = 0; row < mFeatureList.size(); row++)
    {
        QgsFeature feature = mFeatureList[row];
        // 设置坐标
        QgsPointXY centroPoint = feature.geometry().centroid().asPoint();
        mDataPoints.at(row, 0) = centroPoint.x();
        mDataPoints.at(row, 1) = centroPoint.y();
    }
    qDebug() << mFeatureList.size();
    QList<int> inDepVarsIndex = QList<int>();
    for (int i = 0; i < mIndepVars.size(); i++)
    {
        vec AICcs = vec(mIndepVars.size() - i);
        for (int j = 0; j < mIndepVars.size() - i; j++)
        {
            inDepVarsIndex.append(mIndepVarsIndex[j]);
            QList<mat> XY = setXY(mDepVarIndex,inDepVarsIndex);
            if (XY.size() == 0)
            {
                return;
            }
            mX = XY[0];
            mY = XY[1];
            AICcs[j] = gwRegAll();
            QStringList inDepVarsName;
            QList<int> inDepVarsIndexList;
            QString message1 = "";
            for(int inDepVarIndex : inDepVarsIndex){
               QString name = mLayer->fields().field(inDepVarIndex).name();
               inDepVarsName.append(name);
               inDepVarsIndexList.append(inDepVarIndex);
               message1 = message1 +  " " + name;
            }
            mModelInDepVars.append(inDepVarsName);
            mModelInDepVarsIndex.append(inDepVarsIndexList);
            mModelAICcs.append(AICcs[j]);
            message1 = message1 + ":" + (QString::number(AICcs[j],10,5));
            emit message(message1);
            inDepVarsIndex.removeOne(mIndepVarsIndex[j]);
            emit tick(process,total);
            emit message(QString("Model: %1 (AIC: %2").arg(inDepVarsName.join(",")).arg(AICcs[j], 0, 'f', 3));
            process++;
        }
        int index = AICcs.index_min();
        inDepVarsIndex.append(mIndepVarsIndex[index]);
        mIndepVarsIndex.removeOne(mIndepVarsIndex[index]);
    }
    mModelInDepVars = modelSort(mModelInDepVars,mModelAICcs,mModelInDepVarsIndex);
    if (!createdFromGWRTaskThread)
        emit success();
}

QList<mat> GwmGWRModelSelectionThread::setXY(int depVarIndex, QList<int> inDepVarsIndex)
{
    mat X = mat(mFeatureList.size(), inDepVarsIndex.size() + 1);
    mat Y = vec(mFeatureList.size());
    QgsField depField = mLayer->fields()[depVarIndex];
    if (!isNumeric(depField.type()))
    {
        emit error(tr("Dependent variable is not numeric."));
        return QList<mat> ();
    }
    for (int iIndepVar : inDepVarsIndex)
    {
        QgsField indepField = mLayer->fields()[iIndepVar];
        if (!isNumeric(indepField.type()))
        {
            emit error(tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
            return QList<mat> ();
        }
    }
    bool ok = false;
    int row = 0, total = mFeatureList.size();
    for (row = 0; row < total; row++)
    {
        QgsFeature feature = mFeatureList[row];
        double vY = feature.attribute(depVarIndex).toDouble(&ok);
        if (ok)
        {
            Y.at(row, 0) = vY;

            // 设置 X 矩阵
            for (int i = 0; i < inDepVarsIndex.size(); i++)
            {
                int index = inDepVarsIndex[i];
                double vX = feature.attribute(index).toDouble(&ok);
                if (ok)
                {
                    X.at(row, i + 1) = vX;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
        }
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    return QList<mat>() << X << Y;
}


bool GwmGWRModelSelectionThread::calDmat(){
    mDmat = mat(mFeatureList.size(),mFeatureList.size());
    for (int i = 0; i < mFeatureList.size(); i++){
        mat dist = distance(i);
        mDmat.col(i) = dist;
    }
    return true;
}

double GwmGWRModelSelectionThread::gwRegAll()
{
    mat betas = mat(mX.n_cols, mFeatureList.size());
    vec s_hat(2, fill::zeros);
//    calDmat();
    if (mBandwidthType == BandwidthType::Adaptive)
    {
        mBandwidthSize = mFeatureList.size();
    }
    else
    {
        mBandwidthSize = getFixedBwUpper();
    }
    mat ci, si;
    for (int i = 0; i < mFeatureList.size(); i++)
    {
        vec dist = distance(i);
        vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        vec beta = gwRegHatmatrix(mX, mY, weight, i, ci, si);
        betas.col(i) = beta;
        s_hat(0) += si(0, i);
        s_hat(1) += det(si * trans(si));
    }
    return AICc(mY,mX,trans(betas),s_hat);
}


QList<QStringList> GwmGWRModelSelectionThread::modelSort(QList<QStringList> modelList, QList<double> modelAICcs,QList<QList<int>> modelIndexList)
{
    int tag = 0;
    QList<int> sortIndex;
    for(int i = mIndepVars.size(); i > 0; i--){
        vec tmpList = vec(i);
        for(int j = tag; j < tag + i; j++){
            tmpList.row(j - tag) = modelAICcs[j];
        }
        for(int j = 0; j < i; j++){
            int index = tmpList.index_max();
            sortIndex.append(index + tag);
            tmpList.row(index) = 0;
        }
        tag = tag + i;
    }
    QList<QStringList> res;
    mModelAICcs.clear();
    mModelInDepVarsIndex.clear();
    for(int i : sortIndex){
        res.append(modelList[i]);
        mModelInDepVarsIndex.append(modelIndexList[i]);
        mModelAICcs.append(modelAICcs[i]);
//        for(QString inDepVar:modelList[i]){
//            emit message(inDepVar);
//        }
//        emit message(QString::number(modelAICcs[i],10,5));
    }
    return res;
}

QPair<QList<int>,double> GwmGWRModelSelectionThread::modelSelection(){
    for (int i = mModelInDepVars.size() - 1; i >= 0; i--)
    {
        if (mModelAICcs[i-1] - mModelAICcs[i] >= 3)
        {
            QPair<QList<int>,double> res = qMakePair(mModelInDepVarsIndex[i],mModelAICcs[i]);
            QString message1 = "";
            for(QString inDepVar:mModelInDepVars[i]){
                message1 = message1 + " " + inDepVar;
            }
            message1 = message1 + ":" + QString::number(mModelAICcs[i],10,5);
            emit message(message1);
            return res;
        }
    }
    return QPair<QStringList,double>();
}

void GwmGWRModelSelectionThread::viewModels()
{
    //根据mIndepVars mModelAICcs mModelInDepVars绘图
    int n = mIndepVars.size();
    double cex;
    if( n > 10){
        cex = 10/n;
    }
    else{
        cex = 1;
    }
    int numModels = mModelInDepVars.size();
    double alpha = 2 * M_PI / numModels;
    QStringList IndepVarNameList;
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        QString IndepVarName = item->attributeName();
        IndepVarNameList.append(IndepVarName);
    }

    QwtPlotCanvas *canvas=new QwtPlotCanvas();
    canvas->setPalette(Qt::white);
    canvas->setBorderRadius(10);
    QwtPlot* plot = new QwtPlot();
    plot->setCanvas( canvas );
    plot->plotLayout()->setAlignCanvasToScales( true );
    QPolygonF points1;
    int lastnVars = 1;
    for(int i = 0; i < numModels; i++){
        QStringList vars = mModelInDepVars[i];
        int nVars = vars.size();
        points1 << QPointF(i+1,mModelAICcs[i]);
        if(nVars != lastnVars){
            QwtPlotMarker *mX = new QwtPlotMarker();
            mX->setXValue(i);
            mX->setLineStyle(QwtPlotMarker::VLine);
            mX->setLinePen(QPen(Qt::lightGray, 1, Qt::DashDotDotLine));
            mX->attach(plot);
        }
        lastnVars = nVars;
    }
    QwtPlotCurve *curve=new QwtPlotCurve("curve");
    curve->setPen(Qt::blue,cex,Qt::DashLine);//设置曲线颜色 粗细
    curve->setRenderHint(QwtPlotItem::RenderAntialiased,true);//线条光滑化
    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
    QBrush( Qt::yellow ), QPen( Qt::red, 0.5 ), QSize( 5, 5) );//设置样本点的颜色、大小
    curve->setSymbol( symbol );//添加样本点形状
    curve->setSamples(points1);
    curve->attach( plot );
    curve->setLegendAttribute(curve->LegendShowLine);
    plot->resize(600,400);
    plot->replot();
    plot->show();

    QwtPlot* plot2 = new QwtPlot();
    QwtPlotCanvas *canvas1=new QwtPlotCanvas();
    canvas1->setPalette(Qt::white);
    canvas1->setBorderRadius(10);
    plot2->setCanvas( canvas1 );
    plot2->setAxisScale(QwtPlot::yLeft,-n-1, n+1);
    plot2->setAxisScale(QwtPlot::xBottom,-3*n/4,n+6);
    QList<Qt::GlobalColor> colors;
    colors << Qt::red << Qt::cyan << Qt::yellow << Qt::green << Qt::blue << Qt::black << Qt::lightGray << Qt::white;
    QList<QwtSymbol::Style> pointStyles;
    pointStyles << QwtSymbol::Rect << QwtSymbol::Ellipse << QwtSymbol::Diamond << QwtSymbol::Triangle << QwtSymbol::DTriangle;
    int i = 1;
    QList<Qt::GlobalColor> legendColors;
    QList<QwtSymbol::Style> legendStyles;
    for(QString var: IndepVarNameList){
        legendColors.append(colors[IndepVarNameList.indexOf(var)%8]);
        legendStyles.append(pointStyles[IndepVarNameList.indexOf(var)%5]);
    }
    for(QStringList vars:mModelInDepVars){
        QwtPlotCurve *curve2=new QwtPlotCurve("curve2");
        curve2->setPen(Qt::gray,cex,Qt::DashLine);//设置曲线颜色 粗细
        QPolygonF points2;
        int j = 1;
        for(QString var:vars){
            double radius = sqrt(n) * sqrt(j);
            points2 << QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha));
            QwtPlotMarker *mX = new QwtPlotMarker(var);
            mX->setValue(QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha)));
            int k = IndepVarNameList.indexOf(var)%5;
            mX->setSymbol(new QwtSymbol( pointStyles[IndepVarNameList.indexOf(var)%5],
                                         QBrush( colors[IndepVarNameList.indexOf(var)%8] ), QPen( Qt::red, 0.5 ), QSize( 5, 5 )));
            mX->attach(plot2);
//            mX->setLegendIconSize(QSize( 6, 6));
            j++;
        }
        curve2->setSamples(points2);
        curve2->attach(plot2);
        i++;
    }

    QwtLegend *legend = new QwtLegend();
    QwtPlotItemList items = plot2->itemList( QwtPlotItem::Rtti_PlotMarker );
    for ( int i = 0; i < mIndepVars.size(); i++ ){
        items[i]->setItemAttribute(QwtPlotItem::Legend,true);
    }
    items = plot2->itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < items.size(); i++ ){
        const QVariant itemInfo = plot2->itemToInfo( items[i] );
        items[i]->setItemAttribute(QwtPlotItem::Legend,false);
    }
    plot2->insertLegend( legend, QwtPlot::RightLegend );
    plot2->plotLayout()->setAlignCanvasToScales( true );
    plot2->resize(600,400);
    plot2->replot();
    plot2->show();
//    plot->setAutoReplot( true );//设置自动重画，相当于更新
}

void GwmGWRModelSelectionThread::viewModels(QList<GwmLayerAttributeItem *> indepVars, QList<QStringList> modelInDepVars, QList<double> modelAICcs, QwtPlotCanvas *canvas1, QwtPlotCanvas *canvas2){
    int n = indepVars.size();
    double cex;
    if( n > 10){
        cex = 10/n;
    }
    else{
        cex = 1;
    }
    int numModels = modelInDepVars.size();
    double alpha = 2 * M_PI / numModels;
    QStringList IndepVarNameList;
    for (GwmLayerAttributeItem* item : indepVars)
    {
        QString IndepVarName = item->attributeName();
        IndepVarNameList.append(IndepVarName);
    }

    canvas1->setPalette(Qt::white);
    canvas1->setBorderRadius(10);
    QwtPlot* plot = new QwtPlot();
    plot->setCanvas( canvas1 );
    plot->plotLayout()->setAlignCanvasToScales( true );
    QPolygonF points1;
    int lastnVars = 1;
    for(int i = 0; i < numModels; i++){
        QStringList vars = modelInDepVars[i];
        int nVars = vars.size();
        points1 << QPointF(i+1,modelAICcs[i]);
        if(nVars != lastnVars){
            QwtPlotMarker *mX = new QwtPlotMarker();
            mX->setXValue(i);
            mX->setLineStyle(QwtPlotMarker::VLine);
            mX->setLinePen(QPen(Qt::lightGray, 1, Qt::DashDotDotLine));
            mX->attach(plot);
        }
        lastnVars = nVars;
    }
    QwtPlotCurve *curve=new QwtPlotCurve("curve");
    curve->setPen(Qt::blue,cex,Qt::DashLine);//设置曲线颜色 粗细
    curve->setRenderHint(QwtPlotItem::RenderAntialiased,true);//线条光滑化
    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
    QBrush( Qt::yellow ), QPen( Qt::red, 0.5 ), QSize( 5, 5) );//设置样本点的颜色、大小
    curve->setSymbol( symbol );//添加样本点形状
    curve->setSamples(points1);
    curve->attach( plot );
    curve->setLegendAttribute(curve->LegendShowLine);
    plot->resize(600,400);
    plot->replot();
    plot->show();

    QwtPlot* plot2 = new QwtPlot();
    canvas2->setPalette(Qt::white);
    canvas2->setBorderRadius(10);
    plot2->setCanvas( canvas2 );
    plot2->setAxisScale(QwtPlot::yLeft,-n-1, n+1);
    plot2->setAxisScale(QwtPlot::xBottom,-3*n/4,n+6);
    QList<Qt::GlobalColor> colors;
    colors << Qt::red << Qt::cyan << Qt::yellow << Qt::green << Qt::blue << Qt::black << Qt::lightGray << Qt::white;
    QList<QwtSymbol::Style> pointStyles;
    pointStyles << QwtSymbol::Rect << QwtSymbol::Ellipse << QwtSymbol::Diamond << QwtSymbol::Triangle << QwtSymbol::DTriangle;
    int i = 1;
    QList<Qt::GlobalColor> legendColors;
    QList<QwtSymbol::Style> legendStyles;
    for(QString var: IndepVarNameList){
        legendColors.append(colors[IndepVarNameList.indexOf(var)%8]);
        legendStyles.append(pointStyles[IndepVarNameList.indexOf(var)%5]);
    }
    for(QStringList vars:modelInDepVars){
        QwtPlotCurve *curve2=new QwtPlotCurve("curve2");
        curve2->setPen(Qt::gray,cex,Qt::DashLine);//设置曲线颜色 粗细
        QPolygonF points2;
        int j = 1;
        for(QString var:vars){
            double radius = sqrt(n) * sqrt(j);
            points2 << QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha));
            QwtPlotMarker *mX = new QwtPlotMarker(var);
            mX->setValue(QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha)));
            int k = IndepVarNameList.indexOf(var)%5;
            mX->setSymbol(new QwtSymbol( pointStyles[IndepVarNameList.indexOf(var)%5],
                                         QBrush( colors[IndepVarNameList.indexOf(var)%8] ), QPen( Qt::red, 0.5 ), QSize( 5, 5 )));
            mX->attach(plot2);
//            mX->setLegendIconSize(QSize( 6, 6));
            j++;
        }
        curve2->setSamples(points2);
        curve2->attach(plot2);
        i++;
    }

    QwtLegend *legend = new QwtLegend();
    QwtPlotItemList items = plot2->itemList( QwtPlotItem::Rtti_PlotMarker );
    for ( int i = 0; i < indepVars.size(); i++ ){
        items[i]->setItemAttribute(QwtPlotItem::Legend,true);
    }
    items = plot2->itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < items.size(); i++ ){
        const QVariant itemInfo = plot2->itemToInfo( items[i] );
        items[i]->setItemAttribute(QwtPlotItem::Legend,false);
    }
    plot2->insertLegend( legend, QwtPlot::RightLegend );
    plot2->plotLayout()->setAlignCanvasToScales( true );
    plot2->resize(600,400);
    plot2->replot();
    plot2->show();
}

double GwmGWRModelSelectionThread::getFixedBwUpper()
{
    QgsRectangle extent = this->mLayer->extent();
    bool longlat = mLayer->crs().isGeographic();
    mat extentDp(2, 2, fill::zeros);
    extentDp(0, 0) = extent.xMinimum();
    extentDp(0, 1) = extent.yMinimum();
    extentDp(1, 0) = extent.xMaximum();
    extentDp(1, 1) = extent.yMaximum();
    vec dist = gwDist(extentDp, extentDp, 0, 2.0, 0.0, longlat, false);
    return dist(1);
}

QList<QStringList> GwmGWRModelSelectionThread::getModelInDepVars(){
    return mModelInDepVars;
}

QList<double> GwmGWRModelSelectionThread::getModelAICcs()
{
    return mModelAICcs;
}

QList<QList<int>> GwmGWRModelSelectionThread::getModelInDepVarsIndex()
{
    return mModelInDepVarsIndex;
}
