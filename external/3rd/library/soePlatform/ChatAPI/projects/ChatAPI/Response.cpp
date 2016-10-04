#include "Response.h"
#include "ChatEnum.h"
#include "ChatRoom.h"
#include "ChatFriendStatus.h"
#include "ChatFriendStatusCore.h"
#include "ChatRoomCore.h"
#include "ChatAvatarCore.h"
#include "PersistentMessage.h"
#include "PersistentMessageCore.h"
#include "ChatIgnoreStatus.h"
#include "ChatIgnoreStatusCore.h"
#include "AvatarListItem.h"
#include "AvatarListItemCore.h"

namespace ChatSystem
{
	using namespace Base;
	using namespace Plat_Unicode;

	ResLoginAvatar::ResLoginAvatar(void *user, int avatarLoginPriority)
		: GenericResponse(RESPONSE_LOGINAVATAR, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr),
		m_submittedPriority(avatarLoginPriority),
		m_requiredPriority(INT_MAX)
	{
	}

	void ResLoginAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		Plat_Unicode::String email;
		unsigned inboxLimit = 0;

		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			m_avatar = new ChatAvatarCore(iter);

			if (iter.getSize() > 0)
			{
				get(iter, m_requiredPriority);
			}

			if (iter.getSize() > 0)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, email));
				get(iter, inboxLimit);
			}

			if (m_avatar)
			{
				m_avatar->setEmail(email);
				m_avatar->setInboxLimit(inboxLimit);
			}
		}
	}

	ResTemporaryAvatar::ResTemporaryAvatar(void *user)
		: GenericResponse(RESPONSE_TEMPORARYAVATAR, CHATRESULT_TIMEOUT, user)
		, m_avatar(nullptr)
	{
	}

	void ResTemporaryAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			m_avatar = new ChatAvatarCore(iter);
		}
	}

	ResLogoutAvatar::ResLogoutAvatar(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_LOGOUTAVATAR, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID)
	{
	}

	void ResLogoutAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResDestroyAvatar::ResDestroyAvatar(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_DESTROYAVATAR, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID)
	{
	}

	void ResDestroyAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResGetAvatar::ResGetAvatar(void *user)
		: GenericResponse(RESPONSE_GETAVATAR, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr)
	{
	}

	void ResGetAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			Plat_Unicode::String email;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			if (iter.getSize() > 0)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, email));
				get(iter, inboxLimit);
			}

			if (m_avatar)
			{
				m_avatar->setEmail(email);
				m_avatar->setInboxLimit(inboxLimit);
			}
		}
	}

	ResGetAnyAvatar::ResGetAnyAvatar(void* user)
		: GenericResponse(RESPONSE_GETANYAVATAR, CHATRESULT_TIMEOUT, user)
		, m_avatar(nullptr), m_online(0)
	{
	}

	void ResGetAnyAvatar::unpack(Base::ByteStream::ReadIterator& iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_online);

		if (m_result == CHATRESULT_SUCCESS)
		{
			Plat_Unicode::String email;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			if (iter.getSize() > 0)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, email));
				get(iter, inboxLimit);
			}

			if (m_avatar)
			{
				m_avatar->setEmail(email);
				m_avatar->setInboxLimit(inboxLimit);
			}
		}
	}

	ResAvatarList::ResAvatarList(void *user)
		: GenericResponse(RESPONSE_AVATARLIST, CHATRESULT_TIMEOUT, user)
		, m_listLength(0)
		, m_avatarList(nullptr)
		, m_cores(nullptr)
	{
	}

	ResAvatarList::~ResAvatarList()
	{
		delete[] m_avatarList;
		delete[] m_cores;
	}

	void ResAvatarList::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			m_avatarList = new AvatarListItem[m_listLength];
			m_cores = new AvatarListItemCore[m_listLength];

			for (unsigned i = 0; i < m_listLength; i++)
			{
				m_cores[i].load(iter, &m_avatarList[i]);
			}
		}
	}

	ResSetAvatarAttributes::ResSetAvatarAttributes(void *user)
		: GenericResponse(RESPONSE_SETAVATARATTRIBUTES, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr)
	{
	}

	void ResSetAvatarAttributes::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			Plat_Unicode::String email;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			if (iter.getSize() > 0)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, email));
				get(iter, inboxLimit);
			}

			if (m_avatar)
			{
				m_avatar->setEmail(email);
				m_avatar->setInboxLimit(inboxLimit);
			}
		}
	}

	ResSetAvatarStatusMessage::ResSetAvatarStatusMessage(void *user)
		: GenericResponse(RESPONSE_SETSTATUSMESSAGE, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr)
	{
	}

	void ResSetAvatarStatusMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			Plat_Unicode::String email;
			Plat_Unicode::String statusMessage;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			if (iter.getSize() > 0)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, email));
				get(iter, inboxLimit);
				ASSERT_VALID_STRING_LENGTH(get(iter, statusMessage));
			}

			if (m_avatar)
			{
				m_avatar->setEmail(email);
				m_avatar->setInboxLimit(inboxLimit);
				m_avatar->setStatusMessage(statusMessage);
			}
		}
	}

	ResSetAvatarForwardingEmail::ResSetAvatarForwardingEmail(void *user)
		: GenericResponse(RESPONSE_SETAVATAREMAIL, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr)
	{
	}

	void ResSetAvatarForwardingEmail::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			Plat_Unicode::String email;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			ASSERT_VALID_STRING_LENGTH(get(iter, email));
			get(iter, inboxLimit);
			m_avatar->setEmail(email);
			m_avatar->setInboxLimit(inboxLimit);
		}
	}

	ResSetAvatarInboxLimit::ResSetAvatarInboxLimit(void *user)
		: GenericResponse(RESPONSE_SETAVATARINBOXLIMIT, CHATRESULT_TIMEOUT, user),
		m_avatar(nullptr)
	{
	}

	void ResSetAvatarInboxLimit::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			Plat_Unicode::String email;
			unsigned inboxLimit = 0;

			m_avatar = new ChatAvatarCore(iter);

			ASSERT_VALID_STRING_LENGTH(get(iter, email));
			get(iter, inboxLimit);
			m_avatar->setEmail(email);
			m_avatar->setInboxLimit(inboxLimit);
		}
	}

	ResSearchAvatarKeywords::ResSearchAvatarKeywords(void *user)
		: GenericResponse(RESPONSE_SETAVATARKEYWORDS, CHATRESULT_TIMEOUT, user),
		m_numMatches(0),
		m_avatarMatches(nullptr)
	{
	}

	void ResSearchAvatarKeywords::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_numMatches);
		if (m_result == CHATRESULT_SUCCESS)
		{
			m_avatarMatches = new ChatAvatarCore*[m_numMatches];
			for (unsigned i = 0; i < m_numMatches; i++)
			{
				m_avatarMatches[i] = new ChatAvatarCore(iter);
			}
		}
	}

	ResSearchAvatarKeywords::~ResSearchAvatarKeywords()
	{
		for (unsigned i = 0; i < m_numMatches; i++)
		{
			delete m_avatarMatches[i];
		}

		delete[] m_avatarMatches;
	}

	ResSetAvatarKeywords::ResSetAvatarKeywords(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_SETAVATARKEYWORDS, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID)
	{
	}

	void ResSetAvatarKeywords::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResGetAvatarKeywords::ResGetAvatarKeywords(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_GETAVATARKEYWORDS, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_keywordList(nullptr),
		m_chatStrList(nullptr),
		m_keywordLength(0)
	{
	}

	ResGetAvatarKeywords::~ResGetAvatarKeywords()
	{
		delete[] m_keywordList;
		delete[] m_chatStrList;
	}

	void ResGetAvatarKeywords::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_keywordLength);
		if (m_result == CHATRESULT_SUCCESS)
		{
			m_keywordList = new String[m_keywordLength];
			m_chatStrList = new ChatUnicodeString[m_keywordLength];
			for (unsigned i = 0; i < m_keywordLength; i++)
			{
				ASSERT_VALID_STRING_LENGTH(get(iter, m_keywordList[i]));
				m_chatStrList[i] = m_keywordList[i];
			}
		}
	}

	ResGetRoom::ResGetRoom(void *user)
		: GenericResponse(RESPONSE_GETROOM, CHATRESULT_TIMEOUT, user),
		m_room(nullptr),
		m_numExtraRooms(0)
	{
	}

	void ResGetRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			m_room = new ChatRoomCore(iter);

			// any extra parent rooms sent for caching?
			get(iter, m_numExtraRooms);
			for (unsigned i = 0; i != m_numExtraRooms; i++)
			{
				m_setExtraRooms.insert(new ChatRoomCore(iter));
			}
		}
	}

	ResCreateRoom::ResCreateRoom(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_CREATEROOM, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(avatarID),
		m_room(nullptr),
		m_numExtraRooms(0)
	{
	}

	void ResCreateRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		if (m_result == 0)
		{
			m_room = new ChatRoomCore(iter);

			// any extra parent rooms sent for caching?
			get(iter, m_numExtraRooms);
			for (unsigned i = 0; i != m_numExtraRooms; i++)
			{
				m_setExtraRooms.insert(new ChatRoomCore(iter));
			}
		}
	}

	ResDestroyRoom::ResDestroyRoom(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_DESTROYROOM, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_roomID(0)
	{
	}

	void ResDestroyRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_roomID);
	}

	ResSendInstantMessage::ResSendInstantMessage(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_SENDINSTANTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResSendInstantMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResSendRoomMessage::ResSendRoomMessage(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_SENDROOMMESSAGE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_roomID(0)
	{
	}

	void ResSendRoomMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_roomID);
	}

	ResSendBroadcastMessage::ResSendBroadcastMessage(void *user, unsigned avatarID, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_SENDBROADCASTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResSendBroadcastMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResFilterMessage::ResFilterMessage(void *user, unsigned version)
		: GenericResponse(version, CHATRESULT_TIMEOUT, user)
		, m_version(version)
	{
	}

	void ResFilterMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_msg));
	}

	ResAddFriend::ResAddFriend(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment)
		: GenericResponse(RESPONSE_ADDFRIEND, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_comment(comment.string_data, comment.string_length)
	{
	}

	void ResAddFriend::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResAddFriendReciprocate::ResAddFriendReciprocate(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment)
		: GenericResponse(RESPONSE_ADDFRIEND_RECIPROCATE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_comment(comment.string_data, comment.string_length)
	{
	}

	void ResAddFriendReciprocate::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResSetFriendComment::ResSetFriendComment(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment)
		: GenericResponse(RESPONSE_SETFRIENDCOMMENT, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_comment(comment.string_data, comment.string_length)
	{
	}

	void ResSetFriendComment::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResRemoveFriend::ResRemoveFriend(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_REMOVEFRIEND, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResRemoveFriend::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}
	ResRemoveFriendReciprocate::ResRemoveFriendReciprocate(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_REMOVEFRIEND_RECIPROCATE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResRemoveFriendReciprocate::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResFriendStatus::ResFriendStatus(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_FRIENDSTATUS, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_listLength(0),
		m_friendList(nullptr),
		m_cores(nullptr)
	{
	}

	ResFriendStatus::~ResFriendStatus()
	{
		delete[] m_friendList;
		delete[] m_cores;
	}

	void ResFriendStatus::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			m_friendList = new ChatFriendStatus[m_listLength];
			m_cores = new ChatFriendStatusCore[m_listLength];

			for (unsigned i = 0; i < m_listLength; i++)
			{
				m_cores[i].load(iter, &m_friendList[i]);
			}
		}
	}

	ResAddIgnore::ResAddIgnore(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_ADDIGNORE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResAddIgnore::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResRemoveIgnore::ResRemoveIgnore(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address)
		: GenericResponse(RESPONSE_REMOVEIGNORE, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length)
	{
	}

	void ResRemoveIgnore::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResIgnoreStatus::ResIgnoreStatus(void *user, unsigned avatarID)
		: GenericResponse(RESPONSE_IGNORESTATUS, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_listLength(0),
		m_ignoreList(nullptr),
		m_cores(nullptr)
	{
	}

	ResIgnoreStatus::~ResIgnoreStatus()
	{
		delete[] m_ignoreList;
		delete[] m_cores;
	}

	void ResIgnoreStatus::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			m_ignoreList = new ChatIgnoreStatus[m_listLength];
			m_cores = new ChatIgnoreStatusCore[m_listLength];

			for (unsigned i = 0; i < m_listLength; i++)
			{
				m_cores[i].load(iter, &m_ignoreList[i]);
			}
		}
	}

	ResEnterRoom::ResEnterRoom(void *user, unsigned avatarID, const ChatUnicodeString &destAddress)
		: GenericResponse(RESPONSE_ENTERROOM, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_gotRoomObj(false),
		m_room(nullptr),
		m_numExtraRooms(0)
	{
	}

	void ResEnterRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_roomID);
		get(iter, m_gotRoomObj);

		if (m_gotRoomObj)
		{
			// we were sent room info to cache, so create a new ChatRoomCore!
			m_room = new ChatRoomCore(iter);

			// any extra parent rooms sent for caching?
			get(iter, m_numExtraRooms);
			for (unsigned i = 0; i != m_numExtraRooms; i++)
			{
				m_setExtraRooms.insert(new ChatRoomCore(iter));
			}
		}
	}

	ResAllowRoomEntry::ResAllowRoomEntry(void *user, unsigned srcAvatarID, const ChatUnicodeString &destRoomAddress)
		: GenericResponse(RESPONSE_ALLOWROOMENTRY, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_roomID(0),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void ResAllowRoomEntry::unpack(Base::ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_roomID);
	}

	ResLeaveRoom::ResLeaveRoom(void *user, unsigned avatarID, const ChatUnicodeString &destAddress)
		: GenericResponse(RESPONSE_LEAVEROOM, CHATRESULT_TIMEOUT, user),
		m_avatarID(avatarID),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void ResLeaveRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_roomID);
	}

	ResAddModerator::ResAddModerator(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_ADDMODERATOR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResAddModerator::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResRemoveModerator::ResRemoveModerator(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_REMOVEMODERATOR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResRemoveModerator::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResAddTemporaryModerator::ResAddTemporaryModerator(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_ADDTEMPORARYMODERATOR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResAddTemporaryModerator::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResRemoveTemporaryModerator::ResRemoveTemporaryModerator(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_REMOVETEMPORARYMODERATOR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResRemoveTemporaryModerator::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResAddBan::ResAddBan(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_ADDBAN, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResAddBan::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResRemoveBan::ResRemoveBan(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_REMOVEBAN, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResRemoveBan::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResAddInvite::ResAddInvite(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_ADDINVITE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResAddInvite::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResRemoveInvite::ResRemoveInvite(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_REMOVEINVITE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResRemoveInvite::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResGrantVoice::ResGrantVoice(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_GRANTVOICE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResGrantVoice::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResRevokeVoice::ResRevokeVoice(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_REVOKEVOICE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResRevokeVoice::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResKickAvatar::ResKickAvatar(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_KICKAVATAR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResKickAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResSetRoomParams::ResSetRoomParams(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_SETROOMPARAMS, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResSetRoomParams::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResChangeRoomOwner::ResChangeRoomOwner(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_CHANGEROOMOWNER, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destRoomID(0)
	{
	}

	void ResChangeRoomOwner::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResGetRoomSummaries::ResGetRoomSummaries(void *user)
		: GenericResponse(RESPONSE_GETROOMSUMMARIES, CHATRESULT_TIMEOUT, user),
		m_numRooms(0),
		m_roomSummaries(nullptr)
	{
	}

	ResGetRoomSummaries::~ResGetRoomSummaries()
	{
		delete[] m_roomSummaries;
	}

	void ResGetRoomSummaries::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_numRooms);

		if (m_numRooms > 0)
		{
			String tempStr;
			unsigned tempNum;

			m_roomSummaries = new RoomSummary[m_numRooms];

			for (unsigned i = 0; i != m_numRooms; i++)
			{
				// process RoomSummary objects from message
				ASSERT_VALID_STRING_LENGTH(get(iter, tempStr));
				m_roomSummaries[i].setRoomAddress(ChatUnicodeString(tempStr.data(), tempStr.size()));
				ASSERT_VALID_STRING_LENGTH(get(iter, tempStr));
				m_roomSummaries[i].setRoomTopic(ChatUnicodeString(tempStr.data(), tempStr.size()));
				get(iter, tempNum);
				m_roomSummaries[i].setRoomAttributes(tempNum);
				get(iter, tempNum);
				m_roomSummaries[i].setRoomCurSize(tempNum);
				get(iter, tempNum);
				m_roomSummaries[i].setRoomMaxSize(tempNum);
			}
		}
	}

	ResSendPersistentMessage::ResSendPersistentMessage(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_SENDPERSISTENTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_messageID(0)
	{
	}

	void ResSendPersistentMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (iter.getSize() > 0)
		{
			get(iter, m_messageID);
		}
	}

	ResSendMultiplePersistentMessages::ResSendMultiplePersistentMessages(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_SENDMULTIPLEPERSISTENTMESSAGES, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_numResults(0)
	{
	}

	void ResSendMultiplePersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		unsigned resultIndex;

		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		get(iter, m_numResults);

		m_resultVector.resize(m_numResults);
		m_messageIDvector.resize(m_numResults);

		for (resultIndex = 0; resultIndex < m_numResults; resultIndex++)
		{
			get(iter, m_resultVector[resultIndex]);
		}

		if (iter.getSize() > 0)
		{
			for (resultIndex = 0; resultIndex < m_numResults; resultIndex++)
			{
				get(iter, m_messageIDvector[resultIndex]);
			}
		}
	}

	ResAlterPersistentMessage::ResAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, void *user)
		: GenericResponse(RESPONSE_ALTERPERSISTENTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_destAvatarName(destAvatarName.data(), destAvatarName.length()),
		m_destAvatarAddress(destAvatarAddress.data(), destAvatarAddress.length()),
		m_messageID(messageID)
	{
	}

	ResAlterPersistentMessage::~ResAlterPersistentMessage()
	{
	}

	void ResAlterPersistentMessage::unpack(Base::ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResGetPersistentMessage::ResGetPersistentMessage(void *user, unsigned srcAvatarID, unsigned messageID)
		: GenericResponse(RESPONSE_GETPERSISTENTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_messageID(messageID),
		m_core(nullptr),
		m_header(nullptr)
	{
	}

	ResGetPersistentMessage::~ResGetPersistentMessage()
	{
		delete m_header;
		delete m_core;
	}
	void ResGetPersistentMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		if (m_result == 0)
		{
			m_header = new PersistentHeader;
			m_core = new PersistentHeaderCore;
			m_core->load(iter, m_header);
			ASSERT_VALID_STRING_LENGTH(get(iter, m_msg));
			ASSERT_VALID_STRING_LENGTH(get(iter, m_oob));

			if (iter.getSize() > 0)
			{
				m_core->setFolder(iter);
				m_core->setCategory(iter);
			}
		}
	}

	ResGetMultiplePersistentMessages::ResGetMultiplePersistentMessages(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_GETMULTIPLEPERSISTENTMESSAGES, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_messages(nullptr)
	{
	}

	ResGetMultiplePersistentMessages::~ResGetMultiplePersistentMessages()
	{
		unsigned i;

		if (m_messages)
		{
			for (i = 0; i < m_listLength; i++)
			{
				delete m_messages[i];
			}
		}

		delete[] m_messages;
	}

	PersistentMessage ** const ResGetMultiplePersistentMessages::getList() const
	{
		return (PersistentMessage **)m_messages;
	}

	void ResGetMultiplePersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			unsigned i = 0;

			m_messages = new PersistentMessageCore*[m_listLength];

			for (i = 0; i < m_listLength; i++)
			{
				m_messages[i] = new PersistentMessageCore();
				m_messages[i]->load(iter);
			}
		}
	}

	ResGetPersistentHeaders::ResGetPersistentHeaders(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_GETPERSISTENTHEADERS, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_listLength(0),
		m_headers(nullptr),
		m_cores(nullptr)
	{
	}

	ResGetPersistentHeaders::~ResGetPersistentHeaders()
	{
		delete[] m_headers;
		delete[] m_cores;
	}

	void ResGetPersistentHeaders::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			unsigned i = 0;

			this->m_headers = new PersistentHeader[m_listLength];
			m_cores = new PersistentHeaderCore[m_listLength];

			for (i = 0; i < m_listLength; i++)
			{
				m_cores[i].load(iter, &m_headers[i]);
			}

			if (iter.getSize() > 0)
			{
				for (i = 0; i < m_listLength; i++)
				{
					m_cores[i].setFolder(iter);
					m_cores[i].setCategory(iter);
				}
			}
		}
	}

	ResGetPartialPersistentHeaders::ResGetPartialPersistentHeaders(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_PARTIALPERSISTENTHEADERS, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_listLength(0),
		m_headers(nullptr),
		m_cores(nullptr)
	{
	}

	ResGetPartialPersistentHeaders::~ResGetPartialPersistentHeaders()
	{
		delete[] m_headers;
		delete[] m_cores;
	}

	void ResGetPartialPersistentHeaders::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_listLength);

		if (m_listLength > 0)
		{
			unsigned i = 0;

			m_headers = new PersistentHeader[m_listLength];
			m_cores = new PersistentHeaderCore[m_listLength];

			for (i = 0; i < m_listLength; i++)
			{
				m_cores[i].load(iter, &m_headers[i]);
			}

			if (iter.getSize() > 0)
			{
				for (i = 0; i < m_listLength; i++)
				{
					m_cores[i].setFolder(iter);
					m_cores[i].setCategory(iter);
				}
			}
		}
	}

	ResCountPersistentMessages::ResCountPersistentMessages(void *user, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress)
		: GenericResponse(RESPONSE_COUNTPERSISTENTMESSAGES, CHATRESULT_TIMEOUT, user),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_numMessages(0)
	{
	}

	void ResCountPersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_numMessages);
	}

	ResUpdatePersistentMessage::ResUpdatePersistentMessage(void *user, unsigned srcAvatarID, unsigned messageID)
		: GenericResponse(RESPONSE_UPDATEPERSISTENTMESSAGE, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_messageID(messageID)
	{
	}

	void ResUpdatePersistentMessage::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResUpdatePersistentMessages::ResUpdatePersistentMessages(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_UPDATEPERSISTENTMESSAGES, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID)
	{
	}

	void ResUpdatePersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResClassifyPersistentMessages::ResClassifyPersistentMessages(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_CHANGEPERSISTENTFOLDER, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID)
	{
	}

	void ResClassifyPersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResDeleteAllPersistentMessages::ResDeleteAllPersistentMessages(void *user, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress)
		: GenericResponse(RESPONSE_PURGEPERSISTENTMESSAGES, CHATRESULT_TIMEOUT, user),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_numDeleted(0)
	{
	}

	void ResDeleteAllPersistentMessages::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_numDeleted);
	}

	ResUnregisterRoom::ResUnregisterRoom(void *user)
		: GenericResponse(RESPONSE_UNREGISTERROOM, CHATRESULT_TIMEOUT, user),
		m_destRoomID(0)
	{
	}

	void ResUnregisterRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_destRoomID);
	}

	ResFailoverReloginAvatar::ResFailoverReloginAvatar(unsigned avatarID)
		: GenericResponse(RESPONSE_FAILOVER_RELOGINAVATAR, CHATRESULT_TIMEOUT, nullptr),
		m_avatarID(avatarID)
	{
	}

	void ResFailoverReloginAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (iter.getSize() > 0)
		{
			get(iter, m_requiredPriority);
		}
	}

	ResFailoverRecreateRoom::ResFailoverRecreateRoom(unsigned roomID, bool forced)
		: GenericResponse(RESPONSE_FAILOVER_RECREATEROOM, CHATRESULT_TIMEOUT, nullptr),
		m_roomID(roomID),
		m_room(nullptr),
		m_forced(forced)
	{
	}

	void ResFailoverRecreateRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);

		if (m_result == 0)
		{
			m_room = new ChatRoomCore(iter);
		}
	}

	ResFriendConfirm::ResFriendConfirm(unsigned srcAvatarID, void *user)
		: GenericResponse(RESPONSE_CONFIRMFRIEND, CHATRESULT_TIMEOUT, user),
		m_avatarID(srcAvatarID)
	{
	}

	void ResFriendConfirmReciprocate::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResFriendConfirmReciprocate::ResFriendConfirmReciprocate(unsigned srcAvatarID, void *user)
		: GenericResponse(RESPONSE_CONFIRMFRIEND_RECIPROCATE, CHATRESULT_TIMEOUT, user),
		m_avatarID(srcAvatarID)
	{
	}

	void ResFriendConfirm::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResGetFanClubHandle::ResGetFanClubHandle(unsigned stationID, unsigned fanClubCode, void *user)
		: GenericResponse(RESPONSE_GETFANCLUBHANDLE, CHATRESULT_TIMEOUT, user),
		m_stationID(stationID),
		m_fanClubCode(fanClubCode)
	{
	}

	void ResGetFanClubHandle::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_handle));
	}

	ResFindAvatarByUID::ResFindAvatarByUID(void *user)
		: GenericResponse(RESPONSE_FINDAVATARBYUID, CHATRESULT_TIMEOUT, user),
		m_numAvatarsOnline(0),
		m_avatars(nullptr)
	{
	}

	ResFindAvatarByUID::~ResFindAvatarByUID()
	{
		for (unsigned i = 0; i < m_numAvatarsOnline; i++)
		{
			delete m_avatars[i];
		}

		delete[] m_avatars;
	}

	void ResFindAvatarByUID::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_numAvatarsOnline);

		if (m_numAvatarsOnline > 0)
		{
			m_avatars = new ChatAvatarCore*[m_numAvatarsOnline];

			for (unsigned i = 0; i < m_numAvatarsOnline; i++)
			{
				m_avatars[i] = new ChatAvatarCore(iter);
			}
		}
	}

	ResRegistrarGetChatServer::ResRegistrarGetChatServer()
		: GenericResponse(RESPONSE_REGISTRAR_GETCHATSERVER, CHATRESULT_TIMEOUT, nullptr)
	{
	}

	void ResRegistrarGetChatServer::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_hostname));
		get(iter, m_port);
	}

	ResSendApiVersion::ResSendApiVersion()
		: GenericResponse(RESPONSE_SETAPIVERSION, CHATRESULT_TIMEOUT, nullptr)
	{
	}

	void ResSendApiVersion::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, (uint32 &)m_serverVersion);
	}

	ResAddSnoopAvatar::ResAddSnoopAvatar(void *user, unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericResponse(RESPONSE_ADDSNOOPAVATAR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarName(destName.string_data, destName.string_length),
		m_destAvatarAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void ResAddSnoopAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResRemoveSnoopAvatar::ResRemoveSnoopAvatar(void *user, unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericResponse(RESPONSE_REMOVESNOOPAVATAR, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarName(destName.string_data, destName.string_length),
		m_destAvatarAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void ResRemoveSnoopAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResAddSnoopRoom::ResAddSnoopRoom(void *user, unsigned srcAvatarID, const ChatUnicodeString &roomAddress)
		: GenericResponse(RESPONSE_ADDSNOOPROOM, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length)
	{
	}

	void ResAddSnoopRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResRemoveSnoopRoom::ResRemoveSnoopRoom(void *user, unsigned srcAvatarID, const ChatUnicodeString &roomAddress)
		: GenericResponse(RESPONSE_REMOVESNOOPROOM, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length)
	{
	}

	void ResRemoveSnoopRoom::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}

	ResGetSnoopList::ResGetSnoopList(void *user, unsigned srcAvatarID)
		: GenericResponse(RESPONSE_GETSNOOPLIST, CHATRESULT_TIMEOUT, user),
		m_srcAvatarID(srcAvatarID),
		m_avatarSnoops(nullptr),
		m_roomSnoops(nullptr)
	{
	}

	ResGetSnoopList::~ResGetSnoopList()
	{
		for (unsigned i = 0; i < m_avatarSnoopListLength; i++)
		{
			delete m_avatarSnoops[i];
		}
		delete[] m_avatarSnoops;

		for (unsigned j = 0; j < m_roomSnoopListLength; j++)
		{
			delete m_roomSnoops[j];
		}
		delete[] m_roomSnoops;
	}

	void ResGetSnoopList::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_avatarSnoopListLength);
		get(iter, m_roomSnoopListLength);

		if (m_avatarSnoopListLength > 0)
		{
			m_avatarSnoops = new AvatarSnoopPair*[m_avatarSnoopListLength];

			for (unsigned i = 0; i < m_avatarSnoopListLength; i++)
			{
				Plat_Unicode::String name;
				Plat_Unicode::String addr;
				ASSERT_VALID_STRING_LENGTH(get(iter, name));
				ASSERT_VALID_STRING_LENGTH(get(iter, addr));

				m_avatarSnoops[i] = new AvatarSnoopPair(name, addr);
			}
		}

		if (m_roomSnoopListLength > 0)
		{
			m_roomSnoops = new ChatUnicodeString*[m_roomSnoopListLength];

			for (unsigned j = 0; j < m_roomSnoopListLength; j++)
			{
				Plat_Unicode::String room;
				ASSERT_VALID_STRING_LENGTH(get(iter, room));

				m_roomSnoops[j] = new ChatUnicodeString(room);
			}
		}
	}

	ResTransferAvatar::ResTransferAvatar(void *user, unsigned userID, unsigned newUserID, const ChatUnicodeString &avatarName, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &newAvatarAddress)
		: GenericResponse(RESPONSE_TRANSFERAVATAR, CHATRESULT_TIMEOUT, user),
		m_userID(userID),
		m_newUserID(newUserID),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_newAvatarName(newAvatarName.string_data, newAvatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_newAvatarAddress(newAvatarAddress.string_data, newAvatarAddress.string_length)
	{
	}

	void ResTransferAvatar::unpack(ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
	}
}; // end namespace
