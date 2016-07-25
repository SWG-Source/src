#include "Request.h"
#include "ChatRoom.h"
#include "ChatAvatarCore.h"
#include "ChatRoomCore.h"
#include "PersistentMessage.h"

namespace ChatSystem
{
	using namespace Base;
	using namespace Plat_Unicode;

	RLoginAvatar::RLoginAvatar(unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, int avatarLoginAttributes)
		: GenericRequest(REQUEST_LOGINAVATAR),
		m_userID(userID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_loginLocation(avatarLoginLocation.string_data, avatarLoginLocation.string_length),
		m_loginPriority(avatarLoginPriority),
		m_loginAttributes(avatarLoginAttributes)
	{
	}

	void RLoginAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_userID);
		put(msg, m_name);
		put(msg, m_address);
		put(msg, m_loginLocation);
		put(msg, m_loginPriority);
		put(msg, m_loginAttributes);
	}

	RTemporaryAvatar::RTemporaryAvatar(unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &avatarLoginLocation)
		: GenericRequest(REQUEST_TEMPORARYAVATAR),
		m_userID(userID),
		m_name(name.string_data, name.string_length),
		m_address(address.string_data, address.string_length),
		m_loginLocation(avatarLoginLocation.string_data, avatarLoginLocation.string_length)
	{
	}

	void RTemporaryAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_userID);
		put(msg, m_name);
		put(msg, m_address);
		put(msg, m_loginLocation);
	}

	RLogoutAvatar::RLogoutAvatar(unsigned avatarID, const ChatUnicodeString & address)
		: GenericRequest(REQUEST_LOGOUTAVATAR)
		, m_avatarID(avatarID)
		, m_address(address.string_data, address.string_length)
	{
	}

	void RLogoutAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_address);
	}

	RDestroyAvatar::RDestroyAvatar(unsigned avatarID, const ChatUnicodeString & address)
		: GenericRequest(REQUEST_DESTROYAVATAR)
		, m_avatarID(avatarID)
		, m_address(address.string_data, address.string_length)
	{
	}

	void RDestroyAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_address);
	}

	RSetAvatarKeywords::RSetAvatarKeywords(unsigned srcAvatarID, const ChatUnicodeString *keywordsList, unsigned keywordsLength)
		: GenericRequest(REQUEST_SETAVATARKEYWORDS),
		m_srcAvatarID(srcAvatarID),
		m_keywordsLength(keywordsLength)
	{
		m_keywordsList = new String[keywordsLength];
		for (unsigned i = 0; i < keywordsLength; i++)
		{
			m_keywordsList[i].assign(keywordsList[i].string_data, keywordsList[i].string_length);
		}
	}

	RSetAvatarKeywords::~RSetAvatarKeywords()
	{
		delete[] m_keywordsList;
	}

	void RSetAvatarKeywords::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_keywordsLength);
		for (unsigned i = 0; i < m_keywordsLength; i++)
		{
			put(msg, m_keywordsList[i]);
		}
	}

	RGetAvatarKeywords::RGetAvatarKeywords(unsigned srcAvatarID)
		: GenericRequest(REQUEST_GETAVATARKEYWORDS),
		m_srcAvatarID(srcAvatarID)
	{
	}

	void RGetAvatarKeywords::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
	}

	RSearchAvatarKeywords::RSearchAvatarKeywords(const ChatUnicodeString &nodeAddress, const ChatUnicodeString *keywordsList, unsigned keywordsLength)
		: GenericRequest(REQUEST_SEARCHAVATARKEYWORDS),
		m_keywordsLength(keywordsLength),
		m_keywordsList(nullptr)
	{
		m_nodeAddress.assign(nodeAddress.string_data);
		m_keywordsList = new String[keywordsLength];
		for (unsigned i = 0; i < keywordsLength; i++)
		{
			m_keywordsList[i].assign(keywordsList[i].string_data, keywordsList[i].string_length);
		}
	}

	RSearchAvatarKeywords::~RSearchAvatarKeywords()
	{
		delete[] m_keywordsList;
	}

	void RSearchAvatarKeywords::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_nodeAddress);
		put(msg, m_keywordsLength);
		for (unsigned i = 0; i < m_keywordsLength; i++)
		{
			put(msg, m_keywordsList[i]);
		}
	}

	RGetAvatar::RGetAvatar(const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_GETAVATAR),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RGetAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_destName);
		put(msg, m_destAddress);
	}

	RGetAnyAvatar::RGetAnyAvatar(const ChatUnicodeString& destName, const ChatUnicodeString& destAddress)
		: GenericRequest(REQUEST_GETANYAVATAR)
		, m_destName(destName.string_data, destName.string_length)
		, m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RGetAnyAvatar::pack(Base::ByteStream& msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_destName);
		put(msg, m_destAddress);
	}

	RAvatarList::RAvatarList(unsigned userID)
		: GenericRequest(REQUEST_AVATARLIST)
		, m_userID(userID)
	{
	}

	void RAvatarList::pack(Base::ByteStream& msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_userID);
	}

	RSetAvatarAttributes::RSetAvatarAttributes(unsigned avatarID, const ChatUnicodeString &srcAddress, unsigned long avatarAttributes, bool persistent)
		: GenericRequest(REQUEST_SETAVATARATTRIBUTES),
		m_avatarID(avatarID),
		m_avatarAttributes(avatarAttributes),
		m_persistent(persistent),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RSetAvatarAttributes::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, (uint32)m_avatarAttributes);
		put(msg, m_persistent);
		put(msg, m_srcAddress);
	}

	RSetAvatarStatusMessage::RSetAvatarStatusMessage(unsigned avatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString& statusMessage)
		: GenericRequest(REQUEST_SETAVATARSTATUSMESSAGE),
		m_avatarID(avatarID),
		m_statusMessage(statusMessage.data(), statusMessage.length()),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RSetAvatarStatusMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_statusMessage);
		put(msg, m_srcAddress);
	}

	RSetAvatarForwardingEmail::RSetAvatarForwardingEmail(unsigned avatarID, const ChatUnicodeString &avatarForwardingEmail)
		: GenericRequest(REQUEST_SETAVATAREMAIL),
		m_avatarID(avatarID),
		m_avatarForwardingEmail(avatarForwardingEmail.data(), avatarForwardingEmail.length())
	{
	}

	void RSetAvatarForwardingEmail::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_avatarForwardingEmail);
	}

	RSetAvatarInboxLimit::RSetAvatarInboxLimit(unsigned avatarID, unsigned long avatarInboxLimit)
		: GenericRequest(REQUEST_SETAVATARINBOXLIMIT),
		m_avatarID(avatarID),
		m_avatarInboxLimit(avatarInboxLimit)
	{
	}

	void RSetAvatarInboxLimit::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, (uint32)m_avatarInboxLimit);
	}

	RGetRoom::RGetRoom(const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_GETROOM),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length)
	{
	}

	void RGetRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_roomAddress);
	}

	RCreateRoom::RCreateRoom(unsigned avatarID, const ChatUnicodeString & srcAddress, const RoomParams &params, const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_CREATEROOM),
		m_avatarID(avatarID),
		m_roomName(params.getRoomName().string_data, params.getRoomName().string_length),
		m_roomTopic(params.getRoomTopic().string_data, params.getRoomTopic().string_length),
		m_roomPassword(params.getRoomPassword().string_data, params.getRoomPassword().string_length),
		m_roomAttributes(params.getRoomAttributes()),
		m_maxRoomSize(params.getRoomMaxSize()),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RCreateRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_roomName);
		put(msg, m_roomTopic);
		put(msg, m_roomPassword);
		put(msg, m_roomAttributes);
		put(msg, m_maxRoomSize);
		put(msg, m_roomAddress);
		put(msg, m_srcAddress);
	}

	RDestroyRoom::RDestroyRoom(unsigned avatarID, const ChatUnicodeString & srcAddress, const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_DESTROYROOM),
		m_avatarID(avatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RDestroyRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarID);
		put(msg, m_roomAddress);
		put(msg, m_srcAddress);
	}

	RSendInstantMessage::RSendInstantMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
		: GenericRequest(REQUEST_SENDINSTANTMESSAGE),
		m_srcAvatarID(srcAvatarID),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RSendInstantMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_msg);
		put(msg, m_oob);
		put(msg, m_srcAddress);
	}

	RSendRoomMessage::RSendRoomMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
		: GenericRequest(REQUEST_SENDROOMMESSAGE),
		m_srcAvatarID(srcAvatarID),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RSendRoomMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destRoomAddress);
		put(msg, m_msg);
		put(msg, m_oob);
		put(msg, m_srcAddress);
	}

	RSendBroadcastMessage::RSendBroadcastMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
		: GenericRequest(REQUEST_SENDBROADCASTMESSAGE),
		m_srcAvatarID(srcAvatarID),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RSendBroadcastMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAddress);
		put(msg, m_msg);
		put(msg, m_oob);
		put(msg, m_srcAddress);
	}

	RFilterMessage::RFilterMessage(const ChatUnicodeString &msg)
		: GenericRequest(REQUEST_FILTERMESSAGE),
		m_msg(msg.string_data, msg.string_length)
	{
	}

	void RFilterMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_msg);
	}

	RFilterMessageEx::RFilterMessageEx(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg)
		: GenericRequest(REQUEST_FILTERMESSAGE_EX),
		m_srcAvatarID(srcAvatarID),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_msg(msg.string_data, msg.string_length)
	{
	}

	void RFilterMessageEx::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_msg);
	}

	RAddFriend::RAddFriend(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, bool confirm)
		: GenericRequest(REQUEST_ADDFRIEND),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_friendComment(friendComment.string_data, friendComment.string_length),
		m_confirm(confirm)
	{
	}

	void RAddFriend::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_friendComment);
		put(msg, m_confirm);
		put(msg, m_srcAddress);
	}

	RAddFriendReciprocate::RAddFriendReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, bool confirm)
		: GenericRequest(REQUEST_ADDFRIEND_RECIPROCATE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_friendComment(friendComment.string_data, friendComment.string_length),
		m_confirm(confirm)
	{
	}

	void RAddFriendReciprocate::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_friendComment);
		put(msg, m_confirm);
		put(msg, m_srcAddress);
	}

	RSetFriendComment::RSetFriendComment(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment)
		: GenericRequest(REQUEST_SETFRIENDCOMMENT),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_friendComment(friendComment.string_data, friendComment.string_length)
	{
	}

	void RSetFriendComment::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_friendComment);
		put(msg, m_srcAddress);
	}

	RRemoveFriend::RRemoveFriend(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_REMOVEFRIEND),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RRemoveFriend::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_srcAddress);
	}
	RRemoveFriendReciprocate::RRemoveFriendReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_REMOVEFRIEND_RECIPROCATE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RRemoveFriendReciprocate::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_srcAddress);
	}
	RFriendStatus::RFriendStatus(unsigned srcAvatarID, const ChatUnicodeString &srcAddress)
		: GenericRequest(REQUEST_FRIENDSTATUS),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RFriendStatus::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_srcAddress);
	}

	RAddIgnore::RAddIgnore(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_ADDIGNORE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RAddIgnore::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_srcAddress);
	}

	RRemoveIgnore::RRemoveIgnore(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_REMOVEIGNORE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destName(destName.string_data, destName.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RRemoveIgnore::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destName);
		put(msg, m_destAddress);
		put(msg, m_srcAddress);
	}

	REnterRoom::REnterRoom(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &roomAddress, const ChatUnicodeString &roomPassword, const RoomParams *roomParams, bool requestingEntry)
		: GenericRequest(REQUEST_ENTERROOM),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length),
		m_roomPassword(roomPassword.string_data, roomPassword.string_length),
		m_requestingEntry(requestingEntry),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
		if (roomParams)
		{
			// passive room creation if room address does not exist IS enabled
			m_passiveCreate = true;
			m_paramRoomTopic.assign(roomParams->getRoomTopic().string_data, roomParams->getRoomTopic().string_length);
			m_paramRoomAttributes = roomParams->getRoomAttributes();
			m_paramRoomMaxSize = roomParams->getRoomMaxSize();
		}
		else
		{
			// passive room creation disabled
			m_passiveCreate = false;
		}
	}

	void REnterRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_roomAddress);
		put(msg, m_roomPassword);
		put(msg, m_passiveCreate);

		if (m_passiveCreate)
		{
			put(msg, m_paramRoomTopic);
			put(msg, m_paramRoomAttributes);
			put(msg, m_paramRoomMaxSize);
		}

		put(msg, m_requestingEntry);
		put(msg, m_srcAddress);
	}

	RAllowRoomEntry::RAllowRoomEntry(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, bool allow)
		: GenericRequest(REQUEST_ALLOWROOMENTRY),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length),
		m_allow(allow)
	{
	}

	void RAllowRoomEntry::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_allow);
		put(msg, m_srcAddress);
	}

	RLeaveRoom::RLeaveRoom(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_LEAVEROOM),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RLeaveRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_roomAddress);
		put(msg, m_srcAddress);
	}

	RAddModerator::RAddModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_ADDMODERATOR),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RAddModerator::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RRemoveModerator::RRemoveModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_REMOVEMODERATOR),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RRemoveModerator::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RAddTemporaryModerator::RAddTemporaryModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_ADDTEMPORARYMODERATOR),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RAddTemporaryModerator::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RRemoveTemporaryModerator::RRemoveTemporaryModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_REMOVETEMPORARYMODERATOR),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RRemoveTemporaryModerator::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RAddBan::RAddBan(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_ADDBAN),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RAddBan::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RRemoveBan::RRemoveBan(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_REMOVEBAN),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RRemoveBan::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RAddInvite::RAddInvite(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_ADDINVITE),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RAddInvite::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RRemoveInvite::RRemoveInvite(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_REMOVEINVITE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RRemoveInvite::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RGrantVoice::RGrantVoice(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_GRANTVOICE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RGrantVoice::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RRevokeVoice::RRevokeVoice(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_REVOKEVOICE),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RRevokeVoice::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RKickAvatar::RKickAvatar(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_KICKAVATAR),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destAvatarName(destAvatarName.string_data, destAvatarName.string_length),
		m_destAvatarAddress(destAvatarAddress.string_data, destAvatarAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RKickAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
		put(msg, m_destRoomAddress);
		put(msg, m_srcAddress);
	}

	RSetRoomParams::RSetRoomParams(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const RoomParams *params)

		: GenericRequest(REQUEST_SETROOMPARAMS),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
		// we take in a const RoomParams, so make our own and guarantee
		// nullptr-terminated char buffers.
		m_newRoomName.assign(params->getRoomName().string_data, params->getRoomName().string_length);
		m_newRoomTopic.assign(params->getRoomTopic().string_data, params->getRoomTopic().string_length);
		m_newRoomPassword.assign(params->getRoomPassword().string_data, params->getRoomPassword().string_length);
		m_newRoomAttributes = params->getRoomAttributes();
		m_newRoomSize = params->getRoomMaxSize();
	}

	void RSetRoomParams::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destRoomAddress);
		put(msg, m_newRoomName);
		put(msg, m_newRoomTopic);
		put(msg, m_newRoomPassword);
		put(msg, m_newRoomAttributes);
		put(msg, m_newRoomSize);
		put(msg, m_srcAddress);
	}

	RChangeRoomOwner::RChangeRoomOwner(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress)
		: GenericRequest(REQUEST_CHANGEROOMOWNER),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length),
		m_newRoomOwnerName(destAvatarName.data(), destAvatarName.length()),
		m_newRoomOwnerAddress(destAvatarAddress.data(), destAvatarAddress.length())
	{
	}

	void RChangeRoomOwner::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destRoomAddress);
		put(msg, m_newRoomOwnerName);
		put(msg, m_newRoomOwnerAddress);
		put(msg, m_srcAddress);
	}

	RGetRoomSummaries::RGetRoomSummaries(const ChatUnicodeString &startNodeAddress, const ChatUnicodeString &roomFilter)
		: GenericRequest(REQUEST_GETROOMSUMMARIES),
		m_startNodeAddress(startNodeAddress.string_data, startNodeAddress.string_length),
		m_roomFilter(roomFilter.string_data, roomFilter.string_length)
	{
	}

	void RGetRoomSummaries::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_startNodeAddress);
		put(msg, m_roomFilter);
	}

	RSendPersistentMessage::RSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatar, const ChatUnicodeString &destAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit)
		: GenericRequest(REQUEST_SENDPERSISTENTMESSAGE),
		m_avatarPresence(0),
		m_srcName(src.string_data, src.string_length),
		m_destAvatar(destAvatar.string_data, destAvatar.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_subject(subject.string_data, subject.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_category(category.string_data, category.string_length),
		m_enforceInboxLimit(enforceInboxLimit),
		m_categoryLimit(categoryLimit),
		m_srcAvatarID(0)
	{
	}

	RSendPersistentMessage::RSendPersistentMessage(unsigned srcAvatarID, const ChatUnicodeString &destAvatar, const ChatUnicodeString &destAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit)
		: GenericRequest(REQUEST_SENDPERSISTENTMESSAGE),
		m_avatarPresence(1),
		m_srcAvatarID(srcAvatarID),
		m_destAvatar(destAvatar.string_data, destAvatar.string_length),
		m_destAddress(destAddress.string_data, destAddress.string_length),
		m_subject(subject.string_data, subject.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_category(category.string_data, category.string_length),
		m_enforceInboxLimit(enforceInboxLimit),
		m_categoryLimit(categoryLimit)
	{
	}

	void RSendPersistentMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarPresence);
		if (m_avatarPresence)
		{
			put(msg, m_srcAvatarID);
		}
		else
		{
			put(msg, m_srcName);
		}
		put(msg, m_destAvatar);
		put(msg, m_destAddress);
		put(msg, m_subject);
		put(msg, m_msg);
		put(msg, m_oob);
		put(msg, m_category);
		put(msg, m_enforceInboxLimit);
		put(msg, m_categoryLimit);
	}

	RSendMultiplePersistentMessages::RSendMultiplePersistentMessages(const ChatUnicodeString &src, unsigned numDestAvatars, const ChatUnicodeString *destAvatars, const ChatUnicodeString *destAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit)
		: GenericRequest(REQUEST_SENDMULTIPLEPERSISTENTMESSAGES),
		m_avatarPresence(0),
		m_srcName(src.string_data, src.string_length),
		m_numDestAvatars(numDestAvatars),
		m_subject(subject.string_data, subject.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_category(category.string_data, category.string_length),
		m_enforceInboxLimit(enforceInboxLimit),
		m_categoryLimit(categoryLimit),
		m_srcAvatarID(0)
	{
		unsigned destIndex;

		m_destAvatars.resize(m_numDestAvatars);
		m_destAddresses.resize(m_numDestAvatars);

		for (destIndex = 0; destIndex < m_numDestAvatars; destIndex++)
		{
			m_destAvatars[destIndex].assign(destAvatars[destIndex].string_data, destAvatars[destIndex].string_length);
			m_destAddresses[destIndex].assign(destAddresses[destIndex].string_data, destAddresses[destIndex].string_length);
		}
	}

	RSendMultiplePersistentMessages::RSendMultiplePersistentMessages(unsigned srcAvatarID, unsigned numDestAvatars, const ChatUnicodeString *destAvatars, const ChatUnicodeString *destAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit)
		: GenericRequest(REQUEST_SENDMULTIPLEPERSISTENTMESSAGES),
		m_avatarPresence(1),
		m_srcAvatarID(srcAvatarID),
		m_numDestAvatars(numDestAvatars),
		m_subject(subject.string_data, subject.string_length),
		m_msg(msg.string_data, msg.string_length),
		m_oob(oob.string_data, oob.string_length),
		m_category(category.string_data, category.string_length),
		m_enforceInboxLimit(enforceInboxLimit),
		m_categoryLimit(categoryLimit)
	{
		unsigned destIndex;

		m_destAvatars.resize(m_numDestAvatars);
		m_destAddresses.resize(m_numDestAvatars);

		for (destIndex = 0; destIndex < m_numDestAvatars; destIndex++)
		{
			m_destAvatars[destIndex].assign(destAvatars[destIndex].string_data, destAvatars[destIndex].string_length);
			m_destAddresses[destIndex].assign(destAddresses[destIndex].string_data, destAddresses[destIndex].string_length);
		}
	}

	void RSendMultiplePersistentMessages::pack(ByteStream &msg)
	{
		unsigned destIndex;

		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarPresence);
		if (m_avatarPresence)
		{
			put(msg, m_srcAvatarID);
		}
		else
		{
			put(msg, m_srcName);
		}
		put(msg, m_numDestAvatars);
		for (destIndex = 0; destIndex < m_numDestAvatars; destIndex++)
		{
			put(msg, m_destAvatars[destIndex]);
			put(msg, m_destAddresses[destIndex]);
		}
		put(msg, m_subject);
		put(msg, m_msg);
		put(msg, m_oob);
		put(msg, m_category);
		put(msg, m_enforceInboxLimit);
		put(msg, m_categoryLimit);
	}

	RAlterPersistentMessage::RAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, unsigned alterationAttributes, const ChatUnicodeString &newSenderName, const ChatUnicodeString &newSenderAddress, const ChatUnicodeString &newSubject, const ChatUnicodeString &newMsg, const ChatUnicodeString &newOOB, const ChatUnicodeString &newCategory, unsigned newSentTime)
		: GenericRequest(REQUEST_ALTERPERISTENTMESSAGE),
		m_destAvatarName(destAvatarName.data(), destAvatarName.length()),
		m_destAvatarAddress(destAvatarAddress.data(), destAvatarAddress.length()),
		m_messageID(messageID),
		m_alterationAttributes(alterationAttributes),
		m_newSenderName(newSenderName.data(), newSenderName.length()),
		m_newSenderAddress(newSenderAddress.data(), newSenderAddress.length()),
		m_newSubject(newSubject.data(), newSubject.length()),
		m_newMsg(newMsg.data(), newMsg.length()),
		m_newOOB(newOOB.data(), newOOB.length()),
		m_newCategory(newCategory.data(), newCategory.length()),
		m_newSentTime(newSentTime)
	{
	}

	RAlterPersistentMessage::~RAlterPersistentMessage()
	{
	}

	void RAlterPersistentMessage::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);

		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);

		put(msg, m_messageID);
		put(msg, m_alterationAttributes);
		put(msg, m_newSenderName);
		put(msg, m_newSenderAddress);
		put(msg, m_newSubject);
		put(msg, m_newMsg);
		put(msg, m_newOOB);
		put(msg, m_newCategory);
		put(msg, m_newSentTime);
	}

	RGetPersistentHeaders::RGetPersistentHeaders(unsigned srcAvatarID, const ChatUnicodeString &category)
		: GenericRequest(REQUEST_GETPERSISTENTHEADERS),
		m_srcAvatarID(srcAvatarID),
		m_category(category.string_data, category.string_length)
	{
	}

	void RGetPersistentHeaders::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_category);
	}

	RGetPartialPersistentHeaders::RGetPartialPersistentHeaders(unsigned srcAvatarID, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category)
		: GenericRequest(REQUEST_PARTIALPERSISTENTHEADERS),
		m_srcAvatarID(srcAvatarID),
		m_maxHeaders(maxHeaders),
		m_inDescendingOrder(inDescendingOrder),
		m_sentTimeStart(sentTimeStart),
		m_category(category.string_data, category.string_length)
	{
	}

	void RGetPartialPersistentHeaders::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_maxHeaders);
		put(msg, m_inDescendingOrder);
		put(msg, m_sentTimeStart);
		put(msg, m_category);
	}

	RCountPersistentMessages::RCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category)
		: GenericRequest(REQUEST_COUNTPERSISTENTMESSAGES),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_category(category.string_data, category.string_length)
	{
	}

	void RCountPersistentMessages::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarName);
		put(msg, m_avatarAddress);
		put(msg, m_category);
	}

	RGetPersistentMessage::RGetPersistentMessage(unsigned srcAvatarID, unsigned messageID)
		: GenericRequest(REQUEST_GETPERSISTENTMESSAGE),
		m_srcAvatarID(srcAvatarID),
		m_messageID(messageID)
	{
	}

	void RGetPersistentMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_messageID);
	}

	RGetMultiplePersistentMessages::RGetMultiplePersistentMessages(unsigned srcAvatarID, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category)
		: GenericRequest(REQUEST_GETMULTIPLEPERSISTENTMESSAGES),
		m_srcAvatarID(srcAvatarID),
		m_maxHeaders(maxHeaders),
		m_inDescendingOrder(inDescendingOrder),
		m_sentTimeStart(sentTimeStart),
		m_category(category.string_data, category.string_length)
	{
	}

	void RGetMultiplePersistentMessages::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_maxHeaders);
		put(msg, m_inDescendingOrder);
		put(msg, m_sentTimeStart);
		put(msg, m_category);
	}

	RUpdatePersistentMessage::RUpdatePersistentMessage(unsigned srcAvatarID, unsigned messageID, PersistentStatus status)
		: GenericRequest(REQUEST_UPDATEPERSISTENTMESSAGE),
		m_srcAvatarID(srcAvatarID),
		m_messageID(messageID),
		m_status(status)
	{
	}

	void RUpdatePersistentMessage::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_messageID);
		put(msg, m_status);
	}

	RUpdatePersistentMessages::RUpdatePersistentMessages(unsigned srcAvatarID, PersistentStatus currentStatus, PersistentStatus newStatus, const ChatUnicodeString &category)
		: GenericRequest(REQUEST_UPDATEPERSISTENTMESSAGES),
		m_srcAvatarID(srcAvatarID),
		m_currentStatus(currentStatus),
		m_newStatus(newStatus),
		m_category(category.string_data, category.string_length)
	{
	}

	void RUpdatePersistentMessages::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_currentStatus);
		put(msg, m_newStatus);
		put(msg, m_category);
	}

	RClassifyPersistentMessages::RClassifyPersistentMessages(unsigned srcAvatarID, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder)
		: GenericRequest(REQUEST_CHANGEPERSISTENTFOLDER),
		m_srcAvatarID(srcAvatarID),
		m_newFolder(newFolder.data(), newFolder.length())
	{
		for (; numIDs > 0; numIDs--)
		{
			m_messageIDs.insert(messageIDs[numIDs - 1]);
		}
	}

	RClassifyPersistentMessages::RClassifyPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder)
		: GenericRequest(REQUEST_CHANGEPERSISTENTFOLDER),
		m_srcAvatarID(0),
		m_avatarName(avatarName.data(), avatarName.length()),
		m_avatarAddress(avatarAddress.data(), avatarAddress.length()),
		m_newFolder(newFolder.data(), newFolder.length())
	{
		for (; numIDs > 0; numIDs--)
		{
			m_messageIDs.insert(messageIDs[numIDs - 1]);
		}
	}

	void RClassifyPersistentMessages::pack(ByteStream &msg)
	{
		std::set<unsigned>::iterator idIter;

		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_messageIDs.size());
		for (idIter = m_messageIDs.begin(); idIter != m_messageIDs.end(); idIter++)
		{
			put(msg, *idIter);
		}
		put(msg, m_newFolder);
		put(msg, m_avatarName);
		put(msg, m_avatarAddress);
	}

	RDeleteAllPersistentMessages::RDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, unsigned olderThan)
		: GenericRequest(REQUEST_PURGEPERSISTENTMESSAGES),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_category(category.string_data, category.string_length),
		m_olderThan(olderThan)
	{
	}

	void RDeleteAllPersistentMessages::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatarName);
		put(msg, m_avatarAddress);
		put(msg, m_category);
		put(msg, m_olderThan);
	}

	RUnregisterRoom::RUnregisterRoom(const ChatUnicodeString &destRoomAddress)
		: GenericRequest(REQUEST_UNREGISTERROOM),
		m_destRoomAddress(destRoomAddress.string_data, destRoomAddress.string_length)
	{
	}

	void RUnregisterRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_destRoomAddress);
	}

	RIgnoreStatus::RIgnoreStatus(unsigned srcAvatarID, const ChatUnicodeString &srcAddress)
		: GenericRequest(REQUEST_IGNORESTATUS),
		m_srcAvatarID(srcAvatarID),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RIgnoreStatus::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_srcAddress);
	}

	RFailoverReloginAvatar::RFailoverReloginAvatar(ChatAvatarCore *avatar)
		: GenericRequest(REQUEST_FAILOVER_RELOGINAVATAR),
		m_avatar(avatar)
	{
	}

	void RFailoverReloginAvatar::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_avatar->getAvatarID());
		put(msg, m_avatar->getUserID());
		put(msg, m_avatar->getName());
		put(msg, m_avatar->getAddress());
		put(msg, m_avatar->getLoginLocation());
		put(msg, m_avatar->getLoginPriority());
		put(msg, m_avatar->getAttributes());
	}

	RFailoverRecreateRoom::RFailoverRecreateRoom(ChatRoomCore *room)
		: GenericRequest(REQUEST_FAILOVER_RECREATEROOM),
		m_room(room)
	{
	}

	void RFailoverRecreateRoom::pack(ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);

		// send full room object, except that the avatars sent only
		//   includes those on the API--"each API for itself" method reduces
		//   the amount of wasted iterations on the server.
		m_room->serializeWithLocalAvatarsOnly(msg);
	}

	RFriendConfirm::RFriendConfirm(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, unsigned destAvatarID, bool confirm)
		: GenericRequest(REQUEST_CONFIRMFRIEND),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarID(destAvatarID),
		m_confirm(confirm != 0),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RFriendConfirm::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarID);
		put(msg, m_confirm);
		put(msg, m_srcAddress);
	}

	RFriendConfirmReciprocate::RFriendConfirmReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, unsigned destAvatarID, bool confirm)
		: GenericRequest(REQUEST_CONFIRMFRIEND_RECIPROCATE),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarID(destAvatarID),
		m_confirm(confirm != 0),
		m_srcAddress(srcAddress.string_data, srcAddress.string_length)
	{
	}

	void RFriendConfirmReciprocate::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarID);
		put(msg, m_confirm);
		put(msg, m_srcAddress);
	}

	RGetFanClubHandle::RGetFanClubHandle(unsigned stationID, unsigned fanClubCode)
		: GenericRequest(REQUEST_GETFANCLUBHANDLE),
		m_stationID(stationID),
		m_fanClubCode(fanClubCode)
	{
	}

	void RGetFanClubHandle::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_stationID);
		put(msg, m_fanClubCode);
	}

	RFindAvatarByUID::RFindAvatarByUID(unsigned userID)
		: GenericRequest(REQUEST_FINDAVATARBYUID),
		m_userID(userID)
	{
	}

	void RFindAvatarByUID::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_userID);
	}

	RRegistrarGetChatServer::RRegistrarGetChatServer(std::string &hostname, unsigned short port)
		: GenericRequest(REQUEST_REGISTRAR_GETCHATSERVER),
		m_hostname(narrowToWide(hostname)),
		m_port(port)
	{
	}

	void RRegistrarGetChatServer::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_hostname);
		put(msg, m_port);
	}

	RSendApiVersion::RSendApiVersion(unsigned long version)
		: GenericRequest(REQUEST_SETAPIVERSION),
		m_version(version)
	{
	}

	void RSendApiVersion::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, (uint32)m_version);
	}

	RAddSnoopAvatar::RAddSnoopAvatar(unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_ADDSNOOPAVATAR),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarName(destName.string_data, destName.string_length),
		m_destAvatarAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RAddSnoopAvatar::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
	}

	RRemoveSnoopAvatar::RRemoveSnoopAvatar(unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress)
		: GenericRequest(REQUEST_REMOVESNOOPAVATAR),
		m_srcAvatarID(srcAvatarID),
		m_destAvatarName(destName.string_data, destName.string_length),
		m_destAvatarAddress(destAddress.string_data, destAddress.string_length)
	{
	}

	void RRemoveSnoopAvatar::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_destAvatarName);
		put(msg, m_destAvatarAddress);
	}

	RAddSnoopRoom::RAddSnoopRoom(unsigned srcAvatarID, const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_ADDSNOOPROOM),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length)
	{
	}

	void RAddSnoopRoom::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_roomAddress);
	}

	RRemoveSnoopRoom::RRemoveSnoopRoom(unsigned srcAvatarID, const ChatUnicodeString &roomAddress)
		: GenericRequest(REQUEST_REMOVESNOOPROOM),
		m_srcAvatarID(srcAvatarID),
		m_roomAddress(roomAddress.string_data, roomAddress.string_length)
	{
	}

	void RRemoveSnoopRoom::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
		put(msg, m_roomAddress);
	}

	RGetSnoopList::RGetSnoopList(unsigned srcAvatarID)
		: GenericRequest(REQUEST_GETSNOOPLIST),
		m_srcAvatarID(srcAvatarID)
	{
	}

	void RGetSnoopList::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_srcAvatarID);
	}

	RTransferAvatar::RTransferAvatar(unsigned userID, unsigned newUserID, const ChatUnicodeString &avatarName, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &newAvatarAddress, bool transferPersistentMessages)
		: GenericRequest(REQUEST_TRANSFERAVATAR),
		m_userID(userID),
		m_newUserID(newUserID),
		m_avatarName(avatarName.string_data, avatarName.string_length),
		m_newAvatarName(newAvatarName.string_data, newAvatarName.string_length),
		m_avatarAddress(avatarAddress.string_data, avatarAddress.string_length),
		m_newAvatarAddress(newAvatarAddress.string_data, newAvatarAddress.string_length),
		m_transferPersistentMessages(transferPersistentMessages)
	{
	}

	void RTransferAvatar::pack(Base::ByteStream &msg)
	{
		put(msg, m_type);
		put(msg, m_track);
		put(msg, m_userID);
		put(msg, m_newUserID);
		put(msg, m_avatarName);
		put(msg, m_newAvatarName);
		put(msg, m_avatarAddress);
		put(msg, m_newAvatarAddress);
		put(msg, m_transferPersistentMessages);
	}
};