#ifndef __plot_h__
#define __plot_h__

#include <QVector>

#include "qwt3d_coordsys.h"
#include "qwt3d_extglwidget.h"
#include "qwt3d_enrichment_std.h"

namespace Qwt3D
{
    class Curve;
  
//! Base class for all plotting widgets
/*!
  Plot3D handles all the common features for plotting widgets beyond the low-level extensions of 
  ExtGLWidget - coordinate system, labeling and more. It contains some pure virtual functions and is,
  in this respect, an abstract base class. It's nevertheless no pure interface.
	The class provides interfaces for data handling and implements basic data controlled color allocation.
*/
class QWT3D_EXPORT Plot3D : public ExtGLWidget
{
    Q_OBJECT

public:
	
#if QT_VERSION < 0x040000
	Plot3D( QWidget* parent = 0, const char* name = 0 );
#else
	Plot3D ( QWidget * parent = 0, const QGLWidget * shareWidget = 0 );
#endif
	virtual ~Plot3D();

	void setCurve(Qwt3D::Curve* c)				{ curve_p = c; }
	void setTitle(Qwt3D::Label* l)				{ title_p = l; }
	Qwt3D::Curve* curve() const					{ return curve_p; }
	Qwt3D::Label* title() const					{ return title_p; }
	void manageConnect(bool connect, Qwt3D::Curve* curve = 0);
	
	QPixmap renderPixmap (int w = 0, int h = 0, bool useContext = false);	  
	void createCoordinateSystem(Qwt3D::Triple beg, Qwt3D::Triple end);
	
	Qwt3D::CoordinateSystem* coordinates()		{ return &coordinates_p; }	//!< Returns pointer to CoordinateSystem object

	void setBackgroundColor(Qwt3D::RGBA rgba)	{ bgcolor_ = rgba; }		//!< Sets widgets background color
	Qwt3D::RGBA backgroundRGBAColor() const		{ return bgcolor_; }		//!< Returns the widgets background color

	void setDoubleLegend(bool mode = true)		{ doublelegend_ = mode; }	//!< Sets legend drawing mode
	bool isDoubleLegend() const					{ return doublelegend_; }	//!< Returns the legend drawing mode

	void calculateHull();
	Qwt3D::ParallelEpiped hull() const			{ return hull_;}			//!< Returns rectangular hull   

	void setCoordinateStyle(Qwt3D::COORDSTYLE st);							//!< Sets style of coordinate system.

	//! Returns true if valid data available, else false
	bool hasData() const						{ return (!curvelist_p.empty()); }

	void addCurve(Qwt3D::Curve* curve);
	void addDrawable(Qwt3D::Drawable* drawable);
	void addTitle(Qwt3D::Label* label);

	bool removeCurve(Qwt3D::Curve* curve);
	bool removeDrawable(Qwt3D::Drawable* drawable);
	bool removeTitle(Qwt3D::Label* label);
	
	void createCoordinateSystem();
	void normaliseScale(Qwt3D::Curve* curve, Qwt3D::Plot3D* parentplot = 0, Qwt3D::ParallelEpiped* curvehull = 0);

	typedef QVector<Qwt3D::Curve*> CurveList;
	const CurveList& curveList() const			{ return curvelist_p; }

	typedef QVector<Qwt3D::Drawable*> DrawableList;
	const DrawableList& drawableList() const	{ return drawablelist_p; }

	typedef QVector<Qwt3D::Label*> TitleList;
	const TitleList& titleList() const			{ return titlelist_p; }

	bool isExportingVector(){return d_exporting_vector;};
	GLint vectorExportFormat(){return gl2ps_export_format_;};
	void setExportingVector(bool on = true, GLint format = 0){d_exporting_vector = on; gl2ps_export_format_ = format;};

signals:
	// Internally sent signals through signal slot mechanism
	void setCurveResolution(int);								//!< Emits, resolution change to all connected curves
	void setCurvePolygonOffset(double d);						//!< Emits, polygon offset change to all connected curves
	void setCurveIsolines(unsigned int isolines);				//!< Emits, number of isolines change to all connected curves
	void setCurvePlotStyle(Qwt3D::PLOTSTYLE val);				//!< Emits, plotting style change to all connected curves
	void setCurvePlotStyle(Qwt3D::Enrichment& val);				//!< Emits, user defined plotting style change to all connected curves
	void setCurveFloorStyle(Qwt3D::FLOORSTYLE val);				//!< Emits, floor style change to all connected curves

	void showCurveColorLegend(bool on);							//!< Emits, show colour legend mode to all connected curves
	void showCurveNormals(bool on); 							//!< Emits, draw normals to every vertex to all connected curves
	void setCurveNormalLength(double val);						//!< Emits, set length of normals in percent per hull diagonale to all connected curves
	void setCurveNormalQuality(int val);						//!< Emits, increases plotting quality of normal arrows to all connected curves

	void setCurveSmoothMesh(bool val);							//!< Emits, smooth data mesh lines setting to all connected curves
	void setCurveMeshColor(Qwt3D::RGBA rgba);					//!< Emits, colour for data mesh to all connected curves
	void setCurveMeshLineWidth(double lw);						//!< Emits, line width for data mesh to all connected curves

	void setCurveDataColor(Color* col);							//!< Emits, data colour change to all connected curves
	void setCurveDataProjection(bool on);						//!< Emits, data point projection on & off to all connected curves
	void setCurveProjection(Qwt3D::PROJECTMODE val, bool on);	//!< Emits, projection mode to all connected curves
	void setCurveShading(Qwt3D::SHADINGSTYLE val);				//!< Emits, shading mode to all connected curves

	void setCurveTitleColor(Qwt3D::RGBA col);					//!< Emits, caption color
	void setCurveTitle(const QString& title);					//!< Emits, caption text (one row only)
	void setCurveTitlePosition(double rely, double relx, Qwt3D::ANCHOR anchor);
	void setCurveTitleFont(const QString& family, int pointSize, int weight, bool italic);

	void createEnrichments();
	void createData();

public slots:
	void updateData(bool coord = true);							//!< Recalculate data

	void setResolution(int val)							{ emit setCurveResolution(val); }		//!< Set resolution for all attached curves
	void setPolygonOffset(double d)						{ emit setCurvePolygonOffset(d); }		//!< Set polygon offset for all attached curves
	void setIsolines(unsigned int isolines)				{ emit setCurveIsolines(isolines); }	//!< Set number of isolines for all attached curves
	void setPlotStyle(Qwt3D::PLOTSTYLE val)				{ emit setCurvePlotStyle(val); }		//!< Set plot style for all attached curves
	void setPlotStyle(Qwt3D::Enrichment& val)			{ emit setCurvePlotStyle(val); }		//!< Set user defined plot style for all attached curves
	void setFloorStyle(Qwt3D::FLOORSTYLE val)			{ emit setCurveFloorStyle(val); }		//!< Set floor style for all attached curves
	void setShading(Qwt3D::SHADINGSTYLE val)			{ emit setCurveShading(val); }			//!< Set shading modes for all attached curves

	void showColorLegend(bool on) 						{ emit showCurveColorLegend(on); }		//!< Draw colour legend for all attached curves
	void showNormals(bool on) 							{ emit showCurveNormals(on); }			//!< Draw normals to every vertex for all attached curves
	void setNormalLength(double val)					{ emit setCurveNormalLength(val); }		//!< Set length of normals in percent per hull diagonale for all attached curves
	void setNormalQuality(int val)						{ emit setCurveNormalQuality(val); }	//!< Set plotting quality of normal arrows for all attached curves

	void setSmoothMesh(bool val)						{ emit setCurveSmoothMesh(val); } 		//!< Enable/disable smooth data mesh lines for all attached curves
	void setMeshColor(Qwt3D::RGBA rgba)					{ emit setCurveMeshColor(rgba); }		//!< Set colour for data mesh for all attached curves
	void setMeshLineWidth(double lw)					{ emit setCurveMeshLineWidth(lw); }		//!< Set line width for data mesh for all attached curves

	void setDataColor(Color* col)						{ emit setCurveDataColor(col); }		//!< Set data colour for all attached curves
	void setDataProjection(bool on)						{ emit setCurveDataProjection(on); }	//!< Set data point projection on & off for all attached curves
	void setProjection(Qwt3D::PROJECTMODE val, bool on)	{ emit setCurveProjection(val, on); }	//!< Set projection modes for all attached curves

	void setTitleColor(Qwt3D::RGBA col)					{ emit setCurveTitleColor(col); }		//!< Set caption color
	void setTitle(const QString& title)					{ emit setCurveTitle(title); }			//!< Set caption text (one row only)
	void setTitlePosition(double rely, double relx = 0.5, Qwt3D::ANCHOR anchor = Qwt3D::TopCenter)
														{ emit setCurveTitlePosition(rely, relx, anchor); }		//!< Set caption text position
	void setTitleFont(const QString& family, int pointSize, int weight = QFont::Normal, bool italic = false)
														{ emit setCurveTitleFont(family, pointSize, weight, italic); }	//!< Set caption text font

    virtual bool savePixmap(QString const& fileName, QString const& format);					//!<  Saves content to pixmap format
    virtual bool saveVector(QString const& fileName, QString const& format,
    						VectorWriter::TEXTMODE text, VectorWriter::SORTMODE sortmode);		//!<  Saves content to vector format
	virtual bool save(QString const& fileName, QString const& format);							//!<  Saves content

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
	void setHull(Qwt3D::ParallelEpiped p) {hull_ = p;}

	enum OBJECTS {
		LegendObject,
		NormalObject,
		DisplayListSize // only to have a vector length ...
	};

	std::vector<GLuint>		displaylists_p;
	Qwt3D::CoordinateSystem coordinates_p;

    Qwt3D::Curve*	curve_p;
	Qwt3D::Label*	title_p;

    CurveList		curvelist_p;
    DrawableList	drawablelist_p;
    TitleList		titlelist_p;

private:
	void childConnect(bool connect);
	inline void intScale(double& scale)		{ scale = (scale > 10) ? floor(scale + 0.5) : floor(10*scale + 0.5)/10; }

    bool			update_coordinate_sys_;
	Qwt3D::RGBA		meshcolor_;
	double			meshLineWidth_;
	Qwt3D::RGBA		bgcolor_;
	bool			renderpixmaprequest_;
	bool			doublelegend_;
	bool			d_exporting_vector;
	GLint			gl2ps_export_format_;

	Qwt3D::ParallelEpiped	hull_;
};

} // ns
 
#endif
