/* DBModeQuery.h
 *
 * Query objects that support different modes of operation
 *
 */

#ifndef _DBMODEQUERY_H
#define _DBMODEQUERY_H

//-----------------------------------------------------------------------

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbRow.h"

//-----------------------------------------------------------------------

namespace DB
{

/**
 * A ModeQuery is a Query that can be in update, insert, select, or delete mode.
 *
 * This class is designed to work with the TableBuffer class.  
 *
 * Derive from this class to make a query for a specific purpose.
 * Implement the following in the derived class:
 *   1) A constructor that initializes _data_ to point to a row of the appropriate type.
 *   2) A bind function (will need to dynamic_cast _data_ to the appropriate type of row)
 *   3) A getSQL() function
 *
 * Note that the copy() function must be defined for the type of row used.
 *
 * Note also that getSQL() will need to return different values depending on the mode.  Bind()
 * may also need to behave differently in different modes, depending on the query.  The default
 * destructor deletes _data_, so it is not necessary to do so in a derived destructor.
 */
class ModeQuery : public Query
{
  public:
	enum Mode {mode_UPDATE,mode_INSERT,mode_DELETE,mode_SELECT,mode_INVALID};
	Mode mode; //TODO:  rename to m_mode

  protected:
	Row *data;
	
  public:
	explicit ModeQuery(Row *derivedRow);
	virtual ~ModeQuery();

	void setData(const Row &_data);
	const Row &getData(void) const;
	
	void updateMode();
	void insertMode();
	void deleteMode();
	void selectMode();

  private:
	ModeQuery(); //disable
	ModeQuery(const ModeQuery &); //disable
	ModeQuery & operator=(const ModeQuery &); //disable
};

//-----------------------------------------------------------------------
// Inline functions

inline ModeQuery::ModeQuery(Row *derivedRow) :
	Query(),
	mode (mode_INVALID),
	data(derivedRow)
{
}

//------------------------------------------------------------------------

inline void ModeQuery::insertMode()
{
	mode=mode_INSERT;
}

//------------------------------------------------------------------------

inline void ModeQuery::updateMode()
{
	mode=mode_UPDATE;
}

//------------------------------------------------------------------------

inline void ModeQuery::deleteMode()
{
	mode=mode_DELETE;
}

//------------------------------------------------------------------------

inline void ModeQuery::selectMode()
{
	mode=mode_SELECT;
}

//------------------------------------------------------------------------

inline const Row &ModeQuery::getData() const
{
	return *data;
}

// ----------------------------------------------------------------------

inline void ModeQuery::setData(const Row &_data)
{
	data->copy(_data);
}

// ----------------------------------------------------------------------

} // namespace

#endif
