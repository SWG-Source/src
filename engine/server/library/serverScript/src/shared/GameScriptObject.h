//========================================================================
//
// GameScriptObject.h - responsible for managing/executing scripts.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_GameScriptObject_H
#define _INCLUDED_GameScriptObject_H

#ifdef WIN32
#pragma warning(disable:4503)
#endif

#include "Archive/AutoDeltaByteStream.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "Archive/AutoDeltaVector.h"
#include "StringId.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptListEntry.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"

#include <map>
#include <set>


/*************************************************************************/
// constants and typedefs
/*************************************************************************/

namespace std
{
	template <class T> class shared_ptr;
}

class CachedNetworkId;
class GameScriptObject;
class Object;
class Scheduler;
class ScriptDictionary;
class ScriptMessage;
class ScriptParams;
class ServerObject;
typedef std::shared_ptr<ScriptDictionary> ScriptDictionaryPtr;

struct ScriptData
{
	std::set<NetworkId> owners;       // who has this script attached
	std::set<std::string> functions;  // functions available from this script
};
	
typedef std::vector<ScriptListEntry> ScriptList;

typedef Unicode::String        String_t;
typedef std::vector <String_t> StringVector_t;

// values returned from script functions
enum 
{
	SCRIPT_OVERRIDE = 0,
	SCRIPT_CONTINUE = 1,
	SCRIPT_DEFAULT  = 2,
	SCRIPT_INTERNAL_ERROR = 99		// internal scripting engine error
};

const size_t MAX_SCRIPT_NAME_LEN = 512;		// including final '\0'


/*************************************************************************/
// class GameScriptObject
/*************************************************************************/

class GameScriptObject 
{
public:
	GameScriptObject(void);
	~GameScriptObject();

	static GameScriptObject * asGameScriptObject(Object * object);
	static GameScriptObject const * asGameScriptObject(Object const * object);

	void                 setOwner(ServerObject *owner);
	const ServerObject * getOwner(void) const;
	void                 setOwnerIsAuthoritative(bool authoritative, uint32 pid);
	void                 setOwnerIsLoaded(void);
	void                 setOwnerIsInitialized(void);
	void                 setOwnerDestroyed(void);
	
	void alter(real time);

	// add this script object to an object's synchronization package.
	void addToPackage(Archive::AutoDeltaByteStream &);

	// attachScript() adds a mutator script to the GameScriptObject with the given name,  Return 1 if successful. 0 otherwise.
	int attachScript(const std::string& scriptName, bool runTrigger);

	// detachScript() removes a mutator script to the GameScriptObject with the given name. Return 1 if successful, 0 otherwise.
	int detachScript(const std::string& scriptName);

	void  initScriptInstances  ();
	// removeAll() removes all mutator scripts from the obejct.
	void removeAll();

	// triggerAllScripts sends a trigger to the topmost mutator script.  It will
	// continue to send that trigger to each subsequent mutator script until one
	// such trigger function returns 0.  This function returns the value returned
	// by the last trigger.  This will tell the GameObject that invoked the trigger
	// whether to run default behavior.
	int trigAllScripts(const Scripting::TrigId trigId, ScriptParams &params);

	int trigScriptFromConsole(const Scripting::TrigId trigId, const std::string &scriptName, const StringVector_t &params) const;

	int callScriptCommandHandler(std::string const &funcName, ScriptParams &params) const;

	int callScriptBuffHandler(std::string const &funcName, ScriptParams &params) const;

	// hasScript() queries the object for the existance of a particular mutator script.
	bool hasScript(const std::string& scriptName) const;
	bool hasTrigger(const Scripting::TrigId trigId) const;
	bool hasFunction(const std::string & funcName) const;

	size_t             getScriptCount(void) const;
	const ScriptList & getScripts(void) const;
	bool               isScriptListInitialized(void) const;

	// pack and unpack for the synchronized script list
	void packScriptList();
	void unpackScriptList();
	
	// call this function to create a copy of the scriptList used for debugging.
	void enumerateScripts(std::vector<std::string> &scriptNames) const;

	static void makeScriptDictionary(const ScriptParams & params, ScriptDictionaryPtr & dictionary);
	bool        handleMessage(const std::string &messageName, const ScriptDictionaryPtr & data);
	bool        handleMessage(const std::string &messageName, const std::vector<int8> & packedData);
	
	void onWatching(ServerObject & subject);
	void onStopWatching(ServerObject & subject);

	static bool installScriptEngine(void);
	static void removeScriptEngine(void);
	static bool isScriptingEnabled(void);
	static bool reloadScript(const std::string& scriptName);
	static void pauseScripting(bool pause);
	static bool isScriptingPaused(void);
	static void enableLogging(bool enable);
	static void enableNewJediTracking(bool enableTracking);
	static int  getScriptFreeMem();

	static std::set<std::string> & getScriptFunctions(const std::string & script);
	static Scheduler & getScriptScheduler();
	static void runOneScript(
		const std::string & scriptName, 
		const std::string & methodName, 
		const std::string & argTypes, 
		ScriptParams & args);
	static std::string callScriptConsoleHandler(
		const std::string & scriptName, 
		const std::string & methodName, 
		const std::string & argTypes, 
		ScriptParams & args);

	void setScriptVar(const std::string & name, int value);
	void setScriptVar(const std::string & name, float value);
	void setScriptVar(const std::string & name, const std::string & value);

	static void callSpaceMakeOvert(const NetworkId &player);
	static void	callSpaceClearOvert(const NetworkId &ship);

	static std::string callDumpTargetInfo( NetworkId &id );
	
	static void packAllScriptVarDeltas();
	void clearScriptVars();
	void packScriptVars(std::vector<int8> & target) const;
	void unpackScriptVars(const std::vector<int8> & data) const;
	void unpackDeltaScriptVars(const std::vector<int8> & source) const;

	static ScriptListEntry getScriptListEntry(std::string const &scriptName);

	// trigOneScript() sends a trigger to a specific mutator script, thus not invoking the chain.
	// TODO RAD: see if we can make this work off a ScriptListEntry or if we need to change our logic a bit
	int trigOneScript(const std::string& scriptName, Scripting::TrigId trigId, ScriptParams &params) const;

private:
	void addAvailableFunction(const std::string & funcName);
	void removeAvailableFunction(const std::string & funcName);

private:
	struct SyncScriptListCallback
	{
		void modified (GameScriptObject & target, const std::string & oldValue, const std::string & newValue, bool isLocal) const;
	};
	friend struct SyncScriptListCallback;
	
private:

	ServerObject *                   m_owner;       // the network object that controlls me
	ScriptList                       m_scriptList;  // list of scripts to execute
	Archive::AutoDeltaVariableCallback<std::string, SyncScriptListCallback, GameScriptObject>   m_synchronizedScriptList;  // Packed script list to send on the network
	bool                             m_scriptListInitialized;   // flag that initScriptInstances() has been called
	bool                             m_scriptListValid;         // flag that the current script data is good; this is set true in initScriptInstances(), but will be cleared if the owner becomes non-authoritative
	bool                             m_lockScriptList;          // flag to disable manipulation of the script list
	int                              m_attachingScript;		// flag that we are attaching a script

	static bool m_pauseScripting;       // flag to temporarily disable scriping calls

private:

	typedef std::map<std::string, ScriptData> ScriptDataMap;
	static ScriptDataMap * ms_scriptDataMap;

private:
	GameScriptObject(const GameScriptObject &);
};

// ----------------------------------------------------------------------

inline void GameScriptObject::setOwner(ServerObject *owner)
{
	m_owner = owner;
}

inline const ServerObject *GameScriptObject::getOwner(void) const
{
	return m_owner;
}

inline size_t GameScriptObject::getScriptCount(void) const
{
	return m_scriptList.size();
}

inline const ScriptList & GameScriptObject::getScripts(void) const
{
	return m_scriptList;
}

inline std::set<std::string> & GameScriptObject::getScriptFunctions(const std::string & script)
{
	return (*ms_scriptDataMap)[script].functions;
}

inline void GameScriptObject::pauseScripting(bool pause)
{
	m_pauseScripting = pause;
}

inline bool GameScriptObject::isScriptingPaused(void)
{
	return m_pauseScripting;
}

inline bool GameScriptObject::isScriptListInitialized(void) const
{
	return m_scriptListValid;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator &source, ScriptListEntry &target);
	void put(ByteStream &target, ScriptListEntry const &source);
}

// ======================================================================

#endif	// _INCLUDED_GameScriptObject_H

