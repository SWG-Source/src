/* DBQuery.h
 *
 * Definition of the base query class.  Override this class to make
 * queries for specific purposes.
 *
 * ODBC version
 */

#ifndef _DBQUERY_H
#define _DBQUERY_H

//#include "sharedDatabaseInterface/SQLC_Defs.h"
#include "sharedDatabaseInterface/DbQueryImplementation.h" //TODO:  Do we want to include this?
#include "sharedDatabaseInterface/DbProtocol.def"

namespace DB {

    class Session;
	class QueryImpl;
	class BindableLong;
	class BindableVarray;
	
    class Query
        {
		  public:
			enum QueryMode {MODE_SQL, MODE_DML, MODE_PROCEXEC, MODE_PLSQL_REFCURSOR};
			
		  private:
			QueryImpl *impl;
			
			Session *m_session;
            bool inUse;
			int m_execTime;
			int m_fetchTime;
			int m_execCount;
			int m_fetchCount;
			int m_rowsFetched;
			std::string *m_debugName;

/**
 * Flag for the type of statement the query will run.
 *
 * Depending on the mode of operation, the query may need to be executed differently.
 * In particular, some types of queries do not need to call bindColumns(), or may need to
 * call it at different times.
 *
 * MODE_SQL is a query that may return rows.  (The system allows DML statements to be sent using MODE_SQL, provided
 *          they bind no columns and never invoke fetch().)
 * MODE_DML is a statement that updates rows.  It will not return rows.
 * MODE_PROCEXEC is a statement that invokes a stored procedure.  It will not return rows, but it may alter
 *               parameters bound with bindParameter().
 * MODE_PLSQL_REFCURSOR is Oracle-specific.
 */
			QueryMode m_queryMode;
			bool m_useProcFetch;

/** Because database results are bound to the memory location of members
    of Query, copying them is not allowed.  Do not define this function.
*/
            Query(const Query &rhs);

/** Because database results are bound to the memory location of members
    of Query, copying them is not allowed.  Do not define this function.
*/
            Query &operator=(const Query &rhs);

          protected:
/** Switches to array mode to return an array of results, rather than a single row.
 * Call this first, then call the bindCol functions with the first element in the array.
 */
			void setColArrayMode(size_t skipSize, size_t numElements);
			
			bool bindCol(BindableLong &buffer);
			bool bindParameter(BindableLong &buffer);
			bool bindCol(BindableDouble &buffer);
			bool bindParameter(BindableDouble &buffer);
			bool bindCol(BindableStringBase &buffer);
			bool bindParameter(BindableStringBase &buffer);
			bool bindCol(BindableUnicodeBase &buffer);
			bool bindParameter(BindableUnicodeBase &buffer);
			bool bindCol(BindableBool &buffer);
			bool bindParameter(BindableBool &buffer);
			bool bindParameter(BindableVarray &buffer);
			
			Protocol getProtocol() const;

          public:
            // functions to use as-is:
            Query();
            virtual ~Query();

            int fetch();
			int getNumRowsFetched() const;
            void done();

			int rowCount();

			QueryMode getMode() const;
			void setMode(QueryMode newMode);

			std::string outputDataValues() const;

                // functions to override:
/** Override this function to return the SQL command for the query.
 */
            virtual void getSQL(std::string &sql)=0;
/** Override this function to bind the results returned by the query to
    variables or to bind parameters in the query.

    See the Database Library documentation for more information, or refer to
    the	various Bindable types.  An implementation of bind will probably
    include lines like

    <pre>
    mydata.BindCol(1,this);
    myparameter.BindParameter(1,this);
    </pre>
*/
            virtual bool bindParameters()=0;
			virtual bool bindColumns()=0;
			
			virtual QueryMode getExecutionMode() const;

			virtual void getDebugName(std::string &debugName);
			
			bool setup(Session *session);
			bool prepare();
			bool exec();
        };

// ----------------------------------------------------------------------

	inline Query::QueryMode Query::getMode() const
	{
		return m_queryMode;
	}

// ----------------------------------------------------------------------

	inline void Query::setMode(Query::QueryMode newMode)
	{
		m_queryMode=newMode;
	}

// ----------------------------------------------------------------------

	inline int Query::getNumRowsFetched() const
	{
		return m_rowsFetched;
	}
	
// ----------------------------------------------------------------------
} //namespace

// ======================================================================

#endif
