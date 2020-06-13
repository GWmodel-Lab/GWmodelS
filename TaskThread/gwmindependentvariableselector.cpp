#include "gwmindependentvariableselector.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qpen.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_marker.h>

using namespace arma;

void GwmIndependentVariableSelector::PlotModelOrder(QVariant data, QwtPlot *plot)
{
    IndepVarsCriterionList modelInDepVars = data.value<IndepVarsCriterionList>();
    QSet<QString> indepVarNameSet;
    for (auto item : modelInDepVars)
    {
        for (auto variable : item.first)
        {
            indepVarNameSet.insert(variable.name);
        }
    }
    QList<QString> indepVarNameList = indepVarNameSet.toList();
    int n = modelInDepVars.size();
    double cex = (n > 10) ? 10.0/n : 1.0;
    int numModels = modelInDepVars.size();
    double alpha = 2 * M_PI / numModels;

    plot->setAxisScale(QwtPlot::yLeft,-n-1, n+1);
    plot->setAxisScale(QwtPlot::xBottom,-n,n+6);
    QList<Qt::GlobalColor> colors;
    colors << Qt::red << Qt::cyan << Qt::yellow << Qt::green << Qt::blue << Qt::black << Qt::lightGray << Qt::white;
    QList<QwtSymbol::Style> pointStyles;
    pointStyles << QwtSymbol::Rect << QwtSymbol::Ellipse << QwtSymbol::Diamond << QwtSymbol::Triangle << QwtSymbol::DTriangle;
    int i = 1;
    QList<Qt::GlobalColor> legendColors;
    QList<QwtSymbol::Style> legendStyles;
    for (QString var: indepVarNameList)
    {
        legendColors.append(colors[indepVarNameList.indexOf(var)%8]);
        legendStyles.append(pointStyles[indepVarNameList.indexOf(var)%5]);
    }
    for (auto vars : modelInDepVars)
    {
        QwtPlotCurve *curve2=new QwtPlotCurve("curve2");
        curve2->setPen(Qt::gray,cex,Qt::DashLine);//设置曲线颜色 粗细
        QPolygonF points2;
        int j = 1;
        double radius;
        for(GwmVariable var : vars.first){
            radius = sqrt(n) * sqrt(j);
            points2 << QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha));
            QwtPlotMarker *mX = new QwtPlotMarker(var.name);
            mX->setValue(QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha)));
            int k = indepVarNameList.indexOf(var.name)%5;
            mX->setSymbol(new QwtSymbol( pointStyles[indepVarNameList.indexOf(var.name)%5],
                                         QBrush( colors[indepVarNameList.indexOf(var.name)%8] ), QPen( Qt::red, 0.5 ), QSize( 5, 5 )));
            mX->attach(plot);
//            mX->setLegendIconSize(QSize( 6, 6));
            j++;
        }
        curve2->setSamples(points2);
        curve2->attach(plot);
        i++;
    }
    for (i = 1; i <= modelInDepVars.size(); i++)
    {
        int j = modelInDepVars[i-1].first.size();
        double radius;
        radius = sqrt(n) * sqrt(j+1.5);
        QwtPlotMarker *mX = new QwtPlotMarker();
        mX->setValue(QPointF(radius*cos((i-1)*alpha),radius*sin((i-1)*alpha)));
        QwtText* text = new QwtText(QString::number(i,10));
        text->setFont(QFont("MS Shell Dlg 2",int(10-numModels/15+i*3/numModels)));
        mX->setLabel(*text);
        mX->attach(plot);
    }
    QwtLegend *legend = new QwtLegend();
    QwtPlotItemList items = plot->itemList( QwtPlotItem::Rtti_PlotMarker );
    for ( int i = 0; i < indepVarNameList.size(); i++ )
    {
        items[i]->setItemAttribute(QwtPlotItem::Legend,true);
    }
    items = plot->itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < items.size(); i++ )
    {
        const QVariant itemInfo = plot->itemToInfo( items[i] );
        items[i]->setItemAttribute(QwtPlotItem::Legend,false);
    }
    plot->insertLegend( legend, QwtPlot::RightLegend );
    plot->plotLayout()->setAlignCanvasToScales( true );
    plot->resize(600,400);
    plot->replot();
}

void GwmIndependentVariableSelector::PlotModelAICcs(QVariant data, QwtPlot *plot)
{
    IndepVarsCriterionList modelInDepVars = data.value<IndepVarsCriterionList>();
    QSet<QString> indepVarNameSet;
    for (auto item : modelInDepVars)
    {
        for (auto variable : item.first)
        {
            indepVarNameSet.insert(variable.name);
        }
    }
    QList<QString> indepVarNameList = indepVarNameSet.toList();
    int n = indepVarNameList.size();
    double cex = (n > 10) ? 10.0/n : 1.0;
    int numModels = modelInDepVars.size();
    double alpha = 2 * M_PI / numModels;

    plot->plotLayout()->setAlignCanvasToScales( true );
    QPolygonF points1;
    int lastnVars = 1;
    for (int i = 0; i < numModels; i++)
    {
        QList<GwmVariable> vars = modelInDepVars[i].first;
        int nVars = vars.size();
        points1 << QPointF(i+1,modelInDepVars[i].second);
        if (nVars != lastnVars)
        {
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
    QBrush( Qt::yellow ), QPen( Qt::red, 0.5 ), QSize( 3, 3));//设置样本点的颜色、大小
    curve->setSymbol( symbol );//添加样本点形状
    curve->setSamples(points1);
    curve->attach( plot );
    curve->setLegendAttribute(curve->LegendShowLine);
    plot->resize(600,400);
    plot->replot();
    plot->show();
}

GwmIndependentVariableSelector::GwmIndependentVariableSelector()
{

}

QList<GwmVariable> GwmIndependentVariableSelector::optimize(IIndependentVariableSelectable *instance)
{
    QList<int> curIndex, restIndex;
    for (int i = 0; i < mIndepVars.size(); i++)
    {
        restIndex.append(i);
    }
    QList<QPair<QList<int>, double> > modelCriterions;
    for (int i = 0; i < mIndepVars.size(); i++)
    {
        vec criterions = vec(mIndepVars.size() - i);
        for (int j = 0; j < restIndex.size(); j++)
        {
            curIndex.append(restIndex[j]);
            double aic = instance->criterion(getVariables(curIndex));
            criterions(j) = aic;
            modelCriterions.append(qMakePair(curIndex, aic));
            curIndex.removeLast();
        }
        int iBestVar = criterions.index_min();
        curIndex.append(restIndex[iBestVar]);
        restIndex.removeAt(iBestVar);
    }
    mIndepVarsCriterion = sort(modelCriterions);
    return getVariables(select(mIndepVarsCriterion).first);
}

QList<GwmVariable> GwmIndependentVariableSelector::getVariables(QList<int> index)
{
    QList<GwmVariable> variables;
    for (int i : index)
        variables.append(mIndepVars[i]);
    return variables;
}

QList<QPair<QList<int>, double> > GwmIndependentVariableSelector::sort(QList<QPair<QList<int>, double> > models)
{
    int tag = 0;
    QList<int> sortIndex;
    for (int i = mIndepVars.size(); i > 0; i--)
    {
        std::sort(models.begin() + tag, models.begin() + tag + i, [](const QPair<QList<int>, double>& left, const QPair<QList<int>, double>& right){
            return left.second > right.second;
        });
        tag += i;
    }
    return models;
}

QPair<QList<int>, double> GwmIndependentVariableSelector::select(QList<QPair<QList<int>, double> > models)
{
    for (int i = models.size() - 1; i >= 0; i--)
    {
        if (models[i - 1].second - models[i].second >= mThreshold)
        {
            return models[i];
        }
    }
}

QList<QPair<QList<GwmVariable>, double> > GwmIndependentVariableSelector::indepVarsCriterion() const
{
    QList<QPair<QList<GwmVariable>, double> > criterions;
    for (QPair<QList<int>, double> item : mIndepVarsCriterion)
    {
        QList<GwmVariable> variables;
        for (int i : item.first)
        {
            variables.append(mIndepVars[i]);
        }
        criterions.append(qMakePair(variables, item.second));
    }
    return criterions;
}
