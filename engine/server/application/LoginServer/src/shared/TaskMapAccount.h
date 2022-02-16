// ======================================================================
//
// TaskMapAccount.h
// copyright (c) 2016 StellaBellum
//
// ======================================================================

#ifndef INCLUDED_TaskMapAccount_H
#define INCLUDED_TaskMapAccount_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskMapAccount : public DB::TaskRequest
{
  public:
    TaskMapAccount(StationId parentID, StationId childID);

  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskMapAccount(); // disabled default constructor

	class MapAccountQuery : public DB::Query
	{
	  public:
        MapAccountQuery();

		DB::BindableLong parentID; //lint !e1925 // public data member
		DB::BindableLong childID; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
        MapAccountQuery(const MapAccountQuery&);
        MapAccountQuery &operator=(const MapAccountQuery&);
	};

  private:
    StationId m_parentID;
    StationId m_childID;
};

// ======================================================================

#endif
