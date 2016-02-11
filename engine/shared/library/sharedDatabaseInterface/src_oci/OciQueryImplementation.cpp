// ======================================================================
//
// OCIQueryImpl.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "OciQueryImplementation.h"

#include <oci.h>

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbProtocol.def"
#include "sharedDatabaseInterface/DbQuery.h"
#include "OciServer.h"
#include "OciSession.h"
#include "sharedLog/Log.h"

// ======================================================================

DB::OCIQueryImpl::OCIQueryImpl(Query *query, bool autocommit) :
		QueryImpl(query), m_stmthp(0), m_cursorhp(0), m_procBind(0), m_session(0), m_server(0), m_inUse(false), numRowsFetched(0),
		nextColumn(1), nextParameter(1), m_endOfData(false), m_dataReady(false), m_autocommit(autocommit),
		m_skipSize(0),
		m_numElements(1),
		m_sql()
{
}

// ----------------------------------------------------------------------

DB::OCIQueryImpl::~OCIQueryImpl()
{
	if (m_stmthp)
		done();
	
	for (BindRecListType::iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
	{
		delete (*i);
	}
}


// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::setup(Session *session)
{
//	setSession(session);

	DEBUG_FATAL(m_session!=0,("m_session was not nullptr"));
	DEBUG_FATAL(m_server!=0,("m_server was not nullptr"));
	DEBUG_FATAL(m_stmthp!=0,("m_stmthp was not 0"));
	DEBUG_FATAL(m_cursorhp!=0,("m_cursorhp was not 0"));
	
	m_session=dynamic_cast<DB::OCISession*>(session);
	FATAL((m_session==0),("Must pass a non-nullptr OCISession to setup()."));

	m_server=m_session->m_server;
	NOT_NULL(m_server);
	
	if (!m_server->checkerr(*m_session, OCIHandleAlloc( m_session->envhp,
															  reinterpret_cast<void**>(&m_stmthp),
															  OCI_HTYPE_STMT, 0, 0)))
		return false;

	
	m_cursorhp=m_stmthp; // later on, we'll separate these two if needed
	
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::prepare()
{
	NOT_NULL(m_server);
	NOT_NULL(m_stmthp);

	m_query->getSQL(m_sql); // note: do not change m_sql until done() is called, because Oracle keeps a pointer to it
	DEBUG_FATAL(m_sql.size()==0,("Query did not set a SQL statement.\n"));

	if (!m_server->checkerr(*m_session,
							OCIStmtPrepare(m_stmthp,
										   m_session->errhp,
										   reinterpret_cast<OraText*>(const_cast<char*>(m_sql.c_str())),
										   m_sql.length(),
										   (ub4) OCI_NTV_SYNTAX,
										   (ub4) OCI_DEFAULT)))
		return false;
	if (m_query->getMode()==Query::MODE_PLSQL_REFCURSOR)
	{
		m_cursorhp=0; // may not be necessary -- OCI docs unclear
		if (!m_server->checkerr(*m_session, OCIHandleAlloc( m_session->envhp, (dvoid **) &m_cursorhp,
																  OCI_HTYPE_STMT, 0, 0)))
			return false;
		
		if (!m_server->checkerr(*m_session,
								OCIBindByPos (m_stmthp,
											  &m_procBind,
											  m_session->errhp,
											  nextParameter++, // 1st parameter must be the cursor
											  &m_cursorhp,
											  0,
											  SQLT_RSET,
											  0,
											  0,
											  0,
											  0,
											  0,
											  OCI_DEFAULT)))
			return false;
	}
	
	return true;
}

// ----------------------------------------------------------------------

void DB::OCIQueryImpl::preprocessBinds()
{
	//Preprocess binds & defines
	for (BindRecListType::iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
	{
		if ((*i)->owner->isNull())
			(*i)->indicator=-1;
		else
		{
			(*i)->length=(unsigned short)*((*i)->owner->getIndicator()); //TODO:  something better
			(*i)->indicator=1;
			if ((*i)->stringAdjust)
				++(*i)->length;  //TODO:  This feels like a hack
		}
	}
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::exec()
{
	NOT_NULL(m_server);
	NOT_NULL(m_stmthp);

	Query::QueryMode mode=m_query->getMode();
	FATAL((mode!=Query::MODE_SQL) && (mode!=Query::MODE_DML) && (mode!=Query::MODE_PROCEXEC) && (mode!=Query::MODE_PLSQL_REFCURSOR),
		("OCI query is in mode %i, not supported.\n",mode));

	if (!m_inUse)	// if the query was just run, we don't need to do the setup again
	{
		if (!prepare()) return false;
		if (!m_query->bindParameters()) return false;
	}

	if (mode==Query::MODE_SQL)
		if (!m_query->bindColumns()) return false;
	
	preprocessBinds();

	if (m_server->isPrefetchEnabled())
	{
		int numRowsToPrefetch=m_server->getPrefetchRows();
		int prefetchMemory=m_server->getPrefetchMemory();
		if (!m_server->checkerr(*m_session, OCIAttrSet((dvoid *) m_stmthp, (ub4) OCI_HTYPE_STMT,
															 &numRowsToPrefetch, sizeof(int),
															 (ub4) OCI_ATTR_PREFETCH_ROWS, m_session->errhp)))
		{
			LOG("DatabaseError", ("Unable to set OCI_ATTR_PREFETCH_ROWS to %i", numRowsToPrefetch));
			return false;
		}
		if (!m_server->checkerr(*m_session, OCIAttrSet((dvoid *) m_stmthp, (ub4) OCI_HTYPE_STMT,
															 &prefetchMemory, sizeof(int),
															 (ub4) OCI_ATTR_PREFETCH_MEMORY, m_session->errhp)))
		{
			LOG("DatabaseError", ("Unable to set OCI_ATTR_PREFETCH_MEMORY to %i", prefetchMemory));
			return false;
		}
	}

	m_session->setOkToFetch();
	m_session->setLastQueryStatement(m_sql);
	sword status=OCIStmtExecute(m_session->svchp, m_stmthp, m_session->errhp, 1, 0,
								nullptr, nullptr, OCI_DEFAULT);
	
	if (status == OCI_NO_DATA)
	{
		if (mode==Query::MODE_PLSQL_REFCURSOR)
		{
			WARNING_STRICT_FATAL(!m_session->isOkToFetch(),("Calling fetch after commit, without an execute in between (may cause Oracle to crash)."));
			sword status=OCIStmtFetch (m_cursorhp, m_session->errhp, 0, OCI_FETCH_NEXT, OCI_DEFAULT);
			if (!m_server->checkerr(*m_session,status))
				return false;
		}
		m_endOfData=true;
	}
	else
	{
		if (m_server->checkerr(*m_session,status))
		{
			if (mode==Query::MODE_PLSQL_REFCURSOR)
			{
				if (!m_query->bindColumns()) return false;
				
				m_dataReady=false; // in this mode, Oracle doesn't implicitly fetch a row
				m_endOfData=false;
			}
			else
			{
				postProcessResults(); // in this mode, Oracle implicitly fetches the first row
				
				m_endOfData=false;
				m_dataReady=true;
			}
		}
		else
			return false;
	}
	
	m_inUse=true;
	return true;
}

// ----------------------------------------------------------------------

int DB::OCIQueryImpl::fetch()
{
	DEBUG_FATAL((m_query->getMode()!=Query::MODE_SQL) && (m_query->getMode()!=Query::MODE_PLSQL_REFCURSOR),
				("Called fetch() on a query not in a fetch-compatible mode."));
	
	if (m_dataReady) // a row was already fetched implicitly by exec
	{
		m_dataReady = false;
		return 1;
	}
	
	if (m_endOfData) return 0;

	WARNING_STRICT_FATAL(!m_session->isOkToFetch(),("Calling fetch after commit, without an execute in between (may cause Oracle to crash)."));
	sword status=OCIStmtFetch (m_cursorhp, m_session->errhp, m_numElements, OCI_FETCH_NEXT, OCI_DEFAULT);

	if (status == OCI_NO_DATA)
	{
		ub4 rows;
		ub4 sizep = sizeof(ub4);
		OCIAttrGet((dvoid *) m_cursorhp, (ub4) OCI_HTYPE_STMT,
				   (dvoid *)& rows, (ub4 *) &sizep, OCI_ATTR_ROWS_FETCHED, m_session->errhp);

		sword status=OCIStmtFetch (m_cursorhp, m_session->errhp, 0, OCI_FETCH_NEXT, OCI_DEFAULT); // cancel the cursor
		if (!m_server->checkerr(*m_session,status))
			return -1;
		m_endOfData=true;
		if (rows!=0)
		{
			postProcessResults();
			return rows; // last batch of data
		}
		else
			return 0; // no more data
	}
	else
	{
		if (!m_server->checkerr(*m_session,status))
			return -1;
	}
	
	ub4 rows;
	ub4 sizep = sizeof(ub4);
	OCIAttrGet((dvoid *) m_cursorhp, (ub4) OCI_HTYPE_STMT,
			   (dvoid *)& rows, (ub4 *) &sizep, OCI_ATTR_ROWS_FETCHED, m_session->errhp);
	
	m_endOfData=false;
	postProcessResults();
	return rows;
}

// ----------------------------------------------------------------------


void DB::OCIQueryImpl::postProcessResults()
{
	if (m_numElements == 1)
	{
		for (BindRecListType::iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
		{
			if ((*i)->indicator==-1)
				(*i)->owner->setNull();
			else
				*((*i)->owner->getIndicator())=(*i)->length;
		}
	}
	else
	{
		for (BindRecListType::iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
		{
			if ((*i)->m_indicatorArray)
			{
				for (size_t j=0; j<m_numElements; ++j)
				{
					Bindable *owner = reinterpret_cast<Bindable*>(reinterpret_cast<char*>((*i)->owner) + m_skipSize * j); // if OCI can do something this ugly, so can I
				
					if ((*i)->m_indicatorArray[j]==-1)
						owner->setNull();
					else
						*(owner->getIndicator())=(*i)->m_lengthArray[j];
				}
			}
			else
			{
				// this query involves bulk binds, but this parameter is not one of them.  Treat it as a single bind.
				if ((*i)->indicator==-1)
					(*i)->owner->setNull();
				else
					*((*i)->owner->getIndicator())=(*i)->length;
			
			}
		}
	}
}

// ----------------------------------------------------------------------

void DB::OCIQueryImpl::done()
{
	NOT_NULL(m_stmthp);

	if (m_autocommit)	
		m_server->checkerr(*m_session, OCITransCommit(m_session->svchp, m_session->errhp, 0));

	OCIHandleFree(m_stmthp, OCI_HTYPE_STMT);
	if (m_cursorhp!=m_stmthp)
		OCIHandleFree(m_cursorhp, OCI_HTYPE_STMT);
	
	for (BindRecListType::iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
	{
		delete (*i);
	}
	bindRecList.clear();

	m_session=0;
	m_server=0;
	m_stmthp=0;
	m_cursorhp=0;
	nextColumn=1;
	nextParameter=1;
	m_endOfData=true;
	m_inUse=false;
}

// ----------------------------------------------------------------------

int DB::OCIQueryImpl::rowCount()
{
	NOT_NULL(m_stmthp);

	int value;

	m_server->checkerr(*m_session, OCIAttrGet (m_cursorhp, OCI_HTYPE_STMT,&value,0,OCI_ATTR_ROW_COUNT,m_session->errhp));
 
	return value;
}

// ----------------------------------------------------------------------

DB::OCIQueryImpl::BindRec *DB::OCIQueryImpl::addBindRec(Bindable &owner)
{
	BindRec *br=new BindRec(m_numElements);
	br->owner=&owner;
	bindRecList.push_back(br);
	return br;
}

// ----------------------------------------------------------------------

DB::Protocol DB::OCIQueryImpl::getProtocol() const
{
	return DB::PROTOCOL_OCI;
}

// ----------------------------------------------------------------------

void DB::OCIQueryImpl::setColArrayMode(size_t skipSize, size_t numElements)
{
	m_skipSize = skipSize;
	m_numElements = numElements;
}

// ======================================================================
			
DB::OCIQueryImpl::BindRec::BindRec(size_t numElements) :
		defnp(0),
		bindp(0),
		indicator(-1),
		length(0),
		stringAdjust(false),
		m_numElements(numElements),
		m_indicatorArray(nullptr),
		m_lengthArray(nullptr)
{
	// This is ugly, but it avoids having to create two different kinds of bindrecs that inherit from an abstract base class
	if (m_numElements > 1)
	{
		m_indicatorArray = new int16[m_numElements];
		m_lengthArray = new uint16[m_numElements];
	}
}

// ----------------------------------------------------------------------

DB::OCIQueryImpl::BindRec::~BindRec()
{
	delete[] m_indicatorArray;
	delete[] m_lengthArray;
}

// ----------------------------------------------------------------------

int16 * DB::OCIQueryImpl::BindRec::getIndicatorPointer()
{
	if (m_numElements > 1)
		return m_indicatorArray;
	else
		return &indicator;
}

// ----------------------------------------------------------------------

uint16 * DB::OCIQueryImpl::BindRec::getLengthPointer()
{
	if (m_numElements > 1)
		return m_lengthArray;
	else
		return &length;
}

// ----------------------------------------------------------------------

size_t DB::OCIQueryImpl::BindRec::getIndicatorSkipSize()
{
	return reinterpret_cast<char*>(&(m_indicatorArray[1])) - reinterpret_cast<char*>(&(m_indicatorArray[0]));
}

// ----------------------------------------------------------------------

size_t DB::OCIQueryImpl::BindRec::getLengthSkipSize()
{
	return reinterpret_cast<char*>(&(m_lengthArray[1])) - reinterpret_cast<char*>(&(m_lengthArray[0]));
}
	
// ======================================================================

bool DB::OCIQueryImpl::bindCol(BindableLong &buffer)
{
	BindRec *br=addBindRec(buffer);

	m_server->checkerr(*m_session, OCIDefineByPos(m_cursorhp,
														&(br->defnp),
														m_session->errhp,
														nextColumn++,
														buffer.getBuffer(),
														sizeof(long),
														SQLT_INT,
														br->getIndicatorPointer(),
														br->getLengthPointer(),
														(ub2 *)0,
														OCI_DEFAULT));

	if (m_numElements > 1)
	{
		m_server->checkerr(*m_session, OCIDefineArrayOfStruct(br->defnp,
																	m_session->errhp,
																	m_skipSize,
																	br->getIndicatorSkipSize(),
																	br->getLengthSkipSize(),
																	0));
	}
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableLong &buffer)
{
	BindRec *br=addBindRec(buffer);

	return m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
															 &(br->bindp),
															 m_session->errhp,
															 nextParameter++,
															 buffer.getBuffer(),
															 sizeof(long),
															 SQLT_INT,
															 &(br->indicator),
															 &(br->length),
															 0,
															 0,
															 0,
															 OCI_DEFAULT));
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindCol(BindableStringBase &buffer)
{
	BindRec *br=addBindRec(buffer);

	m_server->checkerr(*m_session, OCIDefineByPos(m_cursorhp,
														&(br->defnp),
														m_session->errhp,
														nextColumn++,
														buffer.getBuffer(),
														buffer.getS()+1,
														SQLT_STR,
														br->getIndicatorPointer(),
														br->getLengthPointer(),
														(ub2 *)0,
														OCI_DEFAULT));

	if (m_numElements > 1)
	{
		m_server->checkerr(*m_session, OCIDefineArrayOfStruct(br->defnp,
																	m_session->errhp,
																	m_skipSize,
																	br->getIndicatorSkipSize(),
																	br->getLengthSkipSize(),
																	0));
	}
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableStringBase &buffer)
{
	BindRec *br=addBindRec(buffer);

	br->stringAdjust=true;
	return m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
															 &(br->bindp),
															 m_session->errhp,
															 nextParameter++,
															 buffer.getBuffer(),
															 buffer.getS()+1,
															 SQLT_STR,
															 &(br->indicator),
															 &(br->length),
															 0,
															 0,
															 0,
															 OCI_DEFAULT));
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindCol(BindableUnicodeBase &buffer)
{
	BindRec *br=addBindRec(buffer);

	m_server->checkerr(*m_session, OCIDefineByPos(m_cursorhp,
														&(br->defnp),
														m_session->errhp,
														nextColumn++,
														buffer.getBuffer(),
														buffer.getS()+1,
														SQLT_STR,
														br->getIndicatorPointer(),
														br->getLengthPointer(),
														(ub2 *)0,
														OCI_DEFAULT));

	if (m_numElements > 1)
	{
		m_server->checkerr(*m_session, OCIDefineArrayOfStruct(br->defnp,
																	m_session->errhp,
																	m_skipSize,
																	br->getIndicatorSkipSize(),
																	br->getLengthSkipSize(),
																	0));
	}
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableUnicodeBase &buffer)
{
	BindRec *br=addBindRec(buffer);

	br->stringAdjust=true;
	br->length=static_cast<uint16>(*(buffer.getIndicator()));
	return m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
															 &(br->bindp),
															 m_session->errhp,
															 nextParameter++,
															 buffer.getBuffer(),
															 buffer.getS()+1,
															 SQLT_STR,
															 &(br->indicator),
															 &(br->length),
															 0,
															 0,
															 0,
															 OCI_DEFAULT));
}

// ======================================================================

bool DB::OCIQueryImpl::bindCol(BindableDouble &buffer)
{
	BindRec *br=addBindRec(buffer);

	m_server->checkerr(*m_session, OCIDefineByPos(m_cursorhp,
														&(br->defnp),
														m_session->errhp,
														nextColumn++,
														buffer.getBuffer(),
														sizeof(double),
														SQLT_FLT,
														br->getIndicatorPointer(),
														br->getLengthPointer(),
														(ub2 *)0,
														OCI_DEFAULT));
	if (m_numElements > 1)
	{
		m_server->checkerr(*m_session, OCIDefineArrayOfStruct(br->defnp,
																	m_session->errhp,
																	m_skipSize,
																	br->getIndicatorSkipSize(),
																	br->getLengthSkipSize(),
																	0));
	}
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableDouble &buffer)
{
	BindRec *br=addBindRec(buffer);
	
	return m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
															 &(br->bindp),
															 m_session->errhp,
															 nextParameter++,
															 buffer.getBuffer(),
															 sizeof(double),
															 SQLT_FLT,
															 &(br->indicator),
															 &(br->length),
															 0,
															 0,
															 0,
															 OCI_DEFAULT));
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindCol(BindableBool &buffer)
{
	BindRec *br=addBindRec(buffer);

	m_server->checkerr(*m_session, OCIDefineByPos(m_cursorhp,
														&(br->defnp),
														m_session->errhp,
														nextColumn++,
														buffer.getBuffer(),
														2,
														SQLT_STR,
														br->getIndicatorPointer(),
														br->getLengthPointer(),
														(ub2 *)0,
														OCI_DEFAULT));

	if (m_numElements > 1)
	{
		m_server->checkerr(*m_session, OCIDefineArrayOfStruct(br->defnp,
																	m_session->errhp,
																	m_skipSize,
																	br->getIndicatorSkipSize(),
																	br->getLengthSkipSize(),
																	0));
	}
	return true;
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableBool &buffer)
{
	BindRec *br=addBindRec(buffer);

	br->stringAdjust=true;
	return m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
															 &(br->bindp),
															 m_session->errhp,
															 nextParameter++,
															 buffer.getBuffer(),
															 2,
															 SQLT_STR,
															 &(br->indicator),
															 &(br->length),
															 0,
															 0,
															 0,
															 OCI_DEFAULT));
}

// ----------------------------------------------------------------------

bool DB::OCIQueryImpl::bindParameter(BindableVarray &buffer)
{
	BindRec *br=addBindRec(buffer);

	bool result = m_server->checkerr(*m_session, OCIBindByPos (m_stmthp,
																	 &(br->bindp),
																	 m_session->errhp,
																	 nextParameter++,
																	 nullptr,
																	 0,
																	 SQLT_NTY,
																	 nullptr,
																	 nullptr,
																	 0,
																	 0,
																	 0,
																	 OCI_DEFAULT));
	
	if (!result)
		return false;
	
	return m_server->checkerr(*m_session, OCIBindObject (br->bindp,
															   m_session->errhp,
															   buffer.getTDO(),
															   reinterpret_cast<void**>(buffer.getBuffer()),
															   0,
															   0,
															   0));
}

// ----------------------------------------------------------------------

std::string DB::OCIQueryImpl::outputDataValues() const
{
	std::string results;
	for (BindRecListType::const_iterator i=bindRecList.begin(); i!=bindRecList.end(); ++i)
	{
		if (i!=bindRecList.begin())
			results += ", ";
		results += (*i)->owner->outputValue();
	}
	return results;
}

// ======================================================================
