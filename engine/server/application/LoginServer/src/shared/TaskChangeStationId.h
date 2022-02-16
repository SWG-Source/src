// ======================================================================
//
// TaskChangeStationId.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskChangeStationId_H
#define INCLUDED_TaskChangeStationId_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/StationId.h"

class TransferAccountData;

// ======================================================================

class TaskChangeStationId : public DB::TaskRequest
{
  public:
	TaskChangeStationId(StationId sourceStationId, StationId destinationStationId, const TransferAccountData * requestData);
	~TaskChangeStationId();
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskChangeStationId(); // disabled default constructor
	class ChangeStationIdQuery : public DB::Query
	{
	  public:
		ChangeStationIdQuery();

		DB::BindableLong         destination_station_id; //lint !e1925 // public data member
		DB::BindableLong         source_station_id; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		ChangeStationIdQuery               (const ChangeStationIdQuery&);
		ChangeStationIdQuery &operator=    (const ChangeStationIdQuery&);
	};

  private:
	StationId             m_sourceStationId;
	StationId             m_destinationStationId;
	TransferAccountData   *m_requestData;
	bool                  m_success;
};

// ======================================================================

#endif
