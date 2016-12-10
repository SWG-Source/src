// ======================================================================
//
// TaskSaveObjvarNames.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskSaveObjvarNames_H
#define INCLUDED_TaskSaveObjvarNames_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/Bindable.h"

// ======================================================================

class TaskSaveObjvarNames : public DB::TaskRequest
{
  public:
	typedef std::vector<std::pair<int, std::string> > NameList;
	
  public:
	TaskSaveObjvarNames     (const NameList &names);
	~TaskSaveObjvarNames    ();
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:
	class ObjvarNameQuery : public DB::Query
	{
	  public:
		ObjvarNameQuery();
		
		virtual void       getSQL           (std::string &sql);
		virtual bool       bindParameters   ();
		virtual bool       bindColumns      ();
		virtual QueryMode  getExecutionMode () const;

	  public:
		DB::BindableLong         name_id;
		DB::BindableString<500>  name;

	  private:
		ObjvarNameQuery(const ObjvarNameQuery&);
		ObjvarNameQuery& operator=(const ObjvarNameQuery&);
	};

  private:
	NameList *m_objvarNames;
};

// ======================================================================

#endif
