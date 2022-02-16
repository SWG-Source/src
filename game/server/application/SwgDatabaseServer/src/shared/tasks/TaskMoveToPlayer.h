// ======================================================================
//
// TaskMoveToPlayer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskMoveToPlayer_H
#define INCLUDED_TaskMoveToPlayer_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

//-----------------------------------------------------------------------

class TaskMoveToPlayer:public DB::TaskRequest
{
  public:
	TaskMoveToPlayer(const NetworkId &oid, const NetworkId &player, const std::string &whoRequested);
	virtual ~TaskMoveToPlayer();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class MoveToPlayerQuery : public DB::Query
	{
	  public:
		MoveToPlayerQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId   oid;
		DB::BindableNetworkId   player;
		
		//output:
		DB::BindableLong        result;

	  private:
		MoveToPlayerQuery(const MoveToPlayerQuery&);
		MoveToPlayerQuery& operator=(const MoveToPlayerQuery&);
	};

  private:
	NetworkId m_oid;
	NetworkId m_player;
	std::string m_whoRequested;
	int m_result;
	
  private:
	//Disabled:
	TaskMoveToPlayer(const TaskMoveToPlayer&);
	TaskMoveToPlayer& operator=(const TaskMoveToPlayer& rhs);
};

//-----------------------------------------------------------------------

#endif
