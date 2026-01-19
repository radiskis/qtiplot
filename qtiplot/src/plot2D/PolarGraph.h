/***************************************************************************
    File                 : PolarGraph.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2026 by Ion Vasilief
    Description          : Polar plot window class
 ***************************************************************************/

#ifndef POLARGRAPH_H
#define POLARGRAPH_H

#include <MdiSubWindow.h>
#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_grid.h>

class Table;
class ApplicationWindow;

class PolarGraph: public MdiSubWindow
{
    Q_OBJECT

public:
    PolarGraph(const QString& label, ApplicationWindow* parent = 0, const QString& name = QString(), Qt::WindowFlags f = {});
    ~PolarGraph();

    QwtPolarPlot* plot() { return d_plot; }
    
    void addCurve(Table* t, const QString& rColName, const QString& thetaColName);

    // Serialization
    QString saveToString();
    static PolarGraph* restore(ApplicationWindow* app, const QStringList& lst);

protected:
    void initPlot();

private:
    QwtPolarPlot* d_plot;
    QwtPolarGrid* d_grid;
};

#endif
