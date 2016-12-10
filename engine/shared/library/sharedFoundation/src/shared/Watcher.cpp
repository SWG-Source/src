// ======================================================================
//
// Watcher.cpp
// copyright 1998    Bootprint Entertainment
// Copyright 2000-01 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Watcher.h"

#include "sharedFoundation/ExitChain.h"

#include <vector>

// ======================================================================

namespace WatchedByListNamespace
{
	typedef std::vector<BaseWatcher *> List;
	typedef std::vector<List *> ListList;

	ListList ms_listList;

	void remove();
	List * newList();
	void deleteList(List * list);
}

using namespace WatchedByListNamespace;

// ======================================================================

void WatchedByListNamespace::remove()
{
	while (!ms_listList.empty())
	{
		delete ms_listList.back();
		ms_listList.pop_back();
	}
}

// ----------------------------------------------------------------------

List * WatchedByListNamespace::newList()
{
	if (ms_listList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_listList.push_back(new List);
	}

	List * const result = ms_listList.back();
	ms_listList.pop_back();

	return result;
}

// ----------------------------------------------------------------------

void WatchedByListNamespace::deleteList(List * const list)
{
	if (ms_listList.size() < 256)
	{
		DEBUG_FATAL(!list->empty(), ("list is not empty"));
		list->clear();
		ms_listList.push_back(list);
	}
	else
		delete list;
}

// ======================================================================

void WatchedByList::install()
{
	ExitChain::add(WatchedByListNamespace::remove, "WatchedByListNamespace::remove");
}

// ======================================================================
/**
 * Destroy a WatchedByList.
 *
 * All watchers currently watching the owner of this object will be reset to nullptr.
 */

WatchedByList::~WatchedByList()
{
	nullWatchers();

	if (m_list)
	{
		deleteList(m_list);
		m_list = nullptr;
	}
}

// ----------------------------------------------------------------------

void WatchedByList::nullWatchers()
{
	if (m_list)
	{
		const List::iterator end = m_list->end();
		for (List::iterator i = m_list->begin(); i != end; ++i)
			(*i)->reset();

		m_list->clear();
	}
}

// ----------------------------------------------------------------------
/**
 * Add a watcher to the WatchedByList.
 *
 * If the list hasn't yet been created, create it.
 *
 * @internal
 */

void WatchedByList::add(BaseWatcher &watcher)
{
	if (!m_list)
		m_list = newList();
	m_list->push_back(&watcher);
}

// ----------------------------------------------------------------------
/**
 * Remove a Watcher from the WatchedByList.
 *
 * @internal
 * @todo Consider resizing the vector down to conserve memory.
 * @todo Cconsider deleting the vector if it becomes empty.
 */

void WatchedByList::remove(BaseWatcher &watcher)
{
	DEBUG_FATAL(!m_list, ("remove from empty list"));

	const List::iterator end = m_list->end();
	List::iterator i = m_list->begin();
	for (; i != end && *i != &watcher; ++i)
		{}

	DEBUG_FATAL(i == end, ("did not find watcher on list"));

	// move the last element down to this slot
	*i = *(end-1);
	m_list->pop_back();
}

// ======================================================================
