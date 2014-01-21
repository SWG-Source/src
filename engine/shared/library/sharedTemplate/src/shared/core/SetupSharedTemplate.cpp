//========================================================================
//
// SetupSharedTemplate.cpp - installs all the templates for the template 
// editor/compiler
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplate/SetupSharedTemplate.h"

#include "sharedTemplate/ServerArmorTemplate.h"
#include "sharedTemplate/ServerBattlefieldMarkerObjectTemplate.h"
#include "sharedTemplate/ServerBuildingObjectTemplate.h"
#include "sharedTemplate/ServerCellObjectTemplate.h"
#include "sharedTemplate/ServerCityObjectTemplate.h"
#include "sharedTemplate/ServerConstructionContractObjectTemplate.h"
#include "sharedTemplate/ServerCreatureObjectTemplate.h"
#include "sharedTemplate/ServerDraftSchematicObjectTemplate.h"
#include "sharedTemplate/ServerFactoryObjectTemplate.h"
#include "sharedTemplate/ServerGroupObjectTemplate.h"
#include "sharedTemplate/ServerGuildObjectTemplate.h"
#include "sharedTemplate/ServerHarvesterInstallationObjectTemplate.h"
#include "sharedTemplate/ServerInstallationObjectTemplate.h"
#include "sharedTemplate/ServerIntangibleObjectTemplate.h"
#include "sharedTemplate/ServerJediManagerObjectTemplate.h"
#include "sharedTemplate/ServerManufactureInstallationObjectTemplate.h"
#include "sharedTemplate/ServerManufactureSchematicObjectTemplate.h"
#include "sharedTemplate/ServerMissionObjectTemplate.h"
#include "sharedTemplate/ServerTangibleObjectTemplate.h"
#include "sharedTemplate/ServerTokenObjectTemplate.h"
#include "sharedTemplate/ServerVehicleObjectTemplate.h"
#include "sharedTemplate/ServerWaypointObjectTemplate.h"
#include "sharedTemplate/ServerWeaponObjectTemplate.h"
#include "sharedTemplate/ServerPlanetObjectTemplate.h"
#include "sharedTemplate/ServerPlayerQuestObjectTempate.h"
#include "sharedTemplate/ServerPlayerObjectTemplate.h"
#include "sharedTemplate/ServerResourceClassObjectTemplate.h"
#include "sharedTemplate/ServerResourceContainerObjectTemplate.h"
#include "sharedTemplate/ServerResourcePoolObjectTemplate.h"
#include "sharedTemplate/ServerResourceTypeObjectTemplate.h"
#include "sharedTemplate/ServerShipObjectTemplate.h"
#include "sharedTemplate/ServerStaticObjectTemplate.h"
#include "sharedTemplate/ServerUniverseObjectTemplate.h"
#include "sharedTemplate/ServerXpManagerObjectTemplate.h"

#include "sharedTemplate/SharedBattlefieldMarkerObjectTemplate.h"
#include "sharedTemplate/SharedBuildingObjectTemplate.h"
#include "sharedTemplate/SharedCellObjectTemplate.h"
#include "sharedTemplate/SharedConstructionContractObjectTemplate.h"
#include "sharedTemplate/SharedCreatureObjectTemplate.h"
#include "sharedTemplate/SharedDraftSchematicObjectTemplate.h"
#include "sharedTemplate/SharedFactoryObjectTemplate.h"
#include "sharedTemplate/SharedGroupObjectTemplate.h"
#include "sharedTemplate/SharedGuildObjectTemplate.h"
#include "sharedTemplate/SharedInstallationObjectTemplate.h"
#include "sharedTemplate/SharedIntangibleObjectTemplate.h"
#include "sharedTemplate/SharedJediManagerObjectTemplate.h"
#include "sharedTemplate/SharedManufactureSchematicObjectTemplate.h"
#include "sharedTemplate/SharedMissionObjectTemplate.h"
#include "sharedTemplate/SharedObjectTemplate.h"
#include "sharedTemplate/SharedPlayerQuestObjectTemplate.h"
#include "sharedTemplate/SharedPlayerObjectTemplate.h"
#include "sharedTemplate/SharedShipObjectTemplate.h"
#include "sharedTemplate/SharedStaticObjectTemplate.h"
#include "sharedTemplate/SharedTangibleObjectTemplate.h"
#include "sharedTemplate/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedTemplate/SharedTokenObjectTemplate.h"
#include "sharedTemplate/SharedUniverseObjectTemplate.h"
#include "sharedTemplate/SharedVehicleObjectTemplate.h"
#include "sharedTemplate/SharedWaypointObjectTemplate.h"
#include "sharedTemplate/SharedWeaponObjectTemplate.h"
#include "sharedTemplate/SharedResourceContainerObjectTemplate.h"

/**
 * Installs all the templates.
 */
void SetupSharedTemplate::install(void)
{
	// install templates
	ServerArmorTemplate::install();
	ServerBattlefieldMarkerObjectTemplate::install();
	ServerBuildingObjectTemplate::install();
	ServerCellObjectTemplate::install();
	ServerCityObjectTemplate::install();
	ServerConstructionContractObjectTemplate::install();
	ServerCreatureObjectTemplate::install();
	ServerDraftSchematicObjectTemplate::install();
	ServerFactoryObjectTemplate::install();
	ServerGroupObjectTemplate::install();
	ServerGuildObjectTemplate::install();
	ServerHarvesterInstallationObjectTemplate::install();
	ServerInstallationObjectTemplate::install();
	ServerIntangibleObjectTemplate::install();
	ServerJediManagerObjectTemplate::install();
	ServerManufactureInstallationObjectTemplate::install();
	ServerManufactureSchematicObjectTemplate::install();
	ServerMissionObjectTemplate::install();
	ServerObjectTemplate::install();
	ServerTangibleObjectTemplate::install();
	ServerTokenObjectTemplate::install();
	ServerVehicleObjectTemplate::install();
	ServerWeaponObjectTemplate::install();
	ServerPlanetObjectTemplate::install();
	ServerPlayerQuestObjectTemplate::install();
	ServerPlayerObjectTemplate::install();
	ServerResourceClassObjectTemplate::install();
	ServerResourceContainerObjectTemplate::install();
	ServerResourcePoolObjectTemplate::install();
	ServerResourceTypeObjectTemplate::install();
	ServerShipObjectTemplate::install();
	ServerStaticObjectTemplate::install();
	ServerUniverseObjectTemplate::install();
	ServerWaypointObjectTemplate::install();
	ServerXpManagerObjectTemplate::install();

	SharedBattlefieldMarkerObjectTemplate::install();
	SharedBuildingObjectTemplate::install();
	SharedCellObjectTemplate::install();
	SharedConstructionContractObjectTemplate::install();
	SharedCreatureObjectTemplate::install();
	SharedDraftSchematicObjectTemplate::install();
	SharedFactoryObjectTemplate::install();
	SharedGroupObjectTemplate::install();
	SharedGuildObjectTemplate::install();
	SharedInstallationObjectTemplate::install();
	SharedIntangibleObjectTemplate::install();
	SharedJediManagerObjectTemplate::install();
	SharedManufactureSchematicObjectTemplate::install();
	SharedMissionObjectTemplate::install();
	SharedObjectTemplate::install();
	SharedPlayerQuestObjectTemplate::install();
	SharedPlayerObjectTemplate::install();
	SharedShipObjectTemplate::install();
	SharedStaticObjectTemplate::install();
	SharedTangibleObjectTemplate::install();
	SharedTerrainSurfaceObjectTemplate::install();
	SharedTokenObjectTemplate::install();
	SharedUniverseObjectTemplate::install();
	SharedVehicleObjectTemplate::install();
	SharedWaypointObjectTemplate::install();
	SharedWeaponObjectTemplate::install();
	SharedResourceContainerObjectTemplate::install();
}	// SetupSharedTemplate::install


//==============================================================================
