#if !defined (CHATIGNORESTATUSCORE_H_)
#define CHATIGNORESTATUSCORE_H_

#include <Base/Archive.h>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#include "ChatEnum.h"

namespace ChatSystem 
{


	class ChatIgnoreStatus;

	class ChatIgnoreStatusCore
	{
	public:
		ChatIgnoreStatusCore();
		~ChatIgnoreStatusCore();

		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
	
		void load(Base::ByteStream::ReadIterator &iter, ChatIgnoreStatus *inf);

	private:
		ChatIgnoreStatus *m_interface;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

};

#endif


