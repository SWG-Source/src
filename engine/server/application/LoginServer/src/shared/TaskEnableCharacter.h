// ======================================================================
//
// TaskEnableCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskEnableCharacter_H
#define INCLUDED_TaskEnableCharacter_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskEnableCharacter : public DB::TaskRequest
{
  public:
	TaskEnableCharacter(StationId &stationId, const NetworkId &characterId, const std::string &whoRequested, bool enabled, uint32 clusterId);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskEnableCharacter(); // disabled default constructor
	
	class EnableCharacterQuery : public DB::Query
	{
	  public:
		EnableCharacterQuery();

		DB::BindableLong station_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableNetworkId character_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableBool enabled; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableLong result; //lint !e1925 // public data member : suppressed because this is a private inner class 


		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		EnableCharacterQuery(const EnableCharacterQuery&);
		EnableCharacterQuery &operator=(const EnableCharacterQuery&);
	};

  private:
	StationId m_stationId;
	NetworkId m_characterId;
	std::string m_whoRequested;
	bool m_enabled;
	uint32 m_clusterId;
	int m_result;
};

// ======================================================================

#endif
