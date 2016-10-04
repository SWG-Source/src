#include "ChatAvatarCore.h"
#include "ChatAvatar.h"

namespace ChatSystem
{
	using namespace Plat_Unicode;
	using namespace Base;

	ChatAvatarCore::ChatAvatarCore()
		: m_inboxLimit(0),
		m_loginPriority(0),
		m_userID(0),
		m_avatarID(0),
		m_serverID(0),
		m_gatewayID(0),
		m_attributes(0)
	{
	}

	ChatAvatarCore::ChatAvatarCore(unsigned avatarID, unsigned userID, const unsigned short *name, const unsigned short *address, const unsigned short *gateway, const unsigned short *server, unsigned gatewayID, unsigned serverID, const unsigned short *loginLocation, unsigned attributes)
		: m_name(name),
		m_address(address),
		m_server(server),
		m_gateway(gateway),
		m_loginLocation(loginLocation),
		m_attributes(attributes),
		m_inboxLimit(0),
		m_loginPriority(0),
		m_userID(userID),
		m_avatarID(avatarID),
		m_serverID(serverID),
		m_gatewayID(gatewayID)
	{
	}

	ChatAvatarCore::ChatAvatarCore(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID, const ChatUnicodeString &loginLocation, unsigned attributes)
		: m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_server(server.string_data, server.string_length),
		m_gateway(gateway.string_data, gateway.string_length),
		m_loginLocation(loginLocation.string_data, loginLocation.string_length),
		m_attributes(attributes),
		m_inboxLimit(0),
		m_loginPriority(0),
		m_userID(userID),
		m_avatarID(avatarID),
		m_serverID(serverID),
		m_gatewayID(gatewayID)
	{
	}

	ChatAvatarCore::ChatAvatarCore(unsigned avatarID, unsigned userID, const String &name, const String &address, const String &gateway, const String &server, unsigned gatewayID, unsigned serverID, const Plat_Unicode::String &loginLocation, unsigned attributes)
		: m_name(name),
		m_address(address),
		m_server(server),
		m_gateway(gateway),
		m_loginLocation(loginLocation),
		m_attributes(attributes),
		m_inboxLimit(0),
		m_loginPriority(0),
		m_userID(userID),
		m_avatarID(avatarID),
		m_serverID(serverID),
		m_gatewayID(gatewayID)
	{
	}

	ChatAvatarCore::ChatAvatarCore(ByteStream::ReadIterator &iter)
	{
		get(iter, m_avatarID);
		get(iter, m_userID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_name));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_address));
		get(iter, (uint32 &)m_attributes);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_loginLocation));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_server));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_gateway));
		get(iter, m_serverID);
		get(iter, m_gatewayID);

		m_loginPriority = 0;
		m_inboxLimit = 0;
	}

	ChatAvatarCore::ChatAvatarCore(const ChatAvatarCore &rhs)
		: m_name(rhs.m_name),
		m_address(rhs.m_address),
		m_server(rhs.m_server),
		m_gateway(rhs.m_gateway),
		m_loginLocation(rhs.m_loginLocation),
		m_email(rhs.m_email),
		m_statusMessage(rhs.m_statusMessage),
		m_attributes(rhs.m_attributes),
		m_inboxLimit(rhs.m_inboxLimit),
		m_loginPriority(rhs.m_loginPriority),
		m_userID(rhs.m_userID),
		m_avatarID(rhs.m_avatarID),
		m_serverID(rhs.m_serverID),
		m_gatewayID(rhs.m_gatewayID)
	{
	}

	ChatAvatarCore &ChatAvatarCore::operator=(const ChatAvatarCore &rhs)
	{
		m_name = rhs.m_name;
		m_address = rhs.m_address;
		m_userID = rhs.m_userID;
		m_avatarID = rhs.m_avatarID;
		m_server = rhs.m_server;
		m_gateway = rhs.m_gateway;
		m_serverID = rhs.m_serverID;
		m_gatewayID = rhs.m_gatewayID;
		m_attributes = rhs.m_attributes;
		m_inboxLimit = rhs.m_inboxLimit;
		m_loginPriority = rhs.m_loginPriority;
		m_loginLocation = rhs.m_loginLocation;
		m_email = rhs.m_email;
		m_inboxLimit = rhs.m_inboxLimit;
		m_statusMessage = rhs.m_statusMessage;

		return(*this);
	}

	ChatAvatar *ChatAvatarCore::getNewChatAvatar() const
	{
		ChatUnicodeString addr(m_address.data(), m_address.size());
		ChatUnicodeString name(m_name.data(), m_name.size());
		ChatUnicodeString gateway(m_gateway.data(), m_gateway.size());
		ChatUnicodeString server(m_server.data(), m_server.size());

		ChatAvatar *newChatAvatar = new ChatAvatar(m_avatarID,
			m_userID,
			name,
			addr,
			gateway,
			server,
			m_gatewayID,
			m_serverID,
			m_loginLocation,
			m_attributes);

		newChatAvatar->setLoginPriority(m_loginPriority);
		newChatAvatar->setInboxLimit(m_inboxLimit);
		newChatAvatar->setForwardingEmail(m_email);
		newChatAvatar->setStatusMessage(m_statusMessage);

		return (newChatAvatar);
	}

	void ChatAvatarCore::serialize(Base::ByteStream &msg)
	{
		put(msg, m_avatarID);
		put(msg, m_userID);
		put(msg, m_name);
		put(msg, m_address);
		put(msg, (uint32)m_attributes);
		put(msg, m_loginLocation);
	}

	void ChatAvatarCore::setAttributes(unsigned long attributes)
	{
		m_attributes = attributes;
	}

	void ChatAvatarCore::setLoginPriority(int loginPriority)
	{
		m_loginPriority = loginPriority;
	}

	void ChatAvatarCore::setEmail(const Plat_Unicode::String email)
	{
		m_email = email;
	}

	void ChatAvatarCore::setInboxLimit(unsigned inboxLimit)
	{
		m_inboxLimit = inboxLimit;
	}

	void ChatAvatarCore::setStatusMessage(const Plat_Unicode::String &statusMessage)
	{
		m_statusMessage = statusMessage;
	}
};