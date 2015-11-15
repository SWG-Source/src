#include "ChatAvatar.h"
#include "ChatAvatarCore.h"
#include "Base/MD5.h"
#include "Unicode/utf8.h"
#include <numeric>
#include <cctype>
#include <algorithm>

const unsigned MOD_NUMBER = 256;

namespace ChatSystem 
{

ChatAvatar::ChatAvatar()
: m_core(new ChatAvatarCore())
{
}


ChatAvatar::ChatAvatar(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID)
: m_core(new ChatAvatarCore(avatarID, userID, name, address, gateway, server, gatewayID, serverID, ChatUnicodeString(), 0)),
  m_cServer(m_core->getServer().data(), m_core->getServer().size()),
  m_cGateway(m_core->getGateway().data(), m_core->getGateway().size()),
  m_cName(m_core->getName().data(), m_core->getName().size()),
  m_cAddress(m_core->getAddress().data(), m_core->getAddress().size()),
  m_cLoginLocation(m_core->getLoginLocation().data(), m_core->getLoginLocation().size())
{
}

ChatAvatar::ChatAvatar(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID, const ChatUnicodeString &loginLocation, unsigned attributes)
: m_core(new ChatAvatarCore(avatarID, userID, name, address, gateway, server, gatewayID, serverID, loginLocation, attributes)),
  m_cServer(m_core->getServer().data(), m_core->getServer().size()),
  m_cGateway(m_core->getGateway().data(), m_core->getGateway().size()),
  m_cName(m_core->getName().data(), m_core->getName().size()),
  m_cAddress(m_core->getAddress().data(), m_core->getAddress().size()),
  m_cLoginLocation(m_core->getLoginLocation().data(), m_core->getLoginLocation().size())
{
}

ChatAvatar::ChatAvatar(const ChatAvatar &rhs)
: m_core(new ChatAvatarCore(*(rhs.m_core)))
{
}

ChatAvatar::~ChatAvatar()
{
	delete m_core;
}

ChatAvatar &ChatAvatar::operator=(const ChatAvatar &rhs)
{
	delete m_core;
	m_core = new ChatAvatarCore(rhs.m_core->getAvatarID(),
								rhs.m_core->getUserID(),
								rhs.m_core->getName(),
								rhs.m_core->getAddress(),
								rhs.m_core->getGateway(),
								rhs.m_core->getServer(),
								rhs.m_core->getGatewayID(),
								rhs.m_core->getServerID(),
								rhs.m_core->getLoginLocation(),
								rhs.getAttributes());

	m_cServer = m_core->getServer();
	m_cGateway= m_core->getGateway();
	m_cName = m_core->getName();
	m_cAddress = m_core->getAddress();

	return(*this);
}

const ChatUnicodeString &ChatAvatar::getName() const
{
	m_cName = m_core->getName();
	return m_cName;
}

const ChatUnicodeString &ChatAvatar::getAddress() const
{
	m_cAddress = m_core->getAddress();
	return m_cAddress;
}

unsigned ChatAvatar::getUserID() const
{
	return m_core->getUserID();
}

unsigned ChatAvatar::getAvatarID() const
{
	return m_core->getAvatarID();
}

const ChatUnicodeString &ChatAvatar::getLoginLocation() const
{
	m_cLoginLocation = m_core->getLoginLocation();

	return m_cLoginLocation;
}

int ChatAvatar::getLoginPriority() const
{ 
	return m_core->getLoginPriority(); 
}

unsigned ChatAvatar::getInboxLimit() const
{
	return m_core->getInboxLimit();
}

const ChatUnicodeString &ChatAvatar::getForwardingEmail() const
{
	m_cForwardingEmail = m_core->getEmail();

	return m_cForwardingEmail;
}

unsigned ChatAvatar::getAttributes() const
{
	return m_core->getAttributes();
}

const ChatUnicodeString &ChatAvatar::getServer() const
{
	m_cServer = m_core->getServer();
	return m_cServer;
}

const ChatUnicodeString &ChatAvatar::getGateway() const
{
	m_cGateway = m_core->getGateway();
	return m_cGateway;
}

unsigned ChatAvatar::getServerID() const
{
	return m_core->getServerID();
}

unsigned ChatAvatar::getGatewayID() const
{
	return m_core->getGatewayID();
}

const ChatUnicodeString& ChatAvatar::getStatusMessage() const
{
    m_cStatusMessage = m_core->getStatusMessage();
    return m_cStatusMessage;
}

void ChatAvatar::setAttributes(unsigned long attributes)
{
	m_core->setAttributes(attributes);
}

void ChatAvatar::setLoginPriority(int loginPriority)
{
	m_core->setLoginPriority(loginPriority);
}

void ChatAvatar::setInboxLimit(unsigned inboxLimit)
{
	m_core->setInboxLimit(inboxLimit);
}

void ChatAvatar::setForwardingEmail(const ChatUnicodeString &forwardingEmail)
{
	m_core->setEmail(forwardingEmail.data());
}

void ChatAvatar::setStatusMessage(const ChatUnicodeString& statusMessage)
{
    m_core->setStatusMessage( statusMessage.data() );
}


ChatUnicodeString ChatAvatar::CreateAddressFromName(const ChatUnicodeString & name, const std::string & gameCode)
{
	// convert to utf8
	char utf8Buff[256];
	memset(utf8Buff, 0, 256);
	Plat_Unicode::UTF16_convertToUTF8( (Plat_Unicode::UTF16 *)name.data(), utf8Buff, 256);

	std::string utf8String(utf8Buff);

	std::transform(utf8String.begin(), utf8String.end(), utf8String.begin(), tolower);

	// hash the username
	Base::MD5 md5r(utf8String);
	std::string hexName = md5r.asHex();

	// get the sum of each character
	unsigned sum = std::accumulate(hexName.begin(), hexName.end(), 0);

	// mod to a world ID.
	unsigned short worldID = (sum % MOD_NUMBER);

	char buff[256];
	memset(buff, 0, 256);
	unsigned len = 0;

	if (gameCode.length() < 230)
	{
		len = sprintf (buff, "SOE+%s+World%03d", gameCode.c_str(), worldID);
	}

	return ChatUnicodeString(buff, len);
}


}; // end ChatSystem namespace


