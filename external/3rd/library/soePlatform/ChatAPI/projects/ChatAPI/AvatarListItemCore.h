#if !defined (AVATARLISTITEMCORE_H_)
#define AVATARLISTITEMCORE_H_

#include <Base/Archive.h>
#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>

#include "ChatEnum.h"

namespace ChatSystem 
{


	class AvatarListItem;

	class AvatarListItemCore
	{
	public:
		AvatarListItemCore();
		~AvatarListItemCore();

		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
	
		void load(Base::ByteStream::ReadIterator &iter, AvatarListItem *inf);

	private:
		AvatarListItem *m_interface;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

};

#endif


