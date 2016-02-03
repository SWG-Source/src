

#include "FirstSwgGameServer.h"
#include "ServerObjectLint.h"

#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/NetworkId.h"

#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerDraftSchematicObjectTemplate.h"
#include "serverGame/ServerInstallationObjectTemplate.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerVehicleObjectTemplate.h"
#include "serverGame/ServerWeaponObjectTemplate.h"

#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCellObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedGame/SharedInstallationObjectTemplate.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedGame/SharedStaticObjectTemplate.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedGame/SharedVehicleObjectTemplate.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"

#include <stdio.h>

void ServerObjectLint::install()
{
	ObjectIdManager::getInstance().addBlock(100,10000000); //lint !e747  Significant prototype coercion (arg. no. 1) int to long long
}

void ServerObjectLint::run(const std::vector<std::string> & templateList)
{
	install();
	ServerWorld::install();


	std::vector<std::string>::const_iterator i = templateList.begin();
	FILE * output = fopen("server_object_lint.sln", "w"); //lint !e64 Type mismatch (initialization) (struct _IO_FILE * = int)
	FATAL(!output, ("Could not create output file"));
	for (; i != templateList.end(); ++i)
	{
		if (i->find("base") != std::string::npos)
		{
			//Don't try to instantiate base templtates
			IGNORE_RETURN(fprintf(output, "Skipping %s because it is a base class.\n", i->c_str())); //lint !e64 !e119
			continue;
		}
		DEBUG_REPORT_LOG(true, ("Now processing %s...\n", i->c_str()));
		IGNORE_RETURN(fprintf(output, "Now processing %s...\n", i->c_str()));//lint !e64 !e119
		IGNORE_RETURN(fflush(output));

		ServerObject* obj = ServerWorld::createObjectFromTemplate((*i), NetworkId(static_cast<NetworkId::NetworkIdType>(1)));
		DEBUG_REPORT_LOG(!obj, ("***ERROR: Could not create %s\n", i->c_str()));
		DEBUG_REPORT_LOG(obj,("Successfully created %s\n", i->c_str()));
		if (!obj)
			IGNORE_RETURN(fprintf(output, "***ERROR: Could not create %s\n", i->c_str())); //lint !e64 !e119
		else
		{
			IGNORE_RETURN(fprintf(output, "Successfully created %s\n", i->c_str())); //lint !e64 !e119
			obj->serverObjectInitializeFirstTimeObject(0, Transform::identity);
			delete obj;
			obj = 0;
		}
	}
	DEBUG_REPORT_LOG(true, ("-----Done with Server Objects ----\n"));
	IGNORE_RETURN(fprintf(output, ("-----Done with Server Objects ----\n"))); //lint !e64 !e119
	IGNORE_RETURN(fclose(output));
		
}

