// ======================================================================
//
// TaskRegisterNewCluster.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskRegisterNewCluster_H
#define INCLUDED_TaskRegisterNewCluster_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskRegisterNewCluster : public DB::TaskRequest
{
  public:
	explicit TaskRegisterNewCluster (const std::string &newClusterName, const std::string &address);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskRegisterNewCluster(); // disabled default constructor
	class RegisterClusterQuery : public DB::Query
	{
	  public:
		RegisterClusterQuery();

		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableString<255> cluster_name; //lint !e1925 // public data member
		DB::BindableString<255> address; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		RegisterClusterQuery(const RegisterClusterQuery&);
		RegisterClusterQuery &operator=(const RegisterClusterQuery&);
	};

  private:
	uint32 m_clusterId;
	std::string m_clusterName;
	std::string m_address;
};

// ======================================================================

#endif
