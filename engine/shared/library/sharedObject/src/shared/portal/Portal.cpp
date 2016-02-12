// ======================================================================
//
// Portal.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Portal.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/DoorInfo.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedUtility/DataTable.h"

#include <vector>

// ======================================================================

namespace PortalNamespace
{
	DataTable                              *ms_doorStyleTable;

	Portal::ClosedStateChangedHookFunction  ms_closedStateChangedHookFunction;
	Portal::CreateDpvsPortalHookFunction    ms_createDpvsPortalHookFunction;
	Portal::DestroyDpvsPortalHookFunction   ms_destroyDpvsPortalHookFunction;
	bool                                    ms_createDoors = false;
}
using namespace PortalNamespace;

// ======================================================================

void Portal::install()
{
	Iff dataTableIff;

	if (dataTableIff.open("datatables/appearance/door_style.iff", true))
	{
		delete ms_doorStyleTable;
		ms_doorStyleTable = new DataTable();
		ms_doorStyleTable->load(dataTableIff);
	}

	ExitChain::add(&Portal::remove,"Portal::remove");
}

// ----------------------------------------------------------------------

void Portal::remove()
{
	delete ms_doorStyleTable;
	ms_doorStyleTable = nullptr;
}

// ----------------------------------------------------------------------

void Portal::setHookFunctions(CreateDpvsPortalHookFunction createDpvsPortalHookFunction, DestroyDpvsPortalHookFunction destroyDpvsPortalHookFunction, ClosedStateChangedHookFunction closedStateChangedHookFunction, bool createDoors)
{
	ms_closedStateChangedHookFunction = closedStateChangedHookFunction;
	ms_createDpvsPortalHookFunction = createDpvsPortalHookFunction;
	ms_destroyDpvsPortalHookFunction = destroyDpvsPortalHookFunction;
	ms_createDoors = createDoors;
}

// ----------------------------------------------------------------------

void Portal::preloadDoorStyle(const char* const doorStyle, std::vector<const AppearanceTemplate*>& preloadAppearanceTemplateList)
{
	int row = ms_doorStyleTable->searchColumnString (0, doorStyle);
	if (row == -1)
		return;

	uint i;
	for (i = 1; i < 4; ++i)
	{
		const std::string& appearanceTemplateName = ms_doorStyleTable->getStringValue (i,row);
		if (!appearanceTemplateName.empty ())
		{
			const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch (appearanceTemplateName.c_str ());
			appearanceTemplate->preloadAssets ();
			preloadAppearanceTemplateList.push_back (appearanceTemplate);
		}
	}
}

// ======================================================================

Portal::Portal(const PortalPropertyTemplateCellPortal &portalTemplate, CellProperty *parentCell, Object *relativeTo)
:
	m_template(portalTemplate),
	m_relativeToObject(relativeTo),
	m_closed(false),
	m_neighbor(nullptr),
	m_parentCell(parentCell),
	m_dpvsPortal(nullptr),
	m_door(nullptr),
	m_appearance(nullptr)
{
	if(ms_createDoors)
	{
		createDoor();
	}
}

// ----------------------------------------------------------------------

Portal::~Portal()
{
	if (m_dpvsPortal)
	{
		m_relativeToObject->removeDpvsObject(m_dpvsPortal);
		NOT_NULL(ms_destroyDpvsPortalHookFunction);
		(*ms_destroyDpvsPortalHookFunction)(m_dpvsPortal);
		m_dpvsPortal = nullptr;
	}

	m_relativeToObject = nullptr;
	m_neighbor = nullptr;
	m_parentCell = nullptr;
	m_door = nullptr;
	delete m_appearance;
}

// ----------------------------------------------------------------------

const Transform Portal::getDoorTransform() const
{
	return m_template.getDoorTransform();
}

// ----------------------------------------------------------------------

const AxialBox Portal::getLocalBox() const
{
	const Transform t = getDoorTransform();

	AxialBox tempBox;
	VertexList const &geometry = getGeometry().getVertices();
	VertexList::const_iterator iEnd = geometry.end();
	for (VertexList::const_iterator i = geometry.begin(); i != iEnd; ++i)
		tempBox.add(t.rotateTranslate_p2l(*i));

	return tempBox;
}

// ----------------------------------------------------------------------

void Portal::getLocalVerts(VertexList &vertices) const
{
	const Transform t = getDoorTransform();

	VertexList const &geometry = getGeometry().getVertices();

	vertices.clear();
	vertices.reserve(geometry.size());

	VertexList::const_iterator iEnd = geometry.end();
	for (VertexList::const_iterator i = geometry.begin(); i != iEnd; ++i)
		vertices.push_back(t.rotateTranslate_p2l(*i));
}

// ----------------------------------------------------------------------

IndexedTriangleList const & Portal::getGeometry() const
{
	NOT_NULL(m_template.m_portalGeometry);
	return *m_template.m_portalGeometry;
}

// ----------------------------------------------------------------------

bool Portal::isGeometryWindingOrderClockwise() const
{
	return m_template.m_geometryWindingClockwise;
}

// ----------------------------------------------------------------------

bool Portal::testPenetration(const Vector &begin, const Vector &end, float &t) const
{
	if (isDisabled())
		return false;

	//-- For clockwise ordering, we test from start to end.  For counterclockwise ordering, we test end to start
	Vector result;
	if ((isGeometryWindingOrderClockwise() && getGeometry().collide(begin, end, result)) || (!isGeometryWindingOrderClockwise() && getGeometry().collide(end, begin, result)))
	{
		const float lineLength = begin.magnitudeBetween(end);
		t = lineLength > 0.f ? begin.magnitudeBetween(result) / lineLength : 0.f;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool Portal::testOverlap(const Sphere &sphere) const
{
	if (isDisabled())
		return false;

	IndexedTriangleList const & indexedTriangleList = getGeometry();
	std::vector<Vector> const & vertices = indexedTriangleList.getVertices();
	std::vector<int> const & indices = indexedTriangleList.getIndices();
	uint const numberOfFaces = indices.size() / 3;

	VertexList vertexList;
	for (uint faceIndex = 0, index = 0; faceIndex < numberOfFaces; ++faceIndex)
	{
		Vector const & v0 = vertices[indices[index++]];
		Vector const & v1 = vertices[indices[index++]];
		Vector const & v2 = vertices[indices[index++]];

		vertexList.clear();
		vertexList.push_back(v0);
		vertexList.push_back(v1);
		vertexList.push_back(v2);

		Plane3d const plane(v0, v1, v2);

		if (Overlap3d::TestSpherePoly(sphere, vertexList, plane))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void Portal::createDoor ()
{
	DoorInfo info;
	const char *doorStyle = m_template.m_doorStyle;
	if (!doorStyle || !doorStyle[0])
	{
		info.m_frameAppearance = 0;
		info.m_doorAppearance = 0;
		info.m_doorAppearance2 = 0;
		info.m_doorFlip2 = 0;
		info.m_delta = Vector::zero;
		info.m_openTime = 0.f;
		info.m_closeTime = 0.f;
		info.m_spring = 0.f;
		info.m_smoothness = 0.f;
		info.m_triggerRadius = 0.f;
		info.m_forceField = false;
		info.m_openBeginEffect = 0;
		info.m_openEndEffect = 0;
		info.m_closeBeginEffect = 0;
		info.m_closeEndEffect = 0;
		info.m_portalGeometry = m_template.m_portalGeometry;
		info.m_alwaysOpen = true;
	}
	else
	{
		//@todo - Look up door info from a datatable instead of hardcoding it
		int row = ms_doorStyleTable->searchColumnString(0,doorStyle);
		if(row == -1)
		{
			DEBUG_WARNING(true,("Portal::createDoor - Can't find door style entry for style %s - using placeholder\n", doorStyle));
			row = 0;
		}

		info.m_frameAppearance  = ms_doorStyleTable->getStringValue( 1,row);
		info.m_doorAppearance   = ms_doorStyleTable->getStringValue( 2,row);
		info.m_doorAppearance2  = ms_doorStyleTable->getStringValue( 3,row);
		info.m_doorFlip2        = ms_doorStyleTable->getIntValue    ( 4,row) ? true : false;
		info.m_delta.x          = ms_doorStyleTable->getFloatValue  ( 5,row);
		info.m_delta.y          = ms_doorStyleTable->getFloatValue  ( 6,row);
		info.m_delta.z          = ms_doorStyleTable->getFloatValue  ( 7,row);
		info.m_openTime         = ms_doorStyleTable->getFloatValue  ( 8,row);
		info.m_closeTime        = ms_doorStyleTable->getFloatValue  ( 9,row);
		info.m_spring           = ms_doorStyleTable->getFloatValue  (10,row);
		info.m_smoothness       = ms_doorStyleTable->getFloatValue  (11,row);
		info.m_triggerRadius    = ms_doorStyleTable->getFloatValue  (12,row);
		info.m_forceField       = ms_doorStyleTable->getIntValue    (13,row) ? true : false;
		info.m_openBeginEffect  = ms_doorStyleTable->getStringValue(14,row);
		info.m_openEndEffect    = ms_doorStyleTable->getStringValue(15,row);
		info.m_closeBeginEffect = ms_doorStyleTable->getStringValue(16,row);
		info.m_closeEndEffect   = ms_doorStyleTable->getStringValue(17,row);
		info.m_portalGeometry   = m_template.m_portalGeometry;
		info.m_alwaysOpen       = false;
	}

	m_door = new DoorObject(info, this);
	m_door->setTransform_o2p(getDoorTransform());

	if(m_door->getBarrier())
		m_door->getBarrier()->setTransform_o2p(getDoorTransform());

	CellProperty::addToRenderWorld(*m_door);
	for (int i = 0; i < m_door->getNumberOfDrawnDoors(); ++i)
		CellProperty::addToRenderWorld(*m_door->getDrawnDoor(i));
	if (m_door->getBarrier())
		CellProperty::addToRenderWorld(*m_door->getBarrier());

	m_door->attachToObject_p(m_relativeToObject, true);

	if(m_door->getBarrier())
		m_door->getBarrier()->attachToObject_p(m_relativeToObject,true);
}

// ----------------------------------------------------------------------

bool Portal::isDisabled() const
{
#ifdef _DEBUG
	if (ConfigSharedObject::getAllowDisabledPortals())
		return false;
#endif

	return m_template.m_disabled;
}

// ----------------------------------------------------------------------
// Determine if players are allowed to pass through this portal or not.

bool Portal::isPassable() const
{
	if (m_template.m_passable)
	{
		CellProperty const * const cell = getParentCell();

		if (cell != 0)
		{
			return cell->getAccessAllowed();
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool Portal::isPassageAllowed() const
{
	if (isPassable() && !isDisabled())
	{
		Portal const * neighbor = getNeighbor();

		CellProperty const * neighborCell = (neighbor != 0) ? neighbor->getParentCell() : 0;

		if (neighborCell != 0)
		{
			return neighborCell->getAccessAllowed();
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void Portal::setClosed(bool closed)
{
	if (m_closed != closed) //lint !e731 // (Info -- Boolean argument to equal/not equal) // This is the intended logic.
	{
		m_closed = closed;
		if (ms_closedStateChangedHookFunction)
			(*ms_closedStateChangedHookFunction)(*this);
	}
}

// ----------------------------------------------------------------------

void Portal::setNeighbor(Portal *neighbor)
{
	m_neighbor = neighbor;
	if (m_door)
		m_door->setNeighbor(m_neighbor->m_door);

	if (!m_dpvsPortal && ms_createDpvsPortalHookFunction)
	{
		m_dpvsPortal = (*ms_createDpvsPortalHookFunction)(this);
		m_relativeToObject->addDpvsObject(m_dpvsPortal);
	}
}

// ----------------------------------------------------------------------

void Portal::removeFromDpvs()
{
	if (m_dpvsPortal != 0)
	{
		m_relativeToObject->removeDpvsObject(m_dpvsPortal);
		NOT_NULL(ms_destroyDpvsPortalHookFunction);
		(*ms_destroyDpvsPortalHookFunction)(m_dpvsPortal);
		m_dpvsPortal = 0;
	}

	delete m_appearance;
	m_appearance = 0;
}

// ----------------------------------------------------------------------

void Portal::setAppearance(Appearance *appearance)
{
	delete m_appearance;
	m_appearance = appearance;
}

// ----------------------------------------------------------------------

Object * Portal::getDoorObject()
{
	return m_door;
}

// ======================================================================
