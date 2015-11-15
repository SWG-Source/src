// ======================================================================
//
// TaskRenameCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskRenameCharacter_H
#define INCLUDED_TaskRenameCharacter_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

class TransferCharacterData;

// ======================================================================

class TaskRenameCharacter : public DB::TaskRequest
{
  public:
	TaskRenameCharacter(uint32 clusterId, const NetworkId &characterId, const Unicode::String &newName, const TransferCharacterData * requestData);
	~TaskRenameCharacter();
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	TaskRenameCharacter(); // disabled default constructor
	class RenameCharacterQuery : public DB::Query
	{
	  public:
		RenameCharacterQuery();

		DB::BindableLong         cluster_id; //lint !e1925 // public data member
		DB::BindableNetworkId    character_id; //lint !e1925 // public data member
		DB::BindableString<127>  new_name; //lint !e1925 // public data member

		virtual void getSQL                (std::string &sql);
		virtual bool bindParameters        ();
		virtual bool bindColumns           ();
		virtual QueryMode getExecutionMode () const;

	  private: //disable
		RenameCharacterQuery               (const RenameCharacterQuery&);
		RenameCharacterQuery &operator=    (const RenameCharacterQuery&);
	};

  private:
	uint32           m_clusterId;
	NetworkId        m_characterId;
	Unicode::String  m_newName;
	bool             m_success;
	TransferCharacterData *  m_requestData;
};

// ======================================================================

#endif
