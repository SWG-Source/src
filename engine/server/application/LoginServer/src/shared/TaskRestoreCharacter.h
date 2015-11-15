// ======================================================================
//
// TaskRestoreCharacter.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskRestoreCharacter_H
#define INCLUDED_TaskRestoreCharacter_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskRestoreCharacter : public DB::TaskRequest
{
  public:
	TaskRestoreCharacter(uint32 clusterId, const std::string &whoRequested, StationId stationId, const Unicode::String &characterName, const NetworkId &characterId, int templateId, bool jedi);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskRestoreCharacter(); // disabled default constructor
	class RestoreCharacterQuery : public DB::Query
	{
	  public:
		RestoreCharacterQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableLong         station_id; //lint !e1925 // public data member
		DB::BindableString<127>  character_name; //lint !e1925 // public data member
		DB::BindableNetworkId    character_id; //lint !e1925 // public data member
		DB::BindableLong         template_id; //lint !e1925 // public data member
		DB::BindableLong         character_type; //lint !e1925 // public data member
		DB::BindableLong         result; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		RestoreCharacterQuery               (const RestoreCharacterQuery&);
		RestoreCharacterQuery &operator=    (const RestoreCharacterQuery&);
	};

  private:
	uint32           m_clusterId;
	std::string      m_whoRequested;
	StationId        m_stationId;
	Unicode::String  m_characterName;
	NetworkId        m_characterId;
	int              m_templateId;
	bool             m_jedi;
	int              m_result;
};

// ======================================================================

#endif
