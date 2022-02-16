// ======================================================================
//
// TaskGetAvatarList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetAvatarList_H
#define INCLUDED_TaskGetAvatarList_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TransferAccountData;
class TransferCharacterData;

class TaskGetAvatarList : public DB::TaskRequest
{
  public:
	explicit TaskGetAvatarList  (StationId stationId, int clusterGroupId, const TransferCharacterData * transferCharacterData);
	explicit TaskGetAvatarList  (int clusterGroupId, const TransferAccountData * transferAccountData);
	virtual ~TaskGetAvatarList  ();
	
  public:
	virtual bool process     (DB::Session *session);
	virtual void onComplete  ();
	AvatarList const & getAvatars() const;
	StationId getStationId() const;

	class GetCharactersQuery : public DB::Query
	{
	public:
		DB::BindableLong station_id; //lint !e1925 // public data member
		DB::BindableLong cluster_group_id; //lint !e1925 // public data member
		DB::BindableString<128> character_name; //lint !e1925 // public data member
		DB::BindableLong object_template_id; //lint !e1925 // public data member
		DB::BindableNetworkId object_id; //lint !e1925 // public data member
		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableLong character_type; //lint !e1925 // public data member

		GetCharactersQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:  //disable
		GetCharactersQuery(const GetCharactersQuery&);
		GetCharactersQuery& operator=(const GetCharactersQuery&);
	};

  private:
	TaskGetAvatarList(); // disabled default constructor
	TaskGetAvatarList(const TaskGetAvatarList &);
	TaskGetAvatarList &  operator=  (const TaskGetAvatarList &);
	StationId                m_stationId;
	int                      m_stationIdNumberJediSlot;
	int                      m_clusterGroupId;
	AvatarList               m_avatars;
	TransferCharacterData *  m_transferCharacterData;
	TransferAccountData *    m_transferAccountData;
};

// ======================================================================

#endif
