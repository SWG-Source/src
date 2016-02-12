// ======================================================================
//
// TaskSaveObjvarNames.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskSaveObjvarNames.h"

#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include <vector>

// ======================================================================

TaskSaveObjvarNames::TaskSaveObjvarNames(const NameList &names) :
		m_objvarNames(new NameList(names))
{
}

// ----------------------------------------------------------------------

TaskSaveObjvarNames::~TaskSaveObjvarNames()
{
	delete m_objvarNames;
	m_objvarNames=nullptr;
}

// ----------------------------------------------------------------------

bool TaskSaveObjvarNames::process(DB::Session *session)
{
	ObjvarNameQuery qry;
	for (NameList::const_iterator i=m_objvarNames->begin(); i!=m_objvarNames->end(); ++i)
	{
		qry.name_id = i->first;
		qry.name = i->second;

		if (! (session->exec(&qry)))
			return false;
	}
	return true;
}

// ----------------------------------------------------------------------

void TaskSaveObjvarNames::onComplete()
{
}

// ======================================================================

TaskSaveObjvarNames::ObjvarNameQuery::ObjvarNameQuery()
{
}

// ----------------------------------------------------------------------

void TaskSaveObjvarNames::ObjvarNameQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"objvar_names.add_name(:id, :name); end;";	
}

// ----------------------------------------------------------------------

bool TaskSaveObjvarNames::ObjvarNameQuery::bindParameters()
{
	if (!bindParameter(name_id)) return false;
	if (!bindParameter(name)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskSaveObjvarNames::ObjvarNameQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskSaveObjvarNames::ObjvarNameQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
