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
#include <qwt_scale_div.h>
#include <QApplication>
#include <QImageWriter>
#include <QPainter>
#include <QSvgGenerator>
#include <QtPrintSupport/QPrinter>
#include <QColor>
#include <QPen>

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
    d_plot->setAutoScale(QwtPolar::ScaleAzimuth);
    d_plot->replot();
    
    setWindowTitle(name());
    resize(QSize(400, 400));

    d_plot->canvas()->installEventFilter(this);
}

void PolarGraph::setAxisScale(int axisId, double start, double end, double step)
{
    d_plot->setScale(axisId, start, end, step);
    d_plot->replot();
}

void PolarGraph::setAxisAutoscale(int axisId, bool on)
{
    if (on)
        d_plot->setAutoScale(axisId);
    else {
        const QwtScaleDiv *sd = d_plot->scaleDiv(axisId);
        if (sd)
            d_plot->setScale(axisId, sd->lowerBound(), sd->upperBound());
    }
    d_plot->replot();
}

void PolarGraph::showGrid(int scaleId, bool on)
{
    d_grid->showGrid(scaleId, on);
    d_plot->replot();
}

void PolarGraph::showMinorGrid(int scaleId, bool on)
{
    d_grid->showMinorGrid(scaleId, on);
    d_plot->replot();
}

void PolarGraph::setMajorGridPen(int scaleId, const QPen &p)
{
    d_grid->setMajorGridPen(scaleId, p);
    d_plot->replot();
}

void PolarGraph::setMinorGridPen(int scaleId, const QPen &p)
{
    d_grid->setMinorGridPen(scaleId, p);
    d_plot->replot();
}

#include "dialogs/PolarSettingsDialog.h"
#include <QMenu>
#include <QContextMenuEvent>

void PolarGraph::showSettingsDialog()
{
    PolarSettingsDialog *d = new PolarSettingsDialog(this, applicationWindow());
    d->exec();
    delete d;
}

void PolarGraph::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    menu.addAction(tr("Options..."), this, &PolarGraph::showSettingsDialog);
    menu.exec(e->globalPos());
}

bool PolarGraph::eventFilter(QObject *object, QEvent *e)
{
    if (object == d_plot->canvas() && e->type() == QEvent::MouseButtonDblClick){
        showSettingsDialog();
        return true;
    }
    return MdiSubWindow::eventFilter(object, e);
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

QList<QwtPolarCurve*> PolarGraph::curves() const
{
    return d_curves.keys();
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
    
    PolarCurveInfo info;
    info.tableName = t->name();
    info.rColName = rColName;
    info.thetaColName = thetaColName;
    d_curves.insert(curve, info);

    d_plot->replot();
}


void PolarGraph::save(const QString &fn, const QString &geometry, bool)
{
    QFile f(fn);
    if (!f.isOpen()){
        if (!f.open(QIODevice::Append))
            return;
    }
    QTextStream t( &f );

    t << "<PolarGraph>\n";
    t << geometry;
    t << "<Name>" + name() + "</Name>\n";
    
    // Scale settings
    for (int i = 0; i < 2; i++){
        int axisId = (i == 0) ? QwtPolar::ScaleRadius : QwtPolar::ScaleAzimuth;
        QString axisName = (i == 0) ? "Radius" : "Azimuth";
        const QwtScaleDiv *sd = d_plot->scaleDiv(axisId);
        if (sd){
            t << "<" + axisName + "Scale>\n";
            t << "\t<Start>" + QString::number(sd->lowerBound()) + "</Start>\n";
            t << "\t<End>" + QString::number(sd->upperBound()) + "</End>\n";
            t << "\t<Autoscale>" + QString::number(d_plot->hasAutoScale(axisId)) + "</Autoscale>\n";
            t << "</" + axisName + "Scale>\n";
        }
    }

    // Grid settings
    for (int i = 0; i < 2; i++){
        int axisId = (i == 0) ? QwtPolar::ScaleRadius : QwtPolar::ScaleAzimuth;
        QString axisName = (i == 0) ? "Radius" : "Azimuth";
        t << "<" + axisName + "Grid>\n";
        t << "\t<MajorVisible>" + QString::number(d_grid->isGridVisible(axisId)) + "</MajorVisible>\n";
        t << "\t<MinorVisible>" + QString::number(d_grid->isMinorGridVisible(axisId)) + "</MinorVisible>\n";
        t << "\t<MajorColor>" + d_grid->majorGridPen(axisId).color().name() + "</MajorColor>\n";
        t << "\t<MinorColor>" + d_grid->minorGridPen(axisId).color().name() + "</MinorColor>\n";
        t << "</" + axisName + "Grid>\n";
    }
    
    // Curves
    for (QwtPolarCurve* c : d_curves.keys()){
        PolarCurveInfo info = d_curves.value(c);
        t << "<Curve>\n";
        t << "\t<Table>" + info.tableName + "</Table>\n";
        t << "\t<RCol>" + info.rColName + "</RCol>\n";
        t << "\t<ThetaCol>" + info.thetaColName + "</ThetaCol>\n";
        t << "\t<LineColor>" + c->pen().color().name() + "</LineColor>\n";
        t << "\t<LineWidth>" + QString::number(c->pen().width()) + "</LineWidth>\n";
        t << "\t<LineStyle>" + QString::number((int)c->pen().style()) + "</LineStyle>\n";
        
        const QwtSymbol *s = c->symbol();
        if (s){
             t << "\t<SymbolStyle>" + QString::number((int)s->style()) + "</SymbolStyle>\n";
             t << "\t<SymbolSize>" + QString::number(s->size().width()) + "</SymbolSize>\n";
             t << "\t<SymbolColor>" + s->brush().color().name() + "</SymbolColor>\n";
             t << "\t<SymbolPenColor>" + s->pen().color().name() + "</SymbolPenColor>\n";
             t << "\t<SymbolPenWidth>" + QString::number(s->pen().widthF()) + "</SymbolPenWidth>\n";
        }
        
        t << "</Curve>\n";
    }
    
    t << "</PolarGraph>\n";
}

PolarGraph* PolarGraph::restore(ApplicationWindow* app, const QStringList& lst)
{
    QString name;
    for (QString s : lst){
        if (s.contains("<Name>"))
            name = s.remove("<Name>").remove("</Name>").trimmed();
    }
    PolarGraph* w = new PolarGraph(name, app);
    if (!w) return 0;

    app->restoreWindowGeometry(w, lst[0]);
    
    QwtPolarCurve* currentCurve = nullptr;

    for (int i = 0; i < lst.count(); i++){
        QString s = lst[i];
        if (s.contains("<Curve>")){
             QString tableName, rCol, thetaCol;
             QColor uiLineColor = Qt::black;
             int uiLineWidth = 1;
             int uiLineStyle = Qt::SolidLine;
             
             int symbolStyle = -1; // NoSymbol
             int symbolSize = 8;
             QColor symbolColor = Qt::black;
             QColor symbolPenColor = Qt::black;
             double symbolPenWidth = 1.0;
             
             i++;
             while(i < lst.count() && !lst[i].contains("</Curve>")){
                 QString line = lst[i];
                 if (line.contains("<Table>")) tableName = line.remove("<Table>").remove("</Table>").trimmed();
                 else if (line.contains("<RCol>")) rCol = line.remove("<RCol>").remove("</RCol>").trimmed();
                 else if (line.contains("<ThetaCol>")) thetaCol = line.remove("<ThetaCol>").remove("</ThetaCol>").trimmed();
                 else if (line.contains("<LineColor>")) uiLineColor = QColor(line.remove("<LineColor>").remove("</LineColor>").trimmed());
                 else if (line.contains("<LineWidth>")) uiLineWidth = line.remove("<LineWidth>").remove("</LineWidth>").trimmed().toInt();
                 else if (line.contains("<LineStyle>")) uiLineStyle = line.remove("<LineStyle>").remove("</LineStyle>").trimmed().toInt();
                 else if (line.contains("<SymbolStyle>")) symbolStyle = line.remove("<SymbolStyle>").remove("</SymbolStyle>").trimmed().toInt();
                 else if (line.contains("<SymbolSize>")) symbolSize = line.remove("<SymbolSize>").remove("</SymbolSize>").trimmed().toInt();
                 else if (line.contains("<SymbolColor>")) symbolColor = QColor(line.remove("<SymbolColor>").remove("</SymbolColor>").trimmed());
                 else if (line.contains("<SymbolPenColor>")) symbolPenColor = QColor(line.remove("<SymbolPenColor>").remove("</SymbolPenColor>").trimmed());
                 else if (line.contains("<SymbolPenWidth>")) symbolPenWidth = line.remove("<SymbolPenWidth>").remove("</SymbolPenWidth>").trimmed().toDouble();
                 i++;
             }
             
             if (app && !tableName.isEmpty()){
                 Table *t = app->table(tableName);
                 if (t){
                     w->addCurve(t, rCol, thetaCol);
                     
                     // Access the just added curve. addCurve puts it in the map.
                     // The last added curve is the last key in the map? Map is sorted by pointer.
                     // But we just added it, so it should be there.
                     // However, d_curves is a QMap, which is sorted by key (pointer).
                     // We can't guarantee order. 
                     // Solution: modify addCurve to return the curve or use internal logic.
                     // But since we are inside restore, we can look at w->curves().last()?
                     // Wait, curves() returns d_curves.keys().
                     // If we want to be safe, we should modify addCurve or find the curve via matching info.
                     // Easier: w->d_plot->itemList().last() dynamic_cast to QwtPolarCurve.
                     
                     QList<QwtPolarItem*> items = w->plot()->itemList();
                     for (QwtPolarItem* item : items){
                         if (item->rtti() == QwtPolarItem::Rtti_PolarCurve){
                             if (QwtPolarCurve *c = static_cast<QwtPolarCurve*>(item)){
                                 // Check if it matches our data
                                 PolarCurveInfo info = w->curveInfo(c);
                                 if(info.tableName == tableName && info.rColName == rCol && info.thetaColName == thetaCol){
                                     currentCurve = c;
                                 }
                             }
                         }
                     }
                     
                     if (currentCurve){
                         currentCurve->setPen(QPen(uiLineColor, uiLineWidth, (Qt::PenStyle)uiLineStyle));
                         if (symbolStyle != -1 && symbolStyle != QwtSymbol::NoSymbol){
                              QwtSymbol* symb = new QwtSymbol((QwtSymbol::Style)symbolStyle);
                              symb->setSize(symbolSize);
                              symb->setColor(symbolColor);
                              symb->setPen(QPen(symbolPenColor, symbolPenWidth));
                              currentCurve->setSymbol(symb);
                         }
                     }
                 }
             }

        } else if (s.contains("<RadiusScale>")){
            i++;
            while (i < lst.count() && !lst[i].contains("</RadiusScale>")){
                QString line = lst[i];
                if (line.contains("<Start>")){
                   double start = QString(line).remove("<Start>").remove("</Start>").trimmed().toDouble();
                   w->setAxisScale(QwtPolar::ScaleRadius, start, w->d_plot->scaleDiv(QwtPolar::ScaleRadius)->upperBound());
                } else if (line.contains("<End>")){
                   double end = QString(line).remove("<End>").remove("</End>").trimmed().toDouble();
                   w->setAxisScale(QwtPolar::ScaleRadius, w->d_plot->scaleDiv(QwtPolar::ScaleRadius)->lowerBound(), end);
                } else if (line.contains("<Autoscale>") && QString(line).remove("<Autoscale>").remove("</Autoscale>").trimmed().toInt())
                    w->setAxisAutoscale(QwtPolar::ScaleRadius, true);
                i++;
            }
        } else if (s.contains("<AzimuthScale>")){
             i++;
            while (i < lst.count() && !lst[i].contains("</AzimuthScale>")){
                QString line = lst[i];
                if (line.contains("<Start>")){
                   double start = QString(line).remove("<Start>").remove("</Start>").trimmed().toDouble();
                   w->setAxisScale(QwtPolar::ScaleAzimuth, start, w->d_plot->scaleDiv(QwtPolar::ScaleAzimuth)->upperBound());
                } else if (line.contains("<End>")){
                    double end = QString(line).remove("<End>").remove("</End>").trimmed().toDouble();
                   w->setAxisScale(QwtPolar::ScaleAzimuth, w->d_plot->scaleDiv(QwtPolar::ScaleAzimuth)->lowerBound(), end);
                } else if (line.contains("<Autoscale>") && QString(line).remove("<Autoscale>").remove("</Autoscale>").trimmed().toInt())
                    w->setAxisAutoscale(QwtPolar::ScaleAzimuth, true);
                i++;
            }
        } else if (s.contains("<RadiusGrid>")){
            i++;
            while (i < lst.count() && !lst[i].contains("</RadiusGrid>")){
                QString line = lst[i];
                if (line.contains("<MajorVisible>"))
                    w->showGrid(QwtPolar::ScaleRadius, QString(line).remove("<MajorVisible>").remove("</MajorVisible>").trimmed().toInt());
                else if (line.contains("<MinorVisible>"))
                    w->showMinorGrid(QwtPolar::ScaleRadius, QString(line).remove("<MinorVisible>").remove("</MinorVisible>").trimmed().toInt());
                else if (line.contains("<MajorColor>")){
                    QPen p = w->d_grid->majorGridPen(QwtPolar::ScaleRadius);
                    p.setColor(QColor(QString(line).remove("<MajorColor>").remove("</MajorColor>").trimmed()));
                    w->setMajorGridPen(QwtPolar::ScaleRadius, p);
                } else if (line.contains("<MinorColor>")){
                    QPen p = w->d_grid->minorGridPen(QwtPolar::ScaleRadius);
                    p.setColor(QColor(QString(line).remove("<MinorColor>").remove("</MinorColor>").trimmed()));
                    w->setMinorGridPen(QwtPolar::ScaleRadius, p);
                }
                i++;
            }
        } else if (s.contains("<AzimuthGrid>")){
            i++;
            while (i < lst.count() && !lst[i].contains("</AzimuthGrid>")){
                QString line = lst[i];
                if (line.contains("<MajorVisible>"))
                    w->showGrid(QwtPolar::ScaleAzimuth, QString(line).remove("<MajorVisible>").remove("</MajorVisible>").trimmed().toInt());
                else if (line.contains("<MinorVisible>"))
                    w->showMinorGrid(QwtPolar::ScaleAzimuth, QString(line).remove("<MinorVisible>").remove("</MinorVisible>").trimmed().toInt());
                else if (line.contains("<MajorColor>")){
                    QPen p = w->d_grid->majorGridPen(QwtPolar::ScaleAzimuth);
                    p.setColor(QColor(QString(line).remove("<MajorColor>").remove("</MajorColor>").trimmed()));
                    w->setMajorGridPen(QwtPolar::ScaleAzimuth, p);
                } else if (line.contains("<MinorColor>")){
                    QPen p = w->d_grid->minorGridPen(QwtPolar::ScaleAzimuth);
                    p.setColor(QColor(QString(line).remove("<MinorColor>").remove("</MinorColor>").trimmed()));
                    w->setMinorGridPen(QwtPolar::ScaleAzimuth, p);
                }
                i++;
            }
        }
    }
    w->plot()->replot();
    return w;
}
