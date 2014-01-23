#if 0 //@todo code reorg
// ======================================================================
//
// IndexedNetworkTableBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_IndexedNetworkTableBuffer_H
#define INCLUDED_IndexedNetworkTableBuffer_H

// ======================================================================

#include <unordered_map>
#include "serverDatabase/TableBuffer.h"

// ======================================================================

/**
 * Adds to NetworkTableBuffer the ability to retrieve rows based on an
 * integer index.
 * @todo Remove "Network" from the name because it no longer has anything to
 * do with the network.
 * @todo If we have time someday :)  Play with the idea of making this be a
 * class that's attached to a TableBuffer, much the same way a real DB index
 * is attached to a table.  The TableBuffer could have a list of pointers
 * to indexes, and it would invoke addRow, etc., as needed.
 */

class IndexedNetworkTableBuffer : public TableBuffer
{
  public:
	void addRowToIndex(int indexValue, DB::Row *row);
	DB::Row *findRowByIndex(int indexValue);

  protected:
	IndexedNetworkTableBuffer(TableBufferMode mode, DB::ModeQuery *query);
	
  private:
	typedef std::unordered_map<int, DB::Row*> IndexType;
	/**
	 * Index to locate rows.
	 *
	 * Note that the rows are owned by the TableBuffer base class.  This
	 * is just an index to find a specific row quickly.  Therefore, this
	 * class does not create or delete the rows.
	 */
	IndexType m_index;
};

// ======================================================================

#endif
#endif
