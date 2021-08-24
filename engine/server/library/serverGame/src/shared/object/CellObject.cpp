// ======================================================================
//
// CellObject.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CellObject.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/CellController.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedPathfinding/PathGraph.h"
#include "sharedPathfinding/PathNode.h"

const SharedObjectTemplate * CellObject::m_defaultSharedTemplate = nullptr;

// ======================================================================

CellObject::CellObject(const ServerObjectTemplate *newTemplate)
: ServerObject(newTemplate),
	m_cellNumber(-1),
	m_isPublic(true),
	m_allowed(),
	m_banned(),
	m_cellLabel(),
	m_labelLocationOffset()
{
	setDebugName("PortalProperty cell object");

	auto *cell = new CellProperty(*this);
	addProperty(*cell);

	m_isPublic.setSourceObject(this);
	m_allowed.setSourceObject(this);
	m_banned.setSourceObject(this);

	addMembersToPackages();
}

//-----------------------------------------------------------------------

CellObject::~CellObject()
= default;

//-----------------------------------------------------------------------

Controller * CellObject::createDefaultController()
{
	Controller *controller = new CellController(this);
	setController(controller);
	return controller;
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * CellObject::getDefaultSharedTemplate() const
{
static const ConstCharCrcLowerString templateName("object/cell/base/shared_cell_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "CellObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// CellObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void CellObject::removeDefaultTemplate()
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// CellObject::removeDefaultTemplate

// ----------------------------------------------------------------------

void CellObject::sendObjectSpecificBaselinesToClient(Client const &client) const
{
	ServerObject::sendObjectSpecificBaselinesToClient(client);
	ServerObject const * const so = client.getCharacterObject();
	if (so)
	{
		CreatureObject const * const creature = so->asCreatureObject();
        UpdateCellPermissionMessage message(getNetworkId(), creature ? isAllowed(*creature) : false);
        client.send(message, true);
    }
}

// ----------------------------------------------------------------------

void CellObject::endBaselines()
{
	ServerObject::endBaselines();

	// fix the load_with values if they don't match
	if ( isAuthoritative() && ( getLoadWith() != ContainerInterface::getLoadWithContainerId(*this)))
	{
		setLoadWith(ContainerInterface::getLoadWithContainerId(*this));
	}

	DEBUG_FATAL(m_cellNumber.get() < 1, ("Invalid cell number"));
	if (m_cellNumber.get() < 1)
	{
		LOG("CellProblem", ("Invalid cell number for cell %s", getNetworkId().getValueString().c_str()));
	}
	
	Object *container = ContainerInterface::getContainedByObject(*this);
	PortalProperty *portalProperty = nullptr;
	if (container)
	{
		portalProperty = container->getPortalProperty();
	}
	else
	{
		ContainedByProperty* prop = ContainerInterface::getContainedByProperty(*this);
		LOG("CellProblem",("Cell %s could not resolve contained_by of %s", getNetworkId().getValueString().c_str(), prop->getContainedByNetworkId().getValueString().c_str()));
		WARNING(true, ("CellProblem Cell %s could not resolve contained_by of %s", getNetworkId().getValueString().c_str(), prop->getContainedByNetworkId().getValueString().c_str()));
	}

	if (!portalProperty)
	{
		LOG("CellProblem",("Cell %s is not inside a POB", getNetworkId().getValueString().c_str()));
		WARNING_STRICT_FATAL(true,("Cell %s is not inside a POB.\n",getNetworkId().getValueString().c_str()));
		return;
	}

	if (m_cellNumber.get() < portalProperty->getNumberOfCells())
	{
		portalProperty->cellLoaded(m_cellNumber.get(), *this, false);
	}
	else
	{
		// 	go through cell contents and remove them all.  Give them to the PortalProperty to be fixed when it handles EndBaselines
 		CellProperty *cellProperty = getCellProperty();
		if (cellProperty)
		{
			for (ContainerIterator contents=cellProperty->begin(); contents != cellProperty->end(); ++contents)
			{
 				Object *object = (*contents).getObject();
				portalProperty->queueObjectForFixup(*object);
				Container::ContainerErrorCode tmp = Container::CEC_Success;

				cellProperty->remove(contents, tmp);
			}
		}
		permanentlyDestroy(DeleteReasons::BadContainerTransfer); 
	}

#ifdef _DEBUG
	if (ConfigServerGame::getDebugFloorPathNodeCount())
	{
		CellProperty * cellProperty = getCellProperty();

		if(cellProperty)
		{
			Floor const * floor = cellProperty->getFloor();

			if(floor)
			{
				FloorMesh const * floorMesh = floor->getFloorMesh();

				NOT_NULL(floorMesh);

				auto const * graph = safe_cast<PathGraph const *>(floorMesh->getPathGraph());

				if(graph)
				{
					int floorParts = floorMesh->getPartCount();

					int graphParts = graph->getPartCount();

					if(floorParts != graphParts)
					{
						const char * floorName = floorMesh->getName();
				
						WARNING(true,("ServerCollisionProperty::ServerCollisionProperty - floor %s doesn't have the same number of parts as its graph (floor %d, graph %d)",floorName,floorParts,graphParts));
					}
				}
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------

void CellObject::onLoadedFromDatabase()
{
	ServerObject::onLoadedFromDatabase();

	// We need to run through the set of banned and allowed players
	// and make sure the data is "good".
	// - At one time players were stored by name instead of by network ID

	ObservedPermissionObjectSet* permissionSets[] = { &m_allowed, &m_banned };
	for (auto & permissionSet : permissionSets)
	{
		const CellPermissions::PermissionList& permList = permissionSet->get();

		// Temporary list for processing
		CellPermissions::PermissionList tempList;

		// On the first pass over the permission list, remove the elements that need to be converted
		CellPermissions::PermissionList::const_iterator j;
		for ( j = permList.begin(); j != permList.end(); ++j )
		{
			const CellPermissions::PermissionObject& permObject = (*j);

			// Check if the string from the database was valid
			if ( !permObject.hasBeenConverted() )
			{
				// The string in the database needs to be converted
				tempList.insert( permObject );
			}
		}

		// Take the elements that need to be converted and re-add them to the list
		for ( j = tempList.begin(); j != tempList.end(); ++j )
		{
			const CellPermissions::PermissionObject& permObject = (*j);

			// Remove from the archive set so that it gets marked dirty
			permissionSet->erase( permObject );

			// Add to the archive set so that it gets marked dirty
			// NOTE: The permission object constructor should try to convert the string for us
			permissionSet->insert( CellPermissions::PermissionObject( permObject.m_permissionString ) );
		}
		tempList.clear();

		// On the second pass over the permission list, validate all the names
		for ( j = permList.begin(); j != permList.end();  ++j  )
		{
			const CellPermissions::PermissionObject& permObject = (*j);

			// Make sure we can still obtain a name from the permission object
			// For example, characters may have been purged
			const std::string name = permObject.getName();
			if ( name.empty() )
			{
				// The entry is no longer valid
				tempList.insert( permObject );
			}
		}

		// Remove all the invalid entries
		for ( j = tempList.begin(); j != tempList.end(); ++j )
		{
			const CellPermissions::PermissionObject& permObject = (*j);

			// Remove from the archive set so that it gets marked dirty
			permissionSet->erase( permObject );
		}
		tempList.clear();
	}
}

// ----------------------------------------------------------------------

void CellObject::addAllowed(std::string const &name)
{
	if(isAuthoritative())
	{
		if (!CellPermissions::isOnList(m_allowed.get(), name))
		{
			CellPermissions::PermissionObject newPermission(name);

			// We don't want to add any permissions that can't be converted to names
			if ( !newPermission.getName().empty() )
			{
				m_allowed.insert(name);
			}
		}
		removeBanned(name);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addAllowed, new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void CellObject::removeAllowed(std::string const &name)
{
	if(isAuthoritative())
	{
		m_allowed.erase(name);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllowed, new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void CellObject::removeAllAllowed()
{
	if(isAuthoritative())
	{
		m_allowed.clear();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllAllowed, nullptr);
	}
}

// ----------------------------------------------------------------------

void CellObject::addBanned(std::string const &name)
{
	if(isAuthoritative())
	{
		if (!CellPermissions::isOnList(m_banned.get(), name))
		{
			CellPermissions::PermissionObject newPermission(name);

			// We don't want to add any permissions that can't be converted to names
			if ( !newPermission.getName().empty() )
			{
				m_banned.insert(name);
			}
		}
		removeAllowed(name);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addBanned, new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void CellObject::removeBanned(std::string const &name)
{
	if(isAuthoritative())
	{
		m_banned.erase(name);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeBanned, new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void CellObject::removeAllBanned()
{
	if(isAuthoritative())
	{
		m_banned.clear();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllBanned, nullptr);
	}
}

// ----------------------------------------------------------------------

ServerObject const *CellObject::getOwner() const
{
        Object const * const o = ContainerInterface::getContainedByObject(*this);
        if (o)
                return o->asServerObject();
        return nullptr;
}

// ----------------------------------------------------------------------

ServerObject *CellObject::getOwner()
{
        Object * const o = ContainerInterface::getContainedByObject(*this);
        if (o)
                return o->asServerObject();
        return nullptr;
}

// ----------------------------------------------------------------------

BuildingObject const *CellObject::getOwnerBuilding() const
{
	Object const * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
			return so->asBuildingObject();
	}
	return nullptr;
}

// ----------------------------------------------------------------------

BuildingObject *CellObject::getOwnerBuilding()
{
	Object * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject * const so = o->asServerObject();
		if (so)
			return so->asBuildingObject();
	}
	return nullptr;
}

// ----------------------------------------------------------------------

ShipObject const *CellObject::getOwnerShip() const
{
	Object const * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
			return so->asShipObject();
	}
	return nullptr;
}

// ----------------------------------------------------------------------

ShipObject *CellObject::getOwnerShip()
{
	Object * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject * const so = o->asServerObject();
		if (so)
			return so->asShipObject();
	}
	return nullptr;
}

// ----------------------------------------------------------------------

bool CellObject::isAllowed(CreatureObject const &who) const
{
	if (who.getMasterId() != NetworkId::cms_invalid)
	{
		ServerObject const *master = ServerWorld::findObjectByNetworkId(who.getMasterId());
		if (master && master != &who && master->asCreatureObject())
			return isAllowed(*master->asCreatureObject());
	}

	BuildingObject const *building = getOwnerBuilding();
	if (building)
	{
		if (!building->playersWithAlignedEnemyFlagsAllowed() && (Pvp::hasAnyAlignedTimedEnemyFlag(who) || Pvp::hasAnyBountyDuelEnemyFlag(who)))
			return false;
		if (building->isOwner(who.getNetworkId(), who.getClient()))
			return true;
		if (!building->isAllowed(who))
			return false;
	}

	if (getIsPublic())
		return !CellPermissions::isOnList(m_banned.get(), who);
	else
	{
		// allow non-player-controlled  creatures in private buildings with no owner
		if (!who.isPlayerControlled() && building && building->getOwnerId() == NetworkId::cms_invalid)
			return true;
		// allow vendor-creatures to be placed in the building
		if (who.hasCondition(ServerTangibleObjectTemplate::C_vendor))
			return true;
		return CellPermissions::isOnList(m_allowed.get(), who);
	}
}

// ----------------------------------------------------------------------

bool CellObject::getClosestPathNodePos( const ServerObject & object, Vector & outPos ) const
{
	const PathNode * closestNode = nullptr;
	float closestDistance = 0;
	const Vector objectPos = object.getPosition_w();

	const CellProperty * cell = ContainerInterface::getCell(*this);
	if (cell != nullptr)
	{
		const Floor * floor = cell->getFloor();
		if (floor != nullptr)
		{
			const FloorMesh * mesh = floor->getFloorMesh();
			if (mesh != nullptr)
			{
				const auto * path = safe_cast<const PathGraph *>(mesh->getPathGraph());
				if (path != nullptr)
				{
					int nodeCount = path->getNodeCount();
					for (int i = 0; i < nodeCount; ++i)
					{
						const PathNode * node = path->getNode(i);
						if (node != nullptr)
						{
							float distance = objectPos.magnitudeBetweenSquared(
								rotateTranslate_p2w(node->getPosition_p()));
							if (closestNode == nullptr || distance < closestDistance)
							{
								closestNode = node;
								closestDistance = distance;
							}
						}
					}
				}
			}
		}
	}

	if (closestNode != nullptr)
		outPos = closestNode->getPosition_p();
	return closestNode != nullptr;
}

// ----------------------------------------------------------------------

bool CellObject::isVisibleOnClient (const Client & /*client*/) const
{
	return true;
}

// ----------------------------------------------------------------------

void CellObject::getAttributes (std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	ServerObject::getAttributes(data);
}

// ----------------------------------------------------------------------

bool CellObject::onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	//Cells should first check to see if the building will allow the item to be transferred.
	ServerObject *obj = getOwner();

	if (obj)
	{
		ScriptParams params;
		params.addParam(destination ? destination->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(transferer ? transferer->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(item.getNetworkId());
		if(obj->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_LOSE_ITEM, params) == SCRIPT_OVERRIDE)
		{
			return false;
		}
	}
	return ServerObject::onContainerAboutToLoseItem(destination, item, transferer);
}

// ----------------------------------------------------------------------

int CellObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	// If it's a creature trying to move into the cell, see if it's allowed.
	CreatureObject * const creatureItem = item.asCreatureObject();
	if (creatureItem && !isAllowed(*creatureItem))
		return Container::CEC_SilentError;
	//Cells should first check to see if the building will allow the item to be transfered.
        ServerObject *obj = getOwner();

	if (obj)
	{
		auto const * const source = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item));
		NetworkId sourceId = source ? source->getNetworkId() : NetworkId::cms_invalid;
		ScriptParams params;
		params.addParam(sourceId);
		params.addParam(transferer ? transferer->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(item.getNetworkId());
		if (obj->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_RECEIVE_ITEM, params) == SCRIPT_OVERRIDE)
			return Container::CEC_BlockedByScript;
	}
	return ServerObject::onContainerAboutToGainItem(item, transferer);
}

// ----------------------------------------------------------------------

void CellObject::onContainerLostItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	//Cells should first check to see if the building will allow the item to be transfered.
	ServerObject * const obj = getOwner();

	if (obj)
	{
		ScriptParams params;
		params.addParam(destination ? destination->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(transferer ? transferer->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(item.getNetworkId());
		obj->getScriptObject()->trigAllScripts(Scripting::TRIG_LOST_ITEM, params);

		BuildingObject * const b_obj = getOwnerBuilding();
		if (b_obj && item.isPlayerControlled() && destination == nullptr)
		{
			b_obj->lostPlayer(item);
		}
	}
	ServerObject::onContainerLostItem(destination, item, transferer);
}

// ----------------------------------------------------------------------

void CellObject::onContainerGainItem(ServerObject& item, ServerObject * source, ServerObject* transferer)
{
	//Cells should first check to see if the building will allow the item to be transfered.
	ServerObject * const obj = getOwner();
	if (obj)
	{
		// The default owner of an object in a building is the building's owner, unless it is a creature, ship, or vendor
		if (   !item.asCreatureObject()
		    && !item.asShipObject()
		    && !item.isVendor())
		{
			item.setOwnerId(obj->getOwnerId());
		}

		ScriptParams params;
		params.addParam(source ? source->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(transferer ? transferer->getNetworkId() : NetworkId::cms_invalid);
		params.addParam(item.getNetworkId());

		obj->getScriptObject()->trigAllScripts(Scripting::TRIG_RECEIVED_ITEM, params);

		BuildingObject * const b_obj = getOwnerBuilding();
		if (b_obj && item.isPlayerControlled() && !source)
		{
			b_obj->gainedPlayer(item);
		}
	}
	ServerObject::onContainerGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

NetworkId const &CellObject::getOwnerId() const
{
	Object const * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
			return so->getOwnerId();
	}
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

bool CellObject::isOwner(NetworkId const &id, Client const *client) const
{
	Object const * const o = ContainerInterface::getContainedByObject(*this);
	if (o)
	{
		ServerObject const * const so = o->asServerObject();
		if (so)
			return so->isOwner(id, client);
	}
	
	return false;
}

// ----------------------------------------------------------------------

void CellObject::setOwnerId(NetworkId const &id)
{
	if (isAuthoritative())
	{
		Container * const container = ContainerInterface::getContainer(*this);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				auto * const content = safe_cast<ServerObject *>((*i).getObject());
				if (content && !content->asCreatureObject())
					content->setOwnerId(id);
			}
		}
	}
	//if not authoritative, that's ok.  This should only be called when the authoritative building has setOwnerId called on it.
}

// ----------------------------------------------------------------------

void CellObject::onAllContentsLoaded()
{
	ServerObject::onAllContentsLoaded();
	BuildingObject * owner = getOwnerBuilding();
	if (owner)
		owner->onCellContentsLoaded();
}

// ----------------------------------------------------------------------

CellObject * CellObject::asCellObject()
{
	return this;
}

// ----------------------------------------------------------------------

CellObject const * CellObject::asCellObject() const
{
	return this;
}

// ----------------------------------------------------------------------

CellObject * CellObject::getCellObject(NetworkId const & networkId)
{
	return CellObject::asCellObject(NetworkIdManager::getObjectById(networkId));
}

// ----------------------------------------------------------------------

CellObject * CellObject::asCellObject(Object * object)
{
	ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	CellObject * const cellObject = (serverObject != nullptr) ? serverObject->asCellObject() : nullptr;
	
	return cellObject;
}

// ----------------------------------------------------------------------

CellObject const * CellObject::asCellObject(Object const * object)
{
	ServerObject const * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	CellObject const * const cellObject = (serverObject != nullptr) ? serverObject->asCellObject() : nullptr;
	
	return cellObject;
}

// ======================================================================
