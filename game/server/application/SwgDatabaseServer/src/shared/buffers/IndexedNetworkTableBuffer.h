// ======================================================================
//
// IndexedNetworkTableBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_IndexedNetworkTableBuffer_H
#define INCLUDED_IndexedNetworkTableBuffer_H

// ======================================================================

#include "serverDatabase/AbstractTableBuffer.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkId.h"
#include <map>
#include <set>
#include <vector>

// ======================================================================

namespace IndexedNetworkTableBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace IndexedNetworkTableBufferNamespace;


/**
 * A TableBuffer organized to track data by objects, one row per object.
 *
 * @todo:  Rename the class. Name makes no sense anymore.  Un-templatize.
 */

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
class IndexedNetworkTableBuffer : public AbstractTableBuffer
{
  public:
	explicit IndexedNetworkTableBuffer(DB::ModeQuery::Mode mode);
	~IndexedNetworkTableBuffer();
	
  public:
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool save(DB::Session *session);
	
	BUFFERROW *findRowByIndex(const NetworkId &indexValue);
	const BUFFERROW *findConstRowByIndex(const NetworkId &indexValue) const;

	BUFFERROW *addEmptyRow(const NetworkId &indexValue);
	virtual void removeObject(const NetworkId &object);
	DB::ModeQuery::Mode getMode(void) const;

	void addTag(Tag tag);

	static int getRowsCreated() { return m_sRowsCreated; }
	static int getRowsDeleted() { return m_sRowsDeleted; }
	static int getRowsActive()
	{ 
		int i_retval =  m_sRowsCreated - m_sRowsDeleted; 
		if ( i_retval < 0 )
			return 0;
		return i_retval;
	}
	
  protected:
	typedef std::map<NetworkId, BUFFERROW*> IndexType;
	IndexType m_rows;

  private:
	DB::ModeQuery::Mode m_mode;
	DB::TagSet m_relevantTags;

  private:
	void addRow(const NetworkId &indexValue,BUFFERROW *row);
	static volatile int m_sRowsCreated;
	static volatile int m_sRowsDeleted;

  private:
	IndexedNetworkTableBuffer(); // disable
};

template <class B, class R, class Q, class S> volatile int IndexedNetworkTableBuffer<B,R,Q,S>::m_sRowsCreated = 0;
template <class B, class R, class Q, class S> volatile int IndexedNetworkTableBuffer<B,R,Q,S>::m_sRowsDeleted = 0;

// ======================================================================

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::IndexedNetworkTableBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_rows(),
		m_mode(mode)
{
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::~IndexedNetworkTableBuffer()
{
	for (typename IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		++m_sRowsDeleted;
		i->second=nullptr;
	}

	m_rows.clear();
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
void IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::addRow(const NetworkId &indexValue,BUFFERROW *row)
{
	m_rows[indexValue]=row;
	++m_sRowsCreated;
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
BUFFERROW *IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::findRowByIndex(const NetworkId &indexValue)
{
	typename IndexType::iterator i=m_rows.find(indexValue);
	if (i==m_rows.end())
		return 0;
	else
		return i->second;
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
const BUFFERROW *IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::findConstRowByIndex(const NetworkId &indexValue) const
{
	typename IndexType::const_iterator i=m_rows.find(indexValue);
	if (i==m_rows.end())
		return 0;
	else
		return i->second;
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
BUFFERROW *IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::addEmptyRow(const NetworkId &indexValue)
{
	BUFFERROW *row=new BUFFERROW;
	row->setPrimaryKey(indexValue);
	addRow(indexValue,row);
	return row;
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
void IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::removeObject(const NetworkId &object)
{
	typename IndexType::iterator i=m_rows.find(object);
	if (i!=m_rows.end())
	{
		delete i->second;
		++m_sRowsDeleted;
		i->second=nullptr;
		m_rows.erase(i);
	}
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
DB::ModeQuery::Mode IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::getMode(void) const
{
	return m_mode;
}

// ----------------------------------------------------------------------

template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
bool IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::save(DB::Session *session)
{
	QUERY qry;
		
	if (getMode()==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();

	if (!qry.setupData(session))
		return false;
	
	for (typename IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (!qry.addData((i->second)))
			return false;
		if (qry.getNumItems() == IndexedNetworkTableBufferNamespace::ms_maxItemsPerExec)
		{	
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;
	qry.done();
	qry.freeData();
	return true;
}

// ----------------------------------------------------------------------

/**
 * Load rows into the buffer from the database.
 *
 * The query is assumed to return 0 or more rows, which will be placed
 * in the buffer.  The virtual addRow function is used to make sure indexes,
 * etc., are updated as needed.
 */
template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
bool IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (m_relevantTags.empty() || hasCommonTags(m_relevantTags, tags))
	{
		SELECTQUERY qry(schema);
		
		FATAL (getMode()!=DB::ModeQuery::mode_SELECT,("load() was invoked on a TableBuffer that was not in tbm_select mode."));
		
		if (! (session->exec(&qry)))
			return false;
		
		while ((rowsFetched = qry.fetch()) > 0)
		{
			int numRows = qry.getNumRowsFetched();
			int count = 0;

			const std::vector<ROW> &data = qry.getData();
		
			for (typename std::vector<ROW>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{		
				if (++count > numRows)
					break;
				BUFFERROW *newRow=new BUFFERROW(*i);
				addRow(newRow->getPrimaryKey(),newRow);
			}
		}
		
		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

/**
 * Add a tag to the list we are interested in.  If the current snapshot
 * contains no objects with tags in this list, this buffer will skip its
 * load.  (Exception:  If no tags are registered at all, the load will
 * always happen.)
 */
template <class BUFFERROW, class ROW, class QUERY, class SELECTQUERY>
void IndexedNetworkTableBuffer<BUFFERROW,ROW,QUERY,SELECTQUERY>::addTag(Tag tag)
{
	m_relevantTags.insert(tag);
}

// ======================================================================

#endif
