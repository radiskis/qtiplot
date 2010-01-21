#ifndef qwt3d_SurfacePlot_h__2004_03_05_11_36_begin_guarded_code
#define qwt3d_SurfacePlot_h__2004_03_05_11_36_begin_guarded_code

#include "qwt3d_plot.h"

namespace Qwt3D
{
//! A class representing  Surfaces
/**
	A SurfacePlot ...
	
*/
class QWT3D_EXPORT SurfacePlot : public Plot3D
{
    Q_OBJECT

public:
#if QT_VERSION < 0x040000
  SurfacePlot( QWidget* parent = 0, const char* name = 0 );
#else
  SurfacePlot( QWidget* parent = 0, const QGLWidget* shareWidget = 0 );
#endif  
  ~SurfacePlot();
	void updateNormals(); //!< Recalculates surface normals;
	int	resolution() const {return resolution_p;} //!< Returns data resolution (1 means all data)
	std::pair<int,int> facets() const; //!< Returns the number of mesh cells for the ORIGINAL data
  bool loadFromData(Qwt3D::Triple** data, unsigned int columns, unsigned int rows
    , bool uperiodic = false, bool vperiodic = false);
	bool loadFromData(double** data, unsigned int columns, unsigned int rows
																		,double minx, double maxx, double miny, double maxy);
	bool loadFromData(Qwt3D::TripleField const& data, Qwt3D::CellField const& poly);
 	
  
  //! Equivalent to loadFromData();
  /**
  \deprecated  Use loadFromData instead
  */
  bool createDataRepresentation(Qwt3D::Triple** data, unsigned int columns, unsigned int rows
    , bool uperiodic = false, bool vperiodic = false)
  {
    return loadFromData(data, columns, rows, uperiodic, vperiodic);
  }
  //! Equivalent to loadFromData();
  /**
  \deprecated  Use loadFromData instead
  */
	bool createDataRepresentation(double** data, unsigned int columns, unsigned int rows
																		,double minx, double maxx, double miny, double maxy)
  {
    return loadFromData(data, columns, rows, minx, maxx, miny, maxy);
  }
  //! Equivalent to loadFromData();
  /**
  \deprecated  Use loadFromData instead
  */
	bool createDataRepresentation(Qwt3D::TripleField const& data, Qwt3D::CellField const& poly)
	{
		return loadFromData(data, poly);
	}

	Qwt3D::FLOORSTYLE floorStyle() const		{ return floorstyle_; }		//!< Return floor style
	void setFloorStyle(Qwt3D::FLOORSTYLE val)	{ floorstyle_ = val; }		//!< Sets floor style
	void showNormals(bool); 												//!< Draw normals to every vertex
	bool normals() const						{ return datanormals_p; }	//!< Returns \c true, if normal drawing is on
	
	void setNormalLength(double val);			//!< Sets length of normals in percent per hull diagonale
	double normalLength() const					{ return normalLength_p; }	//!< Returns relative length of normals
	void setNormalQuality(int val);				//!< Increases plotting quality of normal arrows
	int normalQuality() const					{ return normalQuality_p; }	//!< Returns plotting quality of normal arrows

	double** getData(int *cols, int *rows);
	CellField* getCellData(int *cells);
	TripleField* getNodeData(int *nodes);
	
	void deleteData(double**data, int columns);
	void deleteData(CellField* poly);
	void deleteData(TripleField* data);

	void setDataProjection(bool toggle = true)	{ datapoints_ = toggle; }
	bool dataProjected() const					{ return datapoints_; }
	void setProjection(Qwt3D::PROJECTMODE val, bool toggle = true)
	{
		switch(val) {
			case Qwt3D::FACE:
				facemode_  = toggle; break;
			case Qwt3D::SIDE:
				sidemode_  = toggle; break;
			case Qwt3D::BASE:
				floormode_ = toggle; break;
		}
	} //!< Sets projection modes

signals:
	void resolutionChanged(int);

public slots:
	void setResolution( int );

protected:
	bool datanormals_p;
	double normalLength_p;
	int normalQuality_p;
  
	virtual	void calculateHull();
	virtual void createData();
	virtual void createEnrichment(Qwt3D::Enrichment& p);
	virtual void createFloorData();
	virtual void createSideData();
	virtual void createFaceData();

	void drawVertex(Triple& vertex, double shift, unsigned int comp = 3);
	virtual void drawIntersections(std::vector<Triple>& intersection, double shift, unsigned int comp,
								   bool projected, std::vector<RGBA>* colour = 0);

	void createNormals();
	void createPoints();

	int resolution_p;

	void readIn(Qwt3D::GridData& gdata, Triple** data, unsigned int columns, unsigned int rows);
	void readIn(Qwt3D::GridData& gdata, double** data, unsigned int columns, unsigned int rows, 
				double minx, double maxx, double miny, double maxy);
	void calcNormals(GridData& gdata);
	void sewPeriodic(GridData& gdata);

	void animateData(double** data);
	void animateData(TripleField* data, CellField* poly);
	void animateData(TripleField* data);
//	void calcLowResolution();

private:

//	void Data2Floor();
//	void Isolines2Floor();

	Qwt3D::FLOORSTYLE floorstyle_;
	unsigned int point_size_;
	bool datapoints_;
	bool facemode_;
	bool sidemode_;
	bool floormode_;

	// grid plot
	Qwt3D::GridData* actualDataG_;
	virtual void createDataG();
	virtual void createFaceDataG();
	virtual void createSideDataG();
	virtual void createFloorDataG();
	void createNormalsG();

	void Data2FloorG()		{ DatamapG(2); }
	void Data2FrontG()		{ DatamapG(1); }
	void Data2SideG()		{ DatamapG(0); }
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

	void Data2FloorC()		{ DatamapC(2); }
	void Data2FrontC()		{ DatamapC(1); }
	void Data2SideC()		{ DatamapC(0); }
	void DatamapC(unsigned int comp);

	void Isolines2FloorC(bool projected = false)		{ IsolinesC(2, projected); }
	void Isolines2FrontC(bool projected = false)		{ IsolinesC(1, projected); }
	void Isolines2SideC(bool projected = false)			{ IsolinesC(0, projected); }
	void IsolinesC(unsigned int comp, bool projected);
	
	void setColorFromVertexC(int node, bool skip = false);
};

} // ns


#endif
