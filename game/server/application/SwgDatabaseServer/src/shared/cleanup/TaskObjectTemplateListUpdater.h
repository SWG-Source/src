// ======================================================================
//
// TaskObjectTemplateListUpdater.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskObjectTemplateListUpdater_H
#define INCLUDED_TaskObjectTemplateListUpdater_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedDatabaseInterface/Bindable.h"
#include <time.h>

// ======================================================================

/**
 * Runs scripts to do regular daily cleanup in the database.
 */
class TaskObjectTemplateListUpdater : public DB::TaskRequest
{
  public:
			    TaskObjectTemplateListUpdater( const char* sPath ) { m_sPath = sPath; }

	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	std::string         m_sPath;



        class AnyQuery : public DB::Query
        {
          public:
                AnyQuery( const char* pSQL );

                virtual void getSQL(std::string &sql);
                virtual bool bindParameters();
                virtual bool bindColumns();
                virtual QueryMode getExecutionMode() const;

          private:
                AnyQuery(const AnyQuery&);
                AnyQuery& operator=(const AnyQuery&);


                std::string m_sSQL;
        };
};

// ======================================================================

#endif
