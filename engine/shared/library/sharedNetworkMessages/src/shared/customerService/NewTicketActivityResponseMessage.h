// ======================================================================
//
// NewTicketActivityResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_NewTicketActivityResponseMessage_H
#define	INCLUDED_NewTicketActivityResponseMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class NewTicketActivityResponseMessage : public GameNetworkMessage
{
public:

	explicit NewTicketActivityResponseMessage(bool const newActivity, unsigned int const ticketCount);
	explicit NewTicketActivityResponseMessage(Archive::ReadIterator &source);

	~NewTicketActivityResponseMessage();

	bool         isNewActivity() const;
	unsigned int getTicketCount() const;

private:

	Archive::AutoVariable<bool>         m_newActivity;  
	Archive::AutoVariable<unsigned int> m_ticketCount;  
};

// ----------------------------------------------------------------------

inline bool NewTicketActivityResponseMessage::isNewActivity() const
{
	return m_newActivity.get();
}

// ----------------------------------------------------------------------

inline unsigned int NewTicketActivityResponseMessage::getTicketCount() const
{
	return m_ticketCount.get();
}

// ----------------------------------------------------------------------

#endif // INCLUDED_NewTicketActivityResponseMessage_H
