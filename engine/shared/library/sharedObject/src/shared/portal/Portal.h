// ======================================================================
//
// Portal.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Portal_H
#define INCLUDED_Portal_H

// ======================================================================

class Appearance;
class AppearanceTemplate;
class AxialBox;
class Cell;
class CellProperty;
class DoorInfo;
class DoorObject;
class Iff;
class IndexedTriangleList;
class PortalPropertyTemplateCellPortal;
class Vector;

namespace DPVS
{
	class Object;
}

#include "sharedObject/PortalProperty.h"

// ======================================================================

class Portal
{
	friend class Cell;
	friend class PortalProperty;

public:

	typedef std::vector<Vector>     VertexList;
	typedef DPVS::Object            *(*CreateDpvsPortalHookFunction)(Portal *owner);
	typedef void                     (*DestroyDpvsPortalHookFunction)(DPVS::Object *cellToDestroy);
	typedef void                     (*ClosedStateChangedHookFunction)(Portal &);

	static void install();
	static void remove();
	static void setHookFunctions(CreateDpvsPortalHookFunction createDpvsPortalHookFunction, DestroyDpvsPortalHookFunction destroyDpvsPortalHookFunction, ClosedStateChangedHookFunction closedStateChangedHookFunction, bool createDoors);
	static void preloadDoorStyle(const char *doorStyle, std::vector<const AppearanceTemplate*>& preloadAppearanceTemplateList);

public:

	Portal(const PortalPropertyTemplateCellPortal &portalTemplate, CellProperty *parentCell, Object *relativeTo);
	~Portal();

	IndexedTriangleList const &       getGeometry() const;
	bool                              isGeometryWindingOrderClockwise() const;
	bool                              testPenetration(const Vector &begin, const Vector &end, float &t) const;
	bool                              testOverlap(const Sphere &sphere) const;

	// These two methods are slightly different - If a portal represents a door,
	// then isPassable returns true. If the server has told the client that the client
	// isn't allowed to enter the cell on the other side of the portal, then isPassageAllowed
	// returns false

	bool                              isDisabled() const;
	bool                              isPassable() const;
	bool                              isPassageAllowed() const;

	bool                              isClosed() const;
	void                              setClosed(bool closed);

	Portal                           *getNeighbor();
	const Portal                     *getNeighbor() const;

	Transform const                   getDoorTransform() const;

	CellProperty                     *getParentCell() const;

	// Get local (door-relative) versions of the portal's bounding box and vertex list
	AxialBox const                    getLocalBox() const;
	void                              getLocalVerts(VertexList &vertices) const;

	DPVS::Object                     *getDpvsObject();
	void                              removeFromDpvs();

	void                              setAppearance(Appearance *appearance);
	Object *                          getDoorObject();

private:

	Portal();
	Portal(const Portal &);
	Portal &operator =(const Portal &);

	void                              setNeighbor(Portal *neighbor);
	void                              createDoor();

private:

	PortalPropertyTemplateCellPortal const &m_template;
	Object                                 *m_relativeToObject;
	bool                                    m_closed;
	Portal                                 *m_neighbor;
	CellProperty                           *m_parentCell;
	DPVS::Object                           *m_dpvsPortal;
	DoorObject                             *m_door;
	Appearance                             *m_appearance;
};

// ======================================================================

inline bool Portal::isClosed() const
{
	return m_closed;
}

// ----------------------------------------------------------------------

inline Portal * Portal::getNeighbor()
{
	return m_neighbor;
}

// ----------------------------------------------------------------------

inline Portal const * Portal::getNeighbor() const
{
	return m_neighbor;
}

// ----------------------------------------------------------------------

inline CellProperty *Portal::getParentCell() const
{
	return m_parentCell;
}

// ----------------------------------------------------------------------

inline DPVS::Object *Portal::getDpvsObject()
{
	return m_dpvsPortal;
}

// ======================================================================

#endif
