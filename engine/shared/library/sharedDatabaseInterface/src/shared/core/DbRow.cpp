// ======================================================================
//
// DBRow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbRow.h"

volatile int DB::Row::m_sRowsCreated = 0;
volatile int DB::Row::m_sRowsDeleted = 0;


// ----------------------------------------------------------------------

DB::Row::Row()
{
	++m_sRowsCreated;
}

// ----------------------------------------------------------------------
DB::Row::Row( const Row& )
{
	++m_sRowsCreated;
}

// ----------------------------------------------------------------------

DB::Row::~Row()
{
	++m_sRowsDeleted;
}

// ----------------------------------------------------------------------

void DB::Row::copy(const Row &rhs)
{
	UNREF(rhs);
	FATAL(true,("Row::copy() invoked.  This error probably means that copy() was not implemented for a class derived from Row."));
}
// ======================================================================

