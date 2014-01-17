// ======================================================================
//
// UnloadCharacterCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/UnloadCharacterCustomPersistStep.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/Persister.h"
#include "serverNetworkMessages/PersistedPlayerMessage.h"
#include "sharedLog/Log.h"

// ======================================================================

UnloadCharacterCustomPersistStep::UnloadCharacterCustomPersistStep(const NetworkId &characterObject, uint32 serverId) :
		m_characterObject(characterObject),
		m_serverId(serverId)
{
	Loader::getInstance().addLoadLock(m_characterObject);
}

// ----------------------------------------------------------------------

UnloadCharacterCustomPersistStep::~UnloadCharacterCustomPersistStep()
{
}

// ----------------------------------------------------------------------

bool UnloadCharacterCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool UnloadCharacterCustomPersistStep::afterPersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

void UnloadCharacterCustomPersistStep::onComplete()
{
	Loader::getInstance().removeLoadLock(m_characterObject);

	bool hasDataForObject = Persister::getInstance().hasDataForObject(m_characterObject);

	if (hasDataForObject)
		LOG("UnloadCharacterCustomPersistStep::onComplete",("Unloaded character %s still has unsaved data",m_characterObject.getValueString().c_str()));

	if ((hasDataForObject) && (ConfigServerDatabase::getDelayUnloadIfObjectStillHasData()))
	{
		// the object still has unsaved data, so wait until
		// the next save cycle to save/unload the object;
		// otherwise, if we unload the object now, we could
		// lose the unsaved data if the player logs in before
		// the data has been saved
		Persister::getInstance().unloadCharacter(m_characterObject, m_serverId);
	}
	else
	{
		PersistedPlayerMessage const persistedPlayerMessage(m_characterObject);
		DatabaseProcess::getInstance().sendToGameServer(m_serverId, persistedPlayerMessage);
	}
}

// ======================================================================
