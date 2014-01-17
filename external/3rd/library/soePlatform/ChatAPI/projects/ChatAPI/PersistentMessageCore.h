#if !defined (PERSISTENTHEADERCORE_H_)
#define PERSISTENTHEADERCORE_H_

#include <Base/Archive.h>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

namespace ChatSystem 
{

	class PersistentHeader;

	class PersistentHeaderCore
	{
	public:
		PersistentHeaderCore();
		~PersistentHeaderCore();

		unsigned getMessageID() const { return m_messageID; }
		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getFromName() const { return m_fromName; }
		const Plat_Unicode::String &getFromAddress() const { return m_fromAddress; }
		const Plat_Unicode::String &getSubject() const { return m_subject; }
		const Plat_Unicode::String &getFolder() const { return m_folder; }

		unsigned getSentTime() const { return m_sentTime; }
		unsigned getStatus() const { return m_status; }
		void load(Base::ByteStream::ReadIterator &iter, PersistentHeader *inf);
		void setFolder(Base::ByteStream::ReadIterator &iter);
		void setCategory(Base::ByteStream::ReadIterator &iter);
	private:
		unsigned m_messageID;
		unsigned m_avatarID;
		unsigned m_sentTime;
		unsigned m_status;
		Plat_Unicode::String m_fromName;
		Plat_Unicode::String m_fromAddress;
		Plat_Unicode::String m_subject;
		Plat_Unicode::String m_folder;
		Plat_Unicode::String m_category;

		PersistentHeader *m_interface;
	};

	class PersistentMessageCore : public PersistentMessage
	{
	public:
		PersistentMessageCore();
		virtual ~PersistentMessageCore();

		// PersistentMessage implementations
		virtual unsigned getMessageID() const { return m_messageID; }
		virtual unsigned getAvatarID() const { return m_avatarID; }
		virtual const ChatUnicodeString &getFromName() const { return m_fromName; }
		virtual const ChatUnicodeString &getFromAddress() const { return m_fromAddress; }
		virtual const ChatUnicodeString &getSubject() const { return m_subject; }
		virtual const ChatUnicodeString &getFolder() const { return m_folder;  }
		virtual const ChatUnicodeString &getCategory() const { return m_category; }

		virtual unsigned getSentTime() const { return m_sentTime; }
		virtual unsigned getStatus() const { return m_status; }

		virtual const ChatUnicodeString &getMsg() const { return m_msg; }
		virtual const ChatUnicodeString &getOOB() const { return m_oob; }

		void load(Base::ByteStream::ReadIterator &iter);

	private:
		unsigned m_messageID;
		unsigned m_avatarID;
		unsigned m_sentTime;
		unsigned m_status;
		unsigned m_fetchResult;
		ChatUnicodeString m_fromName;
		ChatUnicodeString m_fromAddress;
		ChatUnicodeString m_subject;
		ChatUnicodeString m_folder;
		ChatUnicodeString m_category;
		ChatUnicodeString m_msg;
		ChatUnicodeString m_oob;
	};

};
#endif


