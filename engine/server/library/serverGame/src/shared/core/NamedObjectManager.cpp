// ======================================================================
//
// NamedObjectManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NamedObjectManager.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

static std::map<std::string, NetworkId> s_namedObjects;

// ======================================================================

NetworkId const &NamedObjectManager::getNamedObjectId(std::string const &name) // static
{
	std::map<std::string, NetworkId>::const_iterator i = s_namedObjects.find(name);
	if (i != s_namedObjects.end())
		return (*i).second;
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

void NamedObjectManager::registerNamedObjectId(std::string const &name, NetworkId const &id) // static
{
	if (id == NetworkId::cms_invalid)
		IGNORE_RETURN(s_namedObjects.erase(name));
	else
		s_namedObjects[name] = id;
}

// ----------------------------------------------------------------------

void NamedObjectManager::createZoneObjects() // static
{
	if (!ConfigServerGame::getCreateZoneObjects())
		return;

	std::string zoneObjectFileName = std::string("datatables/zone_objects/") + ConfigServerGame::getSceneID() + std::string(".iff");

	Iff iff;
	if (iff.open(zoneObjectFileName.c_str(), true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			std::string objectName = dataTable.getStringValue("name", row);
			std::string templateName = dataTable.getStringValue("template", row);
			bool persisted = dataTable.getIntValue("persist", row) != 0;

			if (NamedObjectManager::getNamedObjectId(objectName) == NetworkId::cms_invalid)
			{
				REPORT_LOG(true, ("Creating zone object '%s' %s, template '%s'\n", objectName.c_str(), persisted ? "persisted" : "nonpersisted", templateName.c_str()));
				FATAL(persisted && !ConfigServerGame::getAllowMasterObjectCreation(), ("Master object creation is disabled and persisted named object '%s' could not be found!", objectName.c_str()));
				ServerObject *obj = NON_NULL(ServerWorld::createNewObject(templateName, Transform::identity, 0, persisted));
				obj->addToWorld();
				FATAL(persisted && NamedObjectManager::getNamedObjectId(objectName) == NetworkId::cms_invalid, ("Persisted master named object '%s' did not register on creation!", objectName.c_str()));
			}
		}
	}
}

// ======================================================================

