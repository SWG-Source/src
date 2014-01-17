#if !defined (PERSISTENTMESSAGE_H_)
#define PERSISTENTMESSAGE_H_

#include "ChatEnum.h"

namespace ChatSystem 
{
	class PersistentHeaderCore;

	class PersistentHeader
	{
	public:
		PersistentHeader();
		~PersistentHeader();

		unsigned getMessageID() const;
		unsigned getAvatarID() const;
		const ChatUnicodeString &getFromName() const;
		const ChatUnicodeString &getFromAddress() const;
		const ChatUnicodeString &getSubject() const;
		const ChatUnicodeString &getFolder() const;
		const ChatUnicodeString &getCategory() const;

		unsigned getSentTime() const;
		unsigned getStatus() const;

		friend class PersistentHeaderCore;
	private:
		ChatUnicodeString m_fromName;
		ChatUnicodeString m_fromAddress;
		ChatUnicodeString m_subject;
		ChatUnicodeString m_folder;
		ChatUnicodeString m_category;

		PersistentHeaderCore *m_core;
	};

	class PersistentMessage
	{
	public:
		virtual unsigned getMessageID() const = 0;
		virtual unsigned getAvatarID() const = 0;
		virtual const ChatUnicodeString &getFromName() const = 0;
		virtual const ChatUnicodeString &getFromAddress() const = 0;
		virtual const ChatUnicodeString &getSubject() const = 0;
		virtual const ChatUnicodeString &getFolder() const = 0;
		virtual const ChatUnicodeString &getCategory() const = 0;

		virtual unsigned getSentTime() const = 0;
		virtual unsigned getStatus() const = 0;

		virtual const ChatUnicodeString &getMsg() const = 0;
		virtual const ChatUnicodeString &getOOB() const = 0;
	};

};
#endif


