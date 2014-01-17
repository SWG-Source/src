// ======================================================================
//
// TaskRestoreHouse.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskRestoreHouse_H
#define INCLUDED_TaskRestoreHouse_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

//-----------------------------------------------------------------------

class TaskRestoreHouse:public DB::TaskRequest
{
  public:
	TaskRestoreHouse(const NetworkId &houseId, const std::string &whoRequested);
	virtual ~TaskRestoreHouse();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class RestoreHouseQuery : public DB::Query
	{
	  public:
		RestoreHouseQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId   house_id;
		
		//output:
		DB::BindableLong        result;
		
	  private:
		RestoreHouseQuery(const RestoreHouseQuery&);
		RestoreHouseQuery& operator=(const RestoreHouseQuery&);
	};

  private:
	NetworkId m_houseId;
	std::string m_whoRequested;
	int       m_result;
	
  private:
	//Disabled:
	TaskRestoreHouse(const TaskRestoreHouse&);
	TaskRestoreHouse& operator=(const TaskRestoreHouse& rhs);
};

//-----------------------------------------------------------------------

#endif
