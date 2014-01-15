// ======================================================================
//
// TransferAccountData.cpp
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

TransferAccountData::TransferAccountData(const TransferAccountData & rhs) :
m_track(rhs.m_track),
m_sourceStationId(rhs.m_sourceStationId),
m_destinationStationId(rhs.m_destinationStationId),
m_transactionId(rhs.m_transactionId),
m_startGalaxy(rhs.m_startGalaxy),
m_destinationHasAvatars(rhs.m_destinationHasAvatars),
m_sourceAvatarData(rhs.m_sourceAvatarData)
{
}

//-----------------------------------------------------------------------

TransferAccountData::TransferAccountData() :
m_track(0),
m_sourceStationId(0),
m_destinationStationId(0),
m_transactionId(0),
m_startGalaxy(),
m_destinationHasAvatars(),
m_sourceAvatarData()
{
}

//-----------------------------------------------------------------------

TransferAccountData::~TransferAccountData()
{
}

//-----------------------------------------------------------------------

unsigned int TransferAccountData::getTrack() const
{
	return m_track;
}

//-----------------------------------------------------------------------

unsigned int TransferAccountData::getSourceStationId() const
{
	return m_sourceStationId;
}

//-----------------------------------------------------------------------

unsigned int TransferAccountData::getDestinationStationId() const
{
	return m_destinationStationId;
}

//-----------------------------------------------------------------------

unsigned int TransferAccountData::getTransactionId() const
{
	return m_transactionId;
}

//-----------------------------------------------------------------------

const std::vector<AvatarData> & TransferAccountData::getSourceAvatarData() const
{
	return m_sourceAvatarData;
}

//-----------------------------------------------------------------------

const std::string & TransferAccountData::getStartGalaxy() const
{
	return m_startGalaxy;
}

//-----------------------------------------------------------------------

bool TransferAccountData::getDestinationHasAvatars() const
{
	return m_destinationHasAvatars;
}

//-----------------------------------------------------------------------

std::string TransferAccountData::toString() const
{
	char buf[64] = {"\0"};

	std::string result;

	result += "startGalaxy: ";
	result += getStartGalaxy();

	result += " | sourceStationId: ";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getSourceStationId());
	result += buf;

	result += " | destinationStationId: ";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getDestinationStationId());
	result += buf;

	return result;
}

//-----------------------------------------------------------------------

void TransferAccountData::setDestinationStationId(const unsigned int destinationStationId)
{
	m_destinationStationId = destinationStationId;
}

//-----------------------------------------------------------------------

void TransferAccountData::setSourceStationId(const unsigned int sourceStationId)
{
	m_sourceStationId = sourceStationId;
}

//-----------------------------------------------------------------------

void TransferAccountData::setTrack(const unsigned int track)
{
	m_track = track;
}

//-----------------------------------------------------------------------

void TransferAccountData::setTransactionId(const unsigned int transactionId)
{
	m_transactionId = transactionId;
}

//-----------------------------------------------------------------------

void TransferAccountData::setSourceAvatarData(const std::vector<AvatarData> & avatarData)
{
	m_sourceAvatarData = avatarData;
}

//-----------------------------------------------------------------------

void TransferAccountData::setStartGalaxy(const std::string & galaxy)
{
	m_startGalaxy = galaxy;
}

//-----------------------------------------------------------------------

void TransferAccountData::setDestinationHasAvatars(const bool avatars)
{
	m_destinationHasAvatars = avatars;
}

//-----------------------------------------------------------------------
