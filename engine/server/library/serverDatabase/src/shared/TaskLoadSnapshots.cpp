// ======================================================================
//
// TaskLoadSnapshots.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskLoadSnapshots.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/LoaderSnapshotGroup.h"

// ======================================================================

TaskLoadSnapshots::TaskLoadSnapshots(LoaderSnapshotGroup *snap) :
		m_snapshotGroup(snap)
{
	DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(),("Creating TaskLoadSnapshots with %i object locators attached.\n",m_snapshotGroup->getLocatorCount()));
}

// ----------------------------------------------------------------------

TaskLoadSnapshots::~TaskLoadSnapshots()
{
	delete m_snapshotGroup; // normally loader takes custody, but delete it here in case the task is cancelled
	m_snapshotGroup = 0;
}

// ----------------------------------------------------------------------

bool TaskLoadSnapshots::process(DB::Session *session)
{
	return (m_snapshotGroup->load(session));
}

// ----------------------------------------------------------------------

void TaskLoadSnapshots::onComplete()
{
	Loader::getInstance().snapshotLoaded(m_snapshotGroup);
	m_snapshotGroup = 0; // Loader takes custody
}

// ======================================================================
