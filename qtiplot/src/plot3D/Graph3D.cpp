/***************************************************************************
    File                 : Graph3D.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004-2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 3D graph widget

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "Graph3D.h"
#include "Bar.h"
#include "Cone3D.h"
#include <ApplicationWindow.h>
#include <MyParser.h>
#include <MatrixModel.h>
#include <ColorMapEditor.h>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QClipboard>
#include <QPixmap>
#include <QBitmap>
#include <QCursor>
#include <QImageWriter>
#include <QTextStream>
#include <QTextDocumentWriter>

#include <qwt3d_io_gl2ps.h>
#include <qwt3d_coordsys.h>
#include <qwt_color_map.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_math.h>
#include <fstream>

UserFunction::UserFunction(const QString& s, SurfacePlot& pw)
: Function(pw), formula(s)
{}

double UserFunction::operator()(double x, double y)
{
	if (formula.isEmpty())
		return 0.0;

	MyParser parser;
	double result=0.0;
	try
	{
		parser.DefineVar("x", &x);
		parser.DefineVar("y", &y);

		parser.SetExpr((const std::string)formula.ascii());
		result=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(0,"QtiPlot - Input function error",QString::fromStdString(e.GetMsg()));
	}
	return result;
}

void UserFunction::setMesh (unsigned int columns, unsigned int rows)
{
	Function::setMesh (columns, rows);
	d_columns = columns;
	d_rows = rows;
}

UserParametricSurface::UserParametricSurface(const QString& xFormula, const QString& yFormula,
											 const QString& zFormula, SurfacePlot& pw)
: ParametricSurface(pw),
d_x_formula(xFormula),
d_y_formula(yFormula),
d_z_formula(zFormula)
{}

void UserParametricSurface::setDomain(double ul, double ur, double vl, double vr)
{
	ParametricSurface::setDomain(ul, ur, vl, vr);
	d_ul = ul;
	d_ur = ur;
	d_vl = vl;
	d_vr = vr;
}

void UserParametricSurface::setMesh (unsigned int columns, unsigned int rows)
{
	ParametricSurface::setMesh (columns, rows);
	d_columns = columns;
	d_rows = rows;
}

void UserParametricSurface::setPeriodic (bool u, bool v)
{
	ParametricSurface::setPeriodic (u, v);
	d_u_periodic = u;
	d_v_periodic = v;
}

Triple UserParametricSurface::operator()(double u, double v)
{
	if (d_x_formula.isEmpty() || d_y_formula.isEmpty() || d_z_formula.isEmpty())
		return Triple(0.0, 0.0, 0.0);

	double x = 0.0, y = 0.0, z = 0.0;
	MyParser parser;
	try{
		parser.DefineVar("u", &u);
		parser.DefineVar("v", &v);

		parser.SetExpr((const std::string)d_x_formula.ascii());
		x = parser.Eval();
		parser.SetExpr((const std::string)d_y_formula.ascii());
		y = parser.Eval();
		parser.SetExpr((const std::string)d_z_formula.ascii());
		z = parser.Eval();
	}
	catch(mu::ParserError &e){
		QMessageBox::critical(0, "QtiPlot - Input function error", QString::fromStdString(e.GetMsg()));
	}
	return Triple(x, y, z);
}

Graph3D::Graph3D(const QString& label, ApplicationWindow* parent, const char* name, Qt::WFlags f)
: MdiSubWindow(label, parent, name, f)
{
	initPlot();
}

void Graph3D::initPlot()
{
	setAcceptDrops(true);

	d_table = 0;
	d_table_plot_type = NoTable;
	d_matrix = 0;
	plotAssociation = QString();

	animation_redraw_wait = 50;
	d_timer = new QTimer(this);
	connect(d_timer, SIGNAL(timeout()), this, SLOT(rotate()));
	ignoreFonts = false;

	resize(500, 400);

	ApplicationWindow *app = applicationWindow();

	sp = new SurfacePlot(this);
	sp->installEventFilter(this);
	sp->setRotation(30, 0, 15);
	sp->setScale(1, 1, 1);
	sp->setShift(0.15, 0, 0);
	sp->setZoom(0.9);
	sp->setOrtho(app->d_3D_orthogonal);
	sp->setSmoothMesh(app->d_3D_smooth_mesh);
	sp->setResolution(app->d_3D_resolution);
	sp->setFloorStyle((Qwt3D::FLOORSTYLE)app->d_3D_projection);
	setWidget(sp);

	d_autoscale = app->d_3D_autoscale;

	title = QString::null;
	sp->setTitle(title);

	titleCol = Qt::black;
	sp->setTitleColor(Qt2GL(titleCol));

	titleFnt = app->d_3D_title_font;
	sp->setTitleFont(titleFnt.family(), titleFnt.pointSize(), titleFnt.weight(), titleFnt.italic());

	col_ = 0;
	d_color_map_file = QString::null;
	d_color_map = app->d_3D_color_map;

	legendOn = app->d_3D_legend;
	legendMajorTicks = 5;
	sp->showColorLegend(legendOn);
	sp->legend()->setAutoScale(true);
	sp->legend()->setMajors(legendMajorTicks) ;

	labelsDist = 0;

	scaleType = QVector<int>(3);
	for (int j = 0; j < 3; j++)
		scaleType[j] = 0;

	pointStyle = None;
	d_func = 0;
	d_surface = 0;
	d_alpha = 1.0;
	d_bars_rad = 0.007;
	d_bar_lines = true;
	d_filled_bars = true;
	d_point_size = 5; d_smooth_points = false;
	crossHairRad = 0.03, crossHairLineWidth = 2;
	crossHairSmooth = true, crossHairBoxed = false;
	conesQuality = 32; conesRad = 0.5;

	style_ = NOPLOT;
	initCoord();
	sp->coordinates()->setLineSmooth(app->d_3D_smooth_mesh);

	setNumbersFont(app->d_3D_numbers_font);
	setXAxisLabelFont(app->d_3D_axes_font);
	setYAxisLabelFont(app->d_3D_axes_font);
	setZAxisLabelFont(app->d_3D_axes_font);

	setMeshColor(app->d_3D_mesh_color);
	setAxesColor(app->d_3D_axes_color);
	setNumbersColor(app->d_3D_numbers_color);
	setLabelsColor(app->d_3D_labels_color);
	setBackgroundColor(app->d_3D_background_color);
	setGridColor(app->d_3D_grid_color);

	connect(sp,SIGNAL(rotationChanged(double, double, double)),this,SLOT(rotationChanged(double, double, double)));
	connect(sp,SIGNAL(zoomChanged(double)),this,SLOT(zoomChanged(double)));
	connect(sp,SIGNAL(scaleChanged(double, double, double)),this,SLOT(scaleChanged(double, double, double)));
	connect(sp,SIGNAL(shiftChanged(double, double, double)),this,SLOT(shiftChanged(double, double, double)));
}

void Graph3D::initCoord()
{
	sp->makeCurrent();
	for (unsigned i=0; i!=sp->coordinates()->axes.size(); ++i){
		sp->coordinates()->axes[i].setMajors(5);
		sp->coordinates()->axes[i].setMinors(5);
	}

	QString s = tr("X axis");
	sp->coordinates()->axes[X1].setLabelString(s);
	sp->coordinates()->axes[X2].setLabelString(s);
	sp->coordinates()->axes[X3].setLabelString(s);
	sp->coordinates()->axes[X4].setLabelString(s);
	labels<<s;

	s = tr("Y axis");
	sp->coordinates()->axes[Y1].setLabelString(s);
	sp->coordinates()->axes[Y2].setLabelString(s);
	sp->coordinates()->axes[Y3].setLabelString(s);
	sp->coordinates()->axes[Y4].setLabelString(s);
	labels<<s;

	s = tr("Z axis");
	sp->coordinates()->axes[Z1].setLabelString(s);
	sp->coordinates()->axes[Z2].setLabelString(s);
	sp->coordinates()->axes[Z3].setLabelString(s);
	sp->coordinates()->axes[Z4].setLabelString(s);
	labels<<s;

	sp->setCoordinateStyle(BOX);
	sp->coordinates()->setAutoScale(false);
}

void Graph3D::addFunction(const QString& s, double xl, double xr, double yl,
						double yr, double zl, double zr, int columns, int rows)
{
	if (d_surface)
		delete d_surface;
	else if (d_func)
		delete d_func;

	sp->makeCurrent();

	d_func = new UserFunction(s, *sp);

	d_func->setMesh(columns, rows);
	d_func->setDomain(xl, xr, yl, yr);
	d_func->setMinZ(zl);
	d_func->setMaxZ(zr);
	d_func->create();

	sp->legend()->setLimits(zl, zr);

	if (sp->plotStyle() == NOPLOT){
		sp->setPlotStyle(FILLED);
		style_=FILLED;
		pointStyle = None;
	}
  	sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
	findBestLayout();
	update();
}

void Graph3D::addParametricSurface(const QString& xFormula, const QString& yFormula,
						const QString& zFormula, double ul, double ur, double vl, double vr,
						int columns, int rows, bool uPeriodic, bool vPeriodic)
{
	if (d_surface)
		delete d_surface;
	else if (d_func)
		delete d_func;

	sp->makeCurrent();
	sp->resize(this->size());

	d_surface = new UserParametricSurface(xFormula, yFormula, zFormula, *sp);
	d_surface->setMesh(columns, rows);
	d_surface->setDomain(ul, ur, vl, vr);
	d_surface->setPeriodic(uPeriodic, vPeriodic);
	d_surface->create();

	double zl, zr;
	sp->coordinates()->axes[Z1].limits(zl, zr);
	sp->legend()->setLimits(zl, zr);

	if (sp->plotStyle() == NOPLOT){
		sp->setPlotStyle(FILLED);
		style_=FILLED;
		pointStyle = None;
	}
  	//sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
	findBestLayout();
	update();
}

void Graph3D::addRibbon(Table* table,const QString& xColName, const QString& yColName)
{
	if (!table)
		return;

	int xcol = table->colIndex(xColName);
	int ycol =  table->colIndex(yColName);

	if (xcol < 0 || ycol < 0)
		return;

    bool empty = !sp->hasData();

	plotAssociation = xColName+"(X)," + yColName+"(Y)";
	d_table = table;
	d_table_plot_type = Ribbon;
	int r=table->numRows();
	int i, xmesh=0, ymesh=2;
	for (i = 0; i < r; i++){
		if (!table->text(i,xcol).isEmpty() && !table->text(i,ycol).isEmpty())
			xmesh++;
	}

	if (xmesh == 0)
		xmesh++;

	double **data = Matrix::allocateMatrixData(xmesh, ymesh);
	gsl_vector * x = gsl_vector_alloc (xmesh);
	gsl_vector * y = gsl_vector_alloc (xmesh);

	for (int j = 0; j < ymesh; j++){
		int k=0;
		for (i = 0; i < r; i++){
			if (!table->text(i,xcol).isEmpty() && !table->text(i,ycol).isEmpty()){
				gsl_vector_set (x, k, table->cell(i, xcol));

				double yv = table->cell(i, ycol);
				gsl_vector_set (y, k, yv);
				data[k][j] = yv;
				k++;
			}
		}
	}

	double maxy = gsl_vector_max(y);
	double maxz = 0.6*maxy;
	sp->makeCurrent();
	sp->legend()->setLimits(gsl_vector_min(y), maxy);
	sp->loadFromData(data, xmesh, ymesh, gsl_vector_min(x), gsl_vector_max(x), 0, maxz);

	if (empty || d_autoscale)
		findBestLayout();

	gsl_vector_free (x);
	gsl_vector_free (y);
	Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::addRibbon(Table* table,const QString& xColName,const QString& yColName,
		double xl, double xr, double yl, double yr, double zl, double zr)
{
	d_table = table;
    d_table_plot_type = Ribbon;

	int r = table->numRows();
	int xcol = table->colIndex(xColName);
	int ycol = table->colIndex(yColName);

	plotAssociation = xColName + "(X)," + yColName + "(Y)";

	int i, j, xmesh = 0, ymesh = 2;
	double xv, yv;

	for (i = 0; i < r; i++){
		if (!table->text(i,xcol).isEmpty() && !table->text(i,ycol).isEmpty()){
			xv=table->cell(i, xcol);
			if (xv>=xl && xv <= xr)
				xmesh++;
		}
	}

	if (xmesh == 0)
		xmesh++;

	double **data = Matrix::allocateMatrixData(xmesh, ymesh);
	for ( j = 0; j < ymesh; j++){
		int k=0;
		for ( i = 0; i < r; i++){
			if (!table->text(i,xcol).isEmpty() && !table->text(i,ycol).isEmpty()){
				xv=table->cell(i,xcol);
				if (xv>=xl && xv <= xr){
					yv=table->cell(i,ycol);
					if (yv > zr)
						data[k][j] = zr;
					else if (yv < zl)
						data[k][j] = zl;
					else
						data[k][j] = yv;
					k++;
				}
			}
		}
	}
	sp->makeCurrent();
	sp->loadFromData(data, xmesh, ymesh, xl, xr, yl, yr);
	sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
	sp->legend()->setLimits(zl, zr);
	sp->legend()->setMajors(legendMajorTicks);

	Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::addMatrixData(Matrix* m)
{
	if (!m || d_matrix == m)
		return;

	d_table = NULL;

	bool first_time = false;
	if(!d_matrix)
		first_time = true;

	d_matrix = m;
	plotAssociation = "matrix<" + QString(m->objectName()) + ">";

	int cols = m->numCols();
	int rows = m->numRows();

	double **data_matrix = Matrix::allocateMatrixData(cols, rows);
	for (int i = 0; i < cols; i++ ){
		for (int j = 0; j < rows; j++)
			data_matrix[i][j] = m->cell(j, i);
	}

	sp->makeCurrent();
	sp->loadFromData(data_matrix, cols, rows, m->xStart(), m->xEnd(), m->yStart(), m->yEnd());

	double start, end;
	sp->coordinates()->axes[Z1].limits (start, end);
	sp->legend()->setLimits(start, end);
	sp->legend()->setMajors(legendMajorTicks);

	Matrix::freeMatrixData(data_matrix, cols);

	if (d_autoscale || first_time)
		findBestLayout();
	update();
}

void Graph3D::addMatrixData(Matrix* m, double xl, double xr,
		double yl, double yr, double zl, double zr)
{
	if (!m)
		return;

	d_matrix = m;
	d_table = NULL;
	plotAssociation = "matrix<" + QString(m->objectName()) + ">";

	updateScalesFromMatrix(xl, xr, yl, yr, zl, zr);
}

void Graph3D::insertNewData(Table* table, const QString& colName)
{
	int zCol=table->colIndex(colName);
	int yCol=table->colY(zCol);
	int xCol=table->colX(zCol);

	addData(table, xCol, yCol, zCol, Trajectory);
	update();
}

void Graph3D::changeDataColumn(Table* table, const QString& colName, int type)
{
	if (!table)
		return;

	resetNonEmptyStyle();

	if (type == Ribbon) {
		int ycol = table->colIndex(colName);
		int xcol = table->colX(ycol);

        if (d_autoscale)
            addRibbon(table, table->colName(xcol), colName);
        else
            addRibbon(table, table->colName(xcol), colName, xStart(), xStop(), yStart(), yStop(), zStart(), zStop());
		setFilledMeshStyle();
	} else {
		int zCol=table->colIndex(colName);
		int yCol=table->colY(zCol);
		int xCol=table->colX(zCol);

		addData(table, xCol, yCol, zCol, type);
	}

    resetAxesLabels();
}

void Graph3D::addData(Table* table, int xCol, int yCol, int zCol, int type)
{
    d_table_plot_type = (PlotType)type;

	loadData(table, xCol, yCol, zCol);

	//if (d_autoscale)
		findBestLayout();

	switch(type){
	    case Scatter:
            setDotStyle();
        break;
        case Trajectory:
            setWireframeStyle();
        break;
        case Bars:
            setBarStyle();
        break;

        default:
            break;
	}
}

void Graph3D::loadData(Table* table, int xCol, int yCol, int zCol,
		double xl, double xr, double yl, double yr, double zl, double zr)
{
	if (!table || xCol < 0 || yCol < 0 || zCol < 0)
		return;

	d_table = table;
	d_matrix = NULL;

	plotAssociation = table->colName(xCol) + "(X),";
	plotAssociation += table->colName(yCol) + "(Y),";
	plotAssociation += table->colName(zCol) + "(Z)";

	bool check_limits = true;
	if (xl == xr && yl == yr && zl == zr)
		check_limits = false;

	Qwt3D::TripleField data;
	Qwt3D::CellField cells;
	int index = 0;
	for (int i = 0; i < table->numRows(); i++){
		if (!table->text(i, xCol).isEmpty() && !table->text(i, yCol).isEmpty() && !table->text(i, zCol).isEmpty()){
			double x = table->cell(i, xCol);
			double y = table->cell(i, yCol);
			double z = table->cell(i, zCol);

			if (check_limits && (x < xl || x > xr || y < yl || y > yr || z < zl || z > zr))
				continue;

			data.push_back (Triple(x, y, z));
			Qwt3D::Cell cell;
			cell.push_back(index);
			if (index > 0)
                cell.push_back(index-1);
			cells.push_back (cell);
			index ++;
		}
	}
	sp->makeCurrent();
	sp->loadFromData (data, cells);
	if (check_limits)
		sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));

	double start, end;
	sp->coordinates()->axes[Z1].limits (start, end);
	sp->legend()->setLimits(start, end);
	sp->legend()->setMajors(legendMajorTicks);
}

void Graph3D::updateData(Table* table)
{
	if (d_func)// function plot
		return;

	QString name = plotAssociation;
	int pos=name.find("_",0);
	int posX=name.find("(",pos);
	QString xColName=name.mid(pos+1,posX-pos-1);

	pos=name.find(",", posX);
	posX=name.find("(", pos);
	QString yColName=name.mid(pos+1, posX-pos-1);

	int xCol=table->colIndex(xColName);
	int yCol=table->colIndex(yColName);

	if (name.contains("(Z)", true)) {
		pos=name.find(",", posX);
		posX=name.find("(", pos);
		QString zColName=name.mid(pos+1, posX-pos-1);
		int zCol=table->colIndex(zColName);
		resetNonEmptyStyle();
		loadData(table, xCol, yCol, zCol);
	} else
		updateDataXY(table, xCol, yCol);

	if (d_autoscale)
		findBestLayout();
	update();
}

void Graph3D::updateDataXY(Table* table, int xCol, int yCol)
{
	int r=table->numRows();
	int i, j, xmesh=0, ymesh=2;

	for (i = 0; i < r; i++)
	{
		if (!table->text(i,xCol).isEmpty() && !table->text(i,yCol).isEmpty())
			xmesh++;
	}

	if (xmesh<2)
	{
		sp->setPlotStyle(NOPLOT);
		update();
		return;
	}

	double **data = Matrix::allocateMatrixData(xmesh, ymesh);
	gsl_vector * x = gsl_vector_alloc (xmesh);
	gsl_vector * y = gsl_vector_alloc (xmesh);

	for ( j = 0; j < ymesh; j++)
	{
		int k=0;
		for ( i = 0; i < r; i++)
		{
			if (!table->text(i,xCol).isEmpty() && !table->text(i,yCol).isEmpty())
			{
				double xv=table->cell(i,xCol);
				double yv=table->cell(i,yCol);

				gsl_vector_set (x, k, xv);
				gsl_vector_set (y, k, yv);

				data[k][j] =yv;
				k++;
			}
		}
	}

	double minx=gsl_vector_min (x);
	double maxx=gsl_vector_max(x);
	double minz=gsl_vector_min (y);
	double maxz=gsl_vector_max(y);
	double miny, maxy;

	sp->makeCurrent();
	resetNonEmptyStyle();
	sp->coordinates()->axes[Y1].limits (miny,maxy);	//actual Y scale limits
	sp->loadFromData(data, xmesh, ymesh, minx, maxx, miny, maxy);
	sp->legend()->setLimits(minz,maxz);
	sp->legend()->setMajors(legendMajorTicks);

	gsl_vector_free (x);gsl_vector_free (y);
	Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::updateMatrixData(Matrix* m)
{
	int cols=m->numCols();
	int rows=m->numRows();

	double **data = Matrix::allocateMatrixData(cols, rows);
	for (int i = 0; i < cols; i++ ){
		for (int j = 0; j < rows; j++)
			data[i][j] = m->cell(j, i);
	}
	sp->loadFromData(data, cols, rows, m->xStart(), m->xEnd(), m->yStart(), m->yEnd());

	Qwt3D::Axis z_axis = sp->coordinates()->axes[Z1];
	double start, end;
	z_axis.limits (start, end);
	z_axis.setMajors(z_axis.majors());
	z_axis.setMajors(z_axis.minors());

	sp->legend()->setLimits(start, end);
	sp->legend()->setMajors(legendMajorTicks);

	Matrix::freeMatrixData(data, cols);
	if (d_autoscale)
		findBestLayout();
	update();
}

void Graph3D::resetNonEmptyStyle()
{
	if (sp->plotStyle() != Qwt3D::NOPLOT )
		return; // the plot was not previousely emptied

	if (style_== Qwt3D::USER)
	{// reseting the right user plot style
		switch (pointStyle)
		{
			case None:
				break;

			case Dots :
				sp->setPlotStyle(Dot(d_point_size, d_smooth_points));
				break;

			case VerticalBars :
				sp->setPlotStyle(Bar(d_bars_rad));
				break;

			case HairCross :
				sp->setPlotStyle(CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth, crossHairBoxed));
				break;

			case Cones :
				sp->setPlotStyle(Cone3D(conesRad, conesQuality));
				break;
		}
	}
	else
		sp->setPlotStyle(style_);
}

void Graph3D::update()
{
	sp->makeCurrent();

	resetAxesLabels();

	sp->updateData();
	sp->updateGL();
}

void Graph3D::setLabelsDistance(int val)
{
	if (labelsDist != val){
		labelsDist = val;
		sp->coordinates()->adjustLabels(val);
		sp->makeCurrent();
		sp->updateGL();
        emit modified();
	}
}

QFont Graph3D::numbersFont()
{
	return sp->coordinates()->axes[X1].numberFont();
}

void Graph3D::setNumbersFont(const QFont& font)
{
	sp->coordinates()->setNumberFont (font);
	sp->legend()->axis()->setNumberFont (font);
	sp->makeCurrent();
	sp->updateGL();
}

void Graph3D::setXAxisLabelFont(const QFont& fnt)
{
	sp->coordinates()->axes[X1].setLabelFont(fnt);
	sp->coordinates()->axes[X2].setLabelFont(fnt);
	sp->coordinates()->axes[X3].setLabelFont(fnt);
	sp->coordinates()->axes[X4].setLabelFont(fnt);
}

void Graph3D::setYAxisLabelFont(const QFont& fnt)
{
	sp->coordinates()->axes[Y1].setLabelFont(fnt);
	sp->coordinates()->axes[Y2].setLabelFont(fnt);
	sp->coordinates()->axes[Y3].setLabelFont(fnt);
	sp->coordinates()->axes[Y4].setLabelFont(fnt);
}

void Graph3D::setZAxisLabelFont(const QFont& fnt)
{
	sp->coordinates()->axes[Z1].setLabelFont(fnt);
	sp->coordinates()->axes[Z2].setLabelFont(fnt);
	sp->coordinates()->axes[Z3].setLabelFont(fnt);
	sp->coordinates()->axes[Z4].setLabelFont(fnt);
}

void Graph3D::setXAxisLabelFont(const QStringList& lst)
{
	QFont fnt=QFont(lst[1],lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
	sp->coordinates()->axes[X1].setLabelFont(fnt);
	sp->coordinates()->axes[X2].setLabelFont(fnt);
	sp->coordinates()->axes[X3].setLabelFont(fnt);
	sp->coordinates()->axes[X4].setLabelFont(fnt);
}

void Graph3D::setYAxisLabelFont(const QStringList& lst)
{
	QFont fnt=QFont(lst[1],lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
	sp->coordinates()->axes[Y1].setLabelFont(fnt);
	sp->coordinates()->axes[Y2].setLabelFont(fnt);
	sp->coordinates()->axes[Y3].setLabelFont(fnt);
	sp->coordinates()->axes[Y4].setLabelFont(fnt);
}

void Graph3D::setZAxisLabelFont(const QStringList& lst)
{
	QFont fnt=QFont(lst[1],lst[2].toInt(),lst[3].toInt(),lst[4].toInt());
	sp->coordinates()->axes[Z1].setLabelFont(fnt);
	sp->coordinates()->axes[Z2].setLabelFont(fnt);
	sp->coordinates()->axes[Z3].setLabelFont(fnt);
	sp->coordinates()->axes[Z4].setLabelFont(fnt);
}

QStringList Graph3D::axisTickLengths()
{
	QStringList lst;
	double majorl,minorl;

	sp->coordinates()->axes[X1].ticLength (majorl,minorl);
	lst<<QString::number(majorl);
	lst<<QString::number(minorl);

	sp->coordinates()->axes[Y1].ticLength (majorl,minorl);
	lst<<QString::number(majorl);
	lst<<QString::number(minorl);

	sp->coordinates()->axes[Z1].ticLength (majorl,minorl);
	lst<<QString::number(majorl);
	lst<<QString::number(minorl);

	return lst;
}

void Graph3D::setTickLengths(const QStringList& lst)
{
	double majorl, minorl;
	QStringList tick_length = lst;
	if (int(lst.count()) > 6)
		tick_length.remove(tick_length.first());

	majorl=tick_length[0].toDouble();
	minorl=tick_length[1].toDouble();
	sp->coordinates()->axes[X1].setTicLength (majorl,minorl);
	sp->coordinates()->axes[X2].setTicLength (majorl,minorl);
	sp->coordinates()->axes[X3].setTicLength (majorl,minorl);
	sp->coordinates()->axes[X4].setTicLength (majorl,minorl);

	majorl=tick_length[2].toDouble();
	minorl=tick_length[3].toDouble();
	sp->coordinates()->axes[Y1].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Y2].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Y3].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Y4].setTicLength (majorl,minorl);

	majorl=tick_length[4].toDouble();
	minorl=tick_length[5].toDouble();
	sp->coordinates()->axes[Z1].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Z2].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Z3].setTicLength (majorl,minorl);
	sp->coordinates()->axes[Z4].setTicLength (majorl,minorl);
}

void Graph3D::setXAxisTickLength(double majorLength, double minorLength)
{
	double majorl, minorl;
	sp->coordinates()->axes[X1].ticLength (majorl,minorl);
	if (majorl != majorLength || minorl != minorLength){
		sp->coordinates()->axes[X1].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[X2].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[X3].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[X4].setTicLength (majorLength,minorLength);
		}
}

void Graph3D::setYAxisTickLength(double majorLength, double minorLength)
{
	double majorl, minorl;
	sp->coordinates()->axes[Y1].ticLength (majorl,minorl);
	if (majorl != majorLength || minorl != minorLength){
		sp->coordinates()->axes[Y1].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Y2].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Y3].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Y4].setTicLength (majorLength,minorLength);
	}
}

void Graph3D::setZAxisTickLength(double majorLength, double minorLength)
{
	double majorl, minorl;
	sp->coordinates()->axes[Z1].ticLength (majorl,minorl);
	if (majorl != majorLength || minorl != minorLength){
		sp->coordinates()->axes[Z1].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Z2].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Z3].setTicLength (majorLength,minorLength);
		sp->coordinates()->axes[Z4].setTicLength (majorLength,minorLength);
	}
}

void Graph3D::setAxisTickLength(int axis, double majorLength, double minorLength)
{
	double majorl, minorl;
	switch(axis)
	{
		case 0:
			sp->coordinates()->axes[X1].ticLength (majorl,minorl);
			if (majorl != majorLength || minorl != minorLength){
				sp->coordinates()->axes[X1].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[X2].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[X3].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[X4].setTicLength (majorLength,minorLength);
			}
			break;

		case 1:
			sp->coordinates()->axes[Y1].ticLength (majorl,minorl);
			if (majorl != majorLength || minorl != minorLength){
				sp->coordinates()->axes[Y1].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Y2].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Y3].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Y4].setTicLength (majorLength,minorLength);
			}
			break;

		case 2:
			sp->coordinates()->axes[Z1].ticLength (majorl,minorl);
			if (majorl != majorLength || minorl != minorLength){
				sp->coordinates()->axes[Z1].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Z2].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Z3].setTicLength (majorLength,minorLength);
				sp->coordinates()->axes[Z4].setTicLength (majorLength,minorLength);
			}
			break;
	}
	sp->updateGL();
}

void Graph3D::rotationChanged(double, double, double)
{
	emit modified();
}

void Graph3D::scaleChanged(double, double, double)
{
	emit modified();
}

void Graph3D::shiftChanged(double, double, double)
{
	emit modified();
}

void Graph3D::zoomChanged(double)
{
	emit modified();
}

void Graph3D::resetAxesLabels()
{
	sp->coordinates()->axes[X1].setLabelString(labels[0]);
	sp->coordinates()->axes[X2].setLabelString(labels[0]);
	sp->coordinates()->axes[X3].setLabelString(labels[0]);
	sp->coordinates()->axes[X4].setLabelString(labels[0]);

	sp->coordinates()->axes[Y1].setLabelString(labels[1]);
	sp->coordinates()->axes[Y2].setLabelString(labels[1]);
	sp->coordinates()->axes[Y3].setLabelString(labels[1]);
	sp->coordinates()->axes[Y4].setLabelString(labels[1]);

	sp->coordinates()->axes[Z1].setLabelString(labels[2]);
	sp->coordinates()->axes[Z2].setLabelString(labels[2]);
	sp->coordinates()->axes[Z3].setLabelString(labels[2]);
	sp->coordinates()->axes[Z4].setLabelString(labels[2]);
}

void Graph3D::setAxesLabels(const QStringList& l)
{
	if (l.isEmpty())
		return;

	QString label = l[0];
	sp->coordinates()->axes[X1].setLabelString(label);
	sp->coordinates()->axes[X2].setLabelString(label);
	sp->coordinates()->axes[X3].setLabelString(label);
	sp->coordinates()->axes[X4].setLabelString(label);

	label = l[1];
	sp->coordinates()->axes[Y1].setLabelString(label);
	sp->coordinates()->axes[Y2].setLabelString(label);
	sp->coordinates()->axes[Y3].setLabelString(label);
	sp->coordinates()->axes[Y4].setLabelString(label);

	label = l[2];
	sp->coordinates()->axes[Z1].setLabelString(label);
	sp->coordinates()->axes[Z2].setLabelString(label);
	sp->coordinates()->axes[Z3].setLabelString(label);
	sp->coordinates()->axes[Z4].setLabelString(label);

	labels = l;
}

void Graph3D::setXAxisLabel(const QString& label)
{
    if (labels[0] != label){
        sp->coordinates()->axes[X1].setLabelString(label);
        sp->coordinates()->axes[X2].setLabelString(label);
        sp->coordinates()->axes[X3].setLabelString(label);
        sp->coordinates()->axes[X4].setLabelString(label);
        labels[0]=label;
    }

	sp->makeCurrent();
	sp->updateGL();
	emit modified();
}

void Graph3D::setYAxisLabel(const QString& label)
{
    if (labels[1] != label){
        sp->coordinates()->axes[Y1].setLabelString(label);
        sp->coordinates()->axes[Y2].setLabelString(label);
        sp->coordinates()->axes[Y3].setLabelString(label);
        sp->coordinates()->axes[Y4].setLabelString(label);
        labels[1]=label;
    }

	sp->makeCurrent();
	sp->updateGL();
	emit modified();
}

void Graph3D::setZAxisLabel(const QString& label)
{
    if (labels[2] != label){
        sp->coordinates()->axes[Z1].setLabelString(label);
        sp->coordinates()->axes[Z2].setLabelString(label);
        sp->coordinates()->axes[Z3].setLabelString(label);
        sp->coordinates()->axes[Z4].setLabelString(label);
        labels[2]=label;
    }

	sp->makeCurrent();
	sp->updateGL();
	emit modified();
}

QFont Graph3D::xAxisLabelFont()
{
	return sp->coordinates()->axes[X1].labelFont();
}

QFont Graph3D::yAxisLabelFont()
{
	return sp->coordinates()->axes[Y1].labelFont();
}

QFont Graph3D::zAxisLabelFont()
{
	return sp->coordinates()->axes[Z1].labelFont();
}

double Graph3D::xStart()
{
	double start,stop;
	sp->coordinates()->axes[X1].limits (start,stop);
	return start;
}

double Graph3D::xStop()
{
	double start,stop;
	sp->coordinates()->axes[X1].limits (start,stop);
	return stop;
}

double Graph3D::yStart()
{
	double start,stop;
	sp->coordinates()->axes[Y1].limits (start,stop);
	return start;
}

double Graph3D::yStop()
{
	double start,stop;
	sp->coordinates()->axes[Y1].limits (start,stop);
	return stop;
}

double Graph3D::zStart()
{
	double start,stop;
	sp->coordinates()->axes[Z1].limits (start,stop);
	return start;
}

double Graph3D::zStop()
{
	double start,stop;
	sp->coordinates()->axes[Z1].limits(start, stop);
	return stop;
}

QStringList Graph3D::scaleLimits()
{
	QStringList limits;
	double start, stop;
	int majors, minors;

	sp->coordinates()->axes[X1].limits (start,stop);
	majors=sp->coordinates()->axes[X1].majors();
	minors=sp->coordinates()->axes[X1].minors();

	limits<<QString::number(start);
	limits<<QString::number(stop);
	limits<<QString::number(majors);
	limits<<QString::number(minors);
	limits<<QString::number(scaleType[0]);

	sp->coordinates()->axes[Y1].limits (start,stop);
	majors=sp->coordinates()->axes[Y1].majors();
	minors=sp->coordinates()->axes[Y1].minors();

	limits<<QString::number(start);
	limits<<QString::number(stop);
	limits<<QString::number(majors);
	limits<<QString::number(minors);
	limits<<QString::number(scaleType[1]);

	sp->coordinates()->axes[Z1].limits (start,stop);
	majors=sp->coordinates()->axes[Z1].majors();
	minors=sp->coordinates()->axes[Z1].minors();

	limits<<QString::number(start);
	limits<<QString::number(stop);
	limits<<QString::number(majors);
	limits<<QString::number(minors);
	limits<<QString::number(scaleType[2]);

	return limits;
}

QStringList Graph3D::scaleTicks()
{
	QStringList limits;
	int majors,minors;

	majors=sp->coordinates()->axes[X1].majors();
	minors=sp->coordinates()->axes[X1].minors();
	limits<<QString::number(majors);
	limits<<QString::number(minors);

	majors=sp->coordinates()->axes[Y1].majors();
	minors=sp->coordinates()->axes[Y1].minors();
	limits<<QString::number(majors);
	limits<<QString::number(minors);

	majors=sp->coordinates()->axes[Z1].majors();
	minors=sp->coordinates()->axes[Z1].minors();
	limits<<QString::number(majors);
	limits<<QString::number(minors);

	return limits;
}

void Graph3D::setScale(int axis, double start, double end, int majorTicks, int minorTicks, Qwt3D::SCALETYPE type)
{
	double xMin, xMax, yMin, yMax, zMin, zMax;
	int baseAxis = X1;

	sp->makeCurrent();
	sp->coordinates()->axes[X1].limits(xMin, xMax);
	sp->coordinates()->axes[Y1].limits(yMin, yMax);
	sp->coordinates()->axes[Z1].limits(zMin, zMax);

	switch(axis)
	{
	case 0:
		if (xMin != start || xMax != end)
		{
			xMin = start;
			xMax = end;
		}
		baseAxis = X1;
		break;
	case 1:
		if (yMin != start || yMax != end)
		{
			yMin = start;
			yMax = end;
		}
		baseAxis = Y1;
		break;
	case 2:
		if (zMin != start || zMax != end)
		{
			zMin = start;
			zMax = end;
		}
		baseAxis = Z1;
		break;
	}

	if (d_func){
		d_func->setDomain(xMin, xMax, yMin, yMax);
		d_func->setMinZ(zMin);
		d_func->setMaxZ(zMax);
		d_func->create();
		sp->createCoordinateSystem(Triple(xMin, yMin, zMin), Triple(xMax, yMax, zMax));
	} else if (d_surface){
		d_surface->restrictRange(ParallelEpiped(Triple(xMin, yMin, zMin), Triple(xMax, yMax, zMax)));
		d_surface->create();
		sp->createCoordinateSystem(Triple(xMin, yMin, zMin), Triple(xMax, yMax, zMax));
	} else
		setScales(xMin, xMax, yMin, yMax, zMin, zMax);

	if(scaleType[axis] != type){
		sp->coordinates()->axes[baseAxis].setScale(type);
		scaleType[axis] = type;
	}

	if (sp->coordinates()->axes[baseAxis].majors() != majorTicks){
		sp->coordinates()->axes[baseAxis].setMajors(majorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3].setMajors(majorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3+1].setMajors(majorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3+2].setMajors(majorTicks);
	}

	if (sp->coordinates()->axes[baseAxis].minors() != minorTicks){
		sp->coordinates()->axes[baseAxis].setMinors(minorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3].setMinors(minorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3+1].setMinors(minorTicks);
		sp->coordinates()->axes[(baseAxis+1)*3+2].setMinors(minorTicks);
	}

	update();
	emit modified();
}

void Graph3D::updateScale(int axis, const QStringList& options)
{
	QString st = QString::number(scaleType[axis]);
	double start, stop, xl, xr, yl, yr;
	int majors, minors, newMaj, newMin;

	sp->makeCurrent();
	switch(axis)
	{
		case 0:
			majors=sp->coordinates()->axes[X1].majors ();
  	        minors=sp->coordinates()->axes[X1].minors ();
			sp->coordinates()->axes[X1].limits(xl,xr);
			if (xl !=options[0].toDouble() || xr != options[1].toDouble())
			{
				xl=options[0].toDouble();
				xr=options[1].toDouble();
				sp->coordinates()->axes[Y1].limits(yl,yr);
				sp->coordinates()->axes[Z1].limits(start,stop);

				if (d_func){
					d_func->setDomain(xl,xr,yl,yr);
					d_func->create ();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else if (d_surface){
					d_surface->restrictRange (ParallelEpiped(Triple(xl, yl, start), Triple(xr, yr, stop)));
					d_surface->create();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else
					setScales(xl, xr, yl, yr, start, stop);
			}

			if(st != options[4]){
				if (options[4]=="0"){
					sp->coordinates()->axes[X1].setScale (LINEARSCALE);
					scaleType[axis]=0;
				} else {
					sp->coordinates()->axes[X1].setScale (LOG10SCALE);
					scaleType[axis]=1;
				}
			}

			newMaj = options[2].toInt();
			if (majors != newMaj){
				sp->coordinates()->axes[X1].setMajors(newMaj);
				sp->coordinates()->axes[X2].setMajors(newMaj);
				sp->coordinates()->axes[X3].setMajors(newMaj);
				sp->coordinates()->axes[X4].setMajors(newMaj);
			}

			newMin = options[3].toInt();
			if (minors != newMin){
				sp->coordinates()->axes[X1].setMinors(newMin);
				sp->coordinates()->axes[X2].setMinors(newMin);
				sp->coordinates()->axes[X3].setMinors(newMin);
				sp->coordinates()->axes[X4].setMinors(newMin);
			}
			break;

		case 1:
			majors = sp->coordinates()->axes[Y1].majors ();
  	        minors = sp->coordinates()->axes[Y1].minors ();
			sp->coordinates()->axes[Y1].limits(yl, yr);
			if (yl != options[0].toDouble() || yr != options[1].toDouble()){
				yl = options[0].toDouble();
				yr = options[1].toDouble();
				sp->coordinates()->axes[X1].limits(xl, xr);
				sp->coordinates()->axes[Z1].limits(start, stop);

				if (d_func){
					d_func->setDomain(xl, xr, yl, yr);
					d_func->create();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else if (d_surface){
					d_surface->restrictRange (ParallelEpiped(Triple(xl, yl, start), Triple(xr, yr, stop)));
					d_surface->create();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else
					setScales(xl, xr, yl, yr, start, stop);
			}

			newMaj = options[2].toInt();
			if (majors != newMaj ){
				sp->coordinates()->axes[Y1].setMajors(newMaj);
				sp->coordinates()->axes[Y2].setMajors(newMaj);
				sp->coordinates()->axes[Y3].setMajors(newMaj);
				sp->coordinates()->axes[Y4].setMajors(newMaj);
			}

            newMin = options[3].toInt();
			if (minors != newMin){
				sp->coordinates()->axes[Y1].setMinors(newMin);
				sp->coordinates()->axes[Y2].setMinors(newMin);
				sp->coordinates()->axes[Y3].setMinors(newMin);
				sp->coordinates()->axes[Y4].setMinors(newMin);
			}

			if(st != options[4]){
				if (options[4]=="0"){
					sp->coordinates()->axes[Y1].setScale (LINEARSCALE);
					scaleType[axis]=0;
				} else {
					sp->coordinates()->axes[Y1].setScale (LOG10SCALE);
					scaleType[axis]=1;
				}
			}
			break;

		case 2:
			majors=sp->coordinates()->axes[Z1].majors();
			minors=sp->coordinates()->axes[Z1].minors();
			sp->coordinates()->axes[Z1].limits(start,stop);
			if (start != options[0].toDouble() || stop != options[1].toDouble())
			{
				start=options[0].toDouble();
				stop=options[1].toDouble();
				sp->coordinates()->axes[X1].limits(xl,xr);
  	            sp->coordinates()->axes[Y1].limits(yl,yr);
				if (d_func){
					d_func->setMinZ(start);
					d_func->setMaxZ(stop);
					d_func->create ();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else if (d_surface){
					d_surface->restrictRange (ParallelEpiped(Triple(xl, yl, start), Triple(xr, yr, stop)));
					d_surface->create();
					sp->createCoordinateSystem(Triple(xl, yl, start), Triple(xr, yr, stop));
				} else
					setScales(xl, xr, yl, yr, start, stop);
				sp->legend()->setLimits(start,stop);
			}

			newMaj= options[2].toInt();
			if (majors != newMaj ){
				sp->coordinates()->axes[Z1].setMajors(newMaj);
				sp->coordinates()->axes[Z2].setMajors(newMaj);
				sp->coordinates()->axes[Z3].setMajors(newMaj);
				sp->coordinates()->axes[Z4].setMajors(newMaj);
			}

			newMin = options[3].toInt();
			if (minors != newMin){
				sp->coordinates()->axes[Z1].setMinors(newMin);
				sp->coordinates()->axes[Z2].setMinors(newMin);
				sp->coordinates()->axes[Z3].setMinors(newMin);
				sp->coordinates()->axes[Z4].setMinors(newMin);
			}
			if(st != options[4]){
  	         	if (options[4]=="0"){
  	             	sp->coordinates()->axes[Z1].setScale (LINEARSCALE);
  	            	scaleType[axis]=0;
  	            } else {
  	            	sp->coordinates()->axes[Z1].setScale (LOG10SCALE);
  	                scaleType[axis]=1;
  	            }
  	        }
			break;
	}
	update();
	emit modified();
}

void Graph3D::setScales(double xl, double xr, double yl, double yr, double zl, double zr)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	sp->hide();//in order to avoid flickering, due to changes in tick length

	if (d_matrix)
		updateScalesFromMatrix(xl, xr, yl, yr, zl, zr);
    else if (d_func){
        d_func->setDomain(xl, xr, yl, yr);
        d_func->setMinZ(zl);
        d_func->setMaxZ(zr);
        d_func->create ();
        sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
    } else if (d_table){
		QString name = plotAssociation;

		int pos = name.find("_", 0);
		int posX = name.find("(", pos);
		QString xColName = name.mid(pos+1, posX-pos-1);
		int xCol = d_table->colIndex(xColName);

		pos = name.find(",", posX);
		posX = name.find("(", pos);
		QString yColName = name.mid(pos+1, posX-pos-1);
		int yCol = d_table->colIndex(yColName);

		if (name.endsWith("(Z)",true)){
			pos = name.find(",",posX);
			posX = name.find("(",pos);
			QString zColName = name.mid(pos+1,posX-pos-1);
			int zCol = d_table->colIndex(zColName);

			loadData(d_table, xCol, yCol, zCol, xl, xr, yl, yr, zl, zr);
		} else if (name.endsWith("(Y)",true))
			updateScales(xl, xr, yl, yr, zl, zr, xCol, yCol);
	}
    resetAxesLabels();
    findBestLayout();

    sp->show();

	QApplication::restoreOverrideCursor();
}

void Graph3D::updateScalesFromMatrix(double xl, double xr, double yl, double yr, double zl, double zr)
{
	double xStart = qMin(d_matrix->xStart(), d_matrix->xEnd());
	double xEnd = qMax(d_matrix->xStart(), d_matrix->xEnd());
	double yStart = qMin(d_matrix->yStart(), d_matrix->yEnd());
	double yEnd = qMax(d_matrix->yStart(), d_matrix->yEnd());
	double dx = d_matrix->dx();
	double dy = d_matrix->dy();
    double x_begin = qMin(xl, xr);
	double y_begin = qMin(yl, yr);
	int nc = int(fabs(xr - xl)/dx) + 1;// new number of columns
	int nr = int(fabs(yr - yl)/dy) + 1;// new number of rows
	double **data_matrix = Matrix::allocateMatrixData(nc, nr);
	for (int i = 0; i < nc; i++){
		double x = x_begin + i*dx;
		if (x < xStart || x > xEnd){
			for (int j = 0; j < nr; j++)
				data_matrix[i][j] = GSL_NAN;
			continue;
		}

        double dli, dlf;
        dlf = modf(fabs((x - xStart)/dx), &dli);
        int l = int(dli); if (dlf > 0.5) l++;
		for (int j = 0; j < nr; j++){
			double y = y_begin + j*dy;
			if (y < yStart || y > yEnd){
				data_matrix[i][j] = GSL_NAN;
				continue;
			}

			double dki, dkf;
			dkf = modf(fabs((y - yStart)/dy), &dki);
			int k = int(dki); if (dkf > 0.5) k++;
			double val = d_matrix->cell(k, l);

			if (val > zr)
				data_matrix[i][j] = zr;
			else if (val < zl)
				data_matrix[i][j] = zl;
			else
				data_matrix[i][j] = val;
		}
	}
	sp->loadFromData(data_matrix, nc, nr, xl, xr, yl, yr);
	Matrix::freeMatrixData(data_matrix, nc);

	sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
	sp->legend()->setLimits(zl, zr);
	sp->legend()->setMajors(legendMajorTicks);

	update();
}

void Graph3D::updateScales(double xl, double xr, double yl, double yr,double zl, double zr,
		int xcol, int ycol)
{
	int r=d_table->numRows();
	int xmesh=0, ymesh=2;
	double xv, yv;

	for (int i = 0; i < r; i++){
		if (!d_table->text(i,xcol).isEmpty() && !d_table->text(i,ycol).isEmpty()){
			xv=d_table->cell(i,xcol);
			if (xv >= xl && xv <= xr)
				xmesh++;
		}
	}

	if (xmesh == 0)
		xmesh++;

	double **data = Matrix::allocateMatrixData(xmesh, ymesh);

	for (int j = 0; j < ymesh; j++){
		int k=0;
		for (int i = 0; i < r; i++){
			if (!d_table->text(i,xcol).isEmpty() && !d_table->text(i,ycol).isEmpty()){
				xv=d_table->cell(i,xcol);
				if (xv >= xl && xv <= xr){
					yv=d_table->cell(i,ycol);
					if (yv > zr)
						data[k][j] = zr;
					else if (yv < zl)
						data[k][j] = zl;
					else
						data[k][j] = yv;
					k++;
				}
			}
		}
	}

	sp->loadFromData(data, xmesh, ymesh, xl, xr, yl, yr);
	sp->createCoordinateSystem(Triple(xl, yl, zl), Triple(xr, yr, zr));
	Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::setTicks(const QStringList& options)
{
	int min,maj;
	if (int(options.count()) == 6)
	{
		maj=options[0].toInt();
		sp->coordinates()->axes[X1].setMajors(maj);
		sp->coordinates()->axes[X2].setMajors(maj);
		sp->coordinates()->axes[X3].setMajors(maj);
		sp->coordinates()->axes[X4].setMajors(maj);

		min=options[1].toInt();
		sp->coordinates()->axes[X1].setMinors(min);
		sp->coordinates()->axes[X2].setMinors(min);
		sp->coordinates()->axes[X3].setMinors(min);
		sp->coordinates()->axes[X4].setMinors(min);

		maj=options[2].toInt();
		sp->coordinates()->axes[Y1].setMajors(maj);
		sp->coordinates()->axes[Y2].setMajors(maj);
		sp->coordinates()->axes[Y3].setMajors(maj);
		sp->coordinates()->axes[Y4].setMajors(maj);

		min=options[3].toInt();
		sp->coordinates()->axes[Y1].setMinors(min);
		sp->coordinates()->axes[Y2].setMinors(min);
		sp->coordinates()->axes[Y3].setMinors(min);
		sp->coordinates()->axes[Y4].setMinors(min);

		maj=options[4].toInt();
		sp->coordinates()->axes[Z1].setMajors(maj);
		sp->coordinates()->axes[Z2].setMajors(maj);
		sp->coordinates()->axes[Z3].setMajors(maj);
		sp->coordinates()->axes[Z4].setMajors(maj);

		min=options[5].toInt();
		sp->coordinates()->axes[Z1].setMinors(min);
		sp->coordinates()->axes[Z2].setMinors(min);
		sp->coordinates()->axes[Z3].setMinors(min);
		sp->coordinates()->axes[Z4].setMinors(min);
	}
	else
	{
		maj=options[1].toInt();
		sp->coordinates()->axes[X1].setMajors(maj);
		sp->coordinates()->axes[X2].setMajors(maj);
		sp->coordinates()->axes[X3].setMajors(maj);
		sp->coordinates()->axes[X4].setMajors(maj);

		min=options[2].toInt();
		sp->coordinates()->axes[X1].setMinors(min);
		sp->coordinates()->axes[X2].setMinors(min);
		sp->coordinates()->axes[X3].setMinors(min);
		sp->coordinates()->axes[X4].setMinors(min);

		maj=options[3].toInt();
		sp->coordinates()->axes[Y1].setMajors(maj);
		sp->coordinates()->axes[Y2].setMajors(maj);
		sp->coordinates()->axes[Y3].setMajors(maj);
		sp->coordinates()->axes[Y4].setMajors(maj);

		min=options[4].toInt();
		sp->coordinates()->axes[Y1].setMinors(min);
		sp->coordinates()->axes[Y2].setMinors(min);
		sp->coordinates()->axes[Y3].setMinors(min);
		sp->coordinates()->axes[Y4].setMinors(min);

		maj=options[5].toInt();
		sp->coordinates()->axes[Z1].setMajors(maj);
		sp->coordinates()->axes[Z2].setMajors(maj);
		sp->coordinates()->axes[Z3].setMajors(maj);
		sp->coordinates()->axes[Z4].setMajors(maj);

		min=options[6].toInt();
		sp->coordinates()->axes[Z1].setMinors(min);
		sp->coordinates()->axes[Z2].setMinors(min);
		sp->coordinates()->axes[Z3].setMinors(min);
		sp->coordinates()->axes[Z4].setMinors(min);
	}
}

void Graph3D::setMeshColor(const QColor& meshColor)
{
	if (meshCol != meshColor){
		sp->setMeshColor(Qt2GL(meshColor));
		meshCol=meshColor;
	}
}

void Graph3D::setAxesColor(const QColor& axesColor)
{
	if(axesCol != axesColor){
		sp->coordinates()->setAxesColor(Qt2GL(axesColor));
		axesCol=axesColor;
	}
}

void Graph3D::setNumbersColor(const QColor& numColor)
{
	if(numCol != numColor){
		sp->legend()->axis()->setNumberColor(Qt2GL(numColor));
		sp->coordinates()->setNumberColor(Qt2GL(numColor));
		numCol = numColor;
	}
}

void Graph3D::setLabelsColor(const QColor& labelColor)
{
	if(labelsCol !=labelColor){
		sp->coordinates()->setLabelColor(Qt2GL(labelColor));
		labelsCol=labelColor;
	}
}

void Graph3D::setBackgroundColor(const QColor& bgColor)
{
	if(bgCol !=bgColor){
		sp->setBackgroundColor(Qt2GL(bgColor));
		bgCol=bgColor;
	}
}

void Graph3D::setGridColor(const QColor& gridColor)
{
	if(gridCol != gridColor){
		sp->coordinates()->setGridLinesColor(Qt2GL(gridColor));
		gridCol=gridColor;
	}
}

void Graph3D::scaleFonts(double factor)
{
	if (factor == 1.0 || factor <= 0.0)
		return;

	QFont font = sp->coordinates()->axes[X1].numberFont();
	font.setPointSizeFloat(font.pointSizeFloat()*factor);
	sp->coordinates()->setNumberFont (font);
	sp->legend()->axis()->setNumberFont (font);

	titleFnt.setPointSizeFloat(factor*titleFnt.pointSizeFloat());
	sp->setTitleFont(titleFnt.family(),titleFnt.pointSize(),titleFnt.weight(),titleFnt.italic());

	font = xAxisLabelFont();
	font.setPointSizeFloat(factor*font.pointSizeFloat());
	setXAxisLabelFont(font);

	font = yAxisLabelFont();
	font.setPointSizeFloat(factor*font.pointSizeFloat());
	setYAxisLabelFont(font);

	font = zAxisLabelFont();
	font.setPointSizeFloat(factor*font.pointSizeFloat());
	setZAxisLabelFont(font);
}

void Graph3D::resizeEvent(QResizeEvent *e)
{
	if (!ignoreFonts && this->isVisible()){
		double ratio = (double)e->size().height()/(double)e->oldSize().height();
		scaleFonts(ratio);
	}
	emit resizedWindow(this);
	emit modified();
	QMdiSubWindow::resizeEvent(e);
}

void Graph3D::setFramed()
{
	if (sp->coordinates()->style() == FRAME)
		return;

	sp->makeCurrent();
	sp->setCoordinateStyle(FRAME);
}

void Graph3D::setBoxed()
{
	if (sp->coordinates()->style() == BOX)
		return;

	sp->makeCurrent();
	sp->setCoordinateStyle(BOX);
}

void Graph3D::setNoAxes()
{
	if (sp->coordinates()->style() == NOCOORD)
		return;

	sp->makeCurrent();
	sp->setCoordinateStyle(NOCOORD);
}

void Graph3D::setPolygonStyle()
{
	if (sp->plotStyle() == FILLED)
		return;

	sp->makeCurrent();
	sp->setPlotStyle(FILLED);
	sp->updateData();
	sp->updateGL();

	style_=FILLED;
	pointStyle = None;
}

void Graph3D::setFilledMeshStyle()
{
	if (sp->plotStyle() == FILLEDMESH)
		return;

	sp->makeCurrent();
	sp->setPlotStyle(FILLEDMESH);
	sp->updateData();
	sp->updateGL();

	style_=FILLEDMESH;
	pointStyle = None;
}

void Graph3D::setHiddenLineStyle()
{
	if (sp->plotStyle() == HIDDENLINE)
		return;

	sp->makeCurrent();
	sp->setPlotStyle(HIDDENLINE);
	sp->showColorLegend(false);
	sp->updateData();
	sp->updateGL();

	style_=HIDDENLINE;
	pointStyle = None;
	legendOn=false;
}

void Graph3D::setWireframeStyle()
{
	if (sp->plotStyle() == WIREFRAME)
		return;

	sp->makeCurrent();
	sp->setPlotStyle(WIREFRAME);
	sp->showColorLegend(false);
	sp->updateData();
	sp->updateGL();

	pointStyle = None;
	style_=WIREFRAME;
	legendOn=false;
}

void Graph3D::setDotStyle()
{
	pointStyle=Dots;
	style_=Qwt3D::USER;

	sp->makeCurrent();
	sp->setPlotStyle(Dot(d_point_size, d_smooth_points));
	sp->updateData();
	sp->updateGL();
}

void Graph3D::setConeStyle()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	pointStyle=Cones;
	style_=Qwt3D::USER;

	sp->makeCurrent();
	sp->setPlotStyle(Cone3D(conesRad, conesQuality));
	sp->updateData();
	sp->updateGL();

	QApplication::restoreOverrideCursor();
}

void Graph3D::setCrossStyle()
{
	pointStyle=HairCross;
	style_=Qwt3D::USER;

	sp->makeCurrent();
	sp->setPlotStyle(CrossHair(crossHairRad, crossHairLineWidth,crossHairSmooth,crossHairBoxed));
	sp->updateData();
	sp->updateGL();
}

void Graph3D::clearData()
{
	if (d_matrix)
		d_matrix = 0;
	else if (d_table)
		d_table = 0;
	else if (d_func){
		delete d_func;
		d_func = 0;
	}
	plotAssociation = QString();
	d_table_plot_type = NoTable;

	sp->makeCurrent();
	sp->loadFromData (0, 0, 0, false, false);
	sp->updateData();
	sp->updateGL();
}

void Graph3D::setBarStyle()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    d_table_plot_type = Bars;
	pointStyle = VerticalBars;
	style_ = Qwt3D::USER;

	sp->makeCurrent();
	sp->setPlotStyle(Bar(d_bars_rad, d_bar_lines, d_filled_bars, applicationWindow()->d_3D_smooth_mesh));
	sp->updateData();
	sp->updateGL();
	QApplication::restoreOverrideCursor();
}

void Graph3D::setFloorData()
{
	if (sp->floorStyle() == FLOORDATA)
		return;

	sp->makeCurrent();
	sp->setFloorStyle(FLOORDATA);
	sp->updateData();
	sp->updateGL();
}

void Graph3D::setFloorIsolines()
{
	if (sp->floorStyle() == FLOORISO)
		return;

	sp->makeCurrent();
	sp->setFloorStyle(FLOORISO);
	sp->updateData();
	sp->updateGL();
}

void Graph3D::setEmptyFloor()
{
	if (sp->floorStyle() == NOFLOOR)
		return;

	sp->makeCurrent();
	sp->setFloorStyle(NOFLOOR);
	sp->updateData();
	sp->updateGL();
}

void Graph3D::setMeshLineWidth(double lw)
{
	if (sp->meshLineWidth() == lw)
		return;

	sp->makeCurrent();
	sp->setMeshLineWidth(lw);
	sp->updateData();
	sp->updateGL();
}

int Graph3D::grids()
{
	return sp->coordinates()->grids();
}

void Graph3D::setGrid(Qwt3D::SIDE s, bool b)
{
	if (!sp)
		return;

	int sum = sp->coordinates()->grids();

	if (b)
		sum |= s;
	else
		sum &= ~s;

	sp->coordinates()->setGridLines(sum!=Qwt3D::NOSIDEGRID, true, sum);
	sp->updateGL();
	emit modified();
}

void Graph3D::setGrid(int grids)
{
	if (!sp)
		return;

	sp->coordinates()->setGridLines(true, true, grids);
}

void Graph3D::setLeftGrid(bool b)
{
	setGrid(Qwt3D::LEFT,b);
}
void Graph3D::setRightGrid(bool b)
{
	setGrid(Qwt3D::RIGHT,b);
}
void Graph3D::setCeilGrid(bool b)
{
	setGrid(Qwt3D::CEIL,b);
}
void Graph3D::setFloorGrid(bool b)
{
	setGrid(Qwt3D::FLOOR,b);
}
void Graph3D::setFrontGrid(bool b)
{
	setGrid(Qwt3D::FRONT,b);
}
void Graph3D::setBackGrid(bool b)
{
	setGrid(Qwt3D::BACK,b);
}

void Graph3D::print()
{
	QPrinter printer;
	if (width() > height())
        printer.setOrientation(QPrinter::Landscape);
    else
        printer.setOrientation(QPrinter::Portrait);
	printer.setColorMode (QPrinter::Color);
	printer.setFullPage(false);
	if (printer.setup())
		print(&printer);
}

void Graph3D::print(QPrinter *printer)
{
	if (!printer)
		return;

	QRect rect = printer->pageRect();
	QPainter paint(printer);
	paint.drawPixmap(rect, sp->renderPixmap(rect.width(), rect.height()));
	paint.end();
}

void Graph3D::copyImage()
{
    QApplication::clipboard()->setPixmap(sp->renderPixmap(), QClipboard::Clipboard);
    sp->updateData();
}

void Graph3D::exportImage(const QString& fileName, int quality, bool transparent,
						int dpi, const QSizeF& customSize, int unit, double fontsFactor)
{
    if (!dpi)
		dpi = logicalDpiX();

	QSize size = this->size();
	if (customSize.isValid()){
		switch(unit){
			case FrameWidget::Pixel:
				size = customSize.toSize();
			break;
			case FrameWidget::Inch:
				size = QSize((qRound)(customSize.width()*dpi), (qRound)(customSize.height()*dpi));
			break;
			case FrameWidget::Millimeter:
				size = QSize((qRound)(customSize.width()*dpi/25.4), (qRound)(customSize.height()*dpi/25.4));
			break;
			case FrameWidget::Centimeter:
				size = QSize((qRound)(customSize.width()*dpi/2.54), (qRound)(customSize.height()*dpi/2.54));
			break;
			case FrameWidget::Point:
				size = QSize((qRound)(customSize.width()*dpi/72.0), (qRound)(customSize.height()*dpi/72.0));
			break;
		}
	}

	if (fontsFactor == 0.0)
		fontsFactor = (double)size.height()/(double)this->height();

	scaleFonts(fontsFactor);

	QPixmap pic = sp->renderPixmap(size.width(), size.height());

	scaleFonts(1.0/fontsFactor);

	sp->updateData();
	QImage image = pic.toImage();

	if (transparent){
		QBitmap mask(size);
		mask.fill(Qt::color1);
		QPainter p(&mask);
		p.setPen(Qt::color0);

		QRgb backgroundPixel = QColor(Qt::white).rgb ();
		for (int y = 0; y < image.height(); y++){
			for (int x = 0; x < image.width(); x++){
				QRgb rgb = image.pixel(x, y);
				if (rgb == backgroundPixel) // we want the frame transparent
					p.drawPoint(x, y);
			}
		}
		p.end();
		pic.setMask(mask);
		image = pic.toImage();
	}

	int dpm = (int)ceil(100.0/2.54*dpi);
	image.setDotsPerMeterX(dpm);
	image.setDotsPerMeterY(dpm);
	if (fileName.endsWith(".odf")){
		QTextDocument *document = new QTextDocument();
		QTextCursor cursor = QTextCursor(document);
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(objectName());
		cursor.insertBlock();
		cursor.insertImage(image);

		QTextDocumentWriter writer(fileName);
		writer.write(document);
	} else
		image.save(fileName, 0, quality);
}


void Graph3D::exportImage(QTextDocument *document, int, bool transparent,
						int dpi, const QSizeF& customSize, int unit, double fontsFactor)
{
	if (!document)
		return;

    if (!dpi)
		dpi = logicalDpiX();

	QSize size = this->size();
	if (customSize.isValid()){
		switch(unit){
			case FrameWidget::Pixel:
				size = customSize.toSize();
			break;
			case FrameWidget::Inch:
				size = QSize((qRound)(customSize.width()*dpi), (qRound)(customSize.height()*dpi));
			break;
			case FrameWidget::Millimeter:
				size = QSize((qRound)(customSize.width()*dpi/25.4), (qRound)(customSize.height()*dpi/25.4));
			break;
			case FrameWidget::Centimeter:
				size = QSize((qRound)(customSize.width()*dpi/2.54), (qRound)(customSize.height()*dpi/2.54));
			break;
			case FrameWidget::Point:
				size = QSize((qRound)(customSize.width()*dpi/72.0), (qRound)(customSize.height()*dpi/72.0));
			break;
		}
	}

	if (fontsFactor == 0.0)
		fontsFactor = (double)size.height()/(double)this->height();

	scaleFonts(fontsFactor);

	QPixmap pic = sp->renderPixmap(size.width(), size.height());

	scaleFonts(1.0/fontsFactor);

	sp->updateData();
	QImage image = pic.toImage();

	if (transparent){
		QBitmap mask(size);
		mask.fill(Qt::color1);
		QPainter p(&mask);
		p.setPen(Qt::color0);

		QRgb backgroundPixel = QColor(Qt::white).rgb ();
		for (int y = 0; y < image.height(); y++){
			for (int x = 0; x < image.width(); x++){
				QRgb rgb = image.pixel(x, y);
				if (rgb == backgroundPixel) // we want the frame transparent
					p.drawPoint(x, y);
			}
		}
		p.end();
		pic.setMask(mask);
		image = pic.toImage();
	}

	int dpm = (int)ceil(100.0/2.54*dpi);
	image.setDotsPerMeterX(dpm);
	image.setDotsPerMeterY(dpm);

	QTextCursor cursor = QTextCursor(document);
	cursor.movePosition(QTextCursor::End);
	cursor.insertBlock();
	cursor.insertText(objectName());
	cursor.insertBlock();
	cursor.insertImage(image);
}

void Graph3D::exportPDF(const QString& fileName)
{
	exportVector(fileName);
}

void Graph3D::exportVector(const QString& fileName, int textExportMode, int sortMode)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

    QString format = "PDF";
    if (fileName.endsWith(".eps", Qt::CaseInsensitive))
        format = "EPS";
    else if (fileName.endsWith(".ps", Qt::CaseInsensitive))
        format = "PS";
	else if (fileName.endsWith(".svg", Qt::CaseInsensitive))
        format = "SVG";
    else if (fileName.endsWith(".pgf", Qt::CaseInsensitive))
        format = "PGF";

    VectorWriter * gl2ps = (VectorWriter*)IO::outputHandler(format);
    if (gl2ps){
		gl2ps->setTextMode((VectorWriter::TEXTMODE)textExportMode);
		gl2ps->setLandscape(VectorWriter::OFF);
		gl2ps->setSortMode((VectorWriter::SORTMODE)sortMode);
	}

	IO::save(sp, fileName, format);
}

void Graph3D::exportToFile(const QString& fileName)
{
	if ( fileName.isEmpty() ){
		QMessageBox::critical(this, tr("QtiPlot - Error"), tr("Please provide a valid file name!"));
        return;
	}

	if (fileName.contains(".eps") || fileName.contains(".pdf") ||
		fileName.contains(".ps") || fileName.contains(".svg")){
		exportVector(fileName);
		return;
	} else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
    	for(int i=0 ; i<list.count() ; i++){
			if (fileName.contains( "." + list[i].toLower())){
				exportImage(fileName);
				return;
			}
		}
    	QMessageBox::critical(this, tr("QtiPlot - Error"), tr("File format not handled, operation aborted!"));
	}
}

bool Graph3D::eventFilter(QObject *object, QEvent *e)
{
	if (e->type() == QEvent::MouseButtonDblClick && object == (QObject *)this->sp)
	{
		emit showOptionsDialog();
		return TRUE;
	}
	return MdiSubWindow::eventFilter(object, e);
}

double Graph3D::barsRadius()
{
	if (sp->plotStyle() == Qwt3D::USER && sp->plotStyle() != Qwt3D::POINTS)
		return d_bars_rad;
	else
		return 0.0;
}

void Graph3D::setBarRadius(double rad)
{
	if (d_bars_rad == rad)
		return;

	d_bars_rad = rad;
}

void Graph3D::setBarLines(bool lines)
{
    if (d_bar_lines == lines)
		return;

	d_bar_lines = lines;
}

void Graph3D::setFilledBars(bool filled)
{
    if (d_filled_bars == filled)
		return;

	d_filled_bars = filled;
}

void Graph3D::setDotOptions(double size, bool smooth)
{
	d_point_size = size;
	d_smooth_points = smooth;
}

void Graph3D::setConeOptions(double rad, int quality)
{
	conesRad = rad;
	conesQuality = quality;
}

void Graph3D::setCrossOptions(double rad, double linewidth, bool smooth, bool boxed)
{
	crossHairRad = rad;
	crossHairLineWidth=linewidth;
	crossHairSmooth = smooth;
	crossHairBoxed = boxed;
}

void Graph3D::setStyle(const QStringList& st)
{
	if (st[1] == "nocoord")
		sp->setCoordinateStyle(NOCOORD);
	else if (st[1] == "frame")
		sp->setCoordinateStyle(FRAME);
	else if (st[1] == "box")
		sp->setCoordinateStyle(BOX);

	if (st[2] == "nofloor")
		sp->setFloorStyle(NOFLOOR);
	else if (st[2] == "flooriso")
		sp->setFloorStyle(FLOORISO);
	else if (st[2] == "floordata")
		sp->setFloorStyle(FLOORDATA);

	if (st[3] == "filledmesh")
		setFilledMeshStyle();
	else if (st[3] == "filled")
		setPolygonStyle();
	else if (st[3] == "points") {
		d_point_size = st[4].toDouble();
		d_smooth_points = false;
		if (st[5] == "1")
			d_smooth_points = true;
		setDotStyle();
	} else if (st[3] =="wireframe")
		setWireframeStyle();
	else if (st[3] =="hiddenline")
		setHiddenLineStyle();
	else if (st[3] == "bars") {
		d_bars_rad = (st[4]).toDouble();
		if (st.size() == 7){
            d_bar_lines = (st[5]).toInt();
            d_filled_bars = (st[6]).toInt();
		}
		setBarStyle();
	} else if (st[3] =="cones") {
		conesRad = (st[4]).toDouble();
		conesQuality = (st[5]).toInt();
		setConeStyle();
	} else if (st[3] =="cross") {
		crossHairRad = (st[4]).toDouble();
		crossHairLineWidth = (st[5]).toDouble();
		crossHairSmooth=false;
		if (st[6] == "1")
			crossHairSmooth=true;
		crossHairBoxed=false;
		if (st[7] == "1")
			crossHairBoxed=true;
		setCrossStyle();
	}
}

void Graph3D::customPlotStyle(int style)
{
	sp->makeCurrent();
	if (sp->plotStyle() == style)
		return;

	switch (style)
	{
		case WIREFRAME  :
			{
				sp->setPlotStyle(WIREFRAME);
				style_= WIREFRAME ;
				pointStyle = None;

				legendOn = false;
				sp->showColorLegend(legendOn);
				break;
			}

		case FILLED :
			{
				sp->setPlotStyle(FILLED );
				style_= FILLED;
				pointStyle = None;
				break;
			}

		case FILLEDMESH  :
			{
				sp->setPlotStyle(FILLEDMESH);
				style_= FILLEDMESH;
				pointStyle = None;
				break;
			}

		case HIDDENLINE:
			{
				sp->setPlotStyle(HIDDENLINE);
				style_= HIDDENLINE;
				pointStyle = None;
				legendOn = false;
				sp->showColorLegend(legendOn);
				break;
			}

		case Qwt3D::POINTS:
			{
				d_point_size = 5;
				d_smooth_points = true;
				pointStyle=Dots;
				style_ = Qwt3D::USER;

				Dot d(d_point_size, d_smooth_points);
				sp->setPlotStyle(d);
				break;
			}

		case Qwt3D::USER:
			{
				pointStyle = VerticalBars;
				style_ = Qwt3D::USER;
				sp->setPlotStyle(Bar(d_bars_rad));
				break;
			}
	}

	sp->updateData();
	sp->updateGL();
}

void Graph3D::setRotation(double xVal, double yVal, double zVal)
{
	sp->setRotation(xVal, yVal, zVal);
}

void Graph3D::setZoom(double val)
{
    if (sp->zoom() == val)
        return;

    sp->setZoom(val);
}

void Graph3D::setScale(double  xVal, double  yVal, double  zVal)
{
    if (sp->xScale() == xVal && sp->yScale() == yVal && sp->zScale() == zVal)
        return;

	sp->setScale(xVal, yVal, zVal);
}

void Graph3D::setShift(double  xVal,double  yVal,double  zVal)
{
	sp->setShift(xVal, yVal, zVal);
}

Qwt3D::PLOTSTYLE Graph3D::plotStyle()
{
	return sp->plotStyle();
}

Qwt3D::FLOORSTYLE Graph3D::floorStyle()
{
	return sp->floorStyle();
}

Qwt3D::COORDSTYLE Graph3D::coordStyle()
{
	return sp->coordinates()->style();
}

QString Graph3D::formula()
{
	if (d_func)
		return d_func->function();
	else
		return plotAssociation;
}

void Graph3D::save(const QString &fn, const QString &geometry, bool)
{
	QFile f(fn);
	if (!f.isOpen()){
		if (!f.open(QIODevice::Append))
			return;
	}
	QTextStream t( &f );
	t.setEncoding(QTextStream::UnicodeUTF8);
	t << "<SurfacePlot>\n";
	t << QString(name())+"\t";
	t << birthDate() + "\n";
	t << geometry;
	t << "SurfaceFunction\t";

	sp->makeCurrent();
	if (d_func)
		t << d_func->function() + ";" + QString::number(d_func->columns()) + ";" + QString::number(d_func->rows()) + "\t";
	else if (d_surface){
		t << d_surface->xFormula() + "," + d_surface->yFormula() + "," + d_surface->zFormula() + ",";
		t << QString::number(d_surface->uStart(), 'e', 15) + ",";
		t << QString::number(d_surface->uEnd(), 'e', 15) + ",";
		t << QString::number(d_surface->vStart(), 'e', 15) + ",";
		t << QString::number(d_surface->vEnd(), 'e', 15) + ",";
		t << QString::number(d_surface->columns()) + ",";
		t << QString::number(d_surface->rows()) + ",";
		t << QString::number(d_surface->uPeriodic()) + ",";
		t << QString::number(d_surface->vPeriodic());
	} else {
		t << plotAssociation;
		t << "\t";
	}

	double start,stop;
	sp->coordinates()->axes[X1].limits(start,stop);
	t << QString::number(start)+"\t";
	t << QString::number(stop)+"\t";
	sp->coordinates()->axes[Y1].limits(start,stop);
	t << QString::number(start)+"\t";
	t << QString::number(stop)+"\t";
	sp->coordinates()->axes[Z1].limits(start,stop);
	t << QString::number(start)+"\t";
	t << QString::number(stop)+"\n";

	QString st;

	if (sp->coordinates()->style() == Qwt3D::NOCOORD)
		st="nocoord";
	else if (sp->coordinates()->style() == Qwt3D::BOX)
		st="box";
	else
		st="frame";
	t << "Style\t" + st + "\t";

	switch(sp->floorStyle ())
	{
		case NOFLOOR:
			st="nofloor";
			break;

		case FLOORISO:
			st="flooriso";
			break;

		case FLOORDATA:
			st="floordata";
			break;
	}
	t << st+"\t";

	switch(sp->plotStyle())
	{
		case USER:
			if (pointStyle == VerticalBars) {
				st = "bars\t" + QString::number(d_bars_rad);
				st += "\t" + QString::number(d_bar_lines);
				st += "\t" + QString::number(d_filled_bars);
			} else if (pointStyle == Dots){
				st="points\t"+QString::number(d_point_size);
				st+="\t"+QString::number(d_smooth_points);
			} else if (pointStyle == Cones) {
				st="cones\t"+QString::number(conesRad);
				st+="\t"+QString::number(conesQuality);
			} else if (pointStyle == HairCross) {
				st="cross\t"+QString::number(crossHairRad);
				st+="\t"+QString::number(crossHairLineWidth);
				st+="\t"+QString::number(crossHairSmooth);
				st+="\t"+QString::number(crossHairBoxed);
			}
			break;

		case WIREFRAME:
			st="wireframe";
			break;

		case HIDDENLINE:
			st="hiddenline";
			break;

		case FILLED:
			st="filled";
			break;

		case FILLEDMESH:
			st="filledmesh";
			break;

		default:
			;
	}
	t << st + "\n";
	t << "grids\t";
	t << QString::number(sp->coordinates()->grids())+"\n";

	t << "title\t";
	t << title+"\t";
	t << titleCol.name()+"\t";
	t << titleFnt.family()+"\t";
	t << QString::number(titleFnt.pointSize())+"\t";
	t << QString::number(titleFnt.weight())+"\t";
	t << QString::number(titleFnt.italic())+"\n";

	t << "colors\t";
	t << meshCol.name()+"\t";
	t << axesCol.name()+"\t";
	t << numCol.name()+"\t";
	t << labelsCol.name()+"\t";
	t << bgCol.name()+"\t";
	t << gridCol.name()+"\t";
	t << d_color_map.color1().name()+"\t"; // obsolete: saved for compatibility with files older than 0.9.7.3
	t << d_color_map.color2().name()+"\t"; // obsolete: saved for compatibility with files older than 0.9.7.3
	t << QString::number(d_alpha) + "\t" + d_color_map_file + "\n";

	t << "axesLabels\t";
	t << labels.join("\t")+"\n";

	t << "tics\t";
	QStringList tl=scaleTicks();
	t << tl.join("\t")+"\n";

	t << "tickLengths\t";
	tl=axisTickLengths();
	t << tl.join("\t")+"\n";

	t << "options\t";
	t << QString::number(legendOn)+"\t";
	t << QString::number(sp->resolution())+"\t";
	t << QString::number(labelsDist)+"\n";

	t << "numbersFont\t";
	QFont fnt=sp->coordinates()->axes[X1].numberFont();
	t << fnt.family()+"\t";
	t << QString::number(fnt.pointSize())+"\t";
	t << QString::number(fnt.weight())+"\t";
	t << QString::number(fnt.italic())+"\n";

	t << "xAxisLabelFont\t";
	fnt=sp->coordinates()->axes[X1].labelFont();
	t << fnt.family()+"\t";
	t << QString::number(fnt.pointSize())+"\t";
	t << QString::number(fnt.weight())+"\t";
	t << QString::number(fnt.italic())+"\n";

	t << "yAxisLabelFont\t";
	fnt=sp->coordinates()->axes[Y1].labelFont();
	t << fnt.family()+"\t";
	t << QString::number(fnt.pointSize())+"\t";
	t << QString::number(fnt.weight())+"\t";
	t << QString::number(fnt.italic())+"\n";

	t << "zAxisLabelFont\t";
	fnt=sp->coordinates()->axes[Z1].labelFont();
	t << fnt.family()+"\t";
	t << QString::number(fnt.pointSize())+"\t";
	t << QString::number(fnt.weight())+"\t";
	t << QString::number(fnt.italic())+"\n";

	t << "rotation\t";
	t << QString::number(sp->xRotation())+"\t";
	t << QString::number(sp->yRotation())+"\t";
	t << QString::number(sp->zRotation())+"\n";

	t << "zoom\t";
	t << QString::number(sp->zoom())+"\n";

	t << "scaling\t";
	t << QString::number(sp->xScale())+"\t";
	t << QString::number(sp->yScale())+"\t";
	t << QString::number(sp->zScale())+"\n";

	t << "shift\t";
	t << QString::number(sp->xShift())+"\t";
	t << QString::number(sp->yShift())+"\t";
	t << QString::number(sp->zShift())+"\n";

	t << "LineWidth\t";
	t << QString::number(sp->meshLineWidth())+"\n";
	t << "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
	t << "Orthogonal\t" + QString::number(sp->ortho())+"\n";
	if (d_color_map_file.isEmpty())
		t << ColorMapEditor::saveToXmlString(d_color_map);
	t << "</SurfacePlot>\n";
}

void Graph3D::showColorLegend(bool show)
{
	if (legendOn == show)
		return;

	sp->makeCurrent();
	sp->showColorLegend(show);
	legendOn = show;
	sp->updateGL();
	emit modified();
}

void Graph3D::setResolution(int r)
{
	if (sp->resolution() == r)
		return;

	sp->makeCurrent();
	sp->setResolution(r);
	sp->updateData();
	sp->updateGL();
	emit modified();
}

void Graph3D::setTitle(const QStringList& lst)
{
	title=lst[1];
	sp->setTitle(title);

	titleCol=QColor(lst[2]);
	sp->setTitleColor(Qt2GL(titleCol));

	titleFnt=QFont(lst[3],lst[4].toInt(),lst[5].toInt(),lst[6].toInt());
	sp->setTitleFont(titleFnt.family(),titleFnt.pointSize(),titleFnt.weight(),titleFnt.italic());
}

void Graph3D::setTitle(const QString& s,const QColor& color,const QFont& font)
{
	if (title != s){
		title=s;
		sp->setTitle(title);
	}

	titleCol=color;
	sp->setTitleColor(Qt2GL(color));

	if (titleFnt != font){
		titleFnt=font;
		sp->setTitleFont(font.family(),font.pointSize(),font.weight(),font.italic());
	}
}

void Graph3D::setTitleFont(const QFont& font)
{
	if (titleFnt != font)
	{
		titleFnt=font;
		sp->setTitleFont(font.family(),font.pointSize(),font.weight(),font.italic());
	}
}

void Graph3D::setOptions(const QStringList& lst)
{
	legendOn=false;
	if (lst[1].toInt() == 1)
		legendOn=true;
	sp->showColorLegend(legendOn);
	sp->setResolution(lst[2].toInt());
	setLabelsDistance(lst[3].toInt());
}

void Graph3D::setOptions(bool legend, int r, int dist)
{
	sp->showColorLegend(legend);
	legendOn = legend;
	sp->setResolution(r);
	setLabelsDistance(dist);
}

void Graph3D::setDataColorMap(const QwtLinearColorMap& colorMap)
{
	d_color_map = colorMap;
	d_color_map_file = QString::null;

	double zmin, zmax;
	sp->coordinates()->axes[Z1].limits (zmin, zmax);
	const QwtDoubleInterval intensityRange = QwtDoubleInterval(zmin, zmax);

	int size = 255;
	double dsize = size;
	double dz = fabs(zmax - zmin)/dsize;
	Qwt3D::ColorVector cv;
	for (int i = 0; i < size; i++){
		QRgb color = colorMap.rgb(intensityRange, zmin + i*dz);
		RGBA rgb(qRed(color)/dsize, qGreen(color)/dsize, qBlue(color)/dsize, d_alpha);
		cv.push_back(rgb);
	}

	col_ = new StandardColor(sp);
	col_->setColorVector(cv);
	sp->setDataColor(col_);

	if (legendOn){
		sp->showColorLegend(false);
		sp->showColorLegend(legendOn);
	}
}

void Graph3D::setDataColorMap(const ColorVector& colors, const QwtLinearColorMap& colorMap)
{
	d_color_map = colorMap;
	d_color_map_file = QString::null;

	setDataColorMap(colors);
}

void Graph3D::changeTransparency(double t)
{
	if (d_alpha == t)
		return;

	d_alpha = t;

	Qwt3D::StandardColor* color=(StandardColor*) sp->dataColor ();
	color->setAlpha(t);

    sp->showColorLegend(legendOn);
	sp->updateData();
	sp->updateGL();
	emit modified();
}

void Graph3D::setTransparency(double t)
{
	if (d_alpha == t)
		return;

	d_alpha = t;

	Qwt3D::StandardColor* color=(StandardColor*) sp->dataColor ();
	color->setAlpha(t);
}

void Graph3D::showWorksheet()
{
	if (d_table)
		d_table->showMaximized();
	else if (d_matrix)
		d_matrix->showMaximized();
}

void Graph3D::setAntialiasing(bool smooth)
{
    sp->makeCurrent();
	sp->setSmoothMesh(smooth);
	sp->coordinates()->setLineSmooth(smooth);
	if (d_table_plot_type == Bars && pointStyle == VerticalBars)
        setBarStyle();
    else {
        sp->updateData();
        sp->updateGL();
    }
}

/*!
Turns 3D animation on or off
*/
void Graph3D::animate(bool on)
{
if ( on )
    d_timer->start( animation_redraw_wait ); // Wait this many msecs before redraw
else
    d_timer->stop();
}

void Graph3D::rotate()
{
if (!sp)
   return;

sp->setRotation(int(sp->xRotation() + 1) % 360, int(sp->yRotation() + 1) % 360, int(sp->zRotation() + 1) % 360);
}

void Graph3D::setDataColorMap(const QString& fileName)
{
	if (d_color_map_file == fileName)
	   return;

	ColorVector cv;
	if (!openColorMapFile(cv, fileName))
	   return;

	d_color_map_file = fileName;
	setDataColorMap(cv);
}

void Graph3D::setDataColorMap(const ColorVector& colors)
{
	col_ = new StandardColor(sp);
	col_->setColorVector(colors);

	sp->setDataColor(col_);
	sp->updateData();
	sp->showColorLegend(legendOn);
	sp->updateGL();
}

bool Graph3D::openColorMapFile(ColorVector& cv, QString fname)
{
if (fname.isEmpty())
   return false;

using std::ifstream;
ifstream file(fname.toLocal8Bit());
if (!file)
   return false;

RGBA rgb;
cv.clear();

while ( file ) {
      file >> rgb.r >> rgb.g >> rgb.b;
      file.ignore(10000,'\n');
      if (!file.good())
         break;
      else {
          rgb.a = 1;
          rgb.r /= 255;
          rgb.g /= 255;
          rgb.b /= 255;
          cv.push_back(rgb);
          }
      }
return true;
}

void Graph3D::findBestLayout()
{
  	double start, end;
  	sp->coordinates()->axes[X1].limits (start, end);
  	double xScale = 1/fabs(end - start);

  	sp->coordinates()->axes[Y1].limits (start, end);
  	double yScale = 1/fabs(end - start);

  	sp->coordinates()->axes[Z1].limits (start, end);
  	double zScale = 1/fabs(end - start);

  	double d = (sp->hull().maxVertex - sp->hull().minVertex).length();
  	sp->setScale(xScale, yScale, zScale);

  	sp->setZoom(d/sqrt(3.));

	sp->setShift(0, 0, 0);

  	double majl = 0.1/yScale;
  	setAxisTickLength(0, majl, 0.6*majl);
  	majl = 0.1/xScale;
  	setAxisTickLength(1, majl, 0.6*majl);
  	setAxisTickLength(2, majl, 0.6*majl);
}

void Graph3D::copy(Graph3D* g)
{
	if (!g)
        return;

	QString s = g->formula();

	if (g->userFunction()){
		UserFunction *f = g->userFunction();
		addFunction(f->function(), g->xStart(), g->xStop(), g->yStart(), g->yStop(),
					g->zStart(), g->zStop(), f->columns(), f->rows());
	} else if (g->parametricSurface()){
		UserParametricSurface *ps = g->parametricSurface();
		addParametricSurface(ps->xFormula(), ps->yFormula(), ps->zFormula(), ps->uStart(), ps->uEnd(),
				ps->vStart(), ps->vEnd(), ps->columns(), ps->rows(), ps->uPeriodic(), ps->vPeriodic());
	} else if (s.endsWith("(Z)")){
		Table *t = g->table();
		if (!t)
			return;
		s.remove("(X)").remove("(Y)").remove("(Z)");
		QStringList l = s.split(",");
		if (l.size() == 3)
			loadData(t, t->colIndex(l[0]), t->colIndex(l[1]), t->colIndex(l[2]),
			g->xStart(),g->xStop(), g->yStart(),g->yStop(),g->zStart(),g->zStop());
	} else if (s.endsWith("(Y)")){//Ribbon plot
		s.remove("(X)").remove("(Y)");
		QStringList l = s.split(",");
		if (l.size() == 2)
			addRibbon(g->table(), l[0], l[1],g->xStart(),g->xStop(), g->yStart(),g->yStop(),g->zStart(),g->zStop());
	} else
		addMatrixData(g->matrix(), g->xStart(), g->xStop(), g->yStart(), g->yStop(),g->zStart(),g->zStop());


	pointStyle = g->pointType();
	style_ = g->plotStyle();
	if (g->plotStyle() == Qwt3D::USER ){
		switch (pointStyle){
			case None :
				sp->setPlotStyle(g->plotStyle());
			break;

			case Dots :
				d_point_size = g->pointsSize();
				d_smooth_points = g->smoothPoints();
				sp->setPlotStyle(Dot(d_point_size, d_smooth_points));
			break;

			case VerticalBars :
				setBarRadius(g->barsRadius());
				d_bar_lines = g->barLines();
				d_filled_bars = g->filledBars();
				sp->setPlotStyle(Bar(d_bars_rad, d_bar_lines, d_filled_bars));
				break;

			case HairCross :
				setCrossOptions(g->crossHairRadius(), g->crossHairLinewidth(), g->smoothCrossHair(), g->boxedCrossHair());
				sp->setPlotStyle(CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth, crossHairBoxed));
				break;

			case Cones :
				setConeOptions(g->coneRadius(), g->coneQuality());
				sp->setPlotStyle(Cone3D(conesRad, conesQuality));
				break;
		}
	} else
		customPlotStyle(style_);

	sp->setCoordinateStyle(g->coordStyle());
	sp->setFloorStyle(g->floorStyle());

	setGrid(g->grids());
	setTitle(g->plotTitle(),g->titleColor(),g->titleFont());
	setTransparency(g->transparency());
	if (!g->colorMapFile().isEmpty())
		setDataColorMap(g->colorMapFile());
	else
		setDataColorMap(g->colorMap());

	setMeshColor(g->meshColor());
	setAxesColor(g->axesColor());
	setNumbersColor(g->numColor());
	setLabelsColor(g->labelColor());
	setBackgroundColor(g->bgColor());
	setGridColor(g->gridColor());

	setAxesLabels(g->axesLabels());
	setTicks(g->scaleTicks());
	setTickLengths(g->axisTickLengths());
	setOptions(g->isLegendOn(), g->resolution(), g->labelsDistance());
	setNumbersFont(g->numbersFont());
	setXAxisLabelFont(g->xAxisLabelFont());
	setYAxisLabelFont(g->yAxisLabelFont());
	setZAxisLabelFont(g->zAxisLabelFont());
	setRotation(g->xRotation(),g->yRotation(),g->zRotation());
	setZoom(g->zoom());
	setScale(g->xScale(),g->yScale(),g->zScale());
	setShift(g->xShift(),g->yShift(),g->zShift());
	setMeshLineWidth(g->meshLineWidth());

	bool smooth = g->antialiasing();
    sp->setSmoothMesh(smooth);
	sp->coordinates()->setLineSmooth(smooth);

	setOrthogonal(g->isOrthogonal());

	sp->updateData();
	sp->updateGL();
	animate(g->isAnimated());
}

Graph3D* Graph3D::restore(ApplicationWindow* app, const QStringList &lst, int fileVersion)
{
	QStringList fList=lst[0].split("\t");
	QString caption=fList[0];
	QString date=fList[1];
	if (date.isEmpty())
		date = QDateTime::currentDateTime().toString(Qt::LocalDate);

	fList=lst[2].split("\t", QString::SkipEmptyParts);

	Graph3D *plot = app->newPlot3D(caption);

	ApplicationWindow::restoreWindowGeometry(app, plot, lst[1]);
	QString formula = fList[1];
	if (formula.endsWith("(Y)",true)){//Ribbon plot
		Table* t = app->table(formula.left(formula.find("_", 0)));
		if (!t)
			return 0;

		formula.remove("(X)").remove("(Y)");
		QStringList l = formula.split(",");
		if (l.size() == 2)
			plot->addRibbon(t, l[0], l[1], fList[2].toDouble(), fList[3].toDouble(),
			fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(), fList[7].toDouble());
	} else if (formula.contains("(Z)",true) > 0){
		Table* t = app->table(formula.left(formula.find("_", 0)));
		if (!t)
			return 0;

		plot->show();
		formula.remove("(X)").remove("(Y)").remove("(Z)");
		QStringList l = formula.split(",");
		if (l.size() == 3)
			plot->loadData(t, t->colIndex(l[0]), t->colIndex(l[1]), t->colIndex(l[2]),
					   fList[2].toDouble(),fList[3].toDouble(), fList[4].toDouble(),
					   fList[5].toDouble(),fList[6].toDouble(),fList[7].toDouble());
	} else if (formula.startsWith("matrix<",true) && fList[1].endsWith(">",false)){
		formula.remove("matrix<", true).remove(">");
		Matrix* m = app->matrix(formula);
		if (!m)
			return 0;

		plot->addMatrixData(m, fList[2].toDouble(),fList[3].toDouble(),
				fList[4].toDouble(),fList[5].toDouble(),fList[6].toDouble(),fList[7].toDouble());
	} else if (formula.contains(",")){
		QStringList l = formula.split(",", QString::SkipEmptyParts);
		plot->addParametricSurface(l[0], l[1], l[2], l[3].toDouble(), l[4].toDouble(),
				l[5].toDouble(), l[6].toDouble(), l[7].toInt(), l[8].toInt(), l[9].toInt(), l[10].toInt());
	} else {
		QStringList l = formula.split(";", QString::SkipEmptyParts);
		if (l.count() == 1)
			plot->addFunction(formula, fList[2].toDouble(), fList[3].toDouble(),
				fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(), fList[7].toDouble());
		else if (l.count() == 3)
			plot->addFunction(l[0], fList[2].toDouble(), fList[3].toDouble(), fList[4].toDouble(),
					fList[5].toDouble(), fList[6].toDouble(), fList[7].toDouble(), l[1].toInt(), l[2].toInt());
	}

	if (!plot)
		return 0;

	app->setListViewDate(caption, date);
	plot->setBirthDate(date);
	plot->setIgnoreFonts(true);

	fList=lst[4].split("\t", QString::SkipEmptyParts);
	plot->setGrid(fList[1].toInt());

	plot->setTitle(lst[5].split("\t"));

	QStringList colors = lst[6].split("\t", QString::SkipEmptyParts);
	plot->setMeshColor(QColor(colors[1]));
	plot->setAxesColor(QColor(colors[2]));
	plot->setNumbersColor(QColor(colors[3]));
	plot->setLabelsColor(QColor(colors[4]));
	plot->setBackgroundColor(QColor(colors[5]));
	plot->setGridColor(QColor(colors[6]));

	if ((int)colors.count() > 7){
		plot->setTransparency(colors[9].toDouble());
		if ((int)colors.count() == 11)
			plot->setDataColorMap(colors[10]);
		else
			plot->setDataColors(QColor(colors[7]), QColor(colors[8]));
	}

	fList = lst[7].split("\t", QString::SkipEmptyParts);
	fList.pop_front();
	plot->setAxesLabels(fList);

	plot->setTicks(lst[8].split("\t", QString::SkipEmptyParts));
	plot->setTickLengths(lst[9].split("\t", QString::SkipEmptyParts));
	plot->setOptions(lst[10].split("\t", QString::SkipEmptyParts));

	QStringList fLst = lst[11].split("\t", QString::SkipEmptyParts);
	plot->setNumbersFont(QFont(fLst[1], fLst[2].toInt(), fLst[3].toInt(), fLst[4].toInt()));

	plot->setXAxisLabelFont(lst[12].split("\t", QString::SkipEmptyParts));
	plot->setYAxisLabelFont(lst[13].split("\t", QString::SkipEmptyParts));
	plot->setZAxisLabelFont(lst[14].split("\t", QString::SkipEmptyParts));

	fList=lst[15].split("\t", QString::SkipEmptyParts);
	plot->setRotation(fList[1].toDouble(),fList[2].toDouble(),fList[3].toDouble());

	fList=lst[16].split("\t", QString::SkipEmptyParts);
	plot->setZoom(fList[1].toDouble());

	fList=lst[17].split("\t", QString::SkipEmptyParts);
	plot->setScale(fList[1].toDouble(),fList[2].toDouble(),fList[3].toDouble());

	fList=lst[18].split("\t", QString::SkipEmptyParts);
	plot->setShift(fList[1].toDouble(),fList[2].toDouble(),fList[3].toDouble());

	fList=lst[19].split("\t", QString::SkipEmptyParts);
	plot->setMeshLineWidth(fList[1].toDouble());

	if (fileVersion > 71){
		fList=lst[20].split("\t"); // using QString::SkipEmptyParts here causes a crash for empty window labels
		plot->setWindowLabel(fList[1]);
		plot->setCaptionPolicy((MdiSubWindow::CaptionPolicy)fList[2].toInt());
	}

	if (fileVersion >= 88){
		fList=lst[21].split("\t", QString::SkipEmptyParts);
		plot->setOrthogonal(fList[1].toInt());
	}

	plot->setStyle(lst[3].split("\t", QString::SkipEmptyParts));
	plot->setIgnoreFonts(true);

	QListIterator<QString> line = QListIterator<QString>(lst);
	if (!line.findNext("<ColorMap>")){
		plot->update();
		return plot;
	}
	QString s = line.next().stripWhiteSpace();
	int mode = s.remove("<Mode>").remove("</Mode>").stripWhiteSpace().toInt();
	s = line.next();
	QColor color1 = QColor(s.remove("<MinColor>").remove("</MinColor>").stripWhiteSpace());
	s = line.next();
	QColor color2 = QColor(s.remove("<MaxColor>").remove("</MaxColor>").stripWhiteSpace());

	QwtLinearColorMap colorMap = QwtLinearColorMap(color1, color2);
	colorMap.setMode((QwtLinearColorMap::Mode)mode);

	s = line.next();
	int stops = s.remove("<ColorStops>").remove("</ColorStops>").stripWhiteSpace().toInt();
	for (int i = 0; i < stops; i++){
		s = line.next().stripWhiteSpace();
		QStringList l = QStringList::split("\t", s.remove("<Stop>").remove("</Stop>"));
		colorMap.addColorStop(l[0].toDouble(), QColor(l[1]));
	}
	plot->setDataColorMap(colorMap);
	plot->update();
	return plot;
}

void Graph3D::dragEnterEvent( QDragEnterEvent* e )
{
	if (e->mimeData()->hasFormat("text/plain"))
		e->acceptProposedAction();
}

void Graph3D::dropEvent(QDropEvent* event)
{
	Table *t = qobject_cast<Table*>(event->source());
	if (t){
		QStringList columns = event->mimeData()->text().split("\n");
		if (columns.isEmpty())
			return;

		int zcol = t->colIndex(columns[0]);
		if (t->colPlotDesignation(zcol) != Table::Z)
			return;

		addData(t, t->colX(zcol), t->colY(zcol), zcol, Trajectory);
		return;
	}

	Matrix *m = qobject_cast<Matrix*>(event->source());
	if (m)
		addMatrixData(m);
}

Graph3D::~Graph3D()
{
	if (d_surface)
		delete d_surface;
	if (d_func)
		delete d_func;

	delete sp;
}
