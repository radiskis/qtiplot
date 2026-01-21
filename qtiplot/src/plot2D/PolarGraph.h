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
    
    void setAxisScale(int axisId, double start, double end, double step = 0.0);
    void setAxisAutoscale(int axisId, bool on = true);
    
    void showGrid(int scaleId, bool on = true);
    void showMinorGrid(int scaleId, bool on = true);
    void setMajorGridPen(int scaleId, const QPen &p);
    void setMinorGridPen(int scaleId, const QPen &p);
    
    QwtPolarGrid* grid() { return d_grid; }
    
    QList<QwtPolarCurve*> curves() const;

    void exportImage(const QString& fileName, int quality = 100, bool transparent = false, int dpi = 0,
            const QSizeF& customSize = QSizeF(), int unit = 0, double fontsFactor = 1.0, int compression = 0);
    void exportVector(const QString& fileName, int res = 0, bool color = true,
            const QSizeF& customSize = QSizeF(), int unit = 0, double fontsFactor = 1.0);
    void exportSVG(const QString& fname, const QSizeF& customSize = QSizeF(), int unit = 0, double fontsFactor = 1.0);
    void exportPDF(const QString& fname);

    // Serialization
    // Serialization
    QString saveToString();
    void save(const QString &fn, const QString &geometry, bool = false);
    static PolarGraph* restore(ApplicationWindow* app, const QStringList& lst);

public slots:
    void showSettingsDialog();

protected:
    void initPlot();
    bool eventFilter(QObject *object, QEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);

private:

    
    struct PolarCurveInfo {
         QString tableName;
         QString rColName;
         QString thetaColName;
    };
    PolarCurveInfo curveInfo(QwtPolarCurve* c) const { return d_curves.value(c); }
    
private:
    QwtPolarPlot* d_plot;
    QwtPolarGrid* d_grid;
    QMap<QwtPolarCurve*, PolarCurveInfo> d_curves;
};

#endif
