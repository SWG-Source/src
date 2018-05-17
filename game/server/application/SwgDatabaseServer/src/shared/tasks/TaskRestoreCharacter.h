// ======================================================================
//
// TaskRestoreCharacter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskRestoreCharacter_H
#define INCLUDED_TaskRestoreCharacter_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/StationId.h"

//-----------------------------------------------------------------------

class TaskRestoreCharacter:public DB::TaskRequest
{
  public:
	TaskRestoreCharacter(const NetworkId &characterId, const std::string &whoRequested);
	virtual ~TaskRestoreCharacter();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class RestoreCharacterQuery : public DB::Query
	{
	  public:
		RestoreCharacterQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId   character_id;
		
		//output:
		DB::BindableLong        result;
		DB::BindableString<127> character_name;
		DB::BindableLong        account;
		DB::BindableLong        template_id;
		
	  private:
		RestoreCharacterQuery(const RestoreCharacterQuery&);
		RestoreCharacterQuery& operator=(const RestoreCharacterQuery&);
	};

  private:
	NetworkId       m_characterId;
	std::string     m_whoRequested;
	int             m_result;
	Unicode::String m_characterName;
	StationId       m_account;
	int             m_templateId;
	
  private:
	//Disabled:
	TaskRestoreCharacter(const TaskRestoreCharacter&);
	TaskRestoreCharacter& operator=(const TaskRestoreCharacter& rhs);
};

//-----------------------------------------------------------------------

#endif
