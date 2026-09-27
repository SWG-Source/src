// ======================================================================
//
// TaskGetStationIdScheme.h
//
// Reads the station id scheme recorded in the login database
// (table station_id_scheme). The scheme belongs to the data: it is the rule
// the database's station ids were derived with, so the LoginServer takes
// it from the database rather than from configuration.
//
// ======================================================================

#ifndef INCLUDED_TaskGetStationIdScheme_H
#define INCLUDED_TaskGetStationIdScheme_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

#include <string>

// ======================================================================

class TaskGetStationIdScheme : public DB::TaskRequest
{
  public:
	TaskGetStationIdScheme();

	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	class GetStationIdSchemeQuery : public DB::Query
	{
	  public:
		DB::BindableString<16> scheme; //lint !e1925 // public data member
		DB::BindableString<16> basis;  //lint !e1925 // public data member

		GetStationIdSchemeQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		GetStationIdSchemeQuery(const GetStationIdSchemeQuery&);
		GetStationIdSchemeQuery& operator=(const GetStationIdSchemeQuery&);
	};

	bool        m_queried;
	int         m_rows;
	std::string m_scheme;
	std::string m_basis;

  private: //disable
	TaskGetStationIdScheme(const TaskGetStationIdScheme&);
	TaskGetStationIdScheme& operator=(const TaskGetStationIdScheme&);
};

// ======================================================================

#endif
