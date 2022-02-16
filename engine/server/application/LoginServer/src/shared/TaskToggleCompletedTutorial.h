// ======================================================================
//
// TaskToggleCompletedTutorial.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskToggleCompletedTutorial_H
#define INCLUDED_TaskToggleCompletedTutorial_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskToggleCompletedTutorial : public DB::TaskRequest
{
  public:
	TaskToggleCompletedTutorial(StationId stationId, bool completed);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskToggleCompletedTutorial(); // disabled default constructor
	
	class ToggleCompletedTutorialQuery : public DB::Query
	{
	  public:
		ToggleCompletedTutorialQuery();

		DB::BindableLong station_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableBool completed_flag; //lint !e1925 // public data member : suppressed because this is a private inner class 

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		ToggleCompletedTutorialQuery(const ToggleCompletedTutorialQuery&);
		ToggleCompletedTutorialQuery &operator=(const ToggleCompletedTutorialQuery&);
	};

  private:
	StationId m_stationId;
	bool m_completed;
};

// ======================================================================

#endif
