// ======================================================================
//
// TaskSaveSnapshot.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskSaveSnapshot.h"

#include "serverDatabase/Persister.h"
#include "serverDatabase/Snapshot.h"
#include "sharedFoundation/NetworkIdArchive.h" //TODO:  Windows build breaks without this.  Why?  Makes no sense.

// ======================================================================

TaskSaveSnapshot::TaskSaveSnapshot(Snapshot *snapshot) :
		m_snapshot(snapshot)
{
}

// ----------------------------------------------------------------------

bool TaskSaveSnapshot::process(DB::Session *session)
{
	return (m_snapshot->saveToDB(session));
}

// ----------------------------------------------------------------------

void TaskSaveSnapshot::onComplete()
{
	m_snapshot->saveCompleted();
	Persister::getInstance().saveCompleted(m_snapshot);
}

// ======================================================================
