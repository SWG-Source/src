// ======================================================================
//
// TaskUpdateSPCharacterProfileData.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskUpdateSPCharacterProfileData_H
#define INCLUDED_TaskUpdateSPCharacterProfileData_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskUpdateSPCharacterProfileData : public DB::TaskRequest
{
  public:
	TaskUpdateSPCharacterProfileData(const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskUpdateSPCharacterProfileData(); // disabled default constructor
	class UpdateSPCharacterProfileDataQuery : public DB::Query
	{
	  public:
		UpdateSPCharacterProfileDataQuery();
	
		DB::BindableString<50>          clusterName;
		DB::BindableNetworkId           characterId;
		DB::BindableString<127>         characterName;
		DB::BindableString<127>         objectName;
		DB::BindableDouble              x;
		DB::BindableDouble              y;
		DB::BindableDouble              z;
		DB::BindableString<50>          sceneId;
		DB::BindableDouble              cash_balance;
		DB::BindableDouble              bank_balance;
		DB::BindableString<500>         objectTemplateName;
		DB::BindableLong                stationId;
		DB::BindableNetworkId           containedBy;
		DB::BindableLong                createTime;
		DB::BindableLong                playedTime;
		DB::BindableLong                numLots;

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		UpdateSPCharacterProfileDataQuery               (const UpdateSPCharacterProfileDataQuery&);
		UpdateSPCharacterProfileDataQuery &operator=    (const UpdateSPCharacterProfileDataQuery&);
	};

  private:
	std::string      m_clusterName;
	NetworkId        m_characterId;
	std::string      m_characterName;
	std::string      m_objectName;
	float            m_x;
	float            m_y;
	float            m_z;
	std::string      m_sceneId;
	float            m_cash_balance;
	float            m_bank_balance;
	std::string      m_objectTemplateName;
	int              m_stationId;
	NetworkId        m_containedby;
	int              m_createTime;
	int              m_playedTime;
	int              m_numLots;
};

// ======================================================================

#endif
