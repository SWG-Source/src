// ======================================================================
//
// TaskLoadObjvarNames.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskLoadObjvarNames_H
#define INCLUDED_TaskLoadObjvarNames_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include <vector>

// ======================================================================

class TaskLoadObjvarNames : public DB::TaskRequest
{
  public:
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

	static int			getObjvarNamesCount() { return m_sObjvarNamesCount; }
	static int			getGoldObjvarNamesCount() { return m_sGoldObjvarNamesCount; }

  private:
	struct ObjvarNameRow
	{
		DB::BindableLong name_id;
		DB::BindableString<500> name;
	};

	class ObjvarNameQuery : public DB::Query
	{
	  public:
		ObjvarNameQuery(const std::string &schema);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		const std::vector<ObjvarNameRow> & getData() const;
		
	  private:
		
		std::vector<ObjvarNameRow> m_data;
		std::string m_schema;
		
	  private:
		ObjvarNameQuery(const ObjvarNameQuery&);
		ObjvarNameQuery& operator=(const ObjvarNameQuery&);
	};
	
  private:
	std::vector<std::pair<int, std::string> > m_objvarNames;
	std::vector<std::pair<int, std::string> > m_goldObjvarNames;


	static volatile int m_sObjvarNamesCount;
	static volatile int m_sGoldObjvarNamesCount;

};

// ======================================================================

inline const std::vector<TaskLoadObjvarNames::ObjvarNameRow> & TaskLoadObjvarNames::ObjvarNameQuery::getData() const
{
	return m_data;
}

// ======================================================================

#endif
