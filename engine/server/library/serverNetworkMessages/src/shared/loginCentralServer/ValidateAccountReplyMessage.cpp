// ======================================================================
//
// ValidateAccountReplyMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ValidateAccountReplyMessage.h"

// ======================================================================

ValidateAccountReplyMessage::ValidateAccountReplyMessage (StationId stationId, bool canLogin, bool canCreateRegular, bool canCreateJedi, bool canSkipTutorial, unsigned int track, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems) :
GameNetworkMessage("ValidateAccountReplyMessage"),
m_stationId(stationId),
m_canLogin(canLogin),
m_canCreateRegular(canCreateRegular),
m_canCreateJedi(canCreateJedi),
m_canSkipTutorial(canSkipTutorial),
m_track(track),
m_consumedRewardEvents(),
m_claimedRewardItems()
{
	m_consumedRewardEvents.set(consumedRewardEvents);
	m_claimedRewardItems.set(claimedRewardItems);
	
	addVariable(m_stationId);
	addVariable(m_canLogin);
	addVariable(m_canCreateRegular);
	addVariable(m_canCreateJedi);
	addVariable(m_canSkipTutorial);
	addVariable(m_track);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
}

// ----------------------------------------------------------------------

ValidateAccountReplyMessage::ValidateAccountReplyMessage (Archive::ReadIterator & source) :
GameNetworkMessage("ValidateAccountReplyMessage"),
m_stationId(),
m_canLogin(),
m_canCreateRegular(),
m_canCreateJedi(),
m_canSkipTutorial(),
m_track(),
m_consumedRewardEvents(),
m_claimedRewardItems()
{
	addVariable(m_stationId);
	addVariable(m_canLogin);
	addVariable(m_canCreateRegular);
	addVariable(m_canCreateJedi);
	addVariable(m_canSkipTutorial);
	addVariable(m_track);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
	AutoByteStream::unpack(source);
}

// ----------------------------------------------------------------------

ValidateAccountReplyMessage::~ValidateAccountReplyMessage ()
{
}

// ----------------------------------------------------------------------

unsigned int ValidateAccountReplyMessage::getTrack() const
{
	return m_track.get();
}

// ----------------------------------------------------------------------

StationId ValidateAccountReplyMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

bool ValidateAccountReplyMessage::getCanLogin() const
{
	return m_canLogin.get();
}

// ----------------------------------------------------------------------

bool ValidateAccountReplyMessage::getCanCreateRegular() const
{
	return m_canCreateRegular.get();
}

// ----------------------------------------------------------------------

bool ValidateAccountReplyMessage::getCanCreateJedi() const
{
	return m_canCreateJedi.get();
}

// ----------------------------------------------------------------------

bool ValidateAccountReplyMessage::getCanSkipTutorial() const
{
	return m_canSkipTutorial.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & ValidateAccountReplyMessage::getConsumedRewardEvents() const
{
	return m_consumedRewardEvents.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & ValidateAccountReplyMessage::getClaimedRewardItems() const
{
	return m_claimedRewardItems.get();
}

// ======================================================================
