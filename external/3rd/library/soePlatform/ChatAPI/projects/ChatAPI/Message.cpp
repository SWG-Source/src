#include "Message.h"
#include "PersistentMessage.h"
#include "PersistentMessageCore.h"


unsigned numRead;
namespace ChatSystem 
{
	using namespace std;
	using namespace Base;
	using namespace Plat_Unicode;

	MInstantMessage::MInstantMessage(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_INSTANTMESSAGE),
	  m_srcAvatar(iter)
	{
		get(iter, m_destAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_msg));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_oob));
	}

	MRoomMessage::MRoomMessage(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ROOMMESSAGE),
	  m_destList(nullptr),
	  m_srcAvatar(iter),
	  m_messageID(0)
	{
		get(iter, m_roomID);
		get(iter, m_listLength);

		m_destList = new unsigned[m_listLength];
		for(unsigned i = 0; i < m_listLength; i++)
		{
			get(iter, m_destList[i]);
		}
		ASSERT_VALID_STRING_LENGTH(get(iter, m_msg));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_oob));

		if (iter.getSize() >= sizeof(unsigned))
		{
			get(iter, m_messageID);
		}
	}

	MRoomMessage::~MRoomMessage()
	{
		delete[] m_destList;
		m_destList = nullptr;
	}

	MBroadcastMessage::MBroadcastMessage(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_BROADCASTMESSAGE),
	  m_srcAvatar(iter),
	  m_destList(nullptr)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_srcAddress));
		get(iter, m_listLength);

		m_destList = new unsigned[m_listLength];
		for(unsigned i = 0; i < m_listLength; i++)
		{
			get(iter, m_destList[i]);
		}
		ASSERT_VALID_STRING_LENGTH(get(iter, m_msg));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_oob));
	}

	MBroadcastMessage::~MBroadcastMessage()
	{
		delete[] m_destList;
		m_destList = nullptr;
	}

	MFilterMessage::MFilterMessage(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FILTERMESSAGE)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_inmsg));
	}

	MFriendLogin::MFriendLogin(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FRIENDLOGIN),
	  m_friendAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_friendAddress));
		get(iter, m_destAvatarID);
        ASSERT_VALID_STRING_LENGTH(get(iter, m_friendStatus));
	}

	MFriendLogout::MFriendLogout(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FRIENDLOGOUT),
	  m_friendAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_friendAddress));
		get(iter, m_destAvatarID);
	}

    MFriendStatus::MFriendStatus(ByteStream::ReadIterator &iter)
        : GenericMessage(MESSAGE_FRIENDSTATUS),
        m_friendAvatar(iter)
    {
        ASSERT_VALID_STRING_LENGTH(get(iter, m_friendAddress));
        get(iter, m_destAvatarID);
        ASSERT_VALID_STRING_LENGTH(get(iter, m_friendStatus));
    }

	MKickRoom::MKickRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_KICKROOM),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		get(iter, m_roomID);
	}

	MKickAvatar::MKickAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_KICKAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MAddModeratorRoom::MAddModeratorRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDMODERATORROOM)
	{
		get(iter, m_srcAvatarID);
		m_destAvatar = new ChatAvatarCore(iter);
		get(iter, m_roomID);
	}

	MAddModeratorAvatar::MAddModeratorAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDMODERATORAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MRemoveModeratorRoom::MRemoveModeratorRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEMODERATORROOM)
	{
		get(iter, m_srcAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarAddress));
		get(iter, m_roomID);
	}

	MRemoveModeratorAvatar::MRemoveModeratorAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEMODERATORAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MAddTemporaryModeratorRoom::MAddTemporaryModeratorRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDMODERATORROOM)
	{
		get(iter, m_srcAvatarID);
		m_destAvatar = new ChatAvatarCore(iter);
		get(iter, m_roomID);
	}

	MAddTemporaryModeratorAvatar::MAddTemporaryModeratorAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDMODERATORAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MRemoveTemporaryModeratorRoom::MRemoveTemporaryModeratorRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEMODERATORROOM)
	{
		get(iter, m_srcAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarAddress));
		get(iter, m_roomID);
	}

	MRemoveTemporaryModeratorAvatar::MRemoveTemporaryModeratorAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEMODERATORAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MAddBanRoom::MAddBanRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDBANROOM),
	  m_srcAvatar(iter)
	{
		m_destAvatar = new ChatAvatarCore(iter);
		get(iter, m_roomID);
	}

	MAddBanAvatar::MAddBanAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDBANAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MRemoveBanRoom::MRemoveBanRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEBANROOM)
	{
		get(iter, m_srcAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarAddress));
		get(iter, m_roomID);
	}

	MRemoveBanAvatar::MRemoveBanAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEBANAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MAddInviteRoom::MAddInviteRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDINVITEROOM),
	  m_srcAvatar(iter)
	{
		m_destAvatar = new ChatAvatarCore(iter);
		get(iter, m_roomID);
	}

	MAddInviteAvatar::MAddInviteAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDINVITEAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MRemoveInviteRoom::MRemoveInviteRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEINVITEROOM)
	{
		get(iter, m_srcAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarAddress));
		get(iter, m_roomID);
	}

	MRemoveInviteAvatar::MRemoveInviteAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEINVITEAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MGrantVoiceRoom::MGrantVoiceRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDINVITEROOM),
	  m_srcAvatar(iter)
	{
		m_destAvatar = new ChatAvatarCore(iter);
		get(iter, m_roomID);
	}

	MGrantVoiceAvatar::MGrantVoiceAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDINVITEAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MRevokeVoiceRoom::MRevokeVoiceRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEINVITEROOM)
	{
		get(iter, m_srcAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAvatarAddress));
		get(iter, m_roomID);
	}

	MRevokeVoiceAvatar::MRevokeVoiceAvatar(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEINVITEAVATAR),
	  m_srcAvatar(iter),
	  m_destAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomAddress));
	}

	MEnterRoom::MEnterRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ENTERROOM),
	  m_srcAvatar(new ChatAvatarCore(iter))
	{
		get(iter, m_roomID);
	}

	MLeaveRoom::MLeaveRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_LEAVEROOM)
	{
		get(iter, m_avatarID);
		get(iter, m_roomID);
	}

	MDestroyRoom::MDestroyRoom(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_DESTROYROOM),
	  m_srcAvatar(iter)
	{
		get(iter, m_roomID);
	}

	MSetRoomParams::MSetRoomParams(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_SETROOMPARAMS),
	  m_srcAvatar(iter)
	{
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomTopic));
		get(iter, m_roomAttributes);
		get(iter, m_maxRoomSize);
		get(iter, m_roomID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_roomPassword));
	}

	MPersistentMessage::MPersistentMessage(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_PERSISTENTMESSAGE)
	{
		m_core = new PersistentHeaderCore();
		m_header = new PersistentHeader();
		get(iter, m_destAvatarID);

		// unpack messageID, 
		//        destAvatarID,
		//        fromName,
		//        fromAddress,
		//        subject,
		//        sentTime, and
		//        status into PersistentHeaderCore object
		m_core->load(iter, m_header);

		if (iter.getSize() > 0)
		{
			m_core->setCategory(iter);
		}
	}

	MPersistentMessage::~MPersistentMessage()
	{
		delete m_core;
		delete m_header;
	}

	MForcedLogout::MForcedLogout(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FORCEDLOGOUT)
	{
		get(iter, m_avatarID);
	}

	MUnregisterRoomReady::MUnregisterRoomReady(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_UNREGISTERROOMREADY)
	{
		get(iter, m_roomID);
	}

	MAddAdmin::MAddAdmin(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_ADDADMIN),
	  m_avatar(new ChatAvatarCore(iter))
	{
		get(iter, m_roomID);
	}

	MRemoveAdmin::MRemoveAdmin(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REMOVEADMIN)
	{
		get(iter, m_avatarID);
		get(iter, m_roomID);
	}

	MFriendConfirmRequest::MFriendConfirmRequest(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FRIENDCONFIRMREQUEST),
	  m_srcAvatar(iter)
	{
		get(iter, m_destAvatarID);
	}

	MFriendConfirmResponse::MFriendConfirmResponse(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FRIENDCONFIRMRESPONSE),
	  m_srcAvatar(iter)
	{
		get(iter, m_destAvatarID);
		get(iter, m_confirm);
	}

	MFriendConfirmReciprocateRequest::MFriendConfirmReciprocateRequest(ByteStream::ReadIterator &iter)
		: GenericMessage(MESSAGE_FRIENDCONFIRMRECIPROCATE_REQUEST),
		m_srcAvatar(iter)
	{
		get(iter, m_destAvatarID);
	}

	MFriendConfirmReciprocateResponse::MFriendConfirmReciprocateResponse(ByteStream::ReadIterator &iter)
		: GenericMessage(MESSAGE_FRIENDCONFIRMRECIPROCATE_RESPONSE),
		m_srcAvatar(iter)
	{
		get(iter, m_destAvatarID);
		get(iter, m_confirm);
	}

	MChangeRoomOwner::MChangeRoomOwner(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_CHANGEROOMOWNER)
	{
		get(iter, m_destRoomID);
		get(iter, m_newRoomOwnerID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_newRoomOwnerName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_newRoomOwnerAddress));
	}

	MRoomEntryRequest::MRoomEntryRequest(Base::ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_REQUESTROOMENTRY),
	  m_requestorAvatar(iter)
	{	
		get(iter, m_roomOwnerID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_requestedRoomAddress));
	}

	MDelayedRoomEntry::MDelayedRoomEntry(Base::ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_DELAYEDROOMENTRY)
	{
		int numExtraRooms = 0;

		get(iter, m_avatarID);
		get(iter, m_roomID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_enteredAddress));
		get(iter, numExtraRooms);

		while (numExtraRooms > 0)
		{
			m_setExtraRooms.insert(new ChatRoomCore(iter));
			numExtraRooms--;
		}
	}

	MDeniedRoomEntry::MDeniedRoomEntry(Base::ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_DENIEDROOMENTRY)
	{
		get(iter, m_requestorAvatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_requestedRoomAddress));
	}

	MForceRoomFailover::MForceRoomFailover(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_FORCEROOMFAILOVER)
	{
		unsigned numAIDs(0);
		get(iter, numAIDs);

		String aid;
		for (unsigned i = 0; i < numAIDs; i++)
		{
			ASSERT_VALID_STRING_LENGTH(get(iter, aid));
			m_aidList.push_back(aid);
		}
	}

	MSnoop::MSnoop(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_SNOOP)
	{
		get(iter, m_snoopType);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_snooperName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_snooperAddr));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_srcName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_srcAddr));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAddr));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_message));
	}

	MUIDList::MUIDList(ByteStream::ReadIterator &iter)
	: GenericMessage(MESSAGE_UIDLIST)
	{
		unsigned numUIDs;
		get(iter, numUIDs);
		for (unsigned i = 0; i < numUIDs; i++)
		{
			Plat_Unicode::String uid;
			ASSERT_VALID_STRING_LENGTH(get(iter, uid));
			m_uidList.insert(toUpper(uid));
		}
	}

	MNotifyFriendIsRemoved::MNotifyFriendIsRemoved(ByteStream::ReadIterator &iter)
		: GenericMessage(MESSAGE_NOTIFY_FRIEND_IS_REMOVED)
	{
		get(iter, m_avatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_destAddress));

	}

	MNotifyFriendsListChange::MNotifyFriendsListChange(ByteStream::ReadIterator &iter)
		: GenericMessage(MESSAGE_NOTIFY_FRIENDS_LIST_CHANGE)
	{
		get(iter, m_avatarID);
		ASSERT_VALID_STRING_LENGTH(get(iter, m_originalName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_originalAddress));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_newName));
		ASSERT_VALID_STRING_LENGTH(get(iter, m_newAddress));
	}

};
