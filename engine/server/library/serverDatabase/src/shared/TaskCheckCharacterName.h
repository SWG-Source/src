// ======================================================================
//
// TaskCheckCharacterName.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskCheckCharacterName_H
#define INCLUDED_TaskCheckCharacterName_H

// ======================================================================

#include <string>

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskCheckCharacterName : public DB::TaskRequest
{
  public:
		TaskCheckCharacterName(uint32 stationId, const Unicode::String &normalizedName);
	bool process(DB::Session *session);
	void onComplete();
		
  private:
	Unicode::String m_name;
	int m_resultCode;
	uint32 m_stationId;

	class CheckCharacterNameQuery : public DB::Query
	{
	  public:
		CheckCharacterNameQuery(const std::string &name);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		
	  protected:
		virtual QueryMode getExecutionMode() const;

	  public:
		DB::BindableString<127> character_name;
		DB::BindableLong result;
				
	  private:   
		CheckCharacterNameQuery(const CheckCharacterNameQuery&);
		CheckCharacterNameQuery& operator=(const CheckCharacterNameQuery&);
	};
};

// ======================================================================

#endif
