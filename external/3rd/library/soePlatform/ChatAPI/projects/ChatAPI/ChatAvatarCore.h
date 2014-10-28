#if !defined (CHATAVATARCORE_H_)
#define CHATAVATARCORE_H_

#pragma warning (disable : 4786)

#include <Base/Archive.h>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#include "ChatEnum.h"

namespace ChatSystem 
{


	class ChatAvatar;

	class ChatAvatarCore
	{
	public:
		ChatAvatarCore();
		ChatAvatarCore(Base::ByteStream::ReadIterator &iter);
		ChatAvatarCore(unsigned avatarID, unsigned userID, const unsigned short *name, const unsigned short *address, const unsigned short *gateway, const unsigned short *server, unsigned gatewayID, unsigned serverID, const unsigned short *loginLocation, unsigned attributes);
		ChatAvatarCore(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID, const ChatUnicodeString &loginLocation, unsigned attributes);
		ChatAvatarCore(unsigned avatarID, unsigned userID, const Plat_Unicode::String &name, const Plat_Unicode::String &address, const Plat_Unicode::String &gateway, const Plat_Unicode::String &server, unsigned gatewayID, unsigned serverID, const Plat_Unicode::String &loginLocation, unsigned attributes);


		ChatAvatarCore(const ChatAvatarCore &rhs);
		~ChatAvatarCore() {};

		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
		unsigned getUserID() const { return m_userID; }
		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getLoginLocation() const { return m_loginLocation; }
		int getLoginPriority() const { return m_loginPriority; }
		unsigned getAttributes() const { return m_attributes; }
		const Plat_Unicode::String &getEmail() const { return m_email; }
		unsigned getInboxLimit() const { return m_inboxLimit; }
		const Plat_Unicode::String &getServer() const { return m_server; }
		const Plat_Unicode::String &getGateway() const { return m_gateway; }
		unsigned getServerID() const { return m_serverID; } 
		unsigned getGatewayID() const { return m_gatewayID; }
        const Plat_Unicode::String& getStatusMessage() { return m_statusMessage; }

		ChatAvatar *getNewChatAvatar() const;

		void setAttributes(unsigned long attributes);
		void setLoginPriority(int loginPriority);
		void setEmail(const Plat_Unicode::String email);
		void setInboxLimit(unsigned inboxLimit);
        void setStatusMessage(const Plat_Unicode::String &statusMessage);

		ChatAvatarCore &operator=(const ChatAvatarCore &rhs);

		void serialize(Base::ByteStream &msg);
	private:
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
		Plat_Unicode::String m_server;
		Plat_Unicode::String m_gateway;
		Plat_Unicode::String m_loginLocation;
		Plat_Unicode::String m_email;
        Plat_Unicode::String m_statusMessage;

		unsigned long m_attributes;
		unsigned long m_inboxLimit;
		int m_loginPriority;

		unsigned m_userID;
		unsigned m_avatarID;
		unsigned m_serverID;
		unsigned m_gatewayID;
	};

};

#endif

