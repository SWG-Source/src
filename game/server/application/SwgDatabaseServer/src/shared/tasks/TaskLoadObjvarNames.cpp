// ======================================================================
//
// TaskLoadObjvarNames.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskLoadObjvarNames.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"


volatile int TaskLoadObjvarNames::m_sObjvarNamesCount = 0;
volatile int TaskLoadObjvarNames::m_sGoldObjvarNamesCount = 0;

// ======================================================================

bool TaskLoadObjvarNames::process(DB::Session *session)
{
	int rowsFetched = 0;
	{
		ObjvarNameQuery qry(DatabaseProcess::getInstance().getSchemaQualifier());
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<ObjvarNameRow> &data = qry.getData();

			for (std::vector<ObjvarNameRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;
				
				m_objvarNames.push_back(std::make_pair(i->name_id.getValue(), i->name.getValueASCII()));
				++m_sObjvarNamesCount;
			}
		}
	}

	if (rowsFetched < 0)
		return false;
	if (ConfigServerDatabase::getEnableGoldDatabase())
	{
		ObjvarNameQuery qry(DatabaseProcess::getInstance().getGoldSchemaQualifier());
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<ObjvarNameRow> &data = qry.getData();

			for (std::vector<ObjvarNameRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;
				
				m_goldObjvarNames.push_back(std::make_pair(i->name_id.getValue(), i->name.getValueASCII()));
				++m_sGoldObjvarNamesCount;
			}
		}
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskLoadObjvarNames::onComplete()
{
	{
		for (std::vector<std::pair<int, std::string> >::const_iterator i=m_objvarNames.begin(); i!=m_objvarNames.end(); ++i)
		{
			ObjvarNameManager::getInstance().addNameMapping(i->first, i->second);
		}
		ObjvarNameManager::getInstance().allNamesLoaded();
	}
 
	if (ConfigServerDatabase::getEnableGoldDatabase())
	{
		for (std::vector<std::pair<int, std::string> >::const_iterator i=m_goldObjvarNames.begin(); i!=m_goldObjvarNames.end(); ++i)
		{
			ObjvarNameManager::getGoldInstance().addNameMapping(i->first, i->second);
		}
		ObjvarNameManager::getGoldInstance().allNamesLoaded();
	}
	
	Loader::getInstance().startupLoadCompleted(); // Currently objvar names are the only startup data loaded.
}

// ======================================================================

TaskLoadObjvarNames::ObjvarNameQuery::ObjvarNameQuery(const std::string &schema) :
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void TaskLoadObjvarNames::ObjvarNameQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+m_schema+"objvar_names.get_name_list(); end;";
}

// ----------------------------------------------------------------------

bool TaskLoadObjvarNames::ObjvarNameQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskLoadObjvarNames::ObjvarNameQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].name_id)) return false;
	if (!bindCol(m_data[0].name)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskLoadObjvarNames::ObjvarNameQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
