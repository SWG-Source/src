// ======================================================================
//
// TaskLocateStructure.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskLocateStructure_H
#define INCLUDED_TaskLocateStructure_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

//-----------------------------------------------------------------------

class TaskLocateStructure:public DB::TaskRequest
{
  public:
	TaskLocateStructure(const NetworkId &itemId, const std::string &whoRequested);
	virtual ~TaskLocateStructure();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class LocateStructureQuery : public DB::Query
	{
	  public:
		LocateStructureQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId   item_id;

		//output:
		DB::BindableLong	x;
		DB::BindableLong	z;
		DB::BindableString<50>	sceneId;
		DB::BindableLong	found;

	  private:
		LocateStructureQuery(const LocateStructureQuery&);
		LocateStructureQuery& operator=(const LocateStructureQuery&);
	};

  private:
	NetworkId   m_itemId;
	int	    m_x;
	int	    m_z;
	std::string m_sceneId;
	int	    m_found;
	std::string m_whoRequested;

  private:
	//Disabled:
	TaskLocateStructure(const TaskLocateStructure&);
	TaskLocateStructure& operator=(const TaskLocateStructure& rhs);
};

//-----------------------------------------------------------------------

#endif
