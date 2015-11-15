// ======================================================================
//
// DBRow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBRow_H
#define INCLUDED_DBRow_H

// ======================================================================

namespace DB
{

/**
 * This is the base class for classes that represent a row of data returned
 * from the database.
 *
 * The goal is that in many cases, classes derived from Row will be generated
 * directly from the database schema.  Therefore, the class is kept as simple
 * as possible.
 */
struct Row
{
	Row();
	Row( const Row& );

	virtual ~Row();
	/**
	 * This function is like a virtual operator=.
	 *
	 * Implement this function in derived classes.  It should do something
	 * equivalent to   *this = dynamic_cast<const derived_class_name&>(rhs)
	 *
	 * This function is not pure virtual so that derived classes do not need
	 * to define it if they do not plan to use it.  However, the default
	 * implementation will FATAL.
	 *
	 * @todo We could make a CopyableRow class derived from Row that has
	 * this function as pure virtual.  However, that might limit our ability
	 * to generate Row's directly from the database schema without regards to
	 * how they will be used.
	 */
	virtual void copy(const Row &rhs);

	static int getRowsCreated() { return m_sRowsCreated; }
	static int getRowsDeleted() { return m_sRowsDeleted; }

private:
	static volatile int m_sRowsCreated;
	static volatile int m_sRowsDeleted;
};
	
} // namespace

// ======================================================================
#endif
