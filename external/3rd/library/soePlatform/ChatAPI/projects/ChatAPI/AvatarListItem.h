#if !defined (AVATARLISTITEM_H_)
#define AVATARLISTITEM_H_

#include <stdlib.h>

#include "ChatEnum.h"

namespace ChatSystem 
{
	class AvatarListItemCore;

	class AvatarListItem
	{
	public:
		AvatarListItem();
		~AvatarListItem();
		const ChatUnicodeString &getName() const { return m_name; }
		const ChatUnicodeString &getAddress() const { return m_address; }
		
		friend class AvatarListItemCore;
	private:
		ChatUnicodeString m_name;
		ChatUnicodeString m_address;

		AvatarListItemCore *m_core;
	};
};

#endif
