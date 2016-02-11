#include "AvatarListItemCore.h"
#include "AvatarListItem.h"

namespace ChatSystem
{
	using namespace Base;
	using namespace Plat_Unicode;

	AvatarListItem::AvatarListItem()
	: m_core(nullptr)
	{
	}

	AvatarListItem::~AvatarListItem()
	{
	}

	AvatarListItemCore::AvatarListItemCore()
	: m_interface(nullptr)
	{
	}

	AvatarListItemCore::~AvatarListItemCore()
	{
	}

	void AvatarListItemCore::load(Base::ByteStream::ReadIterator &iter, AvatarListItem *inf)
	{
		m_interface = inf;
		inf->m_core = this;
		get(iter, m_name);
		get(iter, m_address);

		m_interface->m_name = m_name;
		m_interface->m_address = m_address;
	}
};
