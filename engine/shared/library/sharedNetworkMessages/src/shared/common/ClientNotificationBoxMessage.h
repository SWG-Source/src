// ClientNotificationBoxMessage.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.

#ifndef	_INCLUDED_ClientNotificationBoxMessage_H
#define	_INCLUDED_ClientNotificationBoxMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class ClientNotificationBoxMessage : public GameNetworkMessage
{

public:

	static const char * const MessageType;

	enum IconStyle
	{
		IS_NONE,
		IS_QUESTION,
		IS_EXCLAMATION,
		IS_MAX
	};

	enum Channel
	{
		NC_HELP,
		NC_SPECIAL_CANCEL = 0xFF,
		NC_SPECIAL_CANCEL_ALL = 0xFE,
		NC_MAX
	};


	ClientNotificationBoxMessage( int sequenceId, NetworkId player, Unicode::String const & contents, bool useNotificationIcon, int iconStyle, float timeout, int channel, std::string const & sound );

	explicit ClientNotificationBoxMessage(Archive::ReadIterator & source);

public:
	int getSequenceId() const;
	const NetworkId &getPlayer() const;
	Unicode::String const &getContents() const;
	bool getUseNotificationIcon() const;
	int getIconStyle() const;
	float getTimeout() const;
	int getChannel() const;
	std::string const & getSound() const;
private:
	Archive::AutoVariable<int>                       m_sequenceId;
	Archive::AutoVariable<NetworkId>                 m_player;
	Archive::AutoVariable<Unicode::String>           m_contents;
	Archive::AutoVariable<bool>                      m_useNotificationIcon;
	Archive::AutoVariable<int>                       m_iconStyle;
	Archive::AutoVariable<float>                     m_timeout;
	Archive::AutoVariable<int>                       m_channel;
	Archive::AutoVariable<std::string>               m_sound;
};

inline int ClientNotificationBoxMessage::getSequenceId() const
{
	return m_sequenceId.get();
}

inline const NetworkId &ClientNotificationBoxMessage::getPlayer() const
{
	return m_player.get();
}

inline Unicode::String const & ClientNotificationBoxMessage::getContents() const
{
	return m_contents.get();
}

inline bool ClientNotificationBoxMessage::getUseNotificationIcon() const
{
	return m_useNotificationIcon.get();
}

inline int ClientNotificationBoxMessage::getIconStyle() const
{
	return m_iconStyle.get();
}

inline float ClientNotificationBoxMessage::getTimeout() const
{
	return m_timeout.get();
}

inline int ClientNotificationBoxMessage::getChannel() const
{
	return m_channel.get();
}

inline std::string const & ClientNotificationBoxMessage::getSound() const
{
	return m_sound.get();
}

#endif // _INCLUDED_MessageQueueCommandTimer_H
