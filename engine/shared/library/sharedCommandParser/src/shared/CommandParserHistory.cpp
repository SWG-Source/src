// ======================================================================
//
// CommandParserHistory.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCommandParser/FirstSharedCommandParser.h"
#include "sharedCommandParser/CommandParserHistory.h"

#include <deque>
#include <string>

// ======================================================================

CommandParserHistory::CommandParserHistory () :
m_index (-1),
m_history (NON_NULL (new HistoryList_t)),
m_maxSize (50)
{
}

//----------------------------------------------------------------------

CommandParserHistory::~CommandParserHistory ()
{
	delete m_history;
	m_history = 0;
}

//-----------------------------------------------------------------

bool CommandParserHistory::back (Unicode::String & result)
{
	if (static_cast<size_t> (m_index + 1) >= m_history->size ())
		return false;

	++m_index;

	result = (*m_history) [static_cast<size_t> (m_index)];

	return true;

}
//-----------------------------------------------------------------

bool CommandParserHistory::forward (Unicode::String & result)
{
	if (m_index - 1 < -1)
		return false;

	if (--m_index < 0)
		return false;

	result = (*m_history) [static_cast<size_t> (m_index)];

	return true;
}

//-----------------------------------------------------------------

void CommandParserHistory::push (const Unicode::String & cmd)
{
	//-- don't push identical commands back-to-back
	if (!m_history->empty ())
	{
		if (m_history->front () == cmd)
		{
			m_index = -1;
			return;
		}
	}

	m_history->push_front (cmd);
	if (m_history->size () > m_maxSize)
		m_history->pop_back ();

	m_index = -1;
}

//-----------------------------------------------------------------

bool CommandParserHistory::getHistoryCommandByAbbrev (const Unicode::String & abbrev, Unicode::String & cmd) const
{
	const size_t len = abbrev.length ();
	const HistoryList_t::const_iterator end = m_history->end ();

	for (HistoryList_t::const_iterator it = m_history->begin (); it != end; ++it)
	{
		if ((*it).compare (0, len, abbrev) == 0)
		{
			cmd = (*it);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------

bool CommandParserHistory::front (Unicode::String & cmd)
{
	if (m_history->empty ())
		return false;

	cmd = (*m_history) [0];
	return true;
}

// ======================================================================
