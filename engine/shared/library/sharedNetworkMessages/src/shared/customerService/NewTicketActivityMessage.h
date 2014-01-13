// ======================================================================
//
// NewTicketActivityMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_NewTicketActivityMessage_H
#define	INCLUDED_NewTicketActivityMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class NewTicketActivityMessage : public GameNetworkMessage
{
public:

	NewTicketActivityMessage();
	explicit NewTicketActivityMessage(Archive::ReadIterator &source);

	~NewTicketActivityMessage();

	void         setStationId(unsigned int id);
	unsigned int getStationId() const;

private:

	Archive::AutoVariable<unsigned int> m_stationId;  

};

// ----------------------------------------------------------------------

inline unsigned int NewTicketActivityMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void NewTicketActivityMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

#endif // INCLUDED_NewTicketActivityMessage_H

