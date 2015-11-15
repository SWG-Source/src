#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbTestQuery.h"

void DBTestQuery::getSQL(std::string &sql)
{
	sql=std::string("select 1 from dual");
}

bool DBTestQuery::bindParameters()
{
	return true;
}

bool DBTestQuery::bindColumns()
{

        if (!bindCol(value)) return false;

        return true;
}

DB::Query::QueryMode DBTestQuery::getExecutionMode() const
{
	return MODE_SQL;
}

// ----------------------------------------------------------------------

DBTestQuery::DBTestQuery() :
	Query(),
	value()
{
}

