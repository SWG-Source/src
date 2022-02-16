// ======================================================================
//
// TaskGetClusterList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetClusterList_H
#define INCLUDED_TaskGetClusterList_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskGetClusterList : public DB::TaskRequest
{
  public:
	explicit TaskGetClusterList (int groupId);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	class GetClustersQuery : public DB::Query
	{
	  public:
		DB::BindableLong group_id; //lint !e1925 // public data member
				
		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableString<255> cluster_name; //lint !e1925 // public data member
		DB::BindableString<255> address; //lint !e1925 // public data member
		DB::BindableLong port; //lint !e1925 // public data member
		DB::BindableBool secret; //lint !e1925 // public data member
		DB::BindableBool locked; //lint !e1925 // public data member
		DB::BindableBool not_recommended; //lint !e1925 // public data member
		DB::BindableLong maxCharacterPerAccount; //lint !e1925 // public data member
		DB::BindableLong online_player_limit; //lint !e1925 // public data member
		DB::BindableLong online_free_trial_limit; //lint !e1925 // public data member
		DB::BindableBool free_trial_can_create_char; //lint !e1925 // public data member
		DB::BindableLong online_tutorial_limit; //lint !e1925 // public data member

		GetClustersQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		GetClustersQuery(const GetClustersQuery&);
		GetClustersQuery& operator=(const GetClustersQuery&);
	};

	struct ClusterData
	{
		uint32 m_clusterId;
		std::string m_clusterName;
		std::string m_address;
		uint16 m_port;
		bool m_secret;
		bool m_locked;
		bool m_notRecommended;
		int m_maxCharactersPerAccount;
		int m_onlinePlayerLimit;
		int m_onlineFreeTrialLimit;
		bool m_freeTrialCanCreateChar;
		int m_onlineTutorialLimit;
	};

	std::vector<ClusterData> m_clusterData;
	int m_groupId;

  private:
	TaskGetClusterList (); // disable
};

// ======================================================================

#endif
