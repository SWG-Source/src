// ======================================================================
//
// SwgLoaderSnapshotGroup.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgLoaderSnapshotGroup.h"

#include "SwgDatabaseServer/SwgSnapshot.h"

// ======================================================================

SwgLoaderSnapshotGroup::SwgLoaderSnapshotGroup(uint32 requestingProcess) :
		LoaderSnapshotGroup(requestingProcess, new SwgSnapshot(DB::ModeQuery::mode_SELECT, false))
{
}

// ----------------------------------------------------------------------

SwgLoaderSnapshotGroup::~SwgLoaderSnapshotGroup()
{
}

// ----------------------------------------------------------------------

Snapshot *SwgLoaderSnapshotGroup::makeGoldSnapshot()
{
	return new SwgSnapshot(DB::ModeQuery::mode_SELECT, true);
}

// ======================================================================
