#ifndef _DB_TEST_QUERY_H
#define _DB_TEST_QUERY_H

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

class DBTestQuery : public DB::Query
{
public:
	DB::BindableLong	value;

	DBTestQuery();
	virtual void getSQL(std::string &sql);
	virtual bool bindParameters();
	virtual bool bindColumns();
	virtual QueryMode getExecutionMode() const;

private:  //disable
	DBTestQuery(const DBTestQuery&);
	DBTestQuery& operator=(const DBTestQuery&);
};

#endif
