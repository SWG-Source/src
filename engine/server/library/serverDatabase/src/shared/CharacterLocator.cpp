// ======================================================================
//
// CharacterLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/CharacterLocator.h"

#include <vector>

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"

// ======================================================================

CharacterLocator::CharacterLocator(const NetworkId &characterId) :
		ObjectLocator(),
		m_characterId(characterId)
{
}

// ----------------------------------------------------------------------

CharacterLocator::~CharacterLocator()
{
	Loader::getInstance().onCharacterLocatorDeleted(m_characterId);
}

// ----------------------------------------------------------------------

bool CharacterLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);

	int startTime = Clock::timeMs();

	LocateCharacterQuery qry(m_characterId, schema);
	bool rval = session->exec(&qry);
	qry.done();

	LOG("TRACE_LOGIN",("Character locator %s -- %i objects %i ms",m_characterId.getValueString().c_str(), qry.object_count.getValue(), Clock::timeMs()-startTime));
	objectsLocated = qry.object_count.getValue();
	return rval;
}

// ======================================================================

CharacterLocator::LocateCharacterQuery::LocateCharacterQuery(const NetworkId &networkId, const std::string &schema) :
		DB::Query(),
		object_id(networkId),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void CharacterLocator::LocateCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :object_count := ") + m_schema + "loader.locate_player (:object_id); end;";
}

// ----------------------------------------------------------------------

bool CharacterLocator::LocateCharacterQuery::bindParameters()
{
	if (!bindParameter(object_count)) return false;
	if (!bindParameter(object_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool CharacterLocator::LocateCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode CharacterLocator::LocateCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
