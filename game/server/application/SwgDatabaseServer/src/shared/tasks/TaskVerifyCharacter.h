// ======================================================================
//
// TaskVerifyCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskVerifyCharacter_H
#define INCLUDED_TaskVerifyCharacter_H

// ======================================================================

#include <vector>

#include "Unicode.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/StationId.h"

class TransferCharacterData;

//-----------------------------------------------------------------------

class TaskVerifyCharacter:public DB::TaskRequest
{
  public:
	explicit TaskVerifyCharacter(const std::string &schema);
	virtual ~TaskVerifyCharacter();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

	void addCharacter(StationId suid, const NetworkId &characterId, const TransferCharacterData * characterData);
	
  private:
	class VerifyCharacterQuery : public DB::Query
	{
	  public:
		VerifyCharacterQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableLong        station_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableNetworkId   character_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableString<50>  gold_schema; //lint !e1925 public data member Suppresed because it's in a private inner class
		
		//output:
		DB::BindableString<128> character_name; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableBool        approved; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableString<128> scene_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableNetworkId   container_id; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableDouble      x; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableDouble      y; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableDouble      z; //lint !e1925 public data member Suppresed because it's in a private inner class
		DB::BindableLong        containment_flag;
		
	  private:
		VerifyCharacterQuery(const VerifyCharacterQuery&);
		VerifyCharacterQuery& operator=(const VerifyCharacterQuery&);
	};

  private:
	class CharacterRecord
	{
	  public:
		StationId m_suid; //lint !e1925 public data member Suppresed because it's in a private inner class
		NetworkId m_characterId; //lint !e1925 public data member Suppresed because it's in a private inner class
		
		Unicode::String m_characterName; //lint !e1925 public data member Suppresed because it's in a private inner class
		bool m_approved; //lint !e1925 public data member Suppresed because it's in a private inner class
		std::string m_sceneId; //lint !e1925 public data member Suppresed because it's in a private inner class
		NetworkId m_containerId; //lint !e1925 public data member Suppresed because it's in a private inner class
		float m_x; //lint !e1925 public data member Suppresed because it's in a private inner class
		float m_y; //lint !e1925 public data member Suppresed because it's in a private inner class
		float m_z; //lint !e1925 public data member Suppresed because it's in a private inner class
		int m_containment_flag;
		const TransferCharacterData *  m_transferCharacterData; //lint !e1925 public data member Suppresed because it's in a private inner class
	};

	std::vector<CharacterRecord*> m_characters;
	std::string m_goldSchema;
	
  private:
	//Disabled:
	TaskVerifyCharacter();
	TaskVerifyCharacter(const TaskVerifyCharacter&);
	TaskVerifyCharacter& operator=(const TaskVerifyCharacter& rhs);
};

//-----------------------------------------------------------------------

#endif
