// ======================================================================
//
// AppendCommentResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AppendCommentResponseMessage_H
#define	_AppendCommentResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class AppendCommentResponseMessage : public GameNetworkMessage
{
public:
	AppendCommentResponseMessage(int32 result, unsigned int ticketId);
	explicit AppendCommentResponseMessage(Archive::ReadIterator &source);

	~AppendCommentResponseMessage();

public: // methods

	int32                            getResult() const;
	unsigned int                     getTicketId() const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoVariable<unsigned int>   m_ticketId;  
};

// ----------------------------------------------------------------------

inline int32 AppendCommentResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

inline unsigned int AppendCommentResponseMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

#endif // _AppendCommentResponseMessage_H

