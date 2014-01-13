// ======================================================================
//
// AppendCommentMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AppendCommentMessage_H
#define	_AppendCommentMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class AppendCommentMessage : public GameNetworkMessage
{
public:

	AppendCommentMessage(unsigned int ticketId, const std::string &characterName, const Unicode::String &comment);
	explicit AppendCommentMessage(Archive::ReadIterator &source);

	~AppendCommentMessage();

public: 

	unsigned int                          getTicketId() const;
	const std::string &                   getCharacterName() const;
	const Unicode::String &               getComment() const;
	unsigned int                          getStationId() const;
	void                                  setStationId(unsigned int id);

private: 
	Archive::AutoVariable<unsigned int>    m_ticketId;  
	Archive::AutoVariable<std::string>     m_characterName;  
	Archive::AutoVariable<Unicode::String> m_comment;  
	Archive::AutoVariable<unsigned int>    m_stationId;  
};

// ----------------------------------------------------------------------

inline const std::string & AppendCommentMessage::getCharacterName() const
{
	return m_characterName.get();
}

// ----------------------------------------------------------------------

inline unsigned int AppendCommentMessage::getTicketId() const
{
	return m_ticketId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AppendCommentMessage::getComment() const
{
	return m_comment.get();
}

// ----------------------------------------------------------------------

inline unsigned int AppendCommentMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void AppendCommentMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

#endif // _AppendCommentMessage_H

