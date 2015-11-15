// ======================================================================
//
// ClientNotificationBoxMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientNotificationBoxMessage.h"

const char * const ClientNotificationBoxMessage::MessageType = "ClientNotificationBoxMessage";

ClientNotificationBoxMessage::ClientNotificationBoxMessage(
	int sequenceId,
	NetworkId player,
	Unicode::String const &contents,
	bool useNotificationIcon,
	int iconStyle,
	float timeout,
	int channel,
	std::string const & sound) :
GameNetworkMessage("ClientNotificationBoxMessage"),
m_sequenceId(sequenceId),
m_player(player),
m_contents(contents),
m_useNotificationIcon(useNotificationIcon),
m_iconStyle(iconStyle),
m_timeout(timeout),
m_channel(channel),
m_sound(sound)
{
	AutoByteStream::addVariable(m_sequenceId);
	AutoByteStream::addVariable(m_player);
	AutoByteStream::addVariable(m_contents);
	AutoByteStream::addVariable(m_useNotificationIcon);
	AutoByteStream::addVariable(m_iconStyle);
	AutoByteStream::addVariable(m_timeout);
	AutoByteStream::addVariable(m_channel);
	AutoByteStream::addVariable(m_sound);
}


ClientNotificationBoxMessage::ClientNotificationBoxMessage(Archive::ReadIterator &source) :
GameNetworkMessage("ClientNotificationBoxMessage"),
m_sequenceId(0),
m_player(),
m_contents(),
m_useNotificationIcon(0),
m_iconStyle(0),
m_timeout(0),
m_channel(0),
m_sound()
{
	AutoByteStream::addVariable(m_sequenceId);
	AutoByteStream::addVariable(m_player);
	AutoByteStream::addVariable(m_contents);
	AutoByteStream::addVariable(m_useNotificationIcon);
	AutoByteStream::addVariable(m_iconStyle);
	AutoByteStream::addVariable(m_timeout);
	AutoByteStream::addVariable(m_channel);
	AutoByteStream::addVariable(m_sound);

	unpack(source);
}
