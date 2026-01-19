/***************************************************************************
    File                 : PolarGraph.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2026 by Ion Vasilief
    Description          : Polar plot window class
 ***************************************************************************/

#include "PolarGraph.h"
#include "Graph.h"
#include <ApplicationWindow.h>
#include <Table.h>
#include <qwt_polar_canvas.h>
#include <qwt_polar_curve.h>
#include <qwt_series_data.h>
#include <qwt_point_polar.h>
#include <qwt_polar_renderer.h>
#include <QApplication>
#include <QImageWriter>
#include <QPainter>
#include <QSvgGenerator>
#include <QtPrintSupport/QPrinter>

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

    d_plot->setAutoScale(QwtPolar::ScaleRadius);
    d_plot->setAutoScale(QwtPolar::ScaleAzimuth); // Ensure azimuth is also handled if possible
    d_plot->replot();
    
    setWindowTitle(name());
    resize(QSize(400, 400));
}

void PolarGraph::exportImage(const QString& fileName, int quality, bool transparent, int dpi,
        const QSizeF& customSize, int unit, double fontsFactor, int compression)
{
    if (!dpi)
        dpi = logicalDpiX();

    QSize size = d_plot->size();
    if (customSize.isValid())
        size = Graph::customPrintSize(customSize, unit, dpi);

    QImage image(size, QImage::Format_ARGB32);
    if (transparent)
        image.fill(Qt::transparent);
    else
        image.fill(Qt::white);

    QPainter painter(&image);
    QwtPolarRenderer renderer;
    renderer.render(d_plot, &painter, QRectF(QPointF(0, 0), size));
    painter.end();

    int dpm = (int)ceil(100.0/2.54*dpi);
    image.setDotsPerMeterX(dpm);
    image.setDotsPerMeterY(dpm);

    QImageWriter writer(fileName);
    if (compression > 0 && writer.supportsOption(QImageIOHandler::CompressionRatio)){
        writer.setQuality(quality);
        writer.setCompression(compression);
        writer.write(image);
    } else
        image.save(fileName, 0, quality);
}

void PolarGraph::exportVector(const QString& fileName, int res, bool color,
        const QSizeF& customSize, int unit, double fontsFactor)
{
    QPrinter printer;
    printer.setOutputFileName(fileName);
    if (res > 0)
        printer.setResolution(res);
    
    if (customSize.isValid()){
        QSize size = Graph::customPrintSize(customSize, unit, res > 0 ? res : logicalDpiX());
        printer.setPageSize(QPageSize(QSizeF(size) / printer.resolution(), QPageSize::Inch));
    }

    if (color)
        printer.setColorMode(QPrinter::Color);
    else
        printer.setColorMode(QPrinter::GrayScale);

    QwtPolarRenderer renderer;
    renderer.renderTo(d_plot, printer);
}

void PolarGraph::exportSVG(const QString& fname, const QSizeF& customSize, int unit, double fontsFactor)
{
    QSvgGenerator svg;
    svg.setFileName(fname);
    
    QSize size = d_plot->size();
    if (customSize.isValid())
        size = Graph::customPrintSize(customSize, unit, 96); // svg default res
        
    svg.setSize(size);
    
    QwtPolarRenderer renderer;
    renderer.renderTo(d_plot, svg);
}

void PolarGraph::exportPDF(const QString& fname)
{
    exportVector(fname);
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
