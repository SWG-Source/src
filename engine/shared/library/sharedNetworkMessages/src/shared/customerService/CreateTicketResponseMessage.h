// ======================================================================
//
// CreateTicketResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateTicketResponseMessage_H
#define	_CreateTicketResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class CreateTicketResponseMessage : public GameNetworkMessage
{
public:
	CreateTicketResponseMessage(int32 result, unsigned int ticketId);
	explicit CreateTicketResponseMessage(Archive::ReadIterator &source);

	~CreateTicketResponseMessage();

public: // methods

	int32                            getResult() const;
	unsigned int                     getTicketId() const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoVariable<unsigned int>   m_ticketId;  
};

// ----------------------------------------------------------------------

inline int32 CreateTicketResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

inline unsigned int CreateTicketResponseMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

#endif // _CreateTicketResponseMessage_H

