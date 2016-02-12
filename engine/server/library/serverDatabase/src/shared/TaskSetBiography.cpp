// ======================================================================
//
// TaskSetBiography.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskSetBiography.h"

#include "serverDatabase/SetBiographyQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskSetBiography::TaskSetBiography(const NetworkId &owner, const Unicode::String &bio) :
		m_owner(owner),
		m_bio(new Unicode::String(bio))
{
}

// ----------------------------------------------------------------------

TaskSetBiography::~TaskSetBiography()
{
	delete m_bio;
	m_bio=nullptr;
}

// ----------------------------------------------------------------------

bool TaskSetBiography::process(DB::Session *session)
{
	NOT_NULL(m_bio);
	DBQuery::SetBiographyQuery qry(m_owner, *m_bio);
	bool rval = session->exec(&qry);
	qry.done();
	return rval;
}

// ----------------------------------------------------------------------

void TaskSetBiography::onComplete()
{
}

// ======================================================================
