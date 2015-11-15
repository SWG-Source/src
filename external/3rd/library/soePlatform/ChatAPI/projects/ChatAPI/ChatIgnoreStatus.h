#if !defined (CHATIGNORESTATUS_H_)
#define CHATIGNORESTATUS_H_

#include <stdlib.h>

#include "ChatEnum.h"

namespace ChatSystem 
{
	class ChatIgnoreStatusCore;

	class ChatIgnoreStatus
	{
	public:
		ChatIgnoreStatus();
		~ChatIgnoreStatus();
		const ChatUnicodeString &getName() const { return m_name; }
		const ChatUnicodeString &getAddress() const { return m_address; }
		
		friend class ChatIgnoreStatusCore;
	private:
		ChatUnicodeString m_name;
		ChatUnicodeString m_address;

		ChatIgnoreStatusCore *m_core;
	};
};

#endif
