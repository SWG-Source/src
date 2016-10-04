#include "PersistentMessage.h"
#include "PersistentMessageCore.h"

namespace ChatSystem 
{

using namespace Base;
using namespace Plat_Unicode;

PersistentHeader::PersistentHeader()
: m_core(nullptr)
{
}

PersistentHeader::~PersistentHeader()
{
}

unsigned PersistentHeader::getMessageID() const
{
	return m_core->getMessageID();
}

unsigned PersistentHeader::getAvatarID() const
{
	return m_core->getAvatarID();
}

const ChatUnicodeString &PersistentHeader::getFromName() const
{
	return m_fromName;
}

const ChatUnicodeString &PersistentHeader::getFromAddress() const
{
	return m_fromAddress;
}

const ChatUnicodeString &PersistentHeader::getSubject() const
{
	return m_subject;
}

const ChatUnicodeString &PersistentHeader::getFolder() const
{
	return m_folder;
}

const ChatUnicodeString &PersistentHeader::getCategory() const
{
	return m_category;
}

unsigned PersistentHeader::getSentTime() const
{
	return m_core->getSentTime();
}

unsigned PersistentHeader::getStatus() const
{
	return m_core->getStatus();
}

PersistentHeaderCore::PersistentHeaderCore()
: m_messageID(0),
  m_avatarID(0),
  m_sentTime(0),
  m_status(0),
  m_interface(nullptr)
{
}

PersistentHeaderCore::~PersistentHeaderCore()
{
}

void PersistentHeaderCore::load(ByteStream::ReadIterator &iter, PersistentHeader *inf)
{
	m_interface = inf;
	inf->m_core = this;
	get(iter, m_messageID);
	get(iter, m_avatarID);
	get(iter, m_fromName);
	get(iter, m_fromAddress);
	get(iter, m_subject);
	get(iter, m_sentTime);
	get(iter, m_status);

	m_interface->m_fromName = m_fromName;
	m_interface->m_fromAddress = m_fromAddress;
	m_interface->m_subject = m_subject;
}

void PersistentHeaderCore::setFolder(Base::ByteStream::ReadIterator &iter)
{
	get(iter, m_folder);

	m_interface->m_folder = m_folder;
}


void PersistentHeaderCore::setCategory(Base::ByteStream::ReadIterator &iter)
{
	get(iter, m_category);

	m_interface->m_category = m_category;
}

PersistentMessageCore::PersistentMessageCore()
: m_messageID(0),
  m_avatarID(0),
  m_sentTime(0),
  m_status(0),
  m_fetchResult(0)
{
}

PersistentMessageCore::~PersistentMessageCore()
{
}

void PersistentMessageCore::load(Base::ByteStream::ReadIterator &iter)
{
	Plat_Unicode::String wideString;

	get(iter, m_messageID);
	get(iter, m_avatarID);

	get(iter, wideString);
	m_fromName = wideString;

	get(iter, wideString);
	m_fromAddress = wideString;

	get(iter, wideString);
	m_subject = wideString;

	get(iter, m_sentTime);
	get(iter, m_status);

	get(iter, wideString);
	m_folder = wideString;

	get(iter, wideString);
	m_category = wideString;

	get(iter, m_fetchResult);

	get(iter, wideString);
	m_msg = wideString;

	get(iter, wideString);
	m_oob = wideString;
}

};
