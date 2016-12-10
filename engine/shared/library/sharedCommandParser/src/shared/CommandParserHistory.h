// ======================================================================
//
// CommandParserHistory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandParserHistory_H
#define INCLUDED_CommandParserHistory_H

// ======================================================================

class CommandParserHistory
{
public:

	typedef std::deque<Unicode::String>   HistoryList_t;

	                         CommandParserHistory ();
	                        ~CommandParserHistory ();

	bool                     back    (Unicode::String & result);
	bool                     forward (Unicode::String & result);

	size_t                   getMaxSize () const;
	void                     setMaxSize (size_t newSize);
	void                     resetIndex ();

	void                     push  (const Unicode::String & cmd);
	bool                     front (Unicode::String & cmd);

	const HistoryList_t &    getHistoryList () const;

	bool                     getHistoryCommandByAbbrev (const Unicode::String & abbrev, Unicode::String & cmd) const;


private:
	                         CommandParserHistory (const CommandParserHistory & rhs);
	CommandParserHistory &   operator= (const CommandParserHistory & rhs);

private:

	/** -1 means no index into history, 0 is most recent command */

	int                      m_index;
	HistoryList_t *          m_history;
	size_t                   m_maxSize;

};
//-----------------------------------------------------------------

inline size_t CommandParserHistory::getMaxSize (void) const
{
	return m_maxSize;
}
//-----------------------------------------------------------------
inline void CommandParserHistory::setMaxSize (const size_t size)
{
	m_maxSize = size;
}
//-----------------------------------------------------------------
inline void CommandParserHistory::resetIndex (void)
{
	m_index = -1;
}
//-----------------------------------------------------------------

inline const CommandParserHistory::HistoryList_t & CommandParserHistory::getHistoryList () const
{
	NOT_NULL (m_history);
	return *m_history;
}
// ======================================================================

#endif
