// ObjectFactory.cpp
// copyright 2000 Verant Interactive
// Author: Calan Thurow

//Contains the ObjectFactory functions
//(This is excluded from ServerWorld.cpp to reduce compile dependancies.)

//-----------------------------------------------------------------------

//TODO: we don't need to include the headers for all the object types after we remove CreateObjectByType()

//TODO:  I have no idea why we need this pragma:
#ifdef WIN32
  #pragma warning (disable: 4100)
#endif

#include "serverGame/FirstServerGame.h"

#include "sharedDebug/Profiler.h"
#include "sharedObject/NetworkController.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerObjectTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerObject.h"

//-----------------------------------------------------------------------

ServerObject * ServerWorld::createObjectFromTemplate(const std::string &templateName, const NetworkId & newId)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::createObjectFromTemplate1");
	return createObjectFromTemplate(Crc::calculate(templateName.c_str()), newId);
}	// ServerWorld::createObjectFromTemplate(const std::string &, NetworkId)

//-----------------------------------------------------------------------

ServerObject * ServerWorld::createObjectFromTemplate(uint32 templateCrc, const NetworkId & newId)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::createObjectFromTemplate1crc");

	ObjectTemplate const *objectTemplate = 0;

	{
		PROFILER_AUTO_BLOCK_DEFINE("ObjectTemplateList::fetch");
		objectTemplate = ObjectTemplateList::fetch(templateCrc);
	}

	if (!objectTemplate) {
		DEBUG_WARNING(!objectTemplate, ("Missing Template!  Can't create object from "
			"template crc %lu(%s), file not found", templateCrc, 
			ObjectTemplateList::lookUp(templateCrc).getString()));
		return 0;
	}

	Object *object = nullptr;

	if (objectTemplate)
	{
		ServerObjectTemplate const * serverObjectTemplate = objectTemplate->asServerObjectTemplate();

		if (   (serverObjectTemplate == nullptr)
		    || ((serverObjectTemplate != nullptr) && (ObjectTemplateList::lookUp(serverObjectTemplate->getSharedTemplate().c_str()).getCrc() != 0)))
		{
			PROFILER_AUTO_BLOCK_DEFINE("ObjectTemplate::createObject");
			object = objectTemplate->createObject();
			if (object)
			{
				ServerObject *serverObject = object->asServerObject();
				if (serverObject)
				{
					if (newId != NetworkId::cms_invalid)
						serverObject->setNetworkId(newId);
					else
						serverObject->setNetworkId(ObjectIdManager::getInstance().getNewObjectId());
					objectTemplate->releaseReference();
					return serverObject;
				}
				WARNING(true, ("tried to create non-server object %lu(%s)", templateCrc,
					ObjectTemplateList::lookUp(templateCrc).getString()));
				delete object;
			}
			objectTemplate->releaseReference();
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("ServerWorld::createObjectFromTemplate() Unable to create the ServerObject because there is no corresponding SharedObjectTemplate for the ServerObjectTemplate(%lu:%s)", templateCrc, ObjectTemplateList::lookUp(templateCrc).getString()));
		}
	}

	return 0;
}	// ServerWorld::createObjectFromTemplate(uint32, NetworkId)

//-----------------------------------------------------------------------

ServerObject * ServerWorld::createObjectFromTemplate(const ServerObjectTemplate &objectTemplate, const NetworkId & newId)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerWorld::createObjectFromTemplate2");

	ServerObject *object = safe_cast<ServerObject*>(objectTemplate.createObject());
	if (object)
	{
		if (newId != NetworkId::cms_invalid)
			object->setNetworkId(newId);
		else
			object->setNetworkId(ObjectIdManager::getInstance().getNewObjectId());
		return object;
	}
	return 0;
}	// ServerWorld::createObjectFromTemplate(const ServerObjectTemplate &, NetworkId)

//-----------------------------------------------------------------------

