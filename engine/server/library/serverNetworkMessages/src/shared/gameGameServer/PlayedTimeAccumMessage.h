// ======================================================================
//
// PlayedTimeAccumMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_PlayedTimeAccumMessage_H
#define	_INCLUDED_PlayedTimeAccumMessage_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

class PlayedTimeAccumMessage: public GameNetworkMessage
{
public:
	PlayedTimeAccumMessage(NetworkId const &networkId, float playedTimeAccum);
	PlayedTimeAccumMessage(Archive::ReadIterator &source);

	NetworkId const &getNetworkId() const;
	float getPlayedTimeAccum() const;
	
private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<float>     m_playedTimeAccum;

private:
	PlayedTimeAccumMessage(PlayedTimeAccumMessage const &);
	PlayedTimeAccumMessage &operator=(PlayedTimeAccumMessage const &);
};

// ======================================================================

inline NetworkId const &PlayedTimeAccumMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline float PlayedTimeAccumMessage::getPlayedTimeAccum() const
{
	return m_playedTimeAccum.get();
}

// ======================================================================

#endif	// _INCLUDED_PlayedTimeAccumMessage_H

