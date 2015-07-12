#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/SwgSnapshot.h"

#include "serverGame/ServerBattlefieldMarkerObjectTemplate.h"
#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerCityObjectTemplate.h"
#include "serverGame/ServerConstructionContractObjectTemplate.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerDraftSchematicObjectTemplate.h"
#include "serverGame/ServerFactoryObjectTemplate.h"
#include "serverGame/ServerGuildObjectTemplate.h"
#include "serverGame/ServerHarvesterInstallationObjectTemplate.h"
#include "serverGame/ServerInstallationObjectTemplate.h"
#include "serverGame/ServerManufactureInstallationObjectTemplate.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "serverGame/ServerPlayerQuestObjectTemplate.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerShipObjectTemplate.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerVehicleObjectTemplate.h"
#include "serverGame/ServerWeaponObjectTemplate.h"


//!!!BEGIN GENERATED LOADOBJECT
void SwgSnapshot::loadBattlefieldMarkerObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_battlefieldMarkerObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newBattlefieldMarkerObject(NetworkId const & objectId)
{
	m_battlefieldMarkerObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadBuildingObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_buildingObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newBuildingObject(NetworkId const & objectId)
{
	m_buildingObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadCellObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_cellObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::newCellObject(NetworkId const & objectId)
{
	m_cellObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::loadCityObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_cityObjectBuffer.addEmptyRow(objectId);
	loadUniverseObject(session,objectId);
}

void SwgSnapshot::newCityObject(NetworkId const & objectId)
{
	m_cityObjectBuffer.addEmptyRow(objectId);
	newUniverseObject(objectId);
}

void SwgSnapshot::loadCreatureObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_creatureObjectBuffer.addEmptyRow(objectId);
	loadAttributes(session,objectId);
	loadSkills(session,objectId);
	loadCommands(session,objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newCreatureObject(NetworkId const & objectId)
{
	m_creatureObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadFactoryObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_factoryObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newFactoryObject(NetworkId const & objectId)
{
	m_factoryObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadGuildObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_guildObjectBuffer.addEmptyRow(objectId);
	loadUniverseObject(session,objectId);
}

void SwgSnapshot::newGuildObject(NetworkId const & objectId)
{
	m_guildObjectBuffer.addEmptyRow(objectId);
	newUniverseObject(objectId);
}

void SwgSnapshot::loadHarvesterInstallationObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_harvesterInstallationObjectBuffer.addEmptyRow(objectId);
	loadInstallationObject(session,objectId);
}

void SwgSnapshot::newHarvesterInstallationObject(NetworkId const & objectId)
{
	m_harvesterInstallationObjectBuffer.addEmptyRow(objectId);
	newInstallationObject(objectId);
}

void SwgSnapshot::loadInstallationObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_installationObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newInstallationObject(NetworkId const & objectId)
{
	m_installationObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadIntangibleObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_intangibleObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::newIntangibleObject(NetworkId const & objectId)
{
	m_intangibleObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::loadManufactureInstallationObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_manufactureInstallationObjectBuffer.addEmptyRow(objectId);
	loadInstallationObject(session,objectId);
}

void SwgSnapshot::newManufactureInstallationObject(NetworkId const & objectId)
{
	m_manufactureInstallationObjectBuffer.addEmptyRow(objectId);
	newInstallationObject(objectId);
}

void SwgSnapshot::loadManufactureSchematicObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_manufactureSchematicObjectBuffer.addEmptyRow(objectId);
	loadIntangibleObject(session,objectId);
}

void SwgSnapshot::newManufactureSchematicObject(NetworkId const & objectId)
{
	m_manufactureSchematicObjectBuffer.addEmptyRow(objectId);
	newIntangibleObject(objectId);
}

void SwgSnapshot::loadMissionObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_missionObjectBuffer.addEmptyRow(objectId);
	loadIntangibleObject(session,objectId);
}

void SwgSnapshot::newMissionObject(NetworkId const & objectId)
{
	m_missionObjectBuffer.addEmptyRow(objectId);
	newIntangibleObject(objectId);
}

void SwgSnapshot::loadPlanetObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_planetObjectBuffer.addEmptyRow(objectId);
	loadUniverseObject(session,objectId);
}

void SwgSnapshot::newPlanetObject(NetworkId const & objectId)
{
	m_planetObjectBuffer.addEmptyRow(objectId);
	newUniverseObject(objectId);
}

void SwgSnapshot::loadPlayerObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_playerObjectBuffer.addEmptyRow(objectId);
	loadIntangibleObject(session,objectId);
}

void SwgSnapshot::newPlayerObject(NetworkId const & objectId)
{
	m_playerObjectBuffer.addEmptyRow(objectId);
	newIntangibleObject(objectId);
}

void SwgSnapshot::loadPlayerQuestObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_playerQuestObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newPlayerQuestObject(NetworkId const & objectId)
{
	m_playerQuestObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadResourceContainerObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_resourceContainerObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newResourceContainerObject(NetworkId const & objectId)
{
	m_resourceContainerObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadShipObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_shipObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newShipObject(NetworkId const & objectId)
{
	m_shipObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadStaticObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_staticObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::newStaticObject(NetworkId const & objectId)
{
	m_staticObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::loadTangibleObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_tangibleObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::newTangibleObject(NetworkId const & objectId)
{
	m_tangibleObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::loadUniverseObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_universeObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::newUniverseObject(NetworkId const & objectId)
{
	m_universeObjectBuffer.addEmptyRow(objectId);
}

void SwgSnapshot::loadVehicleObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_vehicleObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newVehicleObject(NetworkId const & objectId)
{
	m_vehicleObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadWeaponObject(DB::Session *session, NetworkId const & objectId)
{
	UNREF(session);
	m_weaponObjectBuffer.addEmptyRow(objectId);
	loadTangibleObject(session,objectId);
}

void SwgSnapshot::newWeaponObject(NetworkId const & objectId)
{
	m_weaponObjectBuffer.addEmptyRow(objectId);
	newTangibleObject(objectId);
}

void SwgSnapshot::loadObject(DB::Session *session,const Tag &typeId, const NetworkId &objectId)
{
	switch(typeId)
	{
		case ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag:
			loadBattlefieldMarkerObject(session,objectId);
			break;

		case ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag:
			loadBuildingObject(session,objectId);
			break;

		case ServerCellObjectTemplate::ServerCellObjectTemplate_tag:
			loadCellObject(session,objectId);
			break;

		case ServerCityObjectTemplate::ServerCityObjectTemplate_tag:
			loadCityObject(session,objectId);
			break;

		case ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag:
			loadCreatureObject(session,objectId);
			break;

		case ServerFactoryObjectTemplate::ServerFactoryObjectTemplate_tag:
			loadFactoryObject(session,objectId);
			break;

		case ServerGuildObjectTemplate::ServerGuildObjectTemplate_tag:
			loadGuildObject(session,objectId);
			break;

		case ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag:
			loadHarvesterInstallationObject(session,objectId);
			break;

		case ServerInstallationObjectTemplate::ServerInstallationObjectTemplate_tag:
			loadInstallationObject(session,objectId);
			break;

		case ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate_tag:
			loadIntangibleObject(session,objectId);
			break;

		case ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag:
			loadManufactureInstallationObject(session,objectId);
			break;

		case ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag:
			loadManufactureSchematicObject(session,objectId);
			break;

		case ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag:
			loadMissionObject(session,objectId);
			break;

		case ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag:
			loadPlanetObject(session,objectId);
			break;

		case ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag:
			loadPlayerObject(session,objectId);
			break;

		case ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate_tag:
			loadPlayerQuestObject(session,objectId);
			break;

		case ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag:
			loadResourceContainerObject(session,objectId);
			break;

		case ServerShipObjectTemplate::ServerShipObjectTemplate_tag:
			loadShipObject(session,objectId);
			break;

		case ServerStaticObjectTemplate::ServerStaticObjectTemplate_tag:
			loadStaticObject(session,objectId);
			break;

		case ServerTangibleObjectTemplate::ServerTangibleObjectTemplate_tag:
			loadTangibleObject(session,objectId);
			break;

		case ServerUniverseObjectTemplate::ServerUniverseObjectTemplate_tag:
			loadUniverseObject(session,objectId);
			break;

		case ServerVehicleObjectTemplate::ServerVehicleObjectTemplate_tag:
			loadVehicleObject(session,objectId);
			break;

		case ServerWeaponObjectTemplate::ServerWeaponObjectTemplate_tag:
			loadWeaponObject(session,objectId);
			break;

		default:
			WARNING(true,("Attempt to load object %s from the database.  It has unknown type %i.",objectId.getValueString().c_str(),typeId));
	}
}

void SwgSnapshot::newObject(NetworkId const & objectId, int templateId, Tag typeId)
{
	m_objectTableBuffer.newObject(objectId, templateId, typeId);
	switch(typeId)
	{
		case ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag:
			newBattlefieldMarkerObject(objectId);
			break;

		case ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag:
			newBuildingObject(objectId);
			break;

		case ServerCellObjectTemplate::ServerCellObjectTemplate_tag:
			newCellObject(objectId);
			break;

		case ServerCityObjectTemplate::ServerCityObjectTemplate_tag:
			newCityObject(objectId);
			break;

		case ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag:
			newCreatureObject(objectId);
			break;

		case ServerFactoryObjectTemplate::ServerFactoryObjectTemplate_tag:
			newFactoryObject(objectId);
			break;

		case ServerGuildObjectTemplate::ServerGuildObjectTemplate_tag:
			newGuildObject(objectId);
			break;

		case ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag:
			newHarvesterInstallationObject(objectId);
			break;

		case ServerInstallationObjectTemplate::ServerInstallationObjectTemplate_tag:
			newInstallationObject(objectId);
			break;

		case ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate_tag:
			newIntangibleObject(objectId);
			break;

		case ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag:
			newManufactureInstallationObject(objectId);
			break;

		case ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag:
			newManufactureSchematicObject(objectId);
			break;

		case ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag:
			newMissionObject(objectId);
			break;

		case ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag:
			newPlanetObject(objectId);
			break;

		case ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag:
			newPlayerObject(objectId);
			break;

		case ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate_tag:
			newPlayerQuestObject(objectId);
			break;

		case ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag:
			newResourceContainerObject(objectId);
			break;

		case ServerShipObjectTemplate::ServerShipObjectTemplate_tag:
			newShipObject(objectId);
			break;

		case ServerStaticObjectTemplate::ServerStaticObjectTemplate_tag:
			newStaticObject(objectId);
			break;

		case ServerTangibleObjectTemplate::ServerTangibleObjectTemplate_tag:
			newTangibleObject(objectId);
			break;

		case ServerUniverseObjectTemplate::ServerUniverseObjectTemplate_tag:
			newUniverseObject(objectId);
			break;

		case ServerVehicleObjectTemplate::ServerVehicleObjectTemplate_tag:
			newVehicleObject(objectId);
			break;

		case ServerWeaponObjectTemplate::ServerWeaponObjectTemplate_tag:
			newWeaponObject(objectId);
			break;

		default:
			WARNING(true,("Attempt to create object %s.  It has unknown type %i.",objectId.getValueString().c_str(),typeId));
	}
}

void SwgSnapshot::registerTags()
{
	m_battlefieldMarkerObjectBuffer.addTag(ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag);
	m_buildingObjectBuffer.addTag(ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag);
	m_cellObjectBuffer.addTag(ServerCellObjectTemplate::ServerCellObjectTemplate_tag);
	m_cityObjectBuffer.addTag(ServerCityObjectTemplate::ServerCityObjectTemplate_tag);
	m_universeObjectBuffer.addTag(ServerCityObjectTemplate::ServerCityObjectTemplate_tag);
	m_creatureObjectBuffer.addTag(ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag);
	m_factoryObjectBuffer.addTag(ServerFactoryObjectTemplate::ServerFactoryObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerFactoryObjectTemplate::ServerFactoryObjectTemplate_tag);
	m_guildObjectBuffer.addTag(ServerGuildObjectTemplate::ServerGuildObjectTemplate_tag);
	m_universeObjectBuffer.addTag(ServerGuildObjectTemplate::ServerGuildObjectTemplate_tag);
	m_harvesterInstallationObjectBuffer.addTag(ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag);
	m_installationObjectBuffer.addTag(ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag);
	m_installationObjectBuffer.addTag(ServerInstallationObjectTemplate::ServerInstallationObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerInstallationObjectTemplate::ServerInstallationObjectTemplate_tag);
	m_intangibleObjectBuffer.addTag(ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate_tag);
	m_manufactureInstallationObjectBuffer.addTag(ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag);
	m_installationObjectBuffer.addTag(ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag);
	m_manufactureSchematicObjectBuffer.addTag(ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag);
	m_intangibleObjectBuffer.addTag(ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag);
	m_missionObjectBuffer.addTag(ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag);
	m_intangibleObjectBuffer.addTag(ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag);
	m_planetObjectBuffer.addTag(ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag);
	m_universeObjectBuffer.addTag(ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag);
	m_playerObjectBuffer.addTag(ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag);
	m_intangibleObjectBuffer.addTag(ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag);
	m_playerQuestObjectBuffer.addTag(ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate_tag);
	m_resourceContainerObjectBuffer.addTag(ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag);
	m_shipObjectBuffer.addTag(ServerShipObjectTemplate::ServerShipObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerShipObjectTemplate::ServerShipObjectTemplate_tag);
	m_staticObjectBuffer.addTag(ServerStaticObjectTemplate::ServerStaticObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerTangibleObjectTemplate::ServerTangibleObjectTemplate_tag);
	m_universeObjectBuffer.addTag(ServerUniverseObjectTemplate::ServerUniverseObjectTemplate_tag);
	m_vehicleObjectBuffer.addTag(ServerVehicleObjectTemplate::ServerVehicleObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerVehicleObjectTemplate::ServerVehicleObjectTemplate_tag);
	m_weaponObjectBuffer.addTag(ServerWeaponObjectTemplate::ServerWeaponObjectTemplate_tag);
	m_tangibleObjectBuffer.addTag(ServerWeaponObjectTemplate::ServerWeaponObjectTemplate_tag);
}

//!!!END GENERATED LOADOBJECT
