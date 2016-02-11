// ======================================================================
//
// DBBufferRow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//TODO: Is this file obsolete?

#ifndef INCLUDED_DBBufferRow_H
#define INCLUDED_DBBufferRow_H

// ======================================================================

#include "sharedDatabaseInterface/DbRow.h"

// ======================================================================

namespace DB
{

	/**
	 * A BufferRow is a Row that works with the TableBuffer class.
	 *
	 * A BufferRow is a Row that represents a Row from a table (not a view
	 * or a query that joins multiple tables).  It implements various
	 * virtual functions that the TableBuffer class uses.
	 */
	struct BufferRow:public Row
	{
		
		/**
		 * Combines the values in this row with the values in newRow.
		 *
		 * For each column:
		 *    If the column is nullptr in newRow, do nothing.  (Leave the value
		 *        in this row unchanged.)
		 *    If the column is not nullptr in newRow, replace the value in this
		 *        row with the value in newRow.
		 */
		virtual void combine(const DB::Row &newRow) =0;

		/**
		 * Makes a new copy of this row.
		 */
		virtual BufferRow *copy(void) const =0;
	};

} // namespace

// ======================================================================

#endif
