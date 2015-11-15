// ======================================================================
//
// DBModeQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbModeQuery.h"

// ======================================================================

DB::ModeQuery::~ModeQuery()
{
	delete data;
}

// ======================================================================

