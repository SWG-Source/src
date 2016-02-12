#include "ChatFriendStatusCore.h"
#include "ChatFriendStatus.h"

namespace ChatSystem 
{

using namespace Base;
using namespace Plat_Unicode;
ChatFriendStatusCore::ChatFriendStatusCore()
: m_interface(nullptr),
  m_status(0)
{
}

void ChatFriendStatusCore::load(ByteStream::ReadIterator  &iter, ChatFriendStatus *inf)
{
	m_interface = inf;
	inf->m_core = this;
	ASSERT_VALID_STRING_LENGTH(get(iter, m_name));
	ASSERT_VALID_STRING_LENGTH(get(iter, m_address));
	ASSERT_VALID_STRING_LENGTH(get(iter, m_comment));
	get(iter, m_status);

	m_interface->m_name = m_name;
	m_interface->m_address = m_address;
	m_interface->m_comment = m_comment;
}

ChatFriendStatusCore::~ChatFriendStatusCore()
{
}

ChatFriendStatus::ChatFriendStatus()
: m_core(nullptr)
{
}

ChatFriendStatus::~ChatFriendStatus()
{
}

const ChatUnicodeString &ChatFriendStatus::getName() const
{
	return m_name;
}

const ChatUnicodeString &ChatFriendStatus::getAddress() const
{
	return m_address;
}

const ChatUnicodeString &ChatFriendStatus::getComment() const
{
	return m_comment;
}

short ChatFriendStatus::getStatus() const
{
	return m_core->getStatus();
}

};

