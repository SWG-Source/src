// ======================================================================
//
// DBQueryImpl.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBQueryImpl_H
#define INCLUDED_DBQueryImpl_H

// ======================================================================

#include "sharedDatabaseInterface/DbProtocol.def"

// ======================================================================

namespace DB
{
	class BindableBool;
	class BindableDouble;
	class BindableInt64;
	class BindableLong;
	class BindableStringBase;
	class BindableUnicodeBase;
	class BindableVarray;
	class Query;
	class Session;
	
	/**
	 * A class to contain the database-specific implementation of a
	 * query.
	 *
	 * The constructor of Query creates an instance of QueryImpl.  This
	 * allows the library user to derive classes from Query without
	 * knowing specifics of the implementation.
	 */
	class QueryImpl
	{
	  protected:
		Query *m_query;
		
	  public:
		explicit QueryImpl(Query *query);
		virtual ~QueryImpl() {}; //lint !e1540 //We don't delete m_query (because we don't own it)

		virtual bool setup(Session *session) =0;
		virtual bool prepare() =0;
		virtual int fetch() =0;
		virtual void done() =0;
		virtual int rowCount() =0; //TODO:  Should be getRowCount() to be more consistent
		virtual bool exec() =0;

		virtual Protocol getProtocol() const =0;

		virtual void setColArrayMode(size_t skipSize, size_t numElements)=0;
		
		virtual bool bindCol(BindableLong &buffer) =0;
		virtual bool bindParameter(BindableLong &buffer) =0;
		virtual bool bindCol(BindableDouble &buffer) =0;
		virtual bool bindParameter(BindableDouble &buffer) =0;
		virtual bool bindCol(BindableStringBase &buffer) =0;
		virtual bool bindParameter(BindableStringBase &buffer) =0;
		virtual bool bindCol(BindableUnicodeBase &buffer) =0;
		virtual bool bindParameter(BindableUnicodeBase &buffer) =0;
		virtual bool bindCol(BindableBool &buffer) =0;
		virtual bool bindParameter(BindableBool &buffer) =0;
		virtual bool bindParameter(BindableVarray &buffer) =0;

		virtual std::string outputDataValues() const =0;
			
	  private:
		QueryImpl(); // disable
		QueryImpl(const QueryImpl &); // disable
		QueryImpl & operator=(const QueryImpl &); // disable
	};

}

// ======================================================================

inline DB::QueryImpl::QueryImpl(DB::Query *query) :
		m_query(query)
{
}

// ======================================================================

#endif

