/* DBBatchQuery.h
 *
 * Templates for defining batch queries (queries that return multiple
 * rows, read from the database in batches).  See SQLClasses
 * documentation for more details.
 *
 * ODBC version
 */

#ifndef _DBBATCHQUERY_H
#define _DBBATCHQUERY_H

#include "sharedDatabaseInterface/DbQuery.h"

namespace DB {

/** Template representing a Query that returns an array of data

	The template parameter R represent the structure that holds the data of one
	row of the results.  BatchQuery will contain an array of R's.  (R will
	usually be a struct holding	a few Bindable types.)
 */
template <class R>
class BatchQuery : public Query
{
	R *data;
	SQLUSMALLINT *rowStatus; 
	int numRows;

public:
	BatchQuery(int _numRows);
	~BatchQuery();

	bool bind(); 

	const R &operator[](int index) const; 
	R &operator[](int index);

/**	Count how many rows in the array were filled in by the last call to 
	Fetch().
 */
	int getNumDefinedRows() const;

/** Is a particular row in the result array defined?  A row is defined if the
	database filled it in with values.  If a row is not defined, the values
	returned for that row by operator[] are not meaningful.
 */
	bool isRowDefined(int index) const;

/** Returns the SQL string for the query.  The user must define this function; the
	template does not create it.
 */
	char *getSQL();

/** Bind the first row of data.  The user must define this function; the template 
	does not create it.

	Implement this function as if it is binding the first element of the array
	(the array is called data).  The template will take care of binding the rest of 
	the array.  It should contain something like
	
	data[0].member.BindCol(1,this)
 */
	bool bindFirstRow(); 
};

template <class R>
BatchQuery<R>::BatchQuery(int _numRows) : numRows(_numRows)
{
	data = new R[numRows];
	rowStatus = new SQLUSMALLINT[numRows];
}

template <class R>
BatchQuery<R>::~BatchQuery()
{
	delete[] data;
	delete[] rowStatus;
}

template <class R>
const R &BatchQuery<R>::operator[](int index) const
{
	return data[index];
}

template <class R>
R &BatchQuery<R>::operator[](int index)
{
	return data[index];
}

template <class R>
int BatchQuery<R>::getNumDefinedRows() const
// TODO:  hmmm, might be better to figure this once and store it
// in a mutable variable -- but how to know when it changes
// without overriding Fetch()?
{
	for (int num=numRows;num>0;--num)
	{
		if (isRowDefined(num))
			return num+1;
	}
	return 0;
}

template <class R>
bool BatchQuery<R>::isRowDefined(int index) const
{
	return (rowStatus[index]==0);
}

template <class R>
bool BatchQuery<R>::bind()
{
	if (!setArrayMode(sizeof(data[0]),numRows,rowStatus))
		return false;
	return bindFirstRow();
}

}
#endif
