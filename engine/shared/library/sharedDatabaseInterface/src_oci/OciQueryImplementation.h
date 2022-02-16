// ======================================================================
//
// OCIQueryImpl.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OCIQueryImpl_H
#define INCLUDED_OCIQueryImpl_H

// ======================================================================

#include "sharedDatabaseInterface/DbQueryImplementation.h"
#include <list>
#include <string>

// ======================================================================

namespace DB
{
	class BindableVarray;
}

// ======================================================================

struct OCIStmt;
struct OCIDefine;
struct OCIBind;

namespace DB
{
	class Bindable;
	class OCISession;
	class OCIServer;
	
	class OCIQueryImpl : public QueryImpl
	{
		public:
		OCIQueryImpl(Query *query, bool autocommit);
		virtual ~OCIQueryImpl();

		virtual bool setup(Session *session);
		virtual bool prepare();
		virtual int fetch();
		virtual void done();
		virtual int rowCount();
		virtual bool exec();

		virtual Protocol getProtocol() const;

		virtual void setColArrayMode(size_t skipSize, size_t numElements);

		virtual bool bindCol(BindableLong &buffer);
		virtual bool bindParameter(BindableLong &buffer);
		virtual bool bindCol(BindableDouble &buffer);
		virtual bool bindParameter(BindableDouble &buffer);
		virtual bool bindCol(BindableStringBase &buffer);
		virtual bool bindParameter(BindableStringBase &buffer);
		virtual bool bindCol(BindableUnicodeBase &buffer);
		virtual bool bindParameter(BindableUnicodeBase &buffer);
		virtual bool bindCol(BindableBool &buffer);
		virtual bool bindParameter(BindableBool &buffer);
		virtual bool bindParameter(BindableVarray &buffer);

		std::string outputDataValues() const;
			
	  protected:
		struct BindRec
		{
			BindRec(size_t numElements);
			~BindRec();
	
			int16 *  getIndicatorPointer();
			uint16 * getLengthPointer();

			size_t getIndicatorSkipSize();
			size_t getLengthSkipSize();
			
			Bindable *owner;
			OCIDefine *defnp;
			OCIBind *bindp;
			int16 indicator;
			uint16 length;
			bool stringAdjust;
			size_t m_numElements;
			int16 *m_indicatorArray;
			uint16 *m_lengthArray;
		};

		typedef std::list<BindRec*> BindRecListType;
		
	  protected:

		BindRec *addBindRec(Bindable &owner);
		void preprocessBinds();
		void postProcessResults();
		
	  protected:
		/** The OCIStmt handle representing the query.
		 */
		::OCIStmt *m_stmthp;
		/** The handle representing the cursor containing the results of the query.  Usually,
		 * this is set to the same as stmthp.
		 */
		::OCIStmt *m_cursorhp;
		::OCIBind *m_procBind;

		DB::OCISession *m_session;
		DB::OCIServer *m_server;

		bool m_inUse;
		int numRowsFetched;
		int nextColumn;
		int nextParameter;
		bool m_endOfData;
		bool m_dataReady;
		BindRecListType bindRecList;
		bool m_autocommit;
		size_t m_skipSize;
		size_t m_numElements;

		std::string m_sql;
		
	  private:
		  OCIQueryImpl(const OCIQueryImpl&);
		  OCIQueryImpl & operator = (const OCIQueryImpl &);
	};

}

// ======================================================================

#endif
