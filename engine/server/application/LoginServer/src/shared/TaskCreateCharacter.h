// ======================================================================
//
// TaskCreateCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskCreateCharacter_H
#define INCLUDED_TaskCreateCharacter_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskCreateCharacter : public DB::TaskRequest
{
  public:
	TaskCreateCharacter(uint32 clusterId, StationId stationId, const Unicode::String &characterName, const NetworkId &characterId, int templateId, bool jedi);
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskCreateCharacter(); // disabled default constructor
	class CreateCharacterQuery : public DB::Query
	{
	  public:
		CreateCharacterQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableLong         station_id; //lint !e1925 // public data member
		DB::BindableString<127>  character_name; //lint !e1925 // public data member
		DB::BindableNetworkId    character_id; //lint !e1925 // public data member
		DB::BindableLong         template_id; //lint !e1925 // public data member
		DB::BindableLong         character_type; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		CreateCharacterQuery               (const CreateCharacterQuery&);
		CreateCharacterQuery &operator=    (const CreateCharacterQuery&);
	};

  private:
	uint32           m_clusterId;
	StationId        m_stationId;
	Unicode::String  m_characterName;
	NetworkId        m_characterId;
	int              m_templateId;
	bool             m_jedi;
};

// ======================================================================

#endif
