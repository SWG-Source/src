// ======================================================================
//
// StaticLootItemManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/StaticLootItemManager.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/NonCriticalTaskQueue.h"
#include "serverGame/TaskGetAttributes.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

int const MAX_ATTRIBS = 2000;

// ======================================================================

void StaticLootItemManager::sendDataToClient(NetworkId const & playerId, std::string const & staticItemName)
{
	Object * const o = NetworkIdManager::getObjectById(playerId);
	ServerObject * const so = o ? o->asServerObject() : nullptr;
	CreatureObject * const co = so ? so->asCreatureObject() : nullptr;

	if(co)
	{
		ScriptParams params;
		params.addParam(staticItemName.c_str());
		IGNORE_RETURN(co->getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_STATIC_ITEM_DATA, params));
	}

	TaskGetAttributes * const task = new TaskGetAttributes(playerId, staticItemName, 1);
	NonCriticalTaskQueue::getInstance().addTask(task);
}

//----------------------------------------------------------------------

void StaticLootItemManager::getAttributes(NetworkId const & playerId, std::string const & staticItemName, AttributeVector & data)
{
	Object * const o = NetworkIdManager::getObjectById(playerId);
	ServerObject * const so = o ? o->asServerObject() : nullptr;
	CreatureObject * const co = so ? so->asCreatureObject() : nullptr;
	if(co)
	{
		GameScriptObject * const gso = const_cast<GameScriptObject *>(co->getScriptObject());
		if (gso && gso->hasTrigger (Scripting::TRIG_GET_STATIC_ITEM_ATTRIBS))
		{
			// allow scripts to add attribute data to the vector
			static std::vector<const char *>            names   (MAX_ATTRIBS, static_cast<const char *>(0));
			static std::vector<const Unicode::String *> attribs (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

			//-- clear the vectors
			names.clear    ();
			attribs.clear  ();
			names.resize   (MAX_ATTRIBS, static_cast<const char *>(0));
			attribs.resize (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

			ScriptParams params;
			params.addParam (staticItemName.c_str());
			params.addParam (names);
			params.addParam (attribs);

			IGNORE_RETURN(gso->trigAllScripts(Scripting::TRIG_GET_STATIC_ITEM_ATTRIBS, params));

			std::vector<const char *> const & newNames              = params.getStringArrayParam  (1);
			std::vector<const Unicode::String *> const & newAttribs = params.getUnicodeArrayParam (2);

			data.clear();

			// copy the returned strings to data
			for (int i = 0; i < MAX_ATTRIBS; ++i)
			{
				char const * const            newName   = newNames   [i];
				Unicode::String const * const newAttrib = newAttribs [i];

				if (newName && *newName && newAttrib && !newAttrib->empty ())
				{
					data.push_back (std::make_pair (std::string (newName), *newAttrib));
				}
				else
					break;
			}
		}
	}
}

// ======================================================================

