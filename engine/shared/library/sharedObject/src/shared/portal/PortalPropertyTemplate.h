// ======================================================================
//
// PortalPropertyTemplate.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PortalPropertyTemplate_H
#define INCLUDED_PortalPropertyTemplate_H

// ======================================================================

class CellProperty;
class FloorMesh;
class Object;
class PortalPropertyTemplate;
class BaseClass;
class BaseExtent;
class Extent;
class IndexedTriangleList;
class Portal;
class Iff;

#include "sharedMath/Plane.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/Transform.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PersistentCrcString.h"

// ======================================================================

class PortalPropertyTemplateCellPortal
{
	friend class Portal;

public:

	PortalPropertyTemplateCellPortal(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	~PortalPropertyTemplateCellPortal();

	Transform const    getDoorTransform(bool useDoorHardpoint=true) const;

	void               flipPassable();
	void               preloadAssets() const;
	void               garbageCollect() const;

private:

	PortalPropertyTemplateCellPortal();
	PortalPropertyTemplateCellPortal(const PortalPropertyTemplateCellPortal &);
	PortalPropertyTemplateCellPortal &operator =(const PortalPropertyTemplateCellPortal &);

	void load(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	void load_0001(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	void load_0002(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	void load_0003(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	void load_0004(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);
	void load_0005(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff);

	void computePlaneEquation();

private:

	bool              m_disabled;
	bool              m_passable;
	bool              m_geometryWindingClockwise;
	IndexedTriangleList const * m_portalGeometry;
	char             *m_doorStyle;
	bool              m_hasDoorHardpoint;
	Transform         m_doorHardpoint;
	Plane             m_plane;

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager* m_preloadManager;
};

// ----------------------------------------------------------------------

struct PortalPropertyTemplateCellLight
{
	enum Type
	{
		T_ambient,
		T_parallel,
		T_point
	};

	Type       type;
	VectorArgb diffuseColor;
	VectorArgb specularColor;
	Transform  transform;
	real       constantAttenuation;
	real       linearAttenuation;
	real       quadraticAttenuation;
};

// ----------------------------------------------------------------------

class PortalPropertyTemplateCell
{
	friend class PortalProperty;
	friend class PortalPropertyTemplate;

public:

	typedef PortalPropertyTemplateCellLight                     Light;
	typedef std::vector<Light>                               LightList;
	typedef std::vector<Portal *>                            PortalList;
	typedef std::vector<PortalPropertyTemplateCellPortal *>  PortalPropertyTemplateCellPortalList;

public:

	PortalPropertyTemplateCell(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	~PortalPropertyTemplateCell();

	const char           *getName() const;
	const char           *getAppearanceName() const;
	const char           *getFloorName() const;
	FloorMesh const      *getFloorMesh() const;

	bool                  canSeeParentCell() const;
	PortalList           *createPortalList(CellProperty *parentCell, Object *relativeToObject) const;
	const LightList      *getLightList() const;
	const BaseExtent     *getCollisionExtent() const;

	void                  preloadAssets() const;
	void                  garbageCollect() const;

	const PortalPropertyTemplateCellPortalList     *getPortalList() const;

private:

	void load(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	void load_0001(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	void load_0002(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	void load_0003(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	void load_0004(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);
	void load_0005(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, Iff &iff);

	// disabled
	PortalPropertyTemplateCell(const PortalPropertyTemplateCell &);
	PortalPropertyTemplateCell &operator =(PortalPropertyTemplateCell &);

private:

	char                                  *m_name;
	char                                  *m_appearanceName;
	char                                  *m_floorName;
	FloorMesh const                       *m_floorMesh;
	bool                                   m_canSeeParentCell;
	LightList                             *m_lightList;
	PortalPropertyTemplateCellPortalList  *m_portalList;
	Extent                                *m_collisionExtent;

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager* m_preloadManager;
};

//------------------------------------------------------------------------------------------

inline const PortalPropertyTemplateCell::PortalPropertyTemplateCellPortalList* PortalPropertyTemplateCell::getPortalList() const
{
	return m_portalList;
}

// ----------------------------------------------------------------------

class PortalPropertyTemplate
{
public:

	struct PortalOwners
	{
		struct Owner
		{
			int cell;
			int portal;

			Owner();
		};

		Owner owners[2];
	};


	typedef PortalPropertyTemplateCell     Cell;
	typedef std::vector<Cell *>         CellList;
	typedef std::vector<Vector>         VertexList;
	typedef std::vector<Portal *>       PortalList;
	typedef std::vector<PortalOwners>   PortalOwnersList;
	typedef std::vector<IndexedTriangleList *> PortalGeometryList;
	typedef std::vector<const char *>   CellNameList;

public:

	static void install(char const *const pobEjectionTransformFilename);

	static bool extractPortalLayoutCrc(const char *fileName, uint32 &portalLayoutCrc);

public:

	explicit PortalPropertyTemplate(const CrcString &name);
	~PortalPropertyTemplate();
	
	void fetch() const;
	void release() const;

	void preloadAssets() const;
	void garbageCollect() const;

	const CrcString                  &getCrcString() const;
	const CrcString                  &getShortName() const;

	int                               getCrc() const;
	int                               getNumberOfCells() const;
	const CellNameList               &getCellNames() const;

	const char                       *getExteriorAppearanceName() const;
	const char                       *getExteriorFloorName() const;
	PortalList                       *createExteriorPortalList(CellProperty *parentCell, Object *relativeToObject) const;
	Transform const                   getEjectionLocationTransform() const;

	IndexedTriangleList const * getPortalGeometry(int portalIndex) const;
	IndexedTriangleList const * getPortalGeometry(int portalIndex, int cell, int cellPortalIndex, bool clockwise) const;
	const PortalPropertyTemplateCell &getCell(int cellIndex) const;

	PortalOwnersList const           &getPortalOwnersList() const;

	BaseClass const                  *getBuildingPathGraph() const;
	void                              attachBuildingPathGraph(BaseClass *newGraph);

	const VertexList                 *getRadarPortalGeometry() const;

	typedef BaseClass * (*ExpandBuildingGraphHook)( PortalPropertyTemplate * portalTemplate, BaseClass * baseBuildingGraph );

	static void                       setExpandBuildingGraphHook ( ExpandBuildingGraphHook hook );

private:

	// disabled
	PortalPropertyTemplate();
	PortalPropertyTemplate(const PortalPropertyTemplate &);
	PortalPropertyTemplate &operator =(const PortalPropertyTemplate &);

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);
	void load_0004(Iff &iff);

	void buildRadarPortalGeometry();

private:

	mutable int             m_referenceCount;
	PersistentCrcString     m_name;
	PersistentCrcString     m_shortName;
	PortalGeometryList     *m_portalGeometryList;
	PortalOwnersList       *m_portalOwnersList;
	CellList               *m_cellList;
	CellNameList           *m_cellNameList;
	int                     m_crc;
	BaseClass              *m_pathGraph;
	VertexList             *m_radarPortalGeometry;
};



// ======================================================================

inline void PortalPropertyTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline const CrcString &PortalPropertyTemplate::getCrcString() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline PortalPropertyTemplate::PortalOwnersList const &PortalPropertyTemplate::getPortalOwnersList() const
{
	return *m_portalOwnersList;
}

// ======================================================================

#endif
