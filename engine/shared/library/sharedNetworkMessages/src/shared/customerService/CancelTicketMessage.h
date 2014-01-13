// ======================================================================
//
// CancelTicketMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CancelTicketMessage_H
#define	_CancelTicketMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class CancelTicketMessage : public GameNetworkMessage
{
public:

	CancelTicketMessage(unsigned int ticketId, const Unicode::String &comment);
	explicit CancelTicketMessage(Archive::ReadIterator &source);

	~CancelTicketMessage();

public: 

	unsigned int                          getTicketId() const;
	const Unicode::String &               getComment() const;
	unsigned int                          getStationId() const;
	void                                  setStationId(unsigned int id);

private: 
	Archive::AutoVariable<unsigned int>    m_ticketId;  
	Archive::AutoVariable<Unicode::String> m_comment;  
	Archive::AutoVariable<unsigned int>    m_stationId;  
};

// ----------------------------------------------------------------------

inline unsigned int CancelTicketMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CancelTicketMessage::getComment() const
{
	return m_comment.get();
}

// ----------------------------------------------------------------------

inline unsigned int CancelTicketMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void CancelTicketMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

#endif // _CancelTicketMessage_H

