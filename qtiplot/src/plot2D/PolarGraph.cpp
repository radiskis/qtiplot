/***************************************************************************
    File                 : PolarGraph.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2026 by Ion Vasilief
    Description          : Polar plot window class
 ***************************************************************************/

#include "PolarGraph.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <qwt_polar_canvas.h>
#include <qwt_polar_curve.h>
#include <qwt_series_data.h>
#include <qwt_point_polar.h>
#include <QApplication>

PolarGraph::PolarGraph(const QString& label, ApplicationWindow* parent, const QString& name, Qt::WindowFlags f):
	MdiSubWindow(label, parent, name, f)
{
    initPlot();
}

PolarGraph::~PolarGraph()
{
}

void PolarGraph::initPlot()
{
    d_plot = new QwtPolarPlot(this);
    setWidget(d_plot);

    d_grid = new QwtPolarGrid();
    d_grid->setPen(QPen(Qt::black, 1, Qt::DotLine));
    d_grid->attach(d_plot);

    d_plot->replot();
    
    setWindowTitle(name());
}

void PolarGraph::addCurve(Table* t, const QString& rColName, const QString& thetaColName)
{
    if (!t) return;
    
    int rCol = t->colIndex(rColName);
    int thetaCol = t->colIndex(thetaColName);
    if (rCol < 0 || thetaCol < 0) return;

    QwtPolarCurve* curve = new QwtPolarCurve(rColName);
    
    int size = t->numRows();
    QVector<QwtPointPolar> data;
    for (int i = 0; i < size; i++){
        if (!t->text(i, rCol).isEmpty() && !t->text(i, thetaCol).isEmpty()){
            data.append(QwtPointPolar(t->cell(i, thetaCol), t->cell(i, rCol)));
        }
    }

    curve->setData(new QwtArraySeriesData<QwtPointPolar>(data));
    curve->setPen(QPen(Qt::red, 2));
    curve->attach(d_plot);
    
    d_plot->replot();
}

QString PolarGraph::saveToString()
{
    // Basic implementation for now
    QString s = "<PolarGraph>\n";
    s += "<Name>" + name() + "</Name>\n";
    s += "</PolarGraph>\n";
    return s;
}

PolarGraph* PolarGraph::restore(ApplicationWindow* app, const QStringList& lst)
{
    // Minimal restore logic
    QString name;
    foreach(QString s, lst){
        if (s.contains("<Name>"))
            name = s.remove("<Name>").remove("</Name>").trimmed();
    }
    PolarGraph* w = new PolarGraph(name, app);
    return w;
}
