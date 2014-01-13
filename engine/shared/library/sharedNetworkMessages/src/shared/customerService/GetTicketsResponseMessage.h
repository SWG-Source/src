// ======================================================================
//
// GetTicketsResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetTicketsResponseMessage_H
#define	_GetTicketsResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "sharedNetworkMessages/CustomerServiceTicketArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GetTicketsResponseMessage : public GameNetworkMessage
{
public:
	GetTicketsResponseMessage(int32 result, unsigned int totalTickets, const std::vector<CustomerServiceTicket> &tickets );
	explicit GetTicketsResponseMessage(Archive::ReadIterator &source);

	~GetTicketsResponseMessage();

public: // methods

	int32                            getResult()          const;
	unsigned int                     getTotalNumTickets() const;
	const std::vector<CustomerServiceTicket> &    getTickets()         const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoVariable<unsigned int>   m_totalNumTickets;  
	Archive::AutoArray<CustomerServiceTicket>          m_tickets;   
};

// ----------------------------------------------------------------------

inline const std::vector<CustomerServiceTicket> & GetTicketsResponseMessage::getTickets() const
{
	return m_tickets.get();
}

// ----------------------------------------------------------------------

inline int32 GetTicketsResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

inline unsigned int GetTicketsResponseMessage::getTotalNumTickets() const
{
	return m_totalNumTickets.get();
}

// ----------------------------------------------------------------------

#endif // _GetTicketsResponseMessage_H

