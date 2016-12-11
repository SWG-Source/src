// ======================================================================
//
// CellProperty.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CellProperty.h"

#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectNotification.h"
#include "sharedObject/Portal.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedUtility/Location.h"

#include <vector>
#include <algorithm>

// ======================================================================

const Tag TAG_CELP = TAG(C,E,L,P);

// ======================================================================

namespace CellPropertyNamespace
{
	class Notification : public ObjectNotification
	{
	public:

		static void setPortalTransitionsEnabled(bool enabled);

	public:

		Notification();
		virtual ~Notification();

		virtual int  getPriority() const;

		virtual bool positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
		virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;

	private:

		static bool ms_enabled;

	private:

		Notification(const Notification &); //lint -esym(754, Notification::Notification) // (Info -- local structure member 'Notification::Notification(const CellPropertyNamespace::Notification &)' not referenced) // defensive hiding
		Notification &operator =(const Notification &);
	};

 	float const                                        ms_maximumValidCellSpaceCoordinate = 1024.0f;

	Object                                            *ms_worldCellObject;
	Notification                                       ms_portalCrossingNotification;
	bool                                               ms_renderPortals;
	CellProperty::CreateDpvsCellHookFunction           ms_createDpvsCellHookFunction = nullptr;
	CellProperty::DestroyDpvsCellHookFunction          ms_destroyDpvsCellHookFunction = nullptr;
	CellProperty::AddToRenderWorldHookFunction         ms_addToRenderWorldHook = nullptr;

	CellProperty::PolyAppearanceFactory                ms_portalBarrierFactory = nullptr;
	CellProperty::PolyAppearanceFactory                ms_forceFieldFactory = nullptr;

	CellProperty::AccessAllowedHookFunction            ms_accessAllowedHookFunction = nullptr;
}  
using namespace CellPropertyNamespace;

// ======================================================================

bool                                     CellPropertyNamespace::Notification::ms_enabled = true;

CellProperty                            *CellProperty::ms_worldCellProperty;
CellProperty::TextureFetch               CellProperty::ms_textureFetch = nullptr;
CellProperty::TextureRelease             CellProperty::ms_textureRelease = nullptr;
CellProperty::DeleteVisibleCellProperty  CellProperty::ms_deleteVisibleCellProperty = nullptr;

// ======================================================================

CellPropertyNamespace::Notification::Notification() :
	ObjectNotification()
{
}

// ----------------------------------------------------------------------

CellPropertyNamespace::Notification::~Notification()
{
}

// ----------------------------------------------------------------------

int CellPropertyNamespace::Notification::getPriority() const
{
	return -100;
}

// ----------------------------------------------------------------------

bool CellPropertyNamespace::Notification::positionChanged(Object &object, bool const dueToParent, const Vector &oldPosition_p) const
{
	if (!ms_enabled)
		return true;

	// If this notification is due to our parent, we don't need to continue
	if (dueToParent)
		return true;

	// If we're attached to an object and we're not attached to a cell, we don't need to continue
	if (object.getAttachedTo() && !object.getAttachedTo()->getCellProperty())
		return true;

	Vector start = object.getTransform_p2w().rotateTranslate_l2p(oldPosition_p);
	Vector end   = object.getPosition_w();

	if (start == end)
		return true;

	// Move the old and new position up by half a meter so that our line-portal intersection test
	// doesn't miss the bottom of the portal.

	//@todo - This assumes that the position we're getting for the creature is at the creature's feet,
	// that Y is up, and that moving the position up will put it closer to the center of the portal

	// This is intended to fix problems with entering buildings whose doorway portals sit right on the
	// terrain - slight variations in the height of the terrain can cause the creature's position to slide
	// under the portal instead of going through it.

	// The assumption that y is up does not hold in space, so we need to do our correction in the actual
	// up direction of the cell.
	Vector objUpW = object.getObjectFrameJ_w();
	objUpW *= 0.1f;
	start += objUpW;
	end += objUpW;

	CellProperty *targetCell = nullptr;

	CellProperty *lastCell = nullptr;
	CellProperty *currentCell = object.getParentCell();

	while(currentCell)
	{
		Vector const cellStart = currentCell->getOwner().rotateTranslate_w2o(start);
		Vector const cellEnd   = currentCell->getOwner().rotateTranslate_w2o(end);

		float time = 0.0f;
		CellProperty *nextCell = currentCell->getDestinationCell(cellStart, cellEnd, time);

		if(time == 1.0f) break;

		if(nextCell == nullptr) break;
		if(nextCell == currentCell) break;	
		if(nextCell == lastCell) break;

		lastCell = currentCell;
		currentCell = nextCell;

		targetCell = currentCell;

		start = Vector::linearInterpolate(start, end, time);
	}

	if (targetCell)
	{
		// object changed portals, don't continue with this position change because the cell change will handle all that as well
		object.setParentCell(targetCell);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool CellPropertyNamespace::Notification::positionAndRotationChanged(Object &object, bool /* dueToParent */, const Vector &oldPosition) const
{
	return positionChanged(object, false, oldPosition);
}

// ----------------------------------------------------------------------

void CellPropertyNamespace::Notification::setPortalTransitionsEnabled(bool enabled)
{
	ms_enabled = enabled;
}

// ----------------------------------------------------------------------

void CellProperty::setTextureHookFunctions(TextureFetch textureFetch, TextureRelease textureRelease)
{
	ms_textureFetch = textureFetch;
	ms_textureRelease = textureRelease;

}

// ----------------------------------------------------------------------

void CellProperty::setDeleteVisibleCellProperty(DeleteVisibleCellProperty deleteVisibleCellProperty)
{
	ms_deleteVisibleCellProperty = deleteVisibleCellProperty;
}

// ======================================================================

void CellProperty::install()
{
	ms_worldCellObject = new Object();
	ms_worldCellProperty = new CellProperty(*ms_worldCellObject);
	ms_worldCellProperty->m_cellIndex = 0;
	ms_worldCellProperty->m_cellName = "world";
	ms_worldCellProperty->m_cellNameCrc = Crc::normalizeAndCalculate(ms_worldCellProperty->m_cellName);
	ms_worldCellObject->addProperty(*ms_worldCellProperty);
	ExitChain::add(&remove, "CellProperty::remove");

	DebugFlags::registerFlag( ms_renderPortals, "SharedObject", "renderPortals" );
}

// ----------------------------------------------------------------------

void CellProperty::remove()
{
	delete ms_worldCellObject;
	ms_worldCellObject = nullptr;
	ms_worldCellProperty = nullptr;
}

// ----------------------------------------------------------------------

void CellProperty::setAddToRenderWorldHook( AddToRenderWorldHookFunction hook )
{
	ms_addToRenderWorldHook = hook;
}

// ----------------------------------------------------------------------

void CellProperty::setPortalBarrierFactory ( PolyAppearanceFactory factory )
{
	ms_portalBarrierFactory = factory;
}

// ----------------------------------------------------------------------

void CellProperty::setForceFieldFactory ( PolyAppearanceFactory factory )
{
	ms_forceFieldFactory = factory;
}

// ----------------------------------------------------------------------

void CellProperty::setAccessAllowedHookFunction(AccessAllowedHookFunction hook)
{
	ms_accessAllowedHookFunction = hook;
}

// ----------------------------------------------------------------------

void CellProperty::addToRenderWorld (Object & object)
{
	if (ms_addToRenderWorldHook)
		ms_addToRenderWorldHook(object);
}

// ----------------------------------------------------------------------

Appearance *CellProperty::createPortalBarrier(VertexList const & verts, const VectorArgb &color)
{
	if (ms_portalBarrierFactory)
		return ms_portalBarrierFactory(verts,color);

	return nullptr;
}

// ----------------------------------------------------------------------

Appearance *CellProperty::createForceField(VertexList const & verts, const VectorArgb &color)
{
	if (ms_forceFieldFactory)
		return ms_forceFieldFactory(verts,color);

	return nullptr;
}

// ----------------------------------------------------------------------

void CellProperty::setCreateAndDestroyDpvsCellHookFunctions(CreateDpvsCellHookFunction createDpvsCellHookFunction, DestroyDpvsCellHookFunction destroyDpvsCellHookFunction)
{
	ms_createDpvsCellHookFunction = createDpvsCellHookFunction;
	ms_destroyDpvsCellHookFunction = destroyDpvsCellHookFunction;
}

// ----------------------------------------------------------------------

CellProperty *CellProperty::getWorldCellProperty()
{
	return ms_worldCellProperty;
}

// ----------------------------------------------------------------------

PropertyId CellProperty::getClassPropertyId()
{
	return PROPERTY_HASH(CellProperty, 0x9C3DFF88);
}

// ----------------------------------------------------------------------

void CellProperty::addPortalCrossingNotification(Object &object)
{
	object.addNotification(ms_portalCrossingNotification);
}

// ----------------------------------------------------------------------

void CellProperty::removePortalCrossingNotification(Object &object)
{
	object.removeNotification(ms_portalCrossingNotification);
}

// ----------------------------------------------------------------------

void CellProperty::setPortalTransitionsEnabled(bool enabled)
{
	Notification::setPortalTransitionsEnabled(enabled);
}

// ======================================================================

CellProperty::CellProperty(Object &owner)
: Container(getClassPropertyId(), owner),
	m_portalProperty(nullptr),
	m_cellIndex(-1),
	m_appearanceObject(nullptr),
	m_portalObjectList(new PortalObjectList),
	m_visible(false),
	m_floor(nullptr),
	m_cellName(nullptr),
	m_cellNameCrc(0),
	m_dpvsCell(nullptr),
	m_environmentTexture(nullptr),
	m_fogEnabled(false),
	m_fogColor(0),
	m_fogDensity(0.f),
	m_appliedInteriorLayout(false),
	m_preVisibilityTraversalRenderHookFunctionList(nullptr),
	m_enterRenderHookFunctionList(nullptr),
	m_preDrawRenderHookFunctionList(nullptr),
	m_exitRenderHookFunctionList(nullptr)
{
	if (ms_createDpvsCellHookFunction)
		m_dpvsCell = (*ms_createDpvsCellHookFunction)(this);
}

// ----------------------------------------------------------------------

CellProperty::~CellProperty()
{
	DEBUG_FATAL(this != ms_worldCellProperty && m_portalObjectList->size() > 1, ("CellProperty is still attached"));
	DEBUG_FATAL(this == ms_worldCellProperty && m_portalObjectList->size() > 0, ("Other CellProperties are still attached to world"));

	if (m_visible && ms_deleteVisibleCellProperty)
		(*ms_deleteVisibleCellProperty)(this);

	if (m_environmentTexture)
	{
		NOT_NULL(ms_textureRelease);
		ms_textureRelease(m_environmentTexture);
		m_environmentTexture = nullptr;
	}

	if (!m_portalObjectList->empty())
	{
		PortalList *portalList = m_portalObjectList->front().portalList;
		PortalList::iterator iEnd = portalList->end();
		for (PortalList::iterator i = portalList->begin(); i != iEnd; ++i)
			delete *i;
		delete portalList;
	}

	m_portalProperty = nullptr;
	delete m_appearanceObject;
	delete m_portalObjectList;
	delete m_floor;
	m_cellName = nullptr;
	m_cellNameCrc = 0;

	if (m_dpvsCell)
	{
		NOT_NULL(ms_destroyDpvsCellHookFunction);
		(*ms_destroyDpvsCellHookFunction)(m_dpvsCell);
		m_dpvsCell = nullptr;
	}

	delete m_preVisibilityTraversalRenderHookFunctionList;
	delete m_enterRenderHookFunctionList;
	delete m_preDrawRenderHookFunctionList;
	delete m_exitRenderHookFunctionList;
}

// ----------------------------------------------------------------------

void CellProperty::initialize(const PortalProperty &portalProperty, int cellIndex, bool createAppearance)
{
	m_portalProperty = &portalProperty;
	const PortalPropertyTemplateCell &cellTemplate = portalProperty.getPortalPropertyTemplate().getCell(cellIndex);

	PortalObjectEntry poe;
	poe.portalProperty = &portalProperty;
	poe.portalList     = cellTemplate.createPortalList(this, &getOwner());
	poe.attached       = false;
	m_portalObjectList->push_back(poe);	

	if (createAppearance)
	{
		DEBUG_FATAL(m_appearanceObject, ("appearance already created"));
		m_appearanceObject = new Object;
		m_appearanceObject->setDebugName("Cell appearance object");
		m_appearanceObject->attachToObject_p(&getOwner(), true);
		Appearance * const appearance = AppearanceTemplateList::createAppearance(cellTemplate.getAppearanceName());

		if (appearance != nullptr) {
			appearance->setShadowBlobAllowed();
			m_appearanceObject->setAppearance(appearance);

			if (ms_addToRenderWorldHook)
				ms_addToRenderWorldHook(*m_appearanceObject);
		} else {
			DEBUG_WARNING(true, ("FIX ME: Appearance template in CellProperty::initialize missing"));
		}
	}

	// ----------
	// If this cell has a floor, try and load the floor mesh

	if (cellTemplate.getFloorName())
	{
		m_floor = FloorManager::createFloor(cellTemplate.getFloorName(),&getOwner(),nullptr,false);
		DEBUG_WARNING(!m_floor, ("Cell %s could not load floor %s", cellTemplate.getAppearanceName(), cellTemplate.getFloorName()));
	}
	else
	{
		DEBUG_WARNING(true, ("Cell %s has no floor", cellTemplate.getAppearanceName()));
	}

	m_cellIndex = cellIndex;
	m_cellName = cellTemplate.getName();
	m_cellNameCrc = Crc::normalizeAndCalculate(m_cellName);
}

// ----------------------------------------------------------------------

int CellProperty::depersistContents(const Object& item)
{
	setPortalTransitionsEnabled(false);
		Transform t = item.getTransform_o2p();
		const_cast<Object&>(item).setParentCell(this);
		const_cast<Object&>(item).setTransform_o2p(t);
	setPortalTransitionsEnabled(true);

	return Container::depersistContents(item);
}

// ----------------------------------------------------------------------

bool CellProperty::internalItemRemoved(const Object& item)
{
	//-- HACK: the client should just chain up
	{
		if (ms_createDpvsCellHookFunction)
			return Container::internalItemRemoved(item);
	}

	if (Container::internalItemRemoved(item))
	{
		if (item.getAttachedTo())
			const_cast<Object &>(item).detachFromObject(Object::DF_parent);
		
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool CellProperty::mayAdd(const Object & object, ContainerErrorCode& error) const
{
	bool retval = (getOwner().isAuthoritative() == object.isAuthoritative());
	error = retval ? CEC_Success : CEC_NoPermission;

	return retval;
}

// ----------------------------------------------------------------------

bool CellProperty::remove(Object &item, ContainerErrorCode& error)
{
	if (Container::remove(item, error))
	{
		item.Object::setParentCell(getWorldCellProperty());
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool CellProperty::remove(ContainerIterator &pos, ContainerErrorCode& error)
{
	Object *item = (*pos).getObject();
	if (Container::remove(pos, error) && item)
	{
		item->detachFromObject(Object::DF_parent);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool CellProperty::isWorldCell() const
{
	return this == ms_worldCellProperty;
}

// ----------------------------------------------------------------------

int  CellProperty::getTypeId() const
{
	return static_cast<int>(TAG_CELP);
}

// ----------------------------------------------------------------------

void CellProperty::debugPrint(std::string &buffer) const
{
	char tempBuffer[1024];

	buffer += "====[BEGIN: cell property]====\n";

		sprintf(tempBuffer, "container id [%s].\n", getOwner().getNetworkId().getValueString().c_str());
		buffer += tempBuffer;

		buffer += "embedding container contents now:\n";
		Container::debugPrint(buffer);

	buffer += "====[END:   cell property]====\n";
}

// ----------------------------------------------------------------------

void CellProperty::removeFromWorld()
{
	Container::removeFromWorld();

	if (!m_portalObjectList->empty())
	{
		PortalList *portalList = m_portalObjectList->front().portalList;
		PortalList::iterator iEnd = portalList->end();

		for (PortalList::iterator i = portalList->begin(); i != iEnd; ++i)
		{
			Portal * const portal = *i;
			portal->removeFromDpvs();
		}
	}

	m_appliedInteriorLayout = false;
}

// ----------------------------------------------------------------------

int CellProperty::getPortalCount() const
{
	int counter = 0;

	const PortalObjectList::const_iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::const_iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
	{
		const PortalList &portalList   = *i->portalList;

		counter += static_cast<int>(portalList.size());
	}

	return counter;
}

// ----------------------------------------------------------------------

Object *CellProperty::getAppearanceObject()
{
	return m_appearanceObject;
}

// ----------------------------------------------------------------------

const Object *CellProperty::getAppearanceObject() const
{
	NOT_NULL(m_appearanceObject);
	return m_appearanceObject;
}

// ----------------------------------------------------------------------

bool CellProperty::areAdjacent(const CellProperty *cellProperty1, const CellProperty *cellProperty2)
{
	bool result = false;

	if (   (cellProperty1 != nullptr)
	    && (cellProperty2 != nullptr))
	{
		if (cellProperty1 == cellProperty2)
		{
			// Same cell property means they are adjacent

			result = true;
		}
		else if (   (cellProperty1 != getWorldCellProperty())
		         && (cellProperty2 != getWorldCellProperty())
				 && (cellProperty1->getPortalProperty() != cellProperty2->getPortalProperty()))
		{
			// Both cell properties are not the world cell property and
			// they do not share the same portal property

			result = false;
		}
		else if (   (cellProperty1->m_portalObjectList != nullptr)
		         && (cellProperty2->m_portalObjectList != nullptr))
		{
			// Pick the list that is not the world cell property, or the
			// smallest list

			PortalObjectList *portalObjectList = cellProperty1->m_portalObjectList;
			CellProperty const *checkCellProperty = cellProperty2;

			if (   (cellProperty1 == getWorldCellProperty())
			    || ((cellProperty2 != getWorldCellProperty())
				&&  (cellProperty1->m_portalObjectList->size() > cellProperty2->m_portalObjectList->size())))
			{
				portalObjectList = cellProperty2->m_portalObjectList;
				checkCellProperty = cellProperty1;
			}

			if (portalObjectList != nullptr)
			{
				//DEBUG_REPORT_LOG((portalObjectList->size() > 1), ("Portal Object List > 1 - size: %d", portalObjectList->size()));

				PortalObjectList::const_iterator iterPortalObjectList = portalObjectList->begin();

				for (; iterPortalObjectList != portalObjectList->end(); ++iterPortalObjectList)
				{
					// Check all the portals in this portal object

					const PortalList &portalList = *(iterPortalObjectList->portalList);

					PortalList::const_iterator iterPortalList = portalList.begin();

					for (; iterPortalList != portalList.end(); ++iterPortalList)
					{
						// See if the cell property we are looking for is on the
						// other side of any of the portals

						const Portal * const portal = (*iterPortalList);
						const CellProperty * const cellProperty = portal->getNeighbor()->getParentCell();

						if (cellProperty == checkCellProperty)
						{
							result = true;
							break;
						}
					}
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

float CellProperty::getMaximumValidCellSpaceCoordinate()
{
	return ms_maximumValidCellSpaceCoordinate;
}

// ----------------------------------------------------------------------

void CellProperty::releaseWorldCellPropertyEnvironmentTexture()
{
	if (ms_worldCellProperty)
		ms_worldCellProperty->setEnvironmentTexture(0);
}

// ----------------------------------------------------------------------
/**
 * Check line segment for portal crossing.
 *
 * @param startPosition Starting position, in the space of this cell.
 * @param endPosition   Ending position, in the space of this cell.
 * @return  The CellProperty that an object traversing the object is in.  Will return nullptr if remaining in this cell.
 */
 
CellProperty *CellProperty::getDestinationCell(const Vector &startPosition, const Vector &endPosition, float &closestPortalT, bool passableOnly) const
{
	CellProperty *cellProperty = 0;
	closestPortalT = FLT_MAX;

	const PortalObjectList::const_iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::const_iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
	{
		const Object     &portalObject = i->portalProperty->getOwner();
		const PortalList &portalList   = *i->portalList;

		const PortalList::const_iterator jEnd = portalList.end();
		for (PortalList::const_iterator j = portalList.begin(); j != jEnd; ++j)
		{
			const Portal *portal = *j;

			if (passableOnly && !portal->isPassable())
				continue;

			float t = FLT_MAX;
			bool penetrated = true;

			if (i->attached)
			{
				const Vector b = portalObject.rotateTranslate_w2o(getOwner().rotateTranslate_o2w(startPosition));
				const Vector e = portalObject.rotateTranslate_w2o(getOwner().rotateTranslate_o2w(endPosition));
				penetrated = portal->testPenetration(b, e, t);
			}
			else
			{
				penetrated = portal->testPenetration(startPosition, endPosition, t);
			}

			if (penetrated && t <= closestPortalT)
			{
				closestPortalT = t;
				cellProperty = portal->getNeighbor() ? portal->getNeighbor()->getParentCell() : 0;
			}
		}
	}

	return cellProperty;
}

// ----------------------------------------------------------------------

CellProperty *CellProperty::getDestinationCell(const Object *object, int portalId) const
{
	if(portalId < 0) return nullptr;
	if(object == nullptr) return nullptr;

	const PortalObjectList::const_iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::const_iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
	{
		const Object     &portalObject = i->portalProperty->getOwner();
		const PortalList &portalList   = *i->portalList;

		if(this == CellProperty::getWorldCellProperty())
		{
			// We only want to check the list of portals for this cell object
			if(&portalObject != object) continue;
		}
		else
		{
			if(i->portalProperty != m_portalProperty) continue;
		}

		if(portalId >= static_cast<int>(portalList.size()))
		{
			DEBUG_WARNING(true,("CellProperty::getDestinationCell(portalId) - tried to get an invalid portal\n"));

			return nullptr;
		}

		return portalList[static_cast<PortalList::size_type>(portalId)]->getNeighbor()->getParentCell();
	}

	return nullptr;
}

// ----------------------------------------------------------------------
// Return the set of portals the sphere goes through

bool CellProperty::getDestinationCells(const Sphere &sphere, std::vector<CellProperty*> & outCells) const
{
	bool hitPortal = false;

	Sphere testSphere = sphere;

	const PortalObjectList::const_iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::const_iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
	{
		const Object     &portalObject = i->portalProperty->getOwner();
		const PortalList &portalList   = *i->portalList;

		if (i->attached)
		{
			testSphere.setCenter( portalObject.rotateTranslate_w2o(getOwner().rotateTranslate_o2w(sphere.getCenter())) );
		}
		else
		{
			testSphere.setCenter( sphere.getCenter() );
		}

		const PortalList::const_iterator jEnd = portalList.end();
		for (PortalList::const_iterator j = portalList.begin(); j != jEnd; ++j)
		{
			const Portal *portal = *j;

			if(portal->testOverlap(testSphere))
			{
				hitPortal = true;

				const Portal *neighbor = portal->getNeighbor();

				WARNING(neighbor == nullptr,("CellProperty::getDestinationCells - Cell %s has a portal without a neighbor\n",getOwner().getNetworkId().getValueString().c_str()));

				if(neighbor)
				{
					CellProperty *neighborCell = neighbor->getParentCell();

					if(neighborCell)
					{
						outCells.push_back(neighborCell);
					}
				}
			}
		}
	}

	return hitPortal;
}

// ----------------------------------------------------------------------

bool CellProperty::isAdjacentTo(const CellProperty *cell) const
{
	if(this == cell) return true;

	if(cell == nullptr) return false;

	// ----------

	const PortalObjectList::const_iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::const_iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
	{
		const PortalList &portalList   = *i->portalList;

		const PortalList::const_iterator jEnd = portalList.end();
		for (PortalList::const_iterator j = portalList.begin(); j != jEnd; ++j)
		{
			const Portal *portal = *j;

			CellProperty const * const neighbor = portal->getNeighbor() ? portal->getNeighbor()->getParentCell() : 0;
			if (neighbor == cell) 
				return true;
		}
	}

	// ----------

	return false;
}

// ----------------------------------------------------------------------

void CellProperty::attach(const PortalProperty &portalProperty, PortalList *portalList)
{
	PortalObjectEntry poe;
	poe.portalProperty = &portalProperty;
	poe.portalList     = portalList;
	poe.attached       = true;
	m_portalObjectList->push_back(poe);
}

// ----------------------------------------------------------------------

void CellProperty::detach(const PortalProperty &portalProperty)
{
	const PortalObjectList::iterator iEnd = m_portalObjectList->end();
	for (PortalObjectList::iterator i = m_portalObjectList->begin(); i != iEnd; ++i)
		if (i->portalProperty == &portalProperty)
		{
			PortalList *portalList = i->portalList;
			PortalList::iterator jEnd = portalList->end();
			for (PortalList::iterator j = portalList->begin(); j != jEnd; ++j)
				delete *j;

			delete portalList;
			IGNORE_RETURN(m_portalObjectList->erase(i));
			return;
		}

	DEBUG_FATAL(true, ("could not find attached portal object"));
}

// ----------------------------------------------------------------------

void CellProperty::addObject_w(Object &object)
{
	// This is the ONLY TIME that we should be explicitly calling Object::setParentCell - 
	// normally the client should call setParentCell virtually, and the server should always
	// go through ContainerInterface::transferItemToCell
	object.Object::setParentCell(this);


	if (!isWorldCell())
	{
		ContainerErrorCode tmp = CEC_Success;
		IGNORE_RETURN(addToContents(object, tmp));
	}
	else
		DEBUG_WARNING(true, ("attempting to add object %s to world cell", object.getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

Floor *CellProperty::getFloor()
{
	return m_floor;
}

// ----------------------------------------------------------------------

const Floor *CellProperty::getFloor() const
{
	return m_floor;
}

// ----------------------------------------------------------------------

const BaseExtent *CellProperty::getCollisionExtent() const
{
	if(m_portalProperty)
	{
		return m_portalProperty->getPortalPropertyTemplate().getCell(m_cellIndex).getCollisionExtent();
	}
	else
	{
		return nullptr;
	}
}

// ----------------------------------------------------------------------

const BaseClass *CellProperty::getPathGraph() const
{
	if(m_floor)
	{
		FloorMesh const * floorMesh = m_floor->getFloorMesh();

		if(floorMesh)
		{
			return floorMesh->getPathGraph();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

void CellProperty::addHookFunction(RenderHookFunctionList *&renderHookFunctionList, RenderHookFunction renderHookFunction) const
{
	if (!renderHookFunctionList)
		renderHookFunctionList = new RenderHookFunctionList;

	DEBUG_FATAL(std::find(renderHookFunctionList->begin(), renderHookFunctionList->end(), renderHookFunction) != renderHookFunctionList->end(), ("Hook function already present"));
	renderHookFunctionList->push_back(renderHookFunction);
}

// ----------------------------------------------------------------------

void CellProperty::removeHookFunction(RenderHookFunctionList *const &renderHookFunctionList, RenderHookFunction renderHookFunction) const
{
	NOT_NULL(renderHookFunctionList);

	RenderHookFunctionList::iterator i = std::find(renderHookFunctionList->begin(), renderHookFunctionList->end(), renderHookFunction);
	DEBUG_FATAL(i == renderHookFunctionList->end(), ("Hook function not present"));
	IGNORE_RETURN(renderHookFunctionList->erase(i));
}

// ----------------------------------------------------------------------

void CellProperty::callRenderHookFunctions(RenderHookFunctionList *const &renderHookFunctionList) const
{
	if (renderHookFunctionList)
	{
		const RenderHookFunctionList::const_iterator iEnd = renderHookFunctionList->end();
		for (RenderHookFunctionList::const_iterator i = renderHookFunctionList->begin(); i != iEnd; ++i)
			(*i)();
	}
}

// ----------------------------------------------------------------------

void CellProperty::addPreVisibilityTraversalRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	addHookFunction(m_preVisibilityTraversalRenderHookFunctionList, renderHookFunction);
}

// ----------------------------------------------------------------------

void CellProperty::removePreVisibilityTraversalRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	removeHookFunction(m_preVisibilityTraversalRenderHookFunctionList, renderHookFunction);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::callPreVisibilityTraversalRenderHookFunctions() const
{
	callRenderHookFunctions(m_preVisibilityTraversalRenderHookFunctionList);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::addEnterRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	addHookFunction(m_enterRenderHookFunctionList, renderHookFunction);
}

// ----------------------------------------------------------------------

void CellProperty::removeEnterRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	removeHookFunction(m_enterRenderHookFunctionList, renderHookFunction);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::callEnterRenderHookFunctions() const
{
	callRenderHookFunctions(m_enterRenderHookFunctionList);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::addPreDrawRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	addHookFunction(m_preDrawRenderHookFunctionList, renderHookFunction);
}

// ----------------------------------------------------------------------

void CellProperty::removePreDrawRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	removeHookFunction(m_preDrawRenderHookFunctionList, renderHookFunction);
}  //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::callPreDrawRenderHookFunctions() const
{
	callRenderHookFunctions(m_preDrawRenderHookFunctionList);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::addExitRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	addHookFunction(m_exitRenderHookFunctionList, renderHookFunction);
}

// ----------------------------------------------------------------------

void CellProperty::removeExitRenderHookFunction(RenderHookFunction renderHookFunction) const
{
	removeHookFunction(m_exitRenderHookFunctionList, renderHookFunction);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::callExitRenderHookFunctions() const
{
	callRenderHookFunctions(m_exitRenderHookFunctionList);
} //lint !e1763 // Member function marked const indirectly modifies class // intentional

// ----------------------------------------------------------------------

void CellProperty::drawDebugShapes(DebugShapeRenderer * const renderer) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if (renderer == nullptr) 
		return;

	Floor const * const floor = getFloor();

	if (floor)
		floor->drawDebugShapes(renderer,false);

	if (ms_renderPortals)
	{
		for (uint i = 0; i < m_portalObjectList->size(); ++i)
		{
			PortalObjectEntry const & entry = (*m_portalObjectList)[i];

			for (uint j = 0; j < entry.portalList->size(); ++j)
			{
				Portal const * portal = NON_NULL((*entry.portalList)[j]);
				IndexedTriangleList const & portalGeometry = portal->getGeometry();

				renderer->setDepthTest(false);
				renderer->setColor(portal->isClosed() ? VectorArgb::solidYellow : VectorArgb::solidWhite);
				renderer->drawMesh(&portalGeometry);
				renderer->setDepthTest(true);
			}
		}
	}

	PortalPropertyTemplateCell const & cellTemplate = m_portalProperty->getPortalPropertyTemplate().getCell(m_cellIndex);

	BaseExtent const * collisionExtent = cellTemplate.getCollisionExtent();
	
	if(ConfigSharedCollision::getDrawExtents() && collisionExtent) 
	{
		collisionExtent->drawDebugShapes(renderer);
	}

	UNREF(cellTemplate);

#endif
}

// ----------------------------------------------------------------------

Portal *CellProperty::getPortal(int portalIndex)
{
	Portal * result = 0;
	if(m_portalObjectList)
	{
		if(m_portalObjectList->front().portalList)
		{
			result = m_portalObjectList->front().portalList->operator[](static_cast<PortalList::size_type>(portalIndex));
		}
	}
	return result;
}

// ----------------------------------------------------------------------

int CellProperty::getNumberOfPortalObjects() const
{
	return static_cast<int>(m_portalObjectList->size());
}

// ----------------------------------------------------------------------

CellProperty::PortalObjectEntry const &CellProperty::getPortalObject(int index) const
{
	return (*m_portalObjectList)[static_cast<PortalObjectList::size_type>(index)];
}

// ----------------------------------------------------------------------
/**
 * Return true if no access allowed hook function is set; otherwise, return
 * the value returned by the AccessAllowedHookFunction.
 *
 * @return true if no access allowed hook function is set; otherwise, return
 * the value returned by the AccessAllowedHookFunction.
 */

bool CellProperty::getAccessAllowed() const
{
	if (ms_accessAllowedHookFunction != nullptr)
		return (*ms_accessAllowedHookFunction)(*this);
	else
		return true;
}

// ----------------------------------------------------------------------

void CellProperty::setEnvironmentTexture(Texture const * environmentTexture)
{
	NOT_NULL(ms_textureFetch);
	NOT_NULL(ms_textureRelease);

	if (environmentTexture)
		ms_textureFetch(environmentTexture);

	if (m_environmentTexture)
		ms_textureRelease(m_environmentTexture);

	m_environmentTexture = environmentTexture;
}

// ----------------------------------------------------------------------

bool CellProperty::isFogEnabled() const
{
	return m_fogEnabled;
}

// ----------------------------------------------------------------------

void CellProperty::setFogEnabled(bool fogEnabled)
{
	m_fogEnabled = fogEnabled;
}

// ----------------------------------------------------------------------

const PackedArgb &CellProperty::getFogColor() const
{
	return m_fogColor;
}

// ----------------------------------------------------------------------

void CellProperty::setFogColor(const PackedArgb &fogColor)
{
	m_fogColor = fogColor;
}

// ----------------------------------------------------------------------

float CellProperty::getFogDensity() const
{
	return m_fogDensity;
}

// ----------------------------------------------------------------------

void CellProperty::setFogDensity(float fogDensity)
{
	m_fogDensity = fogDensity;
}

// ----------------------------------------------------------------------

bool CellProperty::getAppliedInteriorLayout() const
{
	return m_appliedInteriorLayout;
}

// ----------------------------------------------------------------------

void CellProperty::setAppliedInteriorLayout() const
{
	m_appliedInteriorLayout = true;
}

// ----------------------------------------------------------------------

bool CellProperty::isContentItemExposedWith(Object const &item) const
{
	// Cell contents are always exposed to the world with the cell.
	UNREF(item);
	return true;
}

// ----------------------------------------------------------------------

Vector const CellProperty::getPosition_w(Location const & location)
{
	Vector result = location.getCoordinates();
	CellProperty const * const worldCellProperty = CellProperty::getWorldCellProperty();

	if (worldCellProperty != nullptr)
	{
		if (location.getCell() != worldCellProperty->getOwner().getNetworkId())
		{
			// This is not the world cell, rotate the cell position into
			// world space

			Object const * const cell = NetworkIdManager::getObjectById(location.getCell());

			if (cell != nullptr)
			{
				result = cell->rotateTranslate_o2w(location.getCoordinates());
			}
		}
	}

	return result;
}

// ======================================================================
