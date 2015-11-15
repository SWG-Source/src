// ======================================================================
//
// TableBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TableBuffer_H
#define INCLUDED_TableBuffer_H

// ======================================================================

#include <vector>
#include "serverDatabase/AbstractTableBuffer.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

namespace DB
{
	class Session;
	struct Row;
}

// ======================================================================

/**
 * A TableBuffer is a list of updates to be sent to a table, or a list
 * of rows retrieved from a table.
 * @todo Move to database interface.
 *
 * Note that we do not use the RowsType typedef inside this file to avoid
 * compile problems when trying to get this one file to compile under vs6,
 * vs7, and gcc 2.95.3.  gcc 3.x has not yet been tested.
 *
 */
template <class ROW, class QUERY>
class TableBuffer : public AbstractTableBuffer
{
  public:
	typedef std::vector<ROW *> RowsType;

	virtual ~TableBuffer(void);

	DB::ModeQuery::Mode getMode(void) const;

	virtual bool load(DB::Session *session);
	virtual bool save(DB::Session *session);

  protected:

	typename std::vector<ROW *>::iterator addRow(ROW *newRow);

	TableBuffer(DB::ModeQuery::Mode mode);

  	std::vector<ROW *> m_rows;

  private:
/**
 * The TableBuffer can be for the purpose of selecting, adding, changing, or deleting rows.
 *
 * For now, not supporting holding different types of changes within the same buffer.
 * This may change in the future.
 */
	DB::ModeQuery::Mode m_mode;
};

// ======================================================================

template <class ROW, class QUERY>
TableBuffer<ROW,QUERY>::TableBuffer(DB::ModeQuery::Mode mode) :
	m_mode(mode)
{
}

// ----------------------------------------------------------------------

template <class ROW, class QUERY>
TableBuffer<ROW,QUERY>::~TableBuffer(void)
{
	for (typename std::vector<ROW *>::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete *i;
	}
}

// ----------------------------------------------------------------------

template <class ROW, class QUERY>
DB::ModeQuery::Mode TableBuffer<ROW,QUERY>::getMode(void) const
{
	return m_mode;
}

// ----------------------------------------------------------------------

/**
 * Write all the rows in the buffer to the database.
 *
 * The mode determines whether these rows will be sent as inserts, updates, or deletes.
 * If the mode is select, this function will FATAL.
 */
template <class ROW, class QUERY>
bool TableBuffer<ROW,QUERY>::save(DB::Session *session)
{
	QUERY qry;

	if (getMode()==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();

	for (typename std::vector<ROW *>::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		qry.setData(**i);
		if (! (session->exec(&qry)))
			return false;

	}
	qry.done();
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
template <class ROW, class QUERY>
bool TableBuffer<ROW,QUERY>::load(DB::Session *session)
{
	int rowsFetched;
	QUERY qry;

	FATAL (getMode()!=DB::ModeQuery::mode_SELECT,("load() was invoked on a TableBuffer that was not in tbm_select mode."));

	qry.selectMode();

	if (! (session->exec(&qry)))
		return false;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		ROW *newRow=new ROW(dynamic_cast<const ROW &>(qry.getData()));
		addRow(newRow);
	}

	qry.done();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

/**
 * Add a row to the list of rows in the buffer.
 *
 * Derived classes may override this function (to keep an index
 * of the rows, for example), but they should always back-chain to this
 * function.
 */

template <class ROW, class QUERY>
typename std::vector<ROW *>::iterator TableBuffer<ROW,QUERY>::addRow(ROW *newRow)
{
	m_rows.push_back(newRow);
	typename std::vector<ROW *>::iterator retval=m_rows.end();
	return --retval;
}

// ======================================================================

#endif
