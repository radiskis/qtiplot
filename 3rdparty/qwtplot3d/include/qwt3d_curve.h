#ifndef __CURVE_H__
#define __CURVE_H__

#include <QList>
#include <QVector>
#include <QtDebug>

#include "qwt3d_plot.h"

// Shorthand definitions
#define TITLE		"QwtPlot3D (Use Ctrl-Alt-Shift-LeftBtn-Wheel or keyboard)"

namespace Qwt3D
{

//! A class representing Curved Surfaces
/**
    A Curved Surface ...
    
*/
class QWT3D_EXPORT Curve : public QObject
{
    Q_OBJECT

public:
	Curve(QWidget* parent = 0);
	virtual ~Curve();

	void setPlot(Qwt3D::Plot3D* p)				{ plot_p = p; }
	Qwt3D::Plot3D* plot() const					{ return plot_p; }			//!< Returns pointer to parent Plot3D object
	Qwt3D::Label* title() const					{ return title_p; }			//!< Returns pointer to title Label object
	Qwt3D::ColorLegend* legend()				{ return &legend_; }		//!< Returns pointer to key ColorLegend object
	virtual void draw();

	void connects();
	void disconnects();
	void updateNormals();													//!< Recalculates surface normals;

	std::pair<int,int> facets() const;										//!< Returns the number of mesh cells for the ORIGINAL data
	bool loadFromData(Qwt3D::Triple** data, unsigned int columns, unsigned int rows, QString titlestr = QString(),
					  bool uperiodic = false, bool vperiodic = false);
	bool loadFromData(double** data, unsigned int columns, unsigned int rows,
					  double minx, double maxx, double miny, double maxy, QString titlestr = QString());
	bool loadFromData(Qwt3D::TripleField const& data, Qwt3D::CellField const& poly, QString titlestr = QString());

	Qwt3D::FLOORSTYLE floorStyle() const		{ return floorstyle_; }		//!< Returns floor style
	Qwt3D::Enrichment* userStyle() const		{ return userplotstyle_p; }	//!< Returns current Enrichment object used for plotting styles (if set, zero else)
	Qwt3D::PLOTSTYLE plotStyle() const			{ return plotstyle_; }		//!< Returns plotting style
	Qwt3D::SHADINGSTYLE shading() const			{ return shading_; }		//!< Returns shading style
	unsigned int resolution() const				{ return resolution_p; }	//!< Returns data resolution (1 means all data)
	unsigned int isolines() const				{ return isolines_; }		//!< Returns number of isolines
	bool smoothDataMesh() const					{ return smoothdatamesh_p; }//!< True if mesh antialiasing is on
	Qwt3D::RGBA meshColor() const				{ return meshcolor_; }		//!< Returns color for data mesh
	double meshLineWidth() const				{ return meshLineWidth_; }	//!< Returns line width for data mesh
	const Qwt3D::Color* dataColor() const		{ return datacolor_p; }		//!< Returns data color object
	double polygonOffset() const				{ return polygonOffset_; }	//!< Returns relative value for polygon offset [0..1]

	bool normals() const						{ return datanormals_p; }	//!< Returns \c true, if normal drawing is on
	double normalLength() const					{ return normalLength_p; }	//!< Returns relative length of normals
	int normalQuality() const					{ return normalQuality_p; }	//!< Returns plotting quality of normal arrows

	Qwt3D::ParallelEpiped hull() const			{ return hull_; }			//!< Returns rectangular hull
	const Qwt3D::Data* data() const				{ return actualData_p; }	//!< the data pointer
	bool hasData() const						{ return (actualData_p) ? !actualData_p->empty() : false; }	//!< Returns true if valid data available, false else
	bool dataProjected() const					{ return datapoints_; }		//!< Returns projected state of data points 

	QList<Qwt3D::Enrichment*> enrichmentList(){return elist_p;}; //!< Returns the Enrichments list
	virtual Qwt3D::Enrichment* addEnrichment(Qwt3D::Enrichment const&);		//!< Add an Enrichment
	virtual bool degrade(Qwt3D::Enrichment*);								//!< Remove an Enrichment

	bool isColorLegend() const					{ return displaylegend_; }	//!< Returns Color Legend display state
	void setColorLegend(int index, bool doublemode = false, QSize size = QSize(3, 32), QPoint pos = QPoint(3, 10));

	void clearEnrichments();//!< Remove all Enrichments

	double** getData(int *cols, int *rows);
	Qwt3D::CellField* getCellData(int *cells);
	Qwt3D::TripleField* getNodeData(int *nodes);

	void deleteData(double**data, int columns);
	void deleteData(CellField* poly);
	void deleteData(TripleField* data);

    void queueUpdate();

    // Public data members
	Qwt3D::Tuple	titlerel_;
	Qwt3D::ANCHOR	titleanchor_;
	double			titlespace_;
	unsigned int	titlepos_;

signals:
    void resolutionChanged(int);
    void updatePlotData(bool);
    void updatePlot();
	void readInFinished(const QString&);

public slots:
    void setResolution(int);
    void setPlotStyle(Qwt3D::PLOTSTYLE val);
    void setPlotStyle(Qwt3D::Enrichment& val);
    void setPolygonOffset(double d);
    void setFloorStyle(Qwt3D::FLOORSTYLE val) { floorstyle_ = val; update_displaylists_ = true;} //!< Sets floor style
    void setShading(Qwt3D::SHADINGSTYLE val);
    void setIsolines(unsigned int isolines);

	void updateColorLegend(int majors, int minors);							//!< Updates the curve data color legend
	void showColorLegend(bool);												//!< Show curve data color legend
    void showNormals(bool); 												//!< Draw normals to every vertex
    void setNormalLength(double val);										//!< Sets length of normals in percent per hull diagonale
    void setNormalQuality(int val);											//!< Increases plotting quality of normal arrows

    void setSmoothMesh(bool val) { smoothdatamesh_p = val; update_displaylists_ = true;} //!< Enables/disables smooth data mesh lines. Default is false
    void setMeshColor(Qwt3D::RGBA rgba);									//!< Sets color for data mesh
    void setMeshLineWidth(double lw);										//!< Sets line width for data mesh

    void setDataColor(Color* col);											//!< Sets new data color object
    void setDataProjection(bool toggle = true)	{ datapoints_ = toggle; }	//!< Sets data point projection on & off
    void setProjection(Qwt3D::PROJECTMODE val, bool toggle = true);			//!< Sets projection modes

    void setTitleColor(Qwt3D::RGBA col)			{ title_p->setColor(col); }	//!< Set caption color
    void setTitle(const QString& title) {									//!< Set caption text (one row only)
        if (title.isEmpty() || ((title == TITLE) && (plot_p->titleList().size() > 1)))
            plot_p->removeTitle(title_p);
        title_p->setString(title);				// allows title to be reset & removed
    }

    void setTitlePosition(double rely, double relx = 0.5, Qwt3D::ANCHOR anchor = Qwt3D::TopCenter);
    void setTitleFont(const QString& family, int pointSize, int weight = QFont::Normal, bool italic = false);

    void createData();
    void createEnrichments();

protected:
    typedef QList<Qwt3D::Enrichment*> EnrichmentList;
    typedef EnrichmentList::iterator ELIT;

    Qwt3D::Plot3D*		plot_p;
    Qwt3D::Label*		title_p;
    Qwt3D::Data* 		actualData_p;

    bool				update_displaylists_;
    Qwt3D::Color*		datacolor_p;
    Qwt3D::Enrichment*	userplotstyle_p;
    EnrichmentList		elist_p;

    Qwt3D::RGBA			meshcolor_;
    double				meshLineWidth_;
    Qwt3D::RGBA			bgcolor_;
    Qwt3D::PLOTSTYLE	plotstyle_;
    Qwt3D::SHADINGSTYLE shading_;
    Qwt3D::FLOORSTYLE	floorstyle_;
    double				polygonOffset_;

    unsigned int		isolines_;
    unsigned int		point_size_;

    bool	datapoints_;
    bool	facemode_;
    bool	sidemode_;
    bool	floormode_;
    bool	smoothdatamesh_p;
    bool	datanormals_p;
    double	normalLength_p;
    int		normalQuality_p;
    int		resolution_p;

    void updateData(bool coord = true);
    virtual void drawImplementation();

    void setHull(Qwt3D::ParallelEpiped p) {hull_ = p;}
    virtual void calculateHull();

    virtual void createEnrichment(Qwt3D::Enrichment& p);
    virtual void createFloorData();
    virtual void createSideData();
    virtual void createFaceData();
    void createNormals();
    void createPoints();

    void drawVertex(Qwt3D::Triple& vertex, double shift, unsigned int comp = 3);
    virtual void drawIntersections(std::vector<Qwt3D::Triple>& intersection, double shift, unsigned int comp,
								   bool projected, std::vector<RGBA>* colour = 0);

    void readIn(Qwt3D::GridData& gdata, Qwt3D::Triple** data, unsigned int columns, unsigned int rows);
    void readIn(Qwt3D::GridData& gdata, double** data, unsigned int columns, unsigned int rows,
                double minx, double maxx, double miny, double maxy);
    void calcNormals(Qwt3D::GridData& gdata);
    void sewPeriodic(Qwt3D::GridData& gdata);

    void animateData(double** data);
    void animateData(Qwt3D::TripleField* data, Qwt3D::CellField* poly);
    void animateData(Qwt3D::TripleField* data);

private:
	enum OBJECTS {
		DataObject=0,
		NormalObject,
		DisplayListSize // only to have a vector length ...
	};
	QVector<GLuint> displaylists_p;

	void Data2Floor();
	void Isolines2Floor();
    
    Qwt3D::ParallelEpiped hull_;

	// grid plot
	Qwt3D::GridData* actualDataG_;
	virtual void createDataG();
	virtual void createFaceDataG();
	virtual void createSideDataG();
	virtual void createFloorDataG();
	void createNormalsG();

	void Data2FloorG()									{ DatamapG(2); }
	void Data2FrontG()									{ DatamapG(1); }
	void Data2SideG()									{ DatamapG(0); }
	void DatamapG(unsigned int comp);
	
	void Isolines2FloorG(bool projected = false)		{ IsolinesG(2, projected); }
	void Isolines2FrontG(bool projected = false)		{ IsolinesG(1, projected); }
	void Isolines2SideG(bool projected = false)			{ IsolinesG(0, projected); }
	void IsolinesG(unsigned int comp, bool projected);

	void setColorFromVertexG(int ix, int iy, bool skip = false);

	void DataPointsG(unsigned int comp, bool projected);
	void DataPoints2SideG(bool projected = false)		{ DataPointsG(0, projected); }
	void DataPoints2BackG(bool projected = false)		{ DataPointsG(1, projected); }

	// mesh plot
	Qwt3D::CellData* actualDataC_;
	virtual void createDataC();
	virtual void createFaceDataC();
	virtual void createSideDataC();
	virtual void createFloorDataC();
	void createNormalsC();

	void Data2FloorC()									{ DatamapC(2); }
	void Data2FrontC()									{ DatamapC(1); }
	void Data2SideC()									{ DatamapC(0); }
	void DatamapC(unsigned int comp);

	void Isolines2FloorC(bool projected = false)		{ IsolinesC(2, projected); }
	void Isolines2FrontC(bool projected = false)		{ IsolinesC(1, projected); }
	void Isolines2SideC(bool projected = false)			{ IsolinesC(0, projected); }
	void IsolinesC(unsigned int comp, bool projected);
	
	void setColorFromVertexC(int node, bool skip = false);

	bool					displaylegend_;
    Qwt3D::ColorLegend		legend_;
};

}

#endif
