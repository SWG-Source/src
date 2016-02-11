#include "ChatRoom.h"

#include <string.h>

#include "AvatarIteratorCore.h"
#include "ChatRoomCore.h"
#include "ChatAvatar.h"
#include "ChatAvatarCore.h"
#include "RoomParamsCore.h"
#include "RoomSummaryCore.h"

namespace ChatSystem 
{
using namespace Plat_Unicode;
using namespace std;


// AVATAR ITERATOR

AvatarIterator::AvatarIterator()
{
	m_core = new AvatarIteratorCore();
}

AvatarIterator::AvatarIterator(const AvatarIteratorCore &core)
{
	m_core = new AvatarIteratorCore();
	
	(*m_core) = core;
}

AvatarIterator::~AvatarIterator()
{
	delete m_core;
}

AvatarIterator &AvatarIterator::operator=(const AvatarIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void AvatarIterator::operator++()
{
	m_core->increment();
}

void AvatarIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void AvatarIterator::operator--()
{
	m_core->decrement();
}

void AvatarIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}

ChatAvatar *AvatarIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool AvatarIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}

// MODERATOR ITERATOR

ModeratorIterator::ModeratorIterator()
{
	m_core = new ModeratorIteratorCore();
}

ModeratorIterator::ModeratorIterator(const ModeratorIteratorCore &core)
{
	m_core = new ModeratorIteratorCore();
	
	(*m_core) = core;
}

ModeratorIterator::~ModeratorIterator()
{
	delete m_core;
}

ModeratorIterator &ModeratorIterator::operator=(const ModeratorIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void ModeratorIterator::operator++()
{
	m_core->increment();
}

void ModeratorIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void ModeratorIterator::operator--()
{
	m_core->decrement();
}

void ModeratorIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}
ChatAvatar *ModeratorIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool ModeratorIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}

// TEMPORARY MODERATOR ITERATOR

TemporaryModeratorIterator::TemporaryModeratorIterator()
{
	m_core = new TemporaryModeratorIteratorCore();
}

TemporaryModeratorIterator::TemporaryModeratorIterator(const TemporaryModeratorIteratorCore &core)
{
	m_core = new TemporaryModeratorIteratorCore();
	
	(*m_core) = core;
}

TemporaryModeratorIterator::~TemporaryModeratorIterator()
{
	delete m_core;
}

TemporaryModeratorIterator &TemporaryModeratorIterator::operator=(const TemporaryModeratorIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void TemporaryModeratorIterator::operator++()
{
	m_core->increment();
}

void TemporaryModeratorIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void TemporaryModeratorIterator::operator--()
{
	m_core->decrement();
}

void TemporaryModeratorIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}
ChatAvatar *TemporaryModeratorIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool TemporaryModeratorIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}

// VOICE ITERATOR

VoiceIterator::VoiceIterator()
{
	m_core = new VoiceIteratorCore();
}

VoiceIterator::VoiceIterator(const VoiceIteratorCore &core)
{
	m_core = new VoiceIteratorCore();
	
	(*m_core) = core;
}

VoiceIterator::~VoiceIterator()
{
	delete m_core;
}

VoiceIterator &VoiceIterator::operator=(const VoiceIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void VoiceIterator::operator++()
{
	m_core->increment();
}

void VoiceIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void VoiceIterator::operator--()
{
	m_core->decrement();
}

void VoiceIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}

ChatAvatar *VoiceIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool VoiceIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}

// INVITE ITERATOR

InviteIterator::InviteIterator()
{
	m_core = new InviteIteratorCore();
}

InviteIterator::InviteIterator(const InviteIteratorCore &core)
{
	m_core = new InviteIteratorCore();
	
	(*m_core) = core;
}

InviteIterator::~InviteIterator()
{
	delete m_core;
}

InviteIterator &InviteIterator::operator=(const InviteIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void InviteIterator::operator++()
{
	m_core->increment();
}

void InviteIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void InviteIterator::operator--()
{
	m_core->decrement();
}

void InviteIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}

ChatAvatar *InviteIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool InviteIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}

// BAN ITERATOR

BanIterator::BanIterator()
{
	m_core = new BanIteratorCore();
}

BanIterator::BanIterator(const BanIteratorCore &core)
{
	m_core = new BanIteratorCore();
	
	(*m_core) = core;
}

BanIterator::~BanIterator()
{
	delete m_core;
}

BanIterator &BanIterator::operator=(const BanIterator &rhs)
{
	(*m_core) = *(rhs.m_core);

	return (*this);
}

void BanIterator::operator++()
{
	m_core->increment();
}

void BanIterator::operator++(int dummyForPostfix)
{
	m_core->increment();
}

void BanIterator::operator--()
{
	m_core->decrement();
}

void BanIterator::operator--(int dummyForPostfix)
{
	m_core->decrement();
}

ChatAvatar *BanIterator::operator*()
{
	return (m_core->getCurAvatar());
}

bool BanIterator::outOfBounds()
{
	return (m_core->outOfBounds());
}


// ROOM PARAMS

RoomParams::RoomParams()
{
	m_core = new struct RoomParamsCore;
	m_core->m_attributes = 0;
	m_core->m_size = 0;
}


RoomParams::RoomParams(const ChatUnicodeString &name, const ChatUnicodeString &topic, unsigned attributes, unsigned maxSize)
{
	m_core = new struct RoomParamsCore;
	
	m_core->m_name.assign(name.string_data, name.string_length);
	m_cName = m_core->m_name;

	m_core->m_topic.assign(topic.string_data, topic.string_length);
	m_cTopic = m_core->m_topic;

	m_cPassword = m_core->m_password;

	m_core->m_attributes = attributes;
	m_core->m_size = maxSize;
}

RoomParams::RoomParams(const ChatUnicodeString &name, const ChatUnicodeString &topic, const ChatUnicodeString &password, unsigned attributes, unsigned maxSize)
{
	m_core = new struct RoomParamsCore;
	
	m_core->m_name.assign(name.string_data, name.string_length);
	m_cName = m_core->m_name;

	m_core->m_topic.assign(topic.string_data, topic.string_length);
	m_cTopic = m_core->m_topic;

	m_core->m_password.assign(password.string_data, password.string_length);
	m_cPassword = m_core->m_password;

	m_core->m_attributes = attributes;
	m_core->m_size = maxSize;
}

RoomParams::RoomParams(const RoomParams &orig) : m_core(new struct RoomParamsCore)
{
	this->setRoomName(orig.getRoomName());
	this->setRoomTopic(orig.getRoomTopic());
	this->setRoomPassword(orig.getRoomPassword());
	this->setRoomAttributes(orig.getRoomAttributes());
	this->setRoomMaxSize(orig.getRoomMaxSize());
}

RoomParams::~RoomParams()
{
	delete m_core;
}

RoomParams &RoomParams::operator=(const RoomParams &rhs)
{
	this->setRoomName(rhs.getRoomName());
	this->setRoomTopic(rhs.getRoomTopic());
	this->setRoomPassword(rhs.getRoomPassword());
	this->setRoomAttributes(rhs.getRoomAttributes());
	this->setRoomMaxSize(rhs.getRoomMaxSize());

	return (*this);
}

void RoomParams::setRoomName(const ChatUnicodeString &name)
{
	m_core->m_name.assign(name.string_data, name.string_length);
	m_cName = m_core->m_name;
}

void RoomParams::setRoomTopic(const ChatUnicodeString &topic)
{
	m_core->m_topic.assign(topic.string_data, topic.string_length);
	m_cTopic = m_core->m_topic;
}

void RoomParams::setRoomPassword(const ChatUnicodeString &password)
{
	m_core->m_password.assign(password.string_data, password.string_length);
	m_cPassword = m_core->m_password;
}

void RoomParams::setRoomAttributes(unsigned attributes)
{
	m_core->m_attributes = attributes;
}

void RoomParams::setRoomMaxSize(unsigned size)
{
	m_core->m_size = size;
}

const ChatUnicodeString &RoomParams::getRoomName() const
{
	m_cName = m_core->m_name;
	return (m_cName);
}

const ChatUnicodeString &RoomParams::getRoomTopic() const
{
	m_cTopic = m_core->m_topic;
	return (m_cTopic);
}

const ChatUnicodeString &RoomParams::getRoomPassword() const
{
	m_cPassword = m_core->m_password;
	return (m_cPassword);
}

unsigned RoomParams::getRoomAttributes() const
{
	return (m_core->m_attributes);
}

unsigned RoomParams::getRoomMaxSize() const
{
	return (m_core->m_size);
}

// ROOM SUMMARY

RoomSummary::RoomSummary()
{
	m_core = new struct RoomSummaryCore;
	m_core->m_attributes = 0;
	m_core->m_curSize = 0;
	m_core->m_maxSize = 0;
}

RoomSummary::RoomSummary(const ChatUnicodeString &address, const ChatUnicodeString &topic, unsigned attributes, unsigned curSize, unsigned maxSize)
{
	m_core = new struct RoomSummaryCore;
	
	m_core->m_address.assign(address.string_data, address.string_length);
	m_cAddress = m_core->m_address;

	m_core->m_topic.assign(address.string_data, address.string_length);
	m_cTopic = m_core->m_address;

	m_core->m_attributes = attributes;
	m_core->m_curSize = curSize;
	m_core->m_maxSize = maxSize;
}

RoomSummary::~RoomSummary()
{
	delete m_core;
}

RoomSummary &RoomSummary::operator=(const RoomSummary &rhs)
{
	this->setRoomAddress(rhs.getRoomAddress());
	m_cAddress= m_core->m_address;

	this->setRoomTopic(rhs.getRoomTopic());
	m_cTopic = m_core->m_address;

	this->setRoomAttributes(rhs.getRoomAttributes());
	this->setRoomCurSize(rhs.getRoomCurSize());
	this->setRoomMaxSize(rhs.getRoomMaxSize());

	return (*this);
}

void RoomSummary::setRoomAddress(const ChatUnicodeString &address)
{
	m_core->m_address.assign(address.string_data, address.string_length);
	m_cAddress = m_core->m_address;

}

void RoomSummary::setRoomTopic(const ChatUnicodeString &topic)
{
	m_core->m_topic.assign(topic.string_data, topic.string_length);
	m_cTopic = m_core->m_address;
}

void RoomSummary::setRoomAttributes(unsigned attributes)
{
	m_core->m_attributes = attributes;
}

void RoomSummary::setRoomCurSize(unsigned curSize)
{
	m_core->m_curSize = curSize;
}

void RoomSummary::setRoomMaxSize(unsigned maxSize)
{
	m_core->m_maxSize = maxSize;
}

const ChatUnicodeString &RoomSummary::getRoomAddress() const
{
	m_cAddress = m_core->m_address;

	return (m_cAddress);
}

const ChatUnicodeString &RoomSummary::getRoomTopic() const
{
	m_cTopic = m_core->m_topic;

	return (m_cTopic);
}

unsigned RoomSummary::getRoomAttributes() const
{
	return (m_core->m_attributes);
}

unsigned RoomSummary::getRoomCurSize() const
{
	return (m_core->m_curSize);
}

unsigned RoomSummary::getRoomMaxSize() const
{
	return (m_core->m_maxSize);
}


// CHAT ROOM

ChatRoom::ChatRoom()
{
	m_core = nullptr;
}

ChatRoom::ChatRoom(ChatRoomCore *core)
: m_core(core)
{
}

ChatRoom::~ChatRoom()
{
}

const ChatUnicodeString &ChatRoom::getCreatorName() const
{
	m_cCreatorName = m_core->getCreatorName();

	return (m_cCreatorName);
}

const ChatUnicodeString &ChatRoom::getCreatorAddress() const
{
	m_cCreatorAddress = m_core->getCreatorAddress();

	return (m_cCreatorAddress);
}

unsigned ChatRoom::getCreatorID() const
{
	return (m_core->getCreatorID());
}

unsigned ChatRoom::getRoomID() const
{
	return (m_core->getRoomID());
}

const ChatUnicodeString &ChatRoom::getRoomName() const
{
	m_cRoomName = m_core->getRoomName();

	return(m_cRoomName);
}

const ChatUnicodeString &ChatRoom::getRoomTopic() const
{
	m_cRoomTopic = m_core->getRoomTopic();

	return(m_cRoomTopic);
}

const ChatUnicodeString &ChatRoom::getRoomPassword() const
{
	m_cRoomPassword = m_core->getRoomPassword();

	return(m_cRoomPassword);
}

unsigned ChatRoom::getRoomAttributes() const
{
	return (m_core->getRoomAttributes());
}

unsigned ChatRoom::getMaxRoomSize() const
{
	return (m_core->getMaxRoomSize());
}

const ChatUnicodeString &ChatRoom::getAddress() const
{
	m_cRoomAddress = m_core->getAddress();

	return (m_cRoomAddress);
}

unsigned ChatRoom::getAvatarCount() const
{
	return (m_core->getAvatarCount());
}

unsigned ChatRoom::getBanCount() const
{
	return (m_core->getBanCount());
}

unsigned ChatRoom::getInviteCount() const
{
	return (m_core->getInviteCount());
}

unsigned ChatRoom::getModeratorCount() const
{
	return (m_core->getModeratorCount());
}

unsigned ChatRoom::getTemporaryModeratorCount() const
{
	return (m_core->getTemporaryModeratorCount());
}

unsigned ChatRoom::getVoiceCount() const
{
	return (m_core->getVoiceCount());
}

AvatarIterator ChatRoom::getFirstAvatar() const
{
	return (AvatarIterator(m_core->getFirstAvatar()));
}

AvatarIterator ChatRoom::findAvatar(unsigned avatarID) const
{
	return (AvatarIterator(m_core->findAvatar(avatarID)));
}

AvatarIterator ChatRoom::findAvatar(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (AvatarIterator(m_core->findAvatar(uni_name, uni_addr)));
}

ModeratorIterator ChatRoom::getFirstModerator() const
{
	return (ModeratorIterator(m_core->getFirstModerator()));
}

ModeratorIterator ChatRoom::findModerator(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (ModeratorIterator(m_core->findModerator(uni_name, uni_addr)));
}

TemporaryModeratorIterator ChatRoom::getFirstTemporaryModerator() const
{
	return (TemporaryModeratorIterator(m_core->getFirstTemporaryModerator()));
}

TemporaryModeratorIterator ChatRoom::findTemporaryModerator(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (TemporaryModeratorIterator(m_core->findTemporaryModerator(uni_name, uni_addr)));
}

BanIterator ChatRoom::getFirstBanned() const
{
	return (BanIterator(m_core->getFirstBanned()));
}

BanIterator ChatRoom::findBanned(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (BanIterator(m_core->findBanned(uni_name, uni_addr)));
}

InviteIterator ChatRoom::getFirstInvited() const
{
	return (InviteIterator(m_core->getFirstInvited()));
}

InviteIterator ChatRoom::findInvited(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (InviteIterator(m_core->findInvited(uni_name, uni_addr)));
}

VoiceIterator ChatRoom::getFirstVoice() const
{
	return (VoiceIterator(m_core->getFirstVoice()));
}

VoiceIterator ChatRoom::findVoice(const ChatUnicodeString &name, const ChatUnicodeString &address) const
{
	String uni_name(name.string_data, name.string_length);
	String uni_addr(address.string_data, address.string_length);

	return (VoiceIterator(m_core->findVoice(uni_name, uni_addr)));
}

void ChatRoom::setRoomMessageID(unsigned roomMessageID)
{
	m_core->setRoomMessageID(roomMessageID);
}

};

