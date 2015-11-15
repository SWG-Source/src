// ======================================================================
//
// CancelTicketResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CancelTicketResponseMessage_H
#define	_CancelTicketResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class CancelTicketResponseMessage : public GameNetworkMessage
{
public:
	CancelTicketResponseMessage(int32 result, unsigned int ticketId);
	explicit CancelTicketResponseMessage(Archive::ReadIterator &source);

	~CancelTicketResponseMessage();

public: // methods

	int32                            getResult() const;
	unsigned int                     getTicketId() const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoVariable<unsigned int>   m_ticketId;  
};

// ----------------------------------------------------------------------

inline int32 CancelTicketResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

inline unsigned int CancelTicketResponseMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

#endif // _CancelTicketResponseMessage_H

