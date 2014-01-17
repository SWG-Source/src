// ======================================================================
//
// TaskAnySQL.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskAnySQL.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskAnySQL::process(DB::Session *session)
{
	LOG("TaskANYSQL",("%s", m_sSQL.c_str()   ));
	AnyQuery qry( m_sSQL.c_str() );
	return (session->exec(&qry));
}

// ----------------------------------------------------------------------

void TaskAnySQL::onComplete()
{
}

// ======================================================================

TaskAnySQL::AnyQuery::AnyQuery( const char* pSQL ) : m_sSQL(pSQL)
{
	// Erase trailing semicolon from each line
	size_t pos = m_sSQL.rfind(';');
	if ( pos != std::string::npos )
		m_sSQL.erase( pos );
}

// ----------------------------------------------------------------------
		
void TaskAnySQL::AnyQuery::getSQL(std::string &sql)
{
	sql=m_sSQL;
}

// ----------------------------------------------------------------------

bool TaskAnySQL::AnyQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskAnySQL::AnyQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskAnySQL::AnyQuery::getExecutionMode() const
{
	return MODE_DML;
}


// ======================================================================
