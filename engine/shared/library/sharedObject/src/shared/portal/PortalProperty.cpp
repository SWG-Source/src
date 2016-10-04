// ======================================================================
//
// PortalProperty.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/PortalProperty.h"

#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Portal.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/PortalPropertyTemplateList.h"

#include <algorithm>
#include <limits>
#include <cstdio>

// ======================================================================

const Tag TAG_PRTP = TAG(P,R,T,P);

// ======================================================================

PortalProperty::BeginCreateObjectFunction  PortalProperty::ms_beginCreateObjectFunction;
PortalProperty::EndCreateObjectFunction    PortalProperty::ms_endCreateObjectFunction;

// ======================================================================

void PortalProperty::install(BeginCreateObjectFunction beginCreateObjectFunction, EndCreateObjectFunction endCreateObjectFunction)
{
	ms_beginCreateObjectFunction = beginCreateObjectFunction;
	ms_endCreateObjectFunction = endCreateObjectFunction;
	Portal::install();
}

// ----------------------------------------------------------------------

PropertyId PortalProperty::getClassPropertyId()
{
	return PROPERTY_HASH(PortalObjectProperty, 0x939616B9);
}

// ======================================================================

PortalProperty::PortalProperty(Object &owner, const char *fileName)
: Container(getClassPropertyId(), owner),
	m_template(nullptr),
	m_cellList(new CellList),
	m_fixupList(nullptr),
	m_hasPassablePortalToParentCell(false)
{
#ifdef _DEBUG
	DataLint::pushAsset(fileName);
#endif // _DEBUG

	m_template = PortalPropertyTemplateList::fetch(CrcLowerString(fileName));
	m_cellList->resize(static_cast<CellList::size_type>(m_template->getNumberOfCells()), nullptr);

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG
}

// ----------------------------------------------------------------------

PortalProperty::~PortalProperty()
{
	delete m_cellList;
	delete m_fixupList;
	if (m_template)
	{
		m_template->release();
		m_template = 0;
	}

	m_fixupList = 0;
}

// ----------------------------------------------------------------------

void PortalProperty::initializeFirstTimeObject()
{
	// the first cell is the parent cell, so don't actually create an object/cell pair for it
	int numberOfCells = m_template->getNumberOfCells();
	for (int i = 1; i < numberOfCells; ++i)
	{
		ContainerErrorCode tmp = CEC_Success;
		Object *object = ms_beginCreateObjectFunction(i);
		IGNORE_RETURN(addToContents(*object, tmp));
		cellLoaded(i, *object, false);
		if (ms_endCreateObjectFunction)
			ms_endCreateObjectFunction(object);
	} //lint !e429 // Custodial pointer not freed or returned
}

// ----------------------------------------------------------------------

void PortalProperty::clientSinglePlayerInitializeFirstTimeObject()
{
	// the first cell is the parent cell, so don't actually create an object/cell pair for it
	int numberOfCells = m_template->getNumberOfCells();
	for (int i = 1; i < numberOfCells; ++i)
	{
		ContainerErrorCode tmp = CEC_Success;
		Object *object = ms_beginCreateObjectFunction(i);
		IGNORE_RETURN(addToContents(*object, tmp));
		if (ms_endCreateObjectFunction)
			ms_endCreateObjectFunction(object);
	} //lint !e429 // Custodial pointer not freed or returned
}

// ----------------------------------------------------------------------

const PortalPropertyTemplate &PortalProperty::getPortalPropertyTemplate() const
{
	NOT_NULL(m_template);
	return *m_template;
}

// ----------------------------------------------------------------------

void PortalProperty::addToWorld()
{
#ifdef _DEBUG
	{
		int unloaded = 0;
		int const numberOfCells = static_cast<int>(m_cellList->size());
		for (int i = 1; i < numberOfCells; ++i)
			if ((*m_cellList)[static_cast<CellList::size_type>(i)] == nullptr)
			{
				WARNING(true, ("cell %d/%d not loaded", i, numberOfCells));
				++unloaded;
			}
		DEBUG_FATAL(unloaded , ("%d/%d cells not loaded", unloaded, numberOfCells-1));
	}
#endif


	CellProperty *parentCell = getOwner().getParentCell();

	// create the portals that lead into this portal property
	PortalList *portalList = m_template->createExteriorPortalList(parentCell, &getOwner());

	// now that all the portals have been created, let every portal know who its neighbor is
	PortalPropertyTemplate::PortalOwnersList const &portalOwnersList = m_template->getPortalOwnersList();
	PortalPropertyTemplate::PortalOwnersList::const_iterator const iEnd = portalOwnersList.end();
	for (PortalPropertyTemplate::PortalOwnersList::const_iterator i = portalOwnersList.begin(); i != iEnd; ++i)
	{
		CellProperty *cell[2];
		Portal *portal[2];

		for (int j = 0; j < 2; ++j)
		{
			const int cellIndex = i->owners[j].cell;
			const int portalIndex = i->owners[j].portal;

			if (cellIndex == 0)
			{
				cell[j] = parentCell;
				portal[j] = (*portalList)[static_cast<PortalList::size_type>(portalIndex)];
				if (portal[j]->isPassable())
					m_hasPassablePortalToParentCell = true;
			}
			else
			{
				cell[j] = (*m_cellList)[static_cast<CellList::size_type>(cellIndex)];
				portal[j] = cell[j]->getPortal(portalIndex);
			}
		}

		portal[0]->setNeighbor(portal[1]);
		portal[1]->setNeighbor(portal[0]);
	}

	parentCell->attach(*this, portalList);
	Container::addToWorld();
}

// ----------------------------------------------------------------------

void PortalProperty::removeFromWorld()
{
	Container::removeFromWorld();
	CellProperty *parentCell = getOwner().getParentCell();
	parentCell->detach(*this);
}

// ----------------------------------------------------------------------
	
bool PortalProperty::serverEndBaselines(int serverObjectCrc, std::vector<Object*> &unfixables, bool authoritative)
{
	bool retval = false;
	ContainerErrorCode tmp = CEC_Success;

	// check if the CRC has changed cells
	const int crc = m_template->getCrc();
	if (authoritative && (crc == 0 || serverObjectCrc != crc))
	{
		retval = true;

		// create any missing cells
		uint const numberOfCells = m_cellList->size();
		for (uint i = 1; i < numberOfCells; ++i)
		{
			if ((*m_cellList)[i] == nullptr)
			{
				Object *object = ms_beginCreateObjectFunction(static_cast<int>(i));
				IGNORE_RETURN(addToContents(*object, tmp));
				cellLoaded(static_cast<int>(i), *object, false);
				if (ms_endCreateObjectFunction)
					ms_endCreateObjectFunction(object);
			} //lint !e429 // Custodial pointer not freed or returned
		}

		//-- jww: this fixup code is broken
		//-- fixupObject() causes the fixedup object to get added to the world before its parent has been
		//-- for now, we won't queue anything for fixup

#if 0	//TODO: see above
		// go through the cells and remove objects 
		for (size_t cell=1 ; cell< numberOfCells; ++cell)
		{
			for (ContainerIterator cellContents=(*m_cellList)[cell]->begin(); cellContents != (*m_cellList)[cell]->end(); ++cellContents)
			{
				Object *obj = (*cellContents).getObject();
				if (obj)
				{
					queueObjectForFixup(*obj);
					IGNORE_RETURN((*m_cellList)[cell]->remove(cellContents, tmp)); // does not invalidate the iterator
				}
			}
		}

#endif

	}
#ifdef _DEBUG
	else
	{
		if (ConfigSharedObject::getValidateCellContentsAttached())
		{
			// make sure the contents of every cell is attached to the cell
			uint const numberOfCells = m_cellList->size();
			for (size_t cellIndex = 1 ; cellIndex < numberOfCells; ++cellIndex)
			{
				CellProperty *cell = (*m_cellList)[cellIndex];
				DEBUG_WARNING(! cell, ("Expected to get a cell, none was returned"));
				if(cell)
				{
					Object const * const cellObject = &cell->getOwner();
					for (ContainerIterator cellContents=cell->begin(); cellContents != cell->end(); ++cellContents)
					{
						Object *obj = (*cellContents).getObject();
						DEBUG_FATAL(obj && obj->getAttachedTo() != cellObject, ("Cell content not attached to cell"));
						if (obj && obj->getAttachedTo() != cellObject)
						{
							WARNING(true, ("CellProblem Cell content not attached to cell"));
						}
					}
				}
			}
		}
	}
#endif

	// For each object, find the appropriate cell and place the object in it
	// Note that objects can be put in m_fixupList from other places, not just the above code)
	if (m_fixupList)
	{
		for (FixupList::iterator fixRec=(*m_fixupList).begin(); fixRec!=(*m_fixupList).end(); ++fixRec)
		{
			// put object into appropriate cell
			if (!fixupObject(*((*fixRec).m_obj), (*fixRec).m_transform))
				unfixables.push_back((*fixRec).m_obj);
		}

		delete m_fixupList;
		m_fixupList = 0;
	}
	return retval;
}

// ----------------------------------------------------------------------

/**
 * Given an object, attempt to put it in the POB in the best cell.
 * This function is used when the POB has changed or for player logins.
 * @param object            The object to place.
 * @param intendedTransform Where the object should be placed, relative to the POB.
 *                          May adjust the postion in the transform to make sure
 *                          the object is on a floor, but will preserve the rotation.
 * @return true on success, false if the object cannot be placed.
 * @see PortalProperty::serverEndBaselines
 */
bool PortalProperty::fixupObject (Object &object, Transform intendedTransform)
{
	Object *cell;
	Vector newPosition;
	if (findContainingCell(intendedTransform.getPosition_p(),newPosition,cell))
	{
		CellProperty *cellProperty = cell->getCellProperty();
		if (cellProperty)
		{
			//FIX HERE.  Set the transform before calling add object_w which adds it to the world.
			cellProperty->addObject_w(object);
			intendedTransform.setPosition_p(newPosition);
			object.setTransform_o2p(intendedTransform);
			if (!object.isInWorld())
				object.addToWorld();
			
			object.pobFixupComplete();
			return true;
		}
		else
		{
			WARNING_STRICT_FATAL(true,("findContainingCell returned object %s, which that lacks a cellProperty.\n",cell->getNetworkId().getValueString().c_str()));
		}
	}
	
	return false;
}

// ----------------------------------------------------------------------

/**
 * @param buildingPos - Position inside the building we're trying to find a cell for
 *
 * @param outPos - Position on a floor in the building that's close to the test position
 * @param outCell - Cell object in the building the test position is in
 *
 * @return - True if we found a position, false if not.
 */
bool PortalProperty::findContainingCell(Vector const &buildingPos, Vector &outPos, Object *&outCell) 
{
	Object &buildingObject = getOwner();
	
    // ----------
    // Object is a portallized object. 
    // Search through its cells to find the floor closest to the test point

    int minChild = -1;
    Vector minPos = Vector::zero;
    float minDist = REAL_MAX;

    for(int i = 0; i < buildingObject.getNumberOfChildObjects(); i++)
    {
        // ----------
        // Skip all child objects that aren't cells or that don't have a floor

        Object const * const childObject = buildingObject.getChildObject(i);
        if (!childObject) 
			continue;

        CellProperty const * const cellProperty = childObject->getCellProperty();
        if (!cellProperty) 
			continue;

        Floor const * const floor = cellProperty->getFloor();
        if (!floor) 
			continue;

        // ----------
        // Found a cell with a floor

        FloorLocator loc;

        if(floor->findClosestLocation(buildingPos,loc))
        {
            Vector floorPos = loc.getPosition_p();
        
            float dist = (floorPos - buildingPos).magnitude();

            if(dist < minDist)
            {
                minChild = i;
                minPos = floorPos;
                minDist = dist;
            }
        }
    }

    if(minChild != -1)
    {
        outPos.x = minPos.x;
		outPos.z = minPos.z;
		outPos.y = std::max(minPos.y,buildingPos.y);

        outCell = buildingObject.getChildObject(minChild);

        return true;
    }
    else
    {
        return false;
    }
}


// ----------------------------------------------------------------------

bool PortalProperty::mayAdd(const Object &, ContainerErrorCode& error) const
{
	error = CEC_Success;
	return true;
}

// ----------------------------------------------------------------------

bool PortalProperty::remove(Object &item, ContainerErrorCode& error)
{
	return Container::remove(item, error);
}

// ----------------------------------------------------------------------

bool PortalProperty::remove(ContainerIterator &pos, ContainerErrorCode& error)
{
	return Container::remove(pos, error);
}

// ----------------------------------------------------------------------

int  PortalProperty::getTypeId() const
{
	return static_cast<int>(TAG_PRTP);
}

// ----------------------------------------------------------------------

void PortalProperty::debugPrint(std::string &buffer) const
{
	char tempBuffer[1024];

	buffer += "====[BEGIN: portal property]====\n";

		sprintf(tempBuffer, "container id [%s].\n", getOwner().getNetworkId().getValueString().c_str());
		buffer += tempBuffer;

		buffer += "embedding container contents now:\n";
		Container::debugPrint(buffer);

	buffer += "====[END:   portal property]====\n";
}

// ----------------------------------------------------------------------

void PortalProperty::createAppearance()
{
	Appearance * const appearance = AppearanceTemplateList::createAppearance(m_template->getExteriorAppearanceName());
	if (appearance != nullptr) {
		appearance->setShadowBlobAllowed();
		getOwner().setAppearance(appearance);
	} else {
		DEBUG_WARNING(true, ("FIX ME: Appearance template for PortalProperty::createAppearance missing"));
	}
}

// ----------------------------------------------------------------------

char const *PortalProperty::getExteriorFloorName() const
{
	return m_template->getExteriorFloorName();
}

// ----------------------------------------------------------------------

void PortalProperty::cellLoaded(int cellIndex, Object &cellObject, bool shouldCreateAppearance)
{
	DEBUG_FATAL(cellIndex < 1 || cellIndex >= static_cast<int>(m_cellList->size()), ("invalid cell index %d/%d", cellIndex, static_cast<int>(m_cellList->size())));
	DEBUG_FATAL((*m_cellList)[static_cast<CellList::size_type>(cellIndex)], ("cell already loaded"));
	if ((*m_cellList)[static_cast<CellList::size_type>(cellIndex)])
	{
		WARNING(true, ("CellProblem for portal %s cell index %d already loaded", getOwner().getNetworkId().getValueString().c_str(), cellIndex));
	}

	// attach the cell as a child of the portal object
	cellObject.attachToObject_p(&getOwner(), false);

	// get the cell property from the cell object
	CellProperty *cell = cellObject.getCellProperty();

	// store that
	(*m_cellList)[static_cast<CellList::size_type>(cellIndex)] = cell;

	// let the cell know who its portal object is, and what its portal data is
	cell->initialize(*this, cellIndex, shouldCreateAppearance);
}

// ----------------------------------------------------------------------

int PortalProperty::getNumberOfCells() const
{
	return m_template->getNumberOfCells();
}

// ----------------------------------------------------------------------

CellProperty *PortalProperty::getCell(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCells());
	return (*m_cellList)[static_cast<CellList::size_type>(index)];
}

// ----------------------------------------------------------------------

const CellProperty *PortalProperty::getCell(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCells());
	return (*m_cellList)[static_cast<CellList::size_type>(index)];
}

// ----------------------------------------------------------------------

const PortalProperty::CellNameList &PortalProperty::getCellNames() const
{
	return m_template->getCellNames();
}

// ----------------------------------------------------------------------

CellProperty *PortalProperty::getCell(const char *desiredCellName)
{
	const int numberOfCells = getNumberOfCells();
	for (int i = 1; i < numberOfCells; ++i)
	{
		PortalPropertyTemplateCell const &cell = m_template->getCell(i);
		char const * const cellName = cell.getName();

		if (cellName && strcmp(desiredCellName, cellName) == 0)
			return (*m_cellList)[static_cast<CellList::size_type>(i)];
	}

	return nullptr;
}

// ----------------------------------------------------------------------

const CellProperty *PortalProperty::getCell(const char *cellName) const
{
	return const_cast<PortalProperty*>(this)->getCell(cellName);
}

// ----------------------------------------------------------------------

const char *PortalProperty::getCellAppearanceName(int index) const
{
	return m_template->getCell(index).getAppearanceName();
}

// ----------------------------------------------------------------------

/** 
 * Queue an object to be placed in the appropriate cell after EndBaseslines.
 * Use this function during database load when you find an object that cannot
 * be placed in a cell (due to deleted cells, etc.).  After the POB receives
 * EndBaselines, it will go through every object queued by this function and
 * find an appropriate cell for it.
 *
 * If the object is already in an (incorrect) cell, do not remove the object
 * from the cell before calling this function, because removing the object
 * will change its transform.  This function saves the transform, so you may
 * remove the object after calling this function.
 */

void PortalProperty::queueObjectForFixup(Object &object)
{
	FixupRec temp;
	temp.m_obj = &object;
	temp.m_transform = object.getTransform_o2p();  // removing the object changes its transform, so we save it now

	if (!m_fixupList)
		m_fixupList = new FixupList;
	m_fixupList->push_back(temp);
}

// ----------------------------------------------------------------------

int PortalProperty::getCrc() const
{
	return m_template->getCrc();
}

// ----------------------------------------------------------------------

const char *PortalProperty::getPobName() const
{
	return m_template->getCrcString().getString();
}

// ----------------------------------------------------------------------

const char *PortalProperty::getPobShortName() const
{
	return m_template->getShortName().getString();
}

// ----------------------------------------------------------------------

Transform const PortalProperty::getEjectionLocationTransform() const
{
	return m_template->getEjectionLocationTransform();
}

// ----------------------------------------------------------------------

bool PortalProperty::isContentItemObservedWith(Object const &item) const
{
	// Immediate contents of portallized objects are always observed with them.
	UNREF(item);
	return true;
}

// ----------------------------------------------------------------------

bool PortalProperty::isContentItemExposedWith(Object const &item) const
{
	// Portallized objects always expose their immediate contents with themselves.
	UNREF(item);
	return true;
}

// ----------------------------------------------------------------------

bool PortalProperty::canContentsBeObservedWith() const
{
	return true;
}

// ----------------------------------------------------------------------

bool PortalProperty::hasPassablePortalToParentCell() const
{
	return m_hasPassablePortalToParentCell;
}

// ----------------------------------------------------------------------
// position_l is in the space of the building.

CellProperty const * PortalProperty::findContainingCell(Vector const & position_l) const
{
	CellProperty const * cell = CellProperty::getWorldCellProperty();

	Object const & buildingObject = getOwner();
	
    float minDist = std::numeric_limits<float>::max();
	
    for (int i = 0; i < buildingObject.getNumberOfChildObjects(); i++)
    {
        Object const * const childObject = buildingObject.getChildObject(i);
        if (!childObject) 
			continue;
		
        CellProperty const * const cellProperty = childObject->getCellProperty();
        if (!cellProperty) 
			continue;
		
        Floor const * const floor = cellProperty->getFloor();
        if (!floor) 
			continue;
		
        FloorLocator loc;
	
        if (floor->findClosestLocation(position_l, loc))
        {
			float const distance = (position_l - loc.getPosition_p()).magnitudeSquared();
		
            if(distance < minDist)
            {
				minDist = distance;

				PortalProperty const * const portal = childObject->getPortalProperty();
				if (portal) 
				{
					CellProperty const * const childCellProperty = portal->findContainingCell(loc.getPosition_p());
					cell = childCellProperty && (childCellProperty != CellProperty::getWorldCellProperty()) ? childCellProperty : cellProperty;
				}
				else
				{
					cell = cellProperty;
				}
            }
        }
    }
	
	return cell;
}

// ======================================================================

