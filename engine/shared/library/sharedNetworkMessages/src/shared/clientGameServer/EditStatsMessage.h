// ======================================================================
//
// EditStatsMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_EditStatsMessage_H
#define	_EditStatsMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

class EditStatsMessage : public GameNetworkMessage
{
public:
	static const char * const MessageType;

public:
	explicit EditStatsMessage(const NetworkId & id);
	explicit EditStatsMessage(Archive::ReadIterator &source);

public:
	const NetworkId &                                 getTarget () const;

private:
	Archive::AutoVariable<NetworkId>                  m_target;
};

// ----------------------------------------------------------------------

inline const NetworkId & EditStatsMessage::getTarget () const
{
	return m_target.get ();
}

// ----------------------------------------------------------------------

#endif // _EditStatsMessage_H

