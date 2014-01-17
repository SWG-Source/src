// ======================================================================
//
// MoveToPlayerCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/MoveToPlayerCustomPersistStep.h"

#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/Persister.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

MoveToPlayerCustomPersistStep::MoveToPlayerCustomPersistStep(const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad) :
		m_objectId(objectId),
		m_targetPlayer(targetPlayer),
		m_maxDepth(maxDepth),
		m_useBank(useBank),
		m_useDatapad(useDatapad)
{
}

// ----------------------------------------------------------------------

MoveToPlayerCustomPersistStep::~MoveToPlayerCustomPersistStep()
{
}

// ----------------------------------------------------------------------

bool MoveToPlayerCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool MoveToPlayerCustomPersistStep::afterPersist(DB::Session *session)
{
	MoveToPlayerQuery qry(m_objectId, m_targetPlayer, m_maxDepth, m_useBank, m_useDatapad);
	if (! (session->exec(&qry)))
		return false;
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void MoveToPlayerCustomPersistStep::onComplete()
{
}

// ----------------------------------------------------------------------

MoveToPlayerCustomPersistStep::MoveToPlayerQuery::MoveToPlayerQuery(const NetworkId &objectId, const NetworkId &targetPlayer, int maxDepth, bool useBank, bool useDatapad) :
		object_id(objectId),
		target_player(targetPlayer),
		max_depth(maxDepth),
		m_useBank(useBank),
		m_useDatapad(useDatapad)
{
}

// ----------------------------------------------------------------------
	
void MoveToPlayerCustomPersistStep::MoveToPlayerQuery::getSQL(std::string &sql)
{
	if (m_useBank)
		sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"admin.move_to_player_bank (:object_id, :target_player); end;";
	else if (m_useDatapad)
		sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"admin.move_to_player_datapad (:object_id, :target_player, :max_depth); end;";
	else
		sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"admin.move_to_player (:object_id, :target_player); end;";
}

// ----------------------------------------------------------------------

bool MoveToPlayerCustomPersistStep::MoveToPlayerQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	if (!bindParameter(target_player)) return false;
	if (m_useDatapad && !bindParameter(max_depth)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool MoveToPlayerCustomPersistStep::MoveToPlayerQuery::bindColumns()
{
	return true;
}

// ======================================================================
