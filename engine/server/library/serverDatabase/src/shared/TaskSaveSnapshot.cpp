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

// ======================================================================

TaskSaveSnapshot::TaskSaveSnapshot(Snapshot *snapshot) : m_snapshot(snapshot)
{
}

// ----------------------------------------------------------------------

bool TaskSaveSnapshot::process(DB::Session *session)
{
	if (session != nullptr) {
		return (m_snapshot->saveToDB(session));
	}

	return false;
}

// ----------------------------------------------------------------------

void TaskSaveSnapshot::onComplete()
{
	m_snapshot->saveCompleted();
	Persister::getInstance().saveCompleted(m_snapshot);
}

// ======================================================================
