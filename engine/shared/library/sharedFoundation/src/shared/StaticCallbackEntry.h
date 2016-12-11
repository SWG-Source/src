// ======================================================================
//
// StaticCallbackEntry.h
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	_INCLUDED_StaticCallbackEntry_H
#define	_INCLUDED_StaticCallbackEntry_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"

class MemoryBlockManager;

// ======================================================================
/** @brief A class used by Scheduler to invoke a callback on a static
	function.
*/

class StaticCallbackEntry
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	typedef void (*Callback)(const void *context);

	struct Compare
	{
		bool operator ()(const StaticCallbackEntry * lhs, const StaticCallbackEntry * rhs) const;
	};

public:

	StaticCallbackEntry(unsigned long when, Callback cb, const void *context);
	~StaticCallbackEntry();

	unsigned long  getExpireCount() const;
	void           expired() const;

private:

	struct StaticCallbackEntryFreeList;
	friend struct StaticCallbackEntryFreeList;

private:

	static std::vector<StaticCallbackEntry *> & getStaticCallbackFreeList();

private:

	// Disabled.
	StaticCallbackEntry(const StaticCallbackEntry &);
	StaticCallbackEntry & operator = (const StaticCallbackEntry &);

private:

	unsigned long  m_expireCount;
	Callback       m_staticCallbackFunction;
	const void    *m_context;

};

// ======================================================================

/** @return the expiration count ("time", "frame", whatever) of the callback.
*/
inline unsigned long StaticCallbackEntry::getExpireCount() const
{
	return m_expireCount;
}

// ======================================================================

inline bool StaticCallbackEntry::Compare::operator ()(const StaticCallbackEntry * lhs, const StaticCallbackEntry * rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getExpireCount() > rhs->getExpireCount();
}

// ======================================================================

#endif	// _INCLUDED_StaticCallbackEntry_H
