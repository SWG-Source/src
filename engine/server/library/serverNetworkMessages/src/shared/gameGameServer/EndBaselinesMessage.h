// ======================================================================
//
// EndBaselinesMessage.h - tells Gameserver object baseline data has ended.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_EndBaselinesMessage_H
#define	_INCLUDED_EndBaselinesMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class EndBaselinesMessage: public GameNetworkMessage
{
public:
	explicit EndBaselinesMessage(NetworkId const & id);
	EndBaselinesMessage(Archive::ReadIterator &source);
	~EndBaselinesMessage();

	NetworkId const &getId() const;

private:
	Archive::AutoVariable<NetworkId> m_id;		// id of object we are updating

	EndBaselinesMessage();
	EndBaselinesMessage(EndBaselinesMessage const &);
	EndBaselinesMessage &operator=(EndBaselinesMessage const &);
};

// ----------------------------------------------------------------------

inline NetworkId const &EndBaselinesMessage::getId() const
{
	return m_id.get();
}

// ======================================================================

#endif	// _INCLUDED_EndBaselinesMessage_H

