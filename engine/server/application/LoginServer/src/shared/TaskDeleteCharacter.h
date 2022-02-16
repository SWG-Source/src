// ======================================================================
//
// TaskDeleteCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskDeleteCharacter_H
#define INCLUDED_TaskDeleteCharacter_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskDeleteCharacter : public DB::TaskRequest
{
  public:
	TaskDeleteCharacter(uint32 clusterId, const NetworkId &characterId, StationId stationId);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskDeleteCharacter(); // disabled default constructor
	
	class DeleteCharacterQuery : public DB::Query
	{
	  public:
		DeleteCharacterQuery();

		DB::BindableLong cluster_id; //lint !e1925 // public data member
		DB::BindableNetworkId character_id; //lint !e1925 // public data member
		DB::BindableLong station_id; //lint !e1925 // public data member

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private: //disable
		DeleteCharacterQuery(const DeleteCharacterQuery&);
		DeleteCharacterQuery &operator=(const DeleteCharacterQuery&);
	};

  private:
	uint32 m_clusterId;
	NetworkId m_characterId;
	StationId m_stationId;
};

// ======================================================================

#endif
