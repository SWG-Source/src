// ======================================================================
//
// CreateCharacterCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/CreateCharacterCustomPersistStep.h"

#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/Persister.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

CreateCharacterCustomPersistStep::CreateCharacterCustomPersistStep(uint32 stationId, const NetworkId &characterObject, const Unicode::String &characterName, const std::string &normalizedName, int templateId, bool special) :
		m_stationId(stationId),
		m_characterObject(characterObject),
		m_characterName(characterName),
		m_normalizedName(normalizedName),
		m_templateId(templateId),
		m_clusterName(DatabaseProcess::getInstance().getClusterName()),
		m_special(special)
{
}

// ----------------------------------------------------------------------

CreateCharacterCustomPersistStep::~CreateCharacterCustomPersistStep()
{
}

// ----------------------------------------------------------------------

bool CreateCharacterCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool CreateCharacterCustomPersistStep::afterPersist(DB::Session *session)
{
	DBQuery::AddCharacter qry(m_stationId,m_characterObject, m_characterName, m_normalizedName);

	if (! (session->exec(&qry))) {
		std::string characterName(Unicode::wideToNarrow(m_characterName));
		WARNING(true, ("CreateCharacterCustomPersistStep: Failed saving character and character object for %s", characterName.c_str()));
		return false;
	}

	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void CreateCharacterCustomPersistStep::onComplete()
{
	Persister::getInstance().onNewCharacterSaved(m_stationId,m_characterObject,m_characterName,m_templateId,m_special);
	DataLookup::getInstance().releaseName(m_stationId, NetworkId::cms_invalid);
	LOG("TraceCharacterCreation", ("%d CreateCharacterCustomPersistStep(%s) complete", m_stationId, Unicode::wideToNarrow(m_characterName).c_str()));
}

// ======================================================================
