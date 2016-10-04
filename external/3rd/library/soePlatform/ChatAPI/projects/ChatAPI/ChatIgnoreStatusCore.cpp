#include "ChatIgnoreStatusCore.h"
#include "ChatIgnoreStatus.h"

namespace ChatSystem
{
	using namespace Base;
	using namespace Plat_Unicode;

	ChatIgnoreStatus::ChatIgnoreStatus()
	: m_core(nullptr)
	{
	}

	ChatIgnoreStatus::~ChatIgnoreStatus()
	{
	}

	ChatIgnoreStatusCore::ChatIgnoreStatusCore()
	: m_interface(nullptr)
	{
	}

	ChatIgnoreStatusCore::~ChatIgnoreStatusCore()
	{
	}

	void ChatIgnoreStatusCore::load(Base::ByteStream::ReadIterator &iter, ChatIgnoreStatus *inf)
	{
		m_interface = inf;
		inf->m_core = this;
		ASSERT_VALID_STRING_LENGTH(get(iter, m_name));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_address));

		m_interface->m_name = m_name;
		m_interface->m_address = m_address;
	}
};
