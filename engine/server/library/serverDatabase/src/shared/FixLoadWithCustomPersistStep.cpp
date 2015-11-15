// ======================================================================
//
// FixLoadWithCustomPersistStep.cpp
// Copyright (c) 2007 Sony Online Entertainment, LLC
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/FixLoadWithCustomPersistStep.h"

#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/Persister.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

FixLoadWithCustomPersistStep::FixLoadWithCustomPersistStep(const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth) :
		m_topmostObject(topmostObject),
		m_startingLoadWith(startingLoadWith),
		m_maxDepth(maxDepth)
{
}

// ----------------------------------------------------------------------

FixLoadWithCustomPersistStep::~FixLoadWithCustomPersistStep()
{
}

// ----------------------------------------------------------------------

bool FixLoadWithCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool FixLoadWithCustomPersistStep::afterPersist(DB::Session *session)
{
	FixLoadWithQuery qry(m_topmostObject, m_startingLoadWith, m_maxDepth);
	if (! (session->exec(&qry)))
		return false;
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void FixLoadWithCustomPersistStep::onComplete()
{
}

// ----------------------------------------------------------------------

FixLoadWithCustomPersistStep::FixLoadWithQuery::FixLoadWithQuery(const NetworkId &topmostObject, const NetworkId &startingLoadWith, int maxDepth) :
		topmost_object(topmostObject),
		starting_loadwith(startingLoadWith),
		max_depth(maxDepth)
{
}

// ----------------------------------------------------------------------
	
void FixLoadWithCustomPersistStep::FixLoadWithQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"admin.fix_load_with_depth (:topmost_object, :starting_loadwith, :max_depth); end;";
}

// ----------------------------------------------------------------------

bool FixLoadWithCustomPersistStep::FixLoadWithQuery::bindParameters()
{
	if (!bindParameter(topmost_object)) return false;
	if (!bindParameter(starting_loadwith)) return false;
	if (!bindParameter(max_depth)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool FixLoadWithCustomPersistStep::FixLoadWithQuery::bindColumns()
{
	return true;
}

// ======================================================================
