// ======================================================================
//
// BuildingObject.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BuildingObject.h"

#include "serverGame/BuildingController.h"
#include "serverGame/CellObject.h"
#include "serverGame/CityInterface.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PortalTriggerVolume.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/Pvp.h"
#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverPathfinding/ServerPathfindingNotification.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/LayerProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/Portal.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedTerrain/TerrainModificationHelper.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"


#include <algorithm>
#include <set>

// ======================================================================

const std::string CLONE_RESPAWN_TABLE("datatables/structure/municipal/cloning_facility_respawn.iff");

namespace BuildingObjectNamespace
{
	BuildingObject::AllBuildingsSet g_buildingList;

	bool hasPlayer(const BuildingObject& building)
	{
		const Container * container = ContainerInterface::getContainer(building);
		if (container)
		{
			for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
			{
				const ServerObject *  cell = safe_cast<ServerObject *>((*i).getObject());
				DEBUG_FATAL(!cell, ("Null cell in building list"));
				if (cell)
				{
					const Container * cellContainer = ContainerInterface::getContainer(*cell);
					if (cellContainer)
					{
						for (ContainerConstIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
						{
							const ServerObject * obj = safe_cast<ServerObject *>((*j).getObject());
							if (obj && obj->isPlayerControlled())
							{
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}

	void unloadCellsContents(BuildingObject& building)
	{
		Container *container = ContainerInterface::getContainer(building);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * cell = safe_cast<ServerObject *>((*i).getObject());
				DEBUG_FATAL(!cell, ("Null cell in building list"));
				if (cell)
				{
					cell->unloadPersistedContents();
				}
			}
		}
	}

	void updateLoadWithToCell(const ServerObject& cell, ServerObject& obj)
	{
		if (!obj.isPlayerControlled())
		{
			obj.setLoadWith(cell.getNetworkId());
			PositionUpdateTracker::positionChanged(obj);
			if (obj.getLoadContents())
			{
				Container * objContainer = ContainerInterface::getContainer(obj);
				if (objContainer)
				{
					for (ContainerIterator i = objContainer->begin(); i != objContainer->end(); ++i)
					{
						ServerObject* childObject = safe_cast<ServerObject*>((*i).getObject());
						if (childObject)
						{
							updateLoadWithToCell(cell, *childObject);
						}
					}
				}
			}
		}
	}

}

using namespace BuildingObjectNamespace;


// ======================================================================

const SharedObjectTemplate * BuildingObject::m_defaultSharedTemplate = nullptr;


// ======================================================================

BuildingObject::BuildingObject(const ServerBuildingObjectTemplate* newTemplate) :
	TangibleObject(newTemplate),
	m_maintenanceCost(newTemplate->getMaintenanceCost()),
	m_timeLastChecked(0.f),
	m_cityId(0),
	m_isPublic(newTemplate->getIsPublic()),
	m_allowed(),
	m_banned(),
	m_contentsLoaded(false),
	m_checkCreatePortalVolume(true),
	m_unloadTime(0)
{
	m_isPublic.setSourceObject(this);
	m_allowed.setSourceObject(this);
	m_banned.setSourceObject(this);
	addMembersToPackages();

	//@todo -- HACK! this is supposed to be set by the database,
	// but that code isn't written. Assume all buildings and cells
	// are in the client cache file. //-- it is now [8/4/2002 4:20:48 PM],
	// this should be implemented in a few days. If you see this comment
	// and the code below calls setCacheVersion(1), get with Chris
	// and Calan to determine the status of this code.
	//setCacheVersion(1);

	// All buildings get server pathfinding notifications

	addNotification( ServerPathfindingNotification::getInstance() );

	//-- create test layer (this should load from the template's layer filename)
	const SharedBuildingObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedBuildingObjectTemplate*> (getSharedTemplate ());
	if (sharedObjectTemplate->getTerrainModificationFileName ().length () != 0)
	{
		TerrainGenerator::Layer * layer = TerrainModificationHelper::importLayer (sharedObjectTemplate->getTerrainModificationFileName ().c_str ());
		WARNING (!layer, ("Layer %s not found", sharedObjectTemplate->getTerrainModificationFileName ().c_str ()));

		if (layer)
		{
			setLayer(layer);
		}
	}

	IGNORE_RETURN(g_buildingList.insert(this));

	ObjectTracker::addBuilding();
}

// ----------------------------------------------------------------------

BuildingObject::~BuildingObject()
{
	if (isInWorld())
		removeFromWorld();

	IGNORE_RETURN(g_buildingList.erase(this));

	ObjectTracker::removeBuilding();
}

// ----------------------------------------------------------------------

float BuildingObject::alter(float time)
{
	if (m_checkCreatePortalVolume)
	{

		bool playerPlaced = isPlayerPlaced();
	
		// Trigger volumes are centered on the object, which may be unrelated to the extents of the object, but
		// we need the portal trigger volume to encompass the extents of the object.
		if (!playerPlaced)
		{
			Sphere const &extent = getSphereExtent();
			float radius = extent.getRadius()
				+ getTransform_o2w().getPosition_p().magnitudeBetween(extent.getCenter())
				+ 5.0f;
			if (radius > 0 && !isSpaceDungeon() && ServerWorld::getSceneId() != NewbieTutorial::getSceneId())
			{
				TriggerVolume* t = new PortalTriggerVolume(*this, radius);
				addTriggerVolume(t);
			}

		}
		m_checkCreatePortalVolume = false;
	}

	const unsigned long gameTime = ServerClock::getInstance().getGameTimeSeconds();
	if ((m_unloadTime > 0) && (gameTime >= m_unloadTime))
	{
		if ( hasPlayer (*this) )
		{
			m_unloadTime = gameTime + static_cast<unsigned long>(ConfigServerGame::getUnloadHouseContentsTime());
		}
		else
		{
			m_unloadTime = 0;

			// safety check to make absolutely sure we don't unload a building which shouldn't be unloaded
			if (isPlayerPlaced() && ConfigServerGame::getDemandLoadHouseContents() && isAuthoritative())
			{
				unloadCellsContents( *this );
				m_contentsLoaded = false;
			}
		}
	}
	
	float result =  TangibleObject::alter(time);
	AlterResult::incorporateAlterResult(result, ((m_unloadTime > gameTime) ? static_cast<float>(m_unloadTime - gameTime) : result));

	return result;
}

// ----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void BuildingObject::initializeFirstTimeObject()
{
	TangibleObject::initializeFirstTimeObject();
}

//------------------------------------------------------------------------------------------

void BuildingObject::endBaselines()
{
	TangibleObject::endBaselines();
}

//-----------------------------------------------------------------------

void BuildingObject::onLoadedFromDatabase()
{
	TangibleObject::onLoadedFromDatabase();

	// We need to run through the set of banned and allowed players
	// and make sure the data is "good".
	// - At one time players were stored by name instead of by network ID

	ObservedPermissionObjectSet* permissionSets[] = { &m_allowed, &m_banned };
	for ( unsigned int i = 0; i < sizeof( permissionSets ) / sizeof( permissionSets[0] ); ++i )
	{
		const CellPermissions::PermissionList& permList = permissionSets[i]->get();

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
			permissionSets[i]->erase( permObject );

			// Add to the archive set so that it gets marked dirty
			// NOTE: The permission object constructor should try to convert the string for us
			permissionSets[i]->insert( CellPermissions::PermissionObject( permObject.m_permissionString ) );
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
			permissionSets[i]->erase( permObject );
		}
		tempList.clear();
	}

	// check to see if building is a declared building inside a city limit
	if (ConfigServerGame::getEnableCityCitizenshipFixup())
	{
		if (!isInWorldCell())
			return;

		NetworkId residenceOf;
		if (!getObjVars().getItem("player_structure.residence.building", residenceOf))
			return;

		if (residenceOf != getOwnerId())
			return;

		Vector const posWorld = getPosition_w();
		int const cityId = CityInterface::getCityAtLocation(ServerWorld::getSceneId(), static_cast<int>(posWorld.x), static_cast<int>(posWorld.z), 0);

		if (cityId <= 0)
			return;

		GameServer::reportClusterStartupResidenceStructure(cityId, getNetworkId(), residenceOf);
	}
}

// ----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * BuildingObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/building/base/shared_building_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "BuildingObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// BuildingObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void BuildingObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// BuildingObject::removeDefaultTemplate

// ----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* BuildingObject::createDefaultController(void)
{
	Controller* _controller = new BuildingController(this);

	setController(_controller);
	return _controller;
}	// BuildingObject::createDefaultController

// ----------------------------------------------------------------------

void BuildingObject::expelObject(ServerObject &who)
{
	PortalProperty *portalProperty = getPortalProperty();
	if (portalProperty)
	{
		ServerController * controller = dynamic_cast<ServerController*>(who.getController());
		if (controller)
		{
			CellProperty *parentCell = getParentCell();
			ServerObject *destinationCellObject = nullptr;			
		
			if (!parentCell->isWorldCell())
				destinationCellObject = safe_cast<ServerObject*>(&parentCell->getOwner());

			Transform destinationTransform(Transform::IF_none);
			destinationTransform.multiply(getTransform_o2w(), portalProperty->getEjectionLocationTransform());
			controller->teleport(destinationTransform, destinationCellObject);
		}
	}
}

// ----------------------------------------------------------------------

void BuildingObject::onPermanentlyDestroyed()
{
	ServerPathfindingNotification::destroyBuilding(this);

	GameServer::reportBuildingObjectDestroyed(getNetworkId());

	//Check for vendors since they must be explicitly killed
	Container *container = ContainerInterface::getContainer(*this);
	if (container)
	{

		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject *cell = safe_cast<ServerObject *>((*i).getObject());
			if (cell)
			{
				Container *cellContainer = ContainerInterface::getContainer(*cell);
				if (cellContainer)
				{
					for (ContainerIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
					{
						TangibleObject *child = dynamic_cast<TangibleObject*>((*j).getObject());
						if (child && isPlayerPlaced() && child->hasCondition(static_cast<int>(ServerTangibleObjectTemplate::C_vendor)))
						{
							IGNORE_RETURN(child->permanentlyDestroy(DeleteReasons::ContainerDeleted));
						}
						else if (child && child->getGameObjectType() == static_cast<int>(SharedObjectTemplate::GOT_corpse))
						{
							expelObject(*child);
						}
					} //for contents of cells
				}
			}
		} //for cells
	}
}

// ----------------------------------------------------------------------

void BuildingObject::addAllowed(std::string const &name)
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

void BuildingObject::removeAllowed(std::string const &name)
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

void BuildingObject::removeAllAllowed()
{
	if(isAuthoritative())
	{
		m_allowed.clear();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllAllowed, 0);
	}
}

// ----------------------------------------------------------------------

void BuildingObject::addBanned(std::string const &name)
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

void BuildingObject::removeBanned(std::string const &name)
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

void BuildingObject::removeAllBanned()
{
	if(isAuthoritative())
	{
		m_banned.clear();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllBanned, 0);
	}
}

// ----------------------------------------------------------------------

bool BuildingObject::isAllowed(CreatureObject const &who) const
{
	if (who.getMasterId() != NetworkId::cms_invalid)
	{
		ServerObject const *master = ServerWorld::findObjectByNetworkId(who.getMasterId());
		if (master && master != &who && master->asCreatureObject())
			return isAllowed(*master->asCreatureObject());
	}

	if (!playersWithAlignedEnemyFlagsAllowed() && (Pvp::hasAnyAlignedTimedEnemyFlag(who) || Pvp::hasAnyBountyDuelEnemyFlag(who)))
		return false;

	if (getIsPublic())
		return !CellPermissions::isOnList(m_banned.get(), who);
	else
	{
		// allow non-player-controlled creatures in private buildings with no owner
		if (!who.isPlayerControlled() && getOwnerId() == NetworkId::cms_invalid)
			return true;

		// allow vendor-creatures to be placed in the building
		if (who.hasCondition(static_cast<int>(ServerTangibleObjectTemplate::C_vendor)))
			return true;

		// allow non-player-controlled creatures in private buildings if the *creature* has no owner
		if (!who.isPlayerControlled()
			&& who.getMasterId() == NetworkId::cms_invalid
			&& who.getLevel() < 0
			)
		{
			return true;
		}

		return CellPermissions::isOnList(m_allowed.get(), who);
	}
}

//------------------------------------------------------------------------------------------

bool BuildingObject::isPlayerPlaced() const
{
	// only non-gold/non-buildout buildings can be player placed
	return (getObjVars().hasItem("player_structure") && (getCacheVersion() == 0) && (getNetworkId().getValue() > static_cast<NetworkId::NetworkIdType>(0)));
}

//------------------------------------------------------------------------------------------

bool BuildingObject::isSpaceDungeon() const
{
	return getObjVars().hasItem("space_dungeon.name");
}

// ----------------------------------------------------------------------

bool BuildingObject::playersWithAlignedEnemyFlagsAllowed() const
{
	return !isPlayerPlaced() || getObjVars().hasItem("playersWithAlignedEnemyFlagsAllowed");
}

// ----------------------------------------------------------------------

bool BuildingObject::isVisibleOnClient (const Client & /*client*/) const
{
	return true;
}

// ----------------------------------------------------------------------

void BuildingObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);
}

// ----------------------------------------------------------------------

BuildingObject * BuildingObject::asBuildingObject()
{
	return this;
}

//-----------------------------------------------------------------------

BuildingObject const * BuildingObject::asBuildingObject() const
{
	return this;
}

// ----------------------------------------------------------------------

void BuildingObject::onAddedToWorld()
{
	TangibleObject::onAddedToWorld();

	m_checkCreatePortalVolume = true;
	scheduleForAlter();
	
	Container* container = ContainerInterface::getContainer(*this);
	if (container)
	{
		ContainerIterator i = container->begin();
		for(; i != container->end(); ++i)
		{
			ServerObject * obj = safe_cast<ServerObject*>((*i).getObject());
			if (obj && obj->isPlayerControlled())
			{
				gainedPlayer(*obj);
			}
		}
	}

}

// ----------------------------------------------------------------------

void BuildingObject::onAllContentsLoaded()
{
	TangibleObject::onAllContentsLoaded();
	m_contentsLoaded = true;
	ServerObject * containedBy = safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(*this));
	bool fixHouseSceneIds = true;
	if (containedBy)
	{
		const NetworkId& owner = containedBy->getLoadWith();
		CreatureObject * characterOwner = CreatureObject::getCreatureObject(owner);
		if (characterOwner)
		{
			LOG("CustomerService", ("CharacterTransfer: packed house loaded for building (%s) contained by %s.", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(characterOwner).c_str()));

			Client * const client = containedBy->getClient();
			if (client)
			{
				CreatureObject * const containedByCreature = containedBy->asCreatureObject();	
				if (containedByCreature && containedByCreature->getLoadCTSPackedHouses())
				{
					containedByCreature->packedHouseLoaded();
					fixHouseSceneIds = false;
				}
			}
		}
	}
	if(fixHouseSceneIds && !getObjVars().hasItem("packed_house.fix_contained_scene_ids"))
	{
		setSceneIdOnThisAndContents(getSceneId());
		setObjVarItem("packed_house.fix_contained_scene_ids", 1);
	}

	NetworkId owner;
	if(getObjVars().getItem("purge_process.rollup_on_load", owner))
		rollupStructure(owner, false);
}

//------------------------------------------------------------------------------------------

void BuildingObject::onCellContentsLoaded()
{
	if (!isPlayerPlaced())
		return;
	
	bool doFixup = true;
	Container *container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * cell = safe_cast<ServerObject *>((*i).getObject());
			if (!cell || !cell->areContentsLoaded())
			{
				doFixup = false;
				break;
			}
		}
	}

	if (doFixup)
	{
		fixUpPobItemLimit();
		onAllContentsLoaded();
	}
}

//------------------------------------------------------------------------------------------

void BuildingObject::loadContents(bool immediate /*= false*/)
{
	if (isAuthoritative())
	{
		// delaying the loading of the house's contents is useful to emulate a live
		// environment where it takes significantly longer to load a house's contents
		if (!immediate && (ConfigServerGame::getLoadHouseContentsDelaySeconds() > 0))
		{
			// don't need to queue up multiple C++LoadBuildingContents messages;
			// only one is necessary to delay load the house's contents
			if (timeUntilMessageTo("C++LoadBuildingContents") < 0)
			{
				MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
					"C++LoadBuildingContents",
					"",
					ConfigServerGame::getLoadHouseContentsDelaySeconds(),
					false);
			}

			return;
		}

		Container *container = ContainerInterface::getContainer(*this);
		if (container)
		{
			//Demand load house cells
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * cell = safe_cast<ServerObject *>((*i).getObject());
				DEBUG_FATAL(!cell, ("Null cell in building list"));
				if (cell)
				{
					if (cell->getLoadContents())
					{
						cell->setLoadContents(false);
						Container * cellContainer = ContainerInterface::getContainer(*cell);
						if (cellContainer)
						{
							for (ContainerIterator j = cellContainer->begin(); j != cellContainer->end(); ++j)
							{
								ServerObject * obj = safe_cast<ServerObject *>((*j).getObject());
								if (obj)
								{
									updateLoadWithToCell(*cell, *obj);
								}
							}
						}
					}
				}
			}

			//Demand load contents of house cells
			for (ContainerIterator j = container->begin(); j != container->end(); ++j)
			{
				ServerObject * cell = safe_cast<ServerObject*>((*j).getObject());
				DEBUG_FATAL(!cell, ("Null cell in building list"));
				if (cell)
				{
					if (cell->areContentsLoaded())
						break;

					cell->loadAllContentsFromDB();
					m_unloadTime = ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(ConfigServerGame::getUnloadHouseContentsTime());
					scheduleForAlter();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void BuildingObject::gainedPlayer(ServerObject const &)
{
	if (isPlayerPlaced())
	{
		if (ConfigServerGame::getDemandLoadHouseContents())
		{
			loadContents();
		}
		else
		{
			int version = 0;
			IGNORE_RETURN(getObjVars().getItem("player_structure.version", version));
			int newVersion = version;
			
			if (version < 3)
			{
				fixUpPobItemLimit();
				newVersion = 3;
			}
			IGNORE_RETURN(setObjVarItem("player_structure.version", newVersion));
		}
	}
}

// ----------------------------------------------------------------------

void BuildingObject::lostPlayer(ServerObject const &)
{
	if (isPlayerPlaced() && ConfigServerGame::getDemandLoadHouseContents() && isAuthoritative())
	{
		m_unloadTime = ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(ConfigServerGame::getUnloadHouseContentsTime());
		scheduleForAlter();
	}
}

//------------------------------------------------------------------------------------------

void BuildingObject::changeTeleportDestination(Vector & position, float & yaw) const
{
	if (!isAuthoritative())
	{
		WARNING(true, ("BuildingObject::changeTeleportDestination called on "
			"non-authoritative building %s", getNetworkId().getValueString().c_str()));
		return;
	}

	// call a script trigger that will give us the index of the cloning facility
	// tube to respawn in, if any
	int index = -1;
	ScriptParams params;
	params.addParam(index);
	IGNORE_RETURN(const_cast<GameScriptObject *>(getScriptObject())->trigAllScripts(
		Scripting::TRIG_GET_RESPAWN_LOC, params));
	index = params.getIntParam(0);
	if (index >= 0)
	{
		DataTable * respawnTable = DataTableManager::getTable(CLONE_RESPAWN_TABLE, 
			true);
		if (respawnTable != nullptr)
		{
			int row = respawnTable->searchColumnString(0, getTemplateName());
			if (row >= 0)
			{
				char buffer[32];
				sprintf(buffer, "TUBE%d_X", index+1);
				int column = respawnTable->findColumnNumber(buffer);
				if (column >= 0)
				{
					position.x = respawnTable->getFloatValue(column, row);
					position.z = respawnTable->getFloatValue(column+1, row);
					yaw = convertDegreesToRadians(respawnTable->getFloatValue(column+2, row));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void BuildingObject::setIsPublic(bool isPublic)
{
	if (isAuthoritative())
		m_isPublic.set(isPublic);
	else
		sendControllerMessageToAuthServer(CM_setBuildingIsPublic, new MessageQueueGenericValueType<bool>(isPublic));
}

// ----------------------------------------------------------------------


void BuildingObject::setCityId(int cityId)
{
	if (isAuthoritative())
		m_cityId.set(cityId);
	else
		sendControllerMessageToAuthServer(CM_setBuildingCityId, new MessageQueueGenericValueType<int>(cityId));
}

// ----------------------------------------------------------------------

void BuildingObject::handleCMessageTo(const MessageToPayload &message)
{
	if (message.getMethod() == "C++PurgeStructure")
	{
		NetworkId owner(message.getDataAsString());
		LOG("CustomerService",("Purge:  Purging building %s",getNetworkId().getValueString().c_str()));
		if (m_contentsLoaded.get())
			rollupStructure(owner, false);
		else
		{
			IGNORE_RETURN(setObjVarItem("purge_process.rollup_on_load", owner));
			loadContents();
		}			
	}
	else if (message.getMethod() == "C++CharacterSheetInfoResidenceLocationReq")
	{
		if (!message.getPackedDataVector().empty())
		{
			// get network id of the player asking for the information
			std::string const packedData(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			NetworkId const idRequester(packedData);
			if (idRequester.isValid() && MessageToQueue::isInstalled())
			{
				Vector const loc = getPosition_w();

				char buffer[256];
				snprintf(buffer, sizeof(buffer)-1, "%s %d %d %d", ServerWorld::getSceneId().c_str(), static_cast<int>(loc.x), static_cast<int>(loc.y), static_cast<int>(loc.z));
				buffer[sizeof(buffer)-1] = '\0';

				MessageToQueue::getInstance().sendMessageToC(idRequester,
					"C++CharacterSheetInfoResidenceLocationRsp",
					buffer,
					0,
					false);
			}
		}
	}
	else if (message.getMethod() == "C++CityShareGroupPickupPointTravelCost")
	{
		if (!message.getPackedDataVector().empty())
		{
			std::string const packedData(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			int const cityShareOfGroupPickupPointTravelCost = atoi(packedData.c_str());
			if (cityShareOfGroupPickupPointTravelCost > 0)
				transferBankCreditsFrom("GroupPickupPointTravel", cityShareOfGroupPickupPointTravelCost);
		}
	}
	else if (message.getMethod() == "C++LoadBuildingContents")
	{
		loadContents(true);
	}
	else
		TangibleObject::handleCMessageTo(message);
}

//-----------------------------------------------------------------------

const BuildingObject::AllBuildingsSet & BuildingObject::getAllBuildings()
{
	return g_buildingList;
}

//-----------------------------------------------------------------------

bool BuildingObject::unloadContents()
{
	// make sure there are no players in the building and its valid to have its contents unloaded
	if (!hasPlayer(*this) && isPlayerPlaced() && ConfigServerGame::getDemandLoadHouseContents() && isAuthoritative())
	{
		m_unloadTime = 0;
		unloadCellsContents(*this);
		m_contentsLoaded = false;
	}

	return !m_contentsLoaded.get();
}

// ======================================================================
