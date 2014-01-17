#if !defined (CHATFRIENDSTATUS_H_)
#define CHATFRIENDSTATUS_H_

#include <stdlib.h>

#include "ChatEnum.h"

namespace ChatSystem 
{
	class ChatFriendStatusCore;

	class ChatFriendStatus
	{
	public:
		ChatFriendStatus();
		~ChatFriendStatus();
		const ChatUnicodeString &getName() const;
		const ChatUnicodeString &getAddress() const;
		const ChatUnicodeString &getComment() const;
		short getStatus() const; // 0 = offline, else = online
		
		friend class ChatFriendStatusCore;
	private:
		ChatUnicodeString m_name;
		ChatUnicodeString m_address;
		ChatUnicodeString m_comment;

		ChatFriendStatusCore *m_core;
	};
};

#endif


