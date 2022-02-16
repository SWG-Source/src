// ======================================================================
//
// TaskToggleCharacterDisable.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskToggleCharacterDisable_H
#define INCLUDED_TaskToggleCharacterDisable_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskToggleCharacterDisable : public DB::TaskRequest
{
  public:
	TaskToggleCharacterDisable(uint32 clusterId, const NetworkId &characterId, StationId stationId, bool enabled);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskToggleCharacterDisable(); // disabled default constructor
	
	class ToggleCharacterDisableQuery : public DB::Query
	{
	  public:
		ToggleCharacterDisableQuery();

		DB::BindableLong cluster_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableNetworkId character_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableLong station_id; //lint !e1925 // public data member : suppressed because this is a private inner class
		DB::BindableBool enabled_flag; //lint !e1925 // public data member : suppressed because this is a private inner class 


		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		ToggleCharacterDisableQuery(const ToggleCharacterDisableQuery&);
		ToggleCharacterDisableQuery &operator=(const ToggleCharacterDisableQuery&);
	};

  private:
	uint32 m_clusterId;
	NetworkId m_characterId;
	StationId m_stationId;
	bool m_enabled;
};

// ======================================================================

#endif
