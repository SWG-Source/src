//========================================================================
//
// GameScriptObject.cpp - responsible for managing/executing scripts.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/GameScriptObject.h"

#include "ScriptMessage.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerController.h"
#include "serverNetworkMessages/MessageQueueScriptTrigger.h"
#include "serverScript/ConfigServerScript.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "serverGame/ServerObject.h"

#include <algorithm>
#include <climits>


#ifdef _WIN32
#pragma warning (disable: 4514 4786)
#endif


//========================================================================
// class GameScriptObject static members
//========================================================================

GameScriptObject::ScriptDataMap * GameScriptObject::ms_scriptDataMap = nullptr;
bool GameScriptObject::m_pauseScripting = false;


//========================================================================
// class GameScriptObject methods
//========================================================================

/**
 * Class constructor.
 */
GameScriptObject::GameScriptObject(void) :
	m_owner(nullptr),
	m_scriptList(),
	m_scriptListInitialized(false),
	m_scriptListValid(false),
	m_lockScriptList(false),
	m_attachingScript(0)
{
	m_synchronizedScriptList.setSourceObject (this);
}

/**
 * Class destructor.
 */
GameScriptObject::~GameScriptObject()
{
	{
	PROFILER_AUTO_BLOCK_DEFINE("GameScriptObject::~GameScriptObject removeJavaId\n");
	if (JavaLibrary::instance() != nullptr /*&& m_javaId != nullptr*/)
	{
		NOT_NULL(m_owner);
		JavaLibrary::removeJavaId(m_owner->getNetworkId());
//		m_javaId = nullptr;
	}
	}

	{
	PROFILER_AUTO_BLOCK_DEFINE("GameScriptObject::removeAll\n");
	removeAll();
	}
	m_scriptList.clear();
	m_owner = nullptr;
}	// GameScriptObject::~GameScriptObject

/**
 * Creates the script engine.
 *
 * @return true on success, false on fail
 */
bool GameScriptObject::installScriptEngine(void)
{
	if (JavaLibrary::instance() != nullptr)
	{
		DEBUG_WARNING(true, ("Trying to install script engine more than once"));
		return true;
	}

	ms_scriptDataMap = new GameScriptObject::ScriptDataMap;
    Scripting::InitScriptFuncHashMap();
    JavaLibrary::install();
    if (JavaLibrary::instance() == nullptr)
        return false;
	enableNewJediTracking(ConfigServerGame::getEnableNewJedi());
    return true;
}	// GameScriptObject::installScriptEngine

/**
 * Destroys the script engine.
 */
void GameScriptObject::removeScriptEngine(void)
{
    JavaLibrary::remove();
    delete ms_scriptDataMap;
    ms_scriptDataMap = nullptr;
	Scripting::RemoveScriptFuncHashMap();
}	// GameScriptObject::removeScriptEngine

// ------------------------------------------------------------------------

void GameScriptObject::addToPackage(Archive::AutoDeltaByteStream & bs)

{
	bs.addVariable(m_synchronizedScriptList);
}

// ------------------------------------------------------------------------

/**
 * Called during game alter phase.
 *
 * @param time		time since last frame
 */
void GameScriptObject::alter(real time)
{
	UNREF(time);
}  //lint !e1762 Do not make const

/**
 * Tells the scripting engine that my owner is authoritative on this server.
 *
 * @param authoritative		flag if my owner is authoritative or not
 */
void GameScriptObject::setOwnerIsAuthoritative(bool authoritative, uint32 pid)
{
	if (JavaLibrary::instance() == nullptr)
		return;

	if (m_owner != nullptr && m_owner->getNetworkId() != NetworkId::cms_invalid)
	{
		JavaLibrary::instance()->setObjIdAuthoritative(m_owner->getNetworkId(), authoritative, pid);

		// clear out old data
		m_scriptListValid = false;
		if (authoritative && m_scriptListInitialized)
		{
			initScriptInstances();
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Error setting Java obj_id authority, no owner!"));
	}
}	// GameScriptObject::setOwnerIsAuthoritative

/**
 * Tells the scripting engine that my owner is loaded on this server. You cannot
 * unload an object, it is marked unloaded when it is deleted.
 */
void GameScriptObject::setOwnerIsLoaded(void)
{
	if (JavaLibrary::instance() == nullptr)
		return;
	if (m_owner != nullptr && m_owner->getNetworkId() != NetworkId::cms_invalid)
	{
		JavaLibrary::instance()->setObjIdLoaded(m_owner->getNetworkId());
	}
	else
	{
		DEBUG_WARNING(true, ("Error setting Java obj_id loaded, no owner!"));
	}
} //lint !e1762 Do not make const

/**
 * Tells the scripting engine that my owner is initialized on this server. You cannot
 * uninitialize an object, it is marked uninitialized when it is deleted.
 */
void GameScriptObject::setOwnerIsInitialized(void)
{
	if (JavaLibrary::instance() == nullptr)
		return;
	if (m_owner != nullptr && m_owner->getNetworkId() != NetworkId::cms_invalid)
	{
		JavaLibrary::instance()->setObjIdInitialized(m_owner->getNetworkId());
		if (m_owner->isAuthoritative())
		{
			ScriptParams params;
			IGNORE_RETURN(trigAllScripts(Scripting::TRIG_INITIALIZE, params));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Error setting Java obj_id loaded, no owner!"));
	}
} //lint !e1762 Do not make const

/**
 * Flag the object in script as going to be destroyed, but hasn't actually been removed from the game yet.
 */
void GameScriptObject::setOwnerDestroyed(void)
{
	if (JavaLibrary::instance() == nullptr)
		return;

	if (m_owner != nullptr && m_owner->getNetworkId() != NetworkId::cms_invalid && m_owner->isAuthoritative())
		JavaLibrary::flagDestroyed(m_owner->getNetworkId());

} //lint !e1762 Do not make const

/**
 * Adds a script to the start of the script list.
 *
 * @param scriptName		script to add
 * @param runTrigger		flag to run the TRIG_ATTACH trigger
 *
 * @return SCRIPT_CONTINUE on success, SCRIPT_OVERRIDE on fail
 */
int GameScriptObject::attachScript(const std::string& scriptName, bool runTrigger)
{
	if (!m_owner)
	{
		WARNING_STRICT_FATAL(true, ("Attach script %s called with no owner", scriptName.c_str()));
		return SCRIPT_OVERRIDE;
	}

	if (scriptName.empty())
	{
		DEBUG_WARNING(true, ("ERROR in GameScriptObject::AttachScript: no script name"));
		return SCRIPT_OVERRIDE;
	}
	if (scriptName.size() >= MAX_SCRIPT_NAME_LEN)
	{
		DEBUG_WARNING(true, ("ERROR in GameScriptObject::AttachScript: name too long "
			"(%s)", scriptName.c_str()));
		return SCRIPT_OVERRIDE;
	}

	if(!m_owner->isAuthoritative())
	{

		MessageQueueString * data = new MessageQueueString(scriptName);
		m_owner->getController()->appendMessage(
			static_cast<int>(CM_attachScript),
			0.0f,
			data,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER);
		return SCRIPT_CONTINUE;
	}
	if (m_lockScriptList)
	{
		DEBUG_WARNING(true, ("Trying to attach script %s to object %s while the "
			"script list is locked.", scriptName.c_str(),
			m_owner->getNetworkId().getValueString().c_str()));
		return SCRIPT_OVERRIDE;
	}

	if (hasScript(scriptName))
	{
		DEBUG_WARNING(true, ("WARNING in GameScriptObject::AttachScript: dup script "
			"(%s)", scriptName.c_str()));
		return SCRIPT_OVERRIDE;
	}

	if (ConfigServerScript::getAllowBuildClusterScriptAttach())
	{
		//@todo check for file existance.
		std::pair<ScriptDataMap::iterator, bool> result = ms_scriptDataMap->insert(std::make_pair(scriptName, ScriptData()));
		m_scriptList.push_back(ScriptListEntry(&(*result.first)));
	}
	else
	{
		if (ms_scriptDataMap == nullptr || JavaLibrary::instance() == nullptr)
			return SCRIPT_CONTINUE;

		if (ms_scriptDataMap->find(scriptName) == ms_scriptDataMap->end())
		{
			// if this is the first time the script has been attached, query it to get a
			// list of triggers, messageHandlers, and pathFindHandlers it has
			if (!JavaLibrary::instance()->queryScriptFunctions(scriptName))
			{
				WARNING(true, ("Error while attaching script %s. Does it exist?", scriptName.c_str()));
				return SCRIPT_OVERRIDE;
			}
		}

		ScriptListEntry entry = GameScriptObject::getScriptListEntry(scriptName);

		// put a pointer to the pair of scriptName and ScriptData in the script list
		m_scriptList.push_back(entry);

		if (runTrigger)
		{
			// flag that we are attaching a script to ourselves so we can warn if
			// we get a messageTo during this time
			++m_attachingScript;

			// Make sure to call the attach trigger on this one script if one exists.
			ScriptParams params;
			IGNORE_RETURN(trigOneScript(scriptName, Scripting::TRIG_ATTACH, params));

			--m_attachingScript;
		}

		// if the script list hasn't been initialized, wait until it has,
		// else go ahead and initialize the script
		if (m_scriptListValid && entry.isValid())
		{
			JavaLibrary::instance()->attachScriptToObjId(m_owner->getNetworkId(), scriptName);
			entry.getScriptData().owners.insert(m_owner->getNetworkId());
		}
	}
	packScriptList();
	return SCRIPT_CONTINUE;
}	// GameScriptObject::attachScript

/**
 * Removes a script from the script list.
 *
 * @param scriptName		script to remove
 *
 * @return SCRIPT_CONTINUE on success, SCRIPT_OVERRIDE on fail
 */
int GameScriptObject::detachScript(const std::string& scriptName)
{
	if (!m_owner)
	{
		WARNING_STRICT_FATAL(true, ("Detach script %s called with no owner", scriptName.c_str()));
		return SCRIPT_OVERRIDE;
	}
	if (!m_owner->isAuthoritative())
	{

		MessageQueueString * data = new MessageQueueString(scriptName);
		m_owner->getController()->appendMessage(
			CM_detachScript,
			0.0f,
			data,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER);
		return SCRIPT_CONTINUE;
	}

	if (m_lockScriptList)
	{
		DEBUG_WARNING(true, ("Trying to detach script %s to object %s while the script list is locked.",
			scriptName.c_str(), m_owner->getNetworkId().getValueString().c_str()));
		return SCRIPT_OVERRIDE;
	}

	ScriptList::iterator i;
	for (i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
		if ((*i).isValid() && (*i).getScriptName() == scriptName)
			break;
	if (i==m_scriptList.end())
	{
		return SCRIPT_OVERRIDE;
	}

	int retval = SCRIPT_CONTINUE;
	if (ConfigServerScript::getAllowBuildClusterScriptAttach())
	{
		m_scriptList.erase(i);
	}
	else
	{

		if (JavaLibrary::instance() == nullptr)
			return SCRIPT_CONTINUE;

		// Make sure to call the detach trigger on this one script if one exists.
		ScriptParams params;
		retval = trigOneScript(scriptName, Scripting::TRIG_DETACH, params);

		// Do not remove the script on a return of "override"
		if (retval == SCRIPT_CONTINUE)
		{
			ScriptDataMap::iterator scriptIter = ms_scriptDataMap->find(scriptName);
			if (scriptIter != ms_scriptDataMap->end())
			{
				// remove ourself from the script's owners
				(*scriptIter).second.owners.erase(m_owner->getNetworkId());
			}

			// the OnDetach triggers may have removed other scripts, added new scripts, etc., so we can't
			// depend on our iterator still being valid.  Find the script again:
			for (i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
				if ((*i).isValid() && (*i).getScriptName() == scriptName)
				{
					m_scriptList.erase(i);
					break;
				}

			JavaLibrary::instance()->detachScriptFromObjId(m_owner->getNetworkId(), scriptName);
		}
	}
	packScriptList();
	return retval;
}	// GameScriptObject::detachScript

/**
 * Initialize script instance map with all de-persisted
 * scripts
 */
void GameScriptObject::initScriptInstances()
{
	m_scriptListInitialized = true;

	if (JavaLibrary::instance() == nullptr)
		return;

	if (!m_owner)
	{
		WARNING_STRICT_FATAL(true, ("Use of nullptr m_owner in ::initScriptInstances()"));
		return;
	}

	if (m_scriptListValid || !m_owner->isAuthoritative())
		return;

	// while we are in this function, scripts may not be attached or detached on the object
	m_lockScriptList = true;

	// reset the script list on the Java side
	JavaLibrary::instance()->detachAllScriptsFromObjId(m_owner->getNetworkId());
	JavaLibrary::instance()->attachScriptsToObjId(m_owner->getNetworkId(), m_scriptList);

	for (ScriptList::const_iterator i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
	{
		if ((*i).isValid())
			(*i).getScriptData().owners.insert(m_owner->getNetworkId());
	}

	m_lockScriptList = false;
	m_scriptListValid = true;
}	// GameScriptObject::initScriptInstances

/**
 * Clears the script list.
 */
void GameScriptObject::removeAll(void)
{
	if (JavaLibrary::instance() == nullptr)
		return;

	// removeAll() cannot be overriden by scripts

	// remove ourself from all scripts
	for (ScriptList::const_iterator i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
	{
		if ((*i).isValid())
			(*i).getScriptData().owners.erase(m_owner->getNetworkId());
	}

	// clear our lists
	m_scriptList.clear();
	packScriptList();
}	// GameScriptObject::removeAll

/**
 * Runs all the scripts in the script list.
 *
 * @param trigId			script function id to call
 * @param params			params to pass to the script functions
 *
 * @return SCRIPT_CONTINUE on success, SCRIPT_OVERRIDE on fail
 */
int GameScriptObject::trigAllScripts(Scripting::TrigId trigId, ScriptParams &params)
{
	NOT_NULL(m_owner);

	if (JavaLibrary::instance() == nullptr)
		return SCRIPT_CONTINUE;

//	if (m_javaId == nullptr && m_owner != nullptr && m_owner->getNetworkId().getValue() != 0)
//	{
//		if (JavaLibrary::instance() != nullptr)
//			m_javaId = JavaLibrary::getObjId(m_owner->getNetworkId());
//	}

	if (m_pauseScripting)
		return SCRIPT_CONTINUE;

	//Authoritative check temporarily removed because some triggers aren't being called
	//because the setAuth message hasn't come in yet from Central on load.
	if (m_owner == nullptr) //@todo FIX THIS HACK || !m_owner->isAuthoritative())
		return SCRIPT_CONTINUE;
	
	if(!m_owner->isAuthoritative())
	{
		Archive::ByteStream paramArchive;
		Archive::put(paramArchive, params);
		MessageQueueScriptTrigger * data = new MessageQueueScriptTrigger(static_cast<int>(trigId), paramArchive);
		ServerController * controller = dynamic_cast<ServerController *>(m_owner->getController());
		if(controller != nullptr)
		{
			controller->appendMessage(
				CM_scriptTrigger,
				0.0f,
				data,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER
				);
		}
		return SCRIPT_CONTINUE;
	}

	// handle special login/logout needs
	if (trigId == Scripting::TRIG_LOGIN)
		JavaLibrary::instance()->setObjIdLoggedIn(m_owner->getNetworkId(), true);
	else if (trigId == Scripting::TRIG_LOGOUT || trigId == Scripting::TRIG_IMMEDIATE_LOGOUT)
		JavaLibrary::instance()->setObjIdLoggedIn(m_owner->getNetworkId(), false);

	if (m_scriptList.size() == 0)
		return SCRIPT_CONTINUE;

	Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(static_cast<int>(trigId));
	if (funcIter == Scripting::ScriptFuncHashMap->end())
	{
		WARNING_STRICT_FATAL(true, ("Got bad trigger id\n"));
		return SCRIPT_CONTINUE;
	}
	const Scripting::ScriptFuncTable *func = (*funcIter).second;

	// If any script has a handler for the trigger, invoke the script library's
	// run all scripts function.
	if (hasFunction(func->name))
		return JavaLibrary::instance()->runScripts(m_owner->getNetworkId(), func->name, func->argList, params);

	return SCRIPT_CONTINUE;
}	// GameScriptObject::trigAllScripts

/**
 * Runs one script.
 *
 * @param scriptName		script to run
 * @param trigId			script function id to call
 * @param params			params to pass to the script function
 *
 * @return SCRIPT_CONTINUE on success, SCRIPT_OVERRIDE on fail or the script
 *		does not exist
 */
int GameScriptObject::trigOneScript(const std::string& scriptName, Scripting::TrigId trigId, ScriptParams &params) const
{
	if (JavaLibrary::instance() == nullptr)
		return SCRIPT_CONTINUE;

	if (m_pauseScripting)
		return SCRIPT_CONTINUE;

	if (m_owner == nullptr /*|| !m_owner->isAuthoritative()*/)
		return SCRIPT_OVERRIDE;

	if (!m_owner->isAuthoritative() && (trigId != Scripting::TRIG_ATTACH &&
		trigId != Scripting::TRIG_DETACH && trigId != Scripting::TRIG_INITIALIZE))
	{
		DEBUG_WARNING(true, ("Called trigOneScript on invalid trigger id %d", trigId));
		return SCRIPT_OVERRIDE;
	}

	if (!hasScript(scriptName) && trigId != Scripting::TRIG_ATTACH)
		return SCRIPT_OVERRIDE;

	Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(static_cast<int>(trigId));
	if (funcIter == Scripting::ScriptFuncHashMap->end())
		return SCRIPT_OVERRIDE;
	const Scripting::ScriptFuncTable *func = (*funcIter).second;

	NOT_NULL(m_owner);

	// check to see if the script supports the trigger
	ScriptDataMap::iterator scriptFuncs = ms_scriptDataMap->find(scriptName);
	if (scriptFuncs != ms_scriptDataMap->end())
	{
		std::set<std::string>::const_iterator result = (*scriptFuncs).second.functions.find(func->name);
		if (result == (*scriptFuncs).second.functions.end())
		{
			return SCRIPT_CONTINUE;
		}
	}

	return JavaLibrary::instance()->runScript(m_owner->getNetworkId(), scriptName, func->name, func->argList, params);
}	// GameScriptObject::trigOneScript

/**
 * Runs one or all script(s), as invoked from the console.
 *
 * @param trigId		script function id to call
 * @param scriptName	script to run; if 0 length, runs all scripts
 * @param args			params to send to the script (as a string vector)
 */
int GameScriptObject::trigScriptFromConsole(const Scripting::TrigId trigId,
	const std::string &scriptName, const StringVector_t &args) const
{
	if (JavaLibrary::instance() == nullptr)
	{
		LOG("ScriptInvestigation", ("Returning script continue from console trigger request because there is no JavaLibrary instance\n"));
		return SCRIPT_CONTINUE;
	}

	if (m_pauseScripting)
	{
		LOG("ScriptInvestigation", ("Returning script continue from console trigger request because scripting is paused\n"));
		return SCRIPT_CONTINUE;
	}

	if (m_owner == nullptr )
	{
		LOG("ScriptInvestigation", ("Returning script override from console trigger request becuase m_owner is nullptr\n"));
		return SCRIPT_OVERRIDE;
	}

	if (!m_owner->isAuthoritative())
	{
		LOG("ScriptInvestigation", ("Returning script override from console trigger request becuase owner is not authoritative\n"));
		return SCRIPT_OVERRIDE;
	}

	Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(static_cast<int>(trigId));
	if (funcIter == Scripting::ScriptFuncHashMap->end())
	{	
		LOG("ScriptInvestigation", ("Returning script override from console trigger request becuase trigger id %i was not found\n", trigId));
		return SCRIPT_OVERRIDE;
	}
	const Scripting::ScriptFuncTable *func = (*funcIter).second;

	NOT_NULL(m_owner);

	int retval = SCRIPT_CONTINUE;
	if (scriptName.size() == 0)
	{
		// trigger all scripts
		if (hasFunction(func->name))
			retval = JavaLibrary::instance()->runScripts(m_owner->getNetworkId(), func->name, func->argList, args);
	}
	else
	{
		if (!hasScript(scriptName))
			return SCRIPT_OVERRIDE;
		retval = JavaLibrary::instance()->runScript(m_owner->getNetworkId(), scriptName, func->name, func->argList, args);
	}
	return retval;
}	// GameScriptObject::trigScriptFromConsole

bool GameScriptObject::handleMessage(const std::string &messageName,
	const ScriptDictionaryPtr & data)
{
	if (JavaLibrary::instance() == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("(messageToFailure) GameScriptObject::handleMessage got message "
			"when Java not running"));
		return true;
	}

	if (getOwner() == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("(messageToFailure) GameScriptObject::handleMessage got message "
			"with no owner"));
		return true;
	}

	if (m_pauseScripting)
	{
		LOG("messageToFailure", ("GameScriptObject::handleMessage received "
			"message %s for object %s while scripting is paused",
			messageName.c_str(), getOwner()->getNetworkId().getValueString().c_str()));
		return false;
	}

	if (!m_scriptListValid)
	{
		LOG("messageToFailure", ("GameScriptObject::handleMessage received "
			"message %s for object %s while the script list is invalid",
			messageName.c_str(), getOwner()->getNetworkId().getValueString().c_str()));
		return false;
	}

	WARNING (m_attachingScript > 0, ("(messageToFailure) Object %s received messageTo while attaching "
		"a script, the message will not get to the script's handler",
		getOwner()->getNetworkId().getValueString().c_str()));

	if (hasFunction(messageName))
		IGNORE_RETURN(JavaLibrary::instance()->callMessages(m_owner->getNetworkId(), messageName, data));
	return true;
}	// GameScriptObject::handleMessage

bool GameScriptObject::handleMessage(const std::string &messageName, const std::vector<int8> &packedData)
{
	ScriptDictionaryPtr dictionary;

	if (JavaLibrary::instance() == nullptr)
		return true;

	if (m_pauseScripting)
		return false;

	bool result = false;
	if (JavaLibrary::instance()->unpackDictionary(packedData, dictionary))
	{
		result = handleMessage(messageName, dictionary);
	}
	else
	{
		WARNING(true, ("Failed to unpack dictionary for message %s. This message will not be handled.", messageName.c_str()));
		// prevent the message from being resent
		result = true;
	}
	return result;
}	// GameScriptObject::handleMessage

/**
 * Execute any applicable command handlers in the script list.
 *
 * @param funcName  name of the command handler to call
 * @param params    parameters to pass to the command handler
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int GameScriptObject::callScriptCommandHandler(std::string const &funcName, ScriptParams &params) const
{
	if (JavaLibrary::instance() == nullptr)
		return SCRIPT_CONTINUE;

	if (m_pauseScripting)
		return SCRIPT_CONTINUE;

	NOT_NULL(m_owner);

	if (hasFunction(funcName))
		return JavaLibrary::instance()->runScripts(m_owner->getNetworkId(), funcName, "Ouf", params);

	return SCRIPT_CONTINUE;
}

/**
 * Execute any applicable buff handlers in the script list.
 *
 * @param funcName  name of the buff handler to call
 * @param params    parameters to pass to the buff handler
 *
 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
 */
int GameScriptObject::callScriptBuffHandler(std::string const &funcName, ScriptParams &params) const
{
	if (JavaLibrary::instance() == nullptr)
		return SCRIPT_CONTINUE;

	if (m_pauseScripting)
		return SCRIPT_CONTINUE;

	NOT_NULL(m_owner);

	if (hasFunction(funcName))
		return JavaLibrary::instance()->runScripts(m_owner->getNetworkId(), funcName, "ssffsO", params);

	return SCRIPT_CONTINUE;
}

/**
 * Determines if a script is in the script list.
 *
 * @param scriptName		script to look for
 *
 * @return true if found, false if not
 */
bool GameScriptObject::hasScript(const std::string& scriptName) const
{
	for (ScriptList::const_iterator i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
		if ((*i).isValid() && (*i).getScriptName() == scriptName)
			return true;
	return false;
}	// GameScriptObject::hasScript

/**
 * Determines if any script implements a given trigger.
 *
 * @param trigId		the trigger to check for
 *
 * @return true if found, false if not
 */
bool GameScriptObject::hasTrigger(const Scripting::TrigId trigId) const
{
	// get the trigger fuction description
	Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(static_cast<int>(trigId));
	if (funcIter == Scripting::ScriptFuncHashMap->end())
		return false;
	const Scripting::ScriptFuncTable *func = (*funcIter).second;
	return hasFunction(func->name);
}	// GameScriptObject::hasTrigger

/**
 * Fills in a vector with a copy of the current script list.
 */
void GameScriptObject::enumerateScripts(std::vector<std::string> &scriptNames) const
{
	scriptNames.clear();
	for (ScriptList::const_iterator i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
	{
		if ((*i).isValid())
			scriptNames.push_back((*i).getScriptName());
	}
}	// GameScriptObject::EnumerateScripts

/**
 * Converts ScriptParams into a ScriptDictionary.
 *
 * @param params		the ScriptParams to convert
 * @param dictionary	pointer that will be set to the dictionary. The caller
 *						is responsible for deleting the pointer.
 */
void GameScriptObject::makeScriptDictionary(const ScriptParams & params, ScriptDictionaryPtr & dictionary)
{
	if (JavaLibrary::instance() != nullptr)
	{
		JavaDictionaryPtr jdp;
		JavaLibrary::instance()->convert(params, jdp);
		dictionary = jdp;
	}
}

/**
 * Tests if the scripting engine is running.
 *
 * @return true if the engine is running, false if not
 */
bool GameScriptObject::isScriptingEnabled(void)
{
	if (JavaLibrary::instance() != nullptr)
		return true;
	return false;
}	// GameScriptObject::isScriptingEnabled

/**
 * Tells the script to reload a script file.
 *
 * @param scriptName		the script name
 *
 * @return true on success, false if the script wasn't found
 */
bool GameScriptObject::reloadScript(const std::string& scriptName)
{
	if (JavaLibrary::instance() == nullptr)
		return false;

	
	bool result = false;
	ScriptDataMap::iterator scriptIter;

	scriptIter = ms_scriptDataMap->find(scriptName);
	if (scriptIter != ms_scriptDataMap->end())
		(*scriptIter).second.functions.clear();

	result = JavaLibrary::instance()->reloadScript(scriptName);

	// query the script to get a list of triggers, messageHandlers, and
	// pathFindHandlers it has
	if (result)
	{
		if (!JavaLibrary::instance()->queryScriptFunctions(scriptName))
		{
			WARNING_STRICT_FATAL(true, ("Script query during reload of %s failed\n", scriptName.c_str()));
		}
		for(scriptIter = ms_scriptDataMap->begin(); scriptIter != ms_scriptDataMap->end(); ++scriptIter)
		{
			result = JavaLibrary::instance()->reloadScript(scriptIter->first);
			if(result)
			{
				if (!JavaLibrary::instance()->queryScriptFunctions(scriptIter->first))
				{
					WARNING_STRICT_FATAL(true, ("Script query during reload of %s failed\n", scriptIter->first.c_str()));
				}
			}
			else
			{
				return false;
			}
		}
	}

	return result;
}	// GameScriptObject::reloadScript

//-----------------------------------------------------------------------

/**
 * Turns script logging on or off.
 *
 * @param enable		flag to turn logging on or off
 */
void GameScriptObject::enableLogging(bool enable)
{
	if (JavaLibrary::instance() != nullptr)
		JavaLibrary::instance()->enableLogging(enable);
}	// GameScriptObject::enableLogging

//-----------------------------------------------------------------------

/**
 * Turns tracking of players to see if they can become Jedi on or off.
 *
 * @param enableTracking	flag to turn tracking on or off
 */
void GameScriptObject::enableNewJediTracking(bool enableTracking)
{
	if (JavaLibrary::instance() != nullptr)
		JavaLibrary::instance()->enableNewJediTracking(enableTracking);
}	// GameScriptObject::enableNewJediTracking

//-----------------------------------------------------------------------

/**
 * Returns the amount of free memory left to the script system.
 */
int GameScriptObject::getScriptFreeMem()
{
	int64 freemem = JavaLibrary::getFreeJavaMemory();
	if (freemem > INT_MAX)
		return INT_MAX;
	return static_cast<int>(freemem);
}

//-----------------------------------------------------------------------

Scheduler & GameScriptObject::getScriptScheduler()
{
	static Scheduler scriptScheduler;
	return scriptScheduler;
}

//-----------------------------------------------------------------------

void GameScriptObject::onStopWatching(ServerObject & subject)
{
	if (JavaLibrary::instance() == nullptr)
		return;

	if (m_owner)
	{
		JavaLibrary::onStopWatching(*m_owner, subject);
	}
}

//-----------------------------------------------------------------------

void GameScriptObject::onWatching(ServerObject & subject)
{
	if (JavaLibrary::instance() == nullptr)
		return;

	if (m_owner)
	{
		JavaLibrary::onWatching(*m_owner, subject);
	}
}

//-----------------------------------------------------------------------

void GameScriptObject::runOneScript(const std::string & scriptName, const std::string & methodName, const std::string & argTypes, ScriptParams & args)
{
	if (JavaLibrary::instance() != nullptr)
		IGNORE_RETURN( JavaLibrary::instance()->runScript(NetworkId(), scriptName, methodName, argTypes, args) );
}

//-----------------------------------------------------------------------

/**
 * Calls a special function on a script that returns a string instead of an
 * integer. The script function has no "self" associated with it.
 *
 * @param scriptName	the name of the script to call
 * @param methodName	the function name in the script
 * @param argTypes		the argument types that are being passed to the script
 * @param args			the arguments that are being passed to the script
 *
 * @return the string returned from the script function
 */
std::string GameScriptObject::callScriptConsoleHandler(const std::string & scriptName,
	const std::string & methodName, const std::string & argTypes, ScriptParams & args)
{
static const std::string errorReturnString;

	if (JavaLibrary::instance() != nullptr)
	{
		return JavaLibrary::instance()->callScriptConsoleHandler(scriptName, methodName, argTypes, args);
	}
	return errorReturnString;
}	// GameScriptObject::callScriptConsoleHandler

//-----------------------------------------------------------------------

void GameScriptObject::callSpaceMakeOvert(const NetworkId &player)
{
	JavaLibrary::spaceMakeOvert(player);
}

//-----------------------------------------------------------------------

void GameScriptObject::callSpaceClearOvert(const NetworkId &ship)
{
	JavaLibrary::spaceClearOvert(ship);
}

//-----------------------------------------------------------------------

std::string GameScriptObject::callDumpTargetInfo( NetworkId &id )
{
	if( JavaLibrary::instance() != nullptr )
	{
		return JavaLibrary::instance()->getObjectDumpInfo( id );
	}
	return "";
}

//-----------------------------------------------------------------------

void GameScriptObject::setScriptVar(const std::string & name, int value)
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::setScriptVar(*m_owner, name, value);
}

//-----------------------------------------------------------------------

void GameScriptObject::setScriptVar(const std::string & name, float value)
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::setScriptVar(*m_owner, name, value);
}

//-----------------------------------------------------------------------

void GameScriptObject::setScriptVar(const std::string & name, const std::string & value)
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::setScriptVar(*m_owner, name, value);
}

//-----------------------------------------------------------------------

void GameScriptObject::packAllScriptVarDeltas()
{
	if (JavaLibrary::instance() == nullptr)
		return;

	JavaLibrary::packAllDeltaScriptVars();
}

//-----------------------------------------------------------------------

void GameScriptObject::clearScriptVars()
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::clearScriptVars(*m_owner);
}

//-----------------------------------------------------------------------

void GameScriptObject::packScriptVars(std::vector<int8> & target) const
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::packScriptVars(*m_owner, target);
}

//-----------------------------------------------------------------------

void GameScriptObject::unpackScriptVars(const std::vector<int8> & source) const
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	JavaLibrary::unpackScriptVars(*m_owner, source);
}

//-----------------------------------------------------------------------

void GameScriptObject::unpackDeltaScriptVars(const std::vector<int8> & data) const
{
	if (JavaLibrary::instance() == nullptr || m_owner == nullptr)
		return;

	DEBUG_REPORT_LOG(! m_owner, ("A game script object received a request to unpack script var synchronization data, but it has no owner object!!! All GameScriptObjects MUST have owners!\n"));
	JavaLibrary::unpackDeltaScriptVars(*m_owner, data);
}

//-----------------------------------------------------------------------

bool GameScriptObject::hasFunction(std::string const &funcName) const
{
	for (ScriptList::const_iterator i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
	{
		if ((*i).isValid())
		{
			std::set<std::string> const &functions = (*i).getScriptData().functions;
			if (functions.find(funcName) != functions.end())
				return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

ScriptListEntry GameScriptObject::getScriptListEntry(std::string const &scriptName) // static
{
	// This function gets a valid script list entry for the script name
	// specified, regardless of whether the script has been previously
	// loaded, or exists at all, and if the script was not previously
	// loaded, looks up the accessible functions from the script.
	std::pair<ScriptDataMap::iterator, bool> result = ms_scriptDataMap->insert(std::make_pair(scriptName, ScriptData()));
	ScriptListEntry entry(&(*result.first));
	if (result.second)
	{
		// if the script was not already in the map, then we need to look up the functions in it
		if (!JavaLibrary::instance()->queryScriptFunctions(scriptName))
			WARNING(true, ("Error while attaching script %s. Does it exist?", scriptName.c_str()));
	}
	return entry;
}

// ----------------------------------------------------------------------

/**
 * Pack the script list vector into the synchronized variable
 */
void GameScriptObject::packScriptList()
{
	std::string newPackedList;
	
	for (ScriptList::const_iterator i=m_scriptList.begin(); i!=m_scriptList.end(); ++i)
	{
		newPackedList+=i->getScriptName()+':';
	}

	if (newPackedList.size() > 2000)
	{
		if (m_owner)
			FATAL(true, ("Object %s has a script list that exceeds 2000 characters.  The list:  %s",m_owner->getNetworkId().getValueString().c_str(), newPackedList.c_str()));
		else
			FATAL(true, ("Object has a script list that exceeds 2000 characters, but the object id could not be determined.  The list:  %s", newPackedList.c_str()));
	}
	
	m_synchronizedScriptList=newPackedList;
}

// ----------------------------------------------------------------------

/**
 * Unpack the script list from the synchronized variable
 */
void GameScriptObject::unpackScriptList()
{
	const std::string &packedList = m_synchronizedScriptList.get();

	m_scriptList.clear();
	std::string scriptName;
	for (std::string::const_iterator i=packedList.begin(); i!=packedList.end(); ++i)
	{
		if (*i==':')
		{
			m_scriptList.push_back(getScriptListEntry(scriptName));
			scriptName.clear();
		}
		else
			scriptName+=*i;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when the script list synchronized variable is modified.  Updates
 * the script list on non-authoritative objects.  Updates the data in
 * the Java obj_id object for both authoritative and non-authoritative objects.
 */
void GameScriptObject::SyncScriptListCallback::modified(GameScriptObject & target, const std::string & oldValue, const std::string & newValue, bool isLocal) const
{
	if (!isLocal)
	{
		target.unpackScriptList();
		if (!ConfigServerScript::getAllowBuildClusterScriptAttach())
		{
			JavaLibrary::instance()->detachAllScriptsFromObjId(target.m_owner->getNetworkId());
			JavaLibrary::instance()->attachScriptsToObjId(target.m_owner->getNetworkId(),target.m_scriptList);
		}
	}
}

// ======================================================================

namespace Archive
{

	void get(ReadIterator &source, ScriptListEntry &target)
	{
		std::string scriptName;
		get(source, scriptName);
		target = GameScriptObject::getScriptListEntry(scriptName);
	}

// ----------------------------------------------------------------------

	void put(ByteStream &target, ScriptListEntry const &source)
	{
		if (source.isValid())
			put(target, source.getScriptName());
	}

// ----------------------------------------------------------------------

}

// ----------------------------------------------------------------------

GameScriptObject * GameScriptObject::asGameScriptObject(Object * const object)
{
	ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->getScriptObject() : nullptr;
}

// ----------------------------------------------------------------------

GameScriptObject const * GameScriptObject::asGameScriptObject(Object const * const object)
{
	ServerObject const * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

	return (serverObject != nullptr) ? serverObject->getScriptObject() : nullptr;
}

// ======================================================================
