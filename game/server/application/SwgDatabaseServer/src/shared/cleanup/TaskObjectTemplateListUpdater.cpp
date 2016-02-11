// ======================================================================
//
// TaskObjectTemplateListUpdater.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskObjectTemplateListUpdater.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================
// Update the object_templates table based on the contents of the crc string table file
// This file can be found in:
//     ../../dsrc/sku.0/sys.server/built/game/misc/object_template_crc_string_table.tab
//
bool TaskObjectTemplateListUpdater::process(DB::Session *session)
{
	if ( m_sPath.empty() )
		return true;
	FILE *in = fopen(m_sPath.c_str(),"r");
	if ( in )
	{
		LOG("TaskObjectTemplateListUpdater",("File open OK" ));

		char s_sql[4096];
		int i_lines = 0;
		while(1)
		{
			int i_id;
			char s_name[4096];
			int i_retval = 0;

			if ( i_lines++ == 0 )  // do the delete first
				strcpy(s_sql,"delete object_templates");
			else  // read a line
			{
				i_retval = fscanf(in,"0x%X\t%s\n", &i_id, s_name );   // format = ID(hex)	Name(string)
				if ( EOF == i_retval )   // commit after final one
					strcpy(s_sql,"commit");
				else if ( i_retval == 2 )  // got ID & Name
				{
					s_name[256]=0;  // nullptr term to make sure it fits
					sprintf(s_sql,"insert into object_templates values (%d,'%s')",i_id,s_name);
				}
				else
					continue;
			}
			if ( i_lines % 1000 == 0 )  // periodic log
			{
				LOG("TaskObjectTemplateListUpdater",("(%d) = %s", time(0), s_sql ));
			}
			AnyQuery qry( s_sql );
			session->exec(&qry);
			if ( EOF == i_retval )   // done
				break;
		}
		fclose(in);
	}

#ifndef WIN32
	else
	{
		char cwdbuf[4096];
		LOG("TaskObjectTemplateListUpdater",("Couldn't open %s from %s", m_sPath.c_str(), getcwd(cwdbuf,4096) ));
	}
#endif

	return true;
}

// ----------------------------------------------------------------------

void TaskObjectTemplateListUpdater::onComplete()
{
}

// ======================================================================

TaskObjectTemplateListUpdater::AnyQuery::AnyQuery( const char* pSQL ) : m_sSQL(pSQL)
{
	// Erase trailing semicolon from each line
	size_t pos = m_sSQL.rfind(';');
	if ( pos != std::string::npos )
		m_sSQL.erase( pos );
}

// ----------------------------------------------------------------------
		
void TaskObjectTemplateListUpdater::AnyQuery::getSQL(std::string &sql)
{
	sql=m_sSQL;
}

// ----------------------------------------------------------------------

bool TaskObjectTemplateListUpdater::AnyQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskObjectTemplateListUpdater::AnyQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskObjectTemplateListUpdater::AnyQuery::getExecutionMode() const
{
	return MODE_DML;
}


// ======================================================================
