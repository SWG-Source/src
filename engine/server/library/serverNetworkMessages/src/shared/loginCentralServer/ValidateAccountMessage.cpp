// ======================================================================
//
// ValidateAccountMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ValidateAccountMessage.h"

// ======================================================================

ValidateAccountMessage::ValidateAccountMessage (StationId stationId, unsigned int track, uint32 subscriptionBits) :
	GameNetworkMessage("ValidateAccountMessage"),
	m_stationId(stationId),
	m_track(track),
	m_subscriptionBits(subscriptionBits)
{
	addVariable(m_stationId);
	addVariable(m_track);
	addVariable(m_subscriptionBits);
}

// ----------------------------------------------------------------------

ValidateAccountMessage::ValidateAccountMessage (Archive::ReadIterator & source) :
	GameNetworkMessage("ValidateAccountMessage"),
	m_stationId(),
	m_track(),
	m_subscriptionBits()
{
	addVariable(m_stationId);
	addVariable(m_track);
	addVariable(m_subscriptionBits);

	AutoByteStream::unpack(source);
}

// ----------------------------------------------------------------------

ValidateAccountMessage::~ValidateAccountMessage ()
{
}

// ----------------------------------------------------------------------

unsigned int ValidateAccountMessage::getTrack() const
{
	return m_track.get();
}

// ----------------------------------------------------------------------

uint32 ValidateAccountMessage::getSubscriptionBits() const
{
	return m_subscriptionBits.get();
}

// ======================================================================
