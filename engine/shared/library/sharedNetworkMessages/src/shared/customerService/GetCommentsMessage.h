// ======================================================================
//
// GetCommentsMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetCommentsMessage_H
#define	_GetCommentsMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class GetCommentsMessage : public GameNetworkMessage
{
public:

	GetCommentsMessage(unsigned int ticketId);
	explicit GetCommentsMessage(Archive::ReadIterator &source);

	~GetCommentsMessage();

public: 

	unsigned int                          getTicketId() const;

private: 
	Archive::AutoVariable<unsigned int>    m_ticketId;  
};

// ----------------------------------------------------------------------

inline unsigned int GetCommentsMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

#endif // _GetCommentsMessage_H

