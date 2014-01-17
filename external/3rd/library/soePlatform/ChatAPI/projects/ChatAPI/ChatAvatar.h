#if !defined (CHATAVATAR_H_)
#define CHATAVATAR_H_

#pragma warning (disable : 4786)

#include <string>

#include "ChatEnum.h"

namespace ChatSystem 
{

	enum AvatarAttributes
	{
		AVATARATTR_INVISIBLE    = 1 << 0,
		AVATARATTR_GM           = 1 << 1,
		AVATARATTR_SUPERGM      = 1 << 2,
		AVATARATTR_SUPERSNOOP   = 1 << 3,
        AVATARATTR_EXTENDED     = 1 << 4
	};

	class ChatAvatarCore;

	// class ChatAvatar
	//
	// Description:
	//    This class stores the information about an avatar in the ChatSystem that
	//    is relevant to the API.
	//
	// Usage:
	//    Typically a ChatAvatar will originate from an API callback method.
	//    The ChatAvatar object can be copied for local storage and the data
	//    members can be retrieved from the available get__ methods below.
	class ChatAvatar
	{
	public:
		ChatAvatar();
		ChatAvatar(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID);
		ChatAvatar(unsigned avatarID, unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &gateway, const ChatUnicodeString &server, unsigned gatewayID, unsigned serverID, const ChatUnicodeString &loginLocation, unsigned attributes);
		ChatAvatar(const ChatAvatar &rhs);
		~ChatAvatar();

		ChatAvatar &operator=(const ChatAvatar &rhs);

		const ChatUnicodeString &getName() const;
		const ChatUnicodeString &getAddress() const;

		unsigned getUserID() const;
		unsigned getAvatarID() const;

		const ChatUnicodeString &getServer() const;
		const ChatUnicodeString &getGateway() const;

		unsigned getServerID() const;
		unsigned getGatewayID() const;

		const ChatUnicodeString &getLoginLocation() const;
		int getLoginPriority() const;
		unsigned getAttributes() const;
		unsigned getInboxLimit() const;
		const ChatUnicodeString &getForwardingEmail() const;
        const ChatUnicodeString& getStatusMessage() const;

		void setAttributes(unsigned long attributes);
		void setLoginPriority(int loginPriority);
		void setInboxLimit(unsigned inboxLimit);
		void setForwardingEmail(const ChatUnicodeString &forwardingEmail);
        void setStatusMessage(const ChatUnicodeString& statusMessage);

		static ChatUnicodeString CreateAddressFromName(const ChatUnicodeString & name, const std::string & gameCode);

	private:
		ChatAvatarCore *m_core;
		mutable ChatUnicodeString m_cServer;
		mutable ChatUnicodeString m_cGateway;
		mutable ChatUnicodeString m_cName;
		mutable ChatUnicodeString m_cAddress;
		mutable ChatUnicodeString m_cLoginLocation;
		mutable ChatUnicodeString m_cForwardingEmail;
        mutable ChatUnicodeString m_cStatusMessage;
	};
};

#endif

