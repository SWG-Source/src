// ======================================================================
//
// LazyDeleteQuery.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LazyDeleteQuery_H
#define INCLUDED_LazyDeleteQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class LazyDeleteQuery : public DB::Query
{
  public:
	LazyDeleteQuery();
	
	virtual void getSQL(std::string &sql);
	virtual bool bindParameters();
	virtual bool bindColumns();

	bool setupData(DB::Session *session);
	bool addData(const NetworkId &object);
	void clearData();
	void freeData();

 	int getNumItems() const;

  private:
	DB::BindableVarrayString	m_object_ids;
	DB::BindableLong			m_numItems;
	DB::BindableLong			m_enableDatabaseLogging;

  private:
	LazyDeleteQuery(const LazyDeleteQuery&); //disable
	LazyDeleteQuery& operator=(const LazyDeleteQuery&); //disable
};

// ======================================================================

#endif
