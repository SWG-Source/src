#if !defined (MESSAGE_H_)
#define MESSAGE_H_
#pragma warning (disable : 4786)

#include <GenericAPI/GenericMessage.h>
#include <Unicode/Unicode.h>

#include "ChatRoomCore.h"
#include "ChatAvatarCore.h"

#include <string>
#include <list>
#include <set>

namespace ChatSystem 
{

	class PersistentHeader;
	class PersistentHeaderCore;

	enum eMessageType
	{
		// ChatAvatar message types
		MESSAGE_INSTANTMESSAGE,		// 0
		MESSAGE_ROOMMESSAGE,
		MESSAGE_BROADCASTMESSAGE,
		MESSAGE_FRIENDLOGIN,
		MESSAGE_FRIENDLOGOUT,
		MESSAGE_KICKROOM,			// 5

		// ChatRoom message types
		MESSAGE_ADDMODERATORROOM,
		MESSAGE_REMOVEMODERATORROOM,
		MESSAGE_REMOVEMODERATORAVATAR,
		MESSAGE_ADDBANROOM,
		MESSAGE_REMOVEBANROOM,		// 10
		MESSAGE_REMOVEBANAVATAR,
		MESSAGE_ADDINVITEROOM,
		MESSAGE_ADDINVITEAVATAR,
		MESSAGE_REMOVEINVITEROOM,
		MESSAGE_REMOVEINVITEAVATAR,	// 15
		MESSAGE_ENTERROOM,
		MESSAGE_LEAVEROOM,
		MESSAGE_DESTROYROOM,
		MESSAGE_SETROOMPARAMS,
		MESSAGE_PERSISTENTMESSAGE,	// 20
		MESSAGE_FORCEDLOGOUT,
		MESSAGE_UNREGISTERROOMREADY,
		MESSAGE_KICKAVATAR,
		MESSAGE_ADDMODERATORAVATAR,
		MESSAGE_ADDBANAVATAR,		// 25
		MESSAGE_ADDADMIN,
		MESSAGE_REMOVEADMIN,
		MESSAGE_FRIENDCONFIRMREQUEST,
		MESSAGE_FRIENDCONFIRMRESPONSE,
		MESSAGE_CHANGEROOMOWNER,	// 30
		MESSAGE_FORCEROOMFAILOVER,
		MESSAGE_ADDTEMPORARYMODERATORROOM,
		MESSAGE_ADDTEMPORARYMODERATORAVATAR,
		MESSAGE_REMOVETEMPORARYMODERATORROOM,
		MESSAGE_REMOVETEMPORARYMODERATORAVATAR,	// 35
		MESSAGE_GRANTVOICEROOM,
		MESSAGE_GRANTVOICEAVATAR,
		MESSAGE_REVOKEVOICEROOM,
		MESSAGE_REVOKEVOICEAVATAR,
		MESSAGE_SNOOP,				// 40
		MESSAGE_UIDLIST,
		MESSAGE_REQUESTROOMENTRY,
		MESSAGE_DELAYEDROOMENTRY,
		MESSAGE_DENIEDROOMENTRY,
        MESSAGE_FRIENDSTATUS,		// 45
		MESSAGE_FRIENDCONFIRMRECIPROCATE_REQUEST,
		MESSAGE_FRIENDCONFIRMRECIPROCATE_RESPONSE,
		MESSAGE_FILTERMESSAGE,
		MESSAGE_FAILOVER_AVATAR_LIST,
		MESSAGE_NOTIFY_FRIENDS_LIST_CHANGE, // 50
		MESSAGE_NOTIFY_FRIEND_IS_REMOVED

	};

	class MInstantMessage : public GenericAPI::GenericMessage
	{
	public:
		MInstantMessage(Base::ByteStream::ReadIterator &iter);
		virtual ~MInstantMessage() {};

		unsigned getDestAvatarID() const { return m_destAvatarID; }
		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const Plat_Unicode::String &getMsg() const { return m_msg; }
		const Plat_Unicode::String &getOOB() const { return m_oob; }

	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_destAvatarID;
		Plat_Unicode::String m_msg;
		Plat_Unicode::String m_oob;
	};

	class MRoomMessage : public GenericAPI::GenericMessage
	{
	public:
		MRoomMessage(Base::ByteStream::ReadIterator &iter);
		virtual ~MRoomMessage();

		unsigned getListLength() const { return m_listLength; }
		const unsigned *getDestList() const { return m_destList; }
		ChatAvatarCore *getSrcAvatar() { return &m_srcAvatar; }
		unsigned getRoomID() const { return m_roomID; }
		const Plat_Unicode::String &getMsg() const { return m_msg; }
		const Plat_Unicode::String &getOOB() const { return m_oob; }
		unsigned getMessageID() const { return m_messageID; }

	private:
		unsigned m_listLength;
		unsigned *m_destList;
		ChatAvatarCore m_srcAvatar;
		unsigned m_roomID;
		Plat_Unicode::String m_msg;
		Plat_Unicode::String m_oob;
		unsigned m_messageID;
	};

	class MBroadcastMessage : public GenericAPI::GenericMessage
	{
	public:
		MBroadcastMessage(Base::ByteStream::ReadIterator &iter);
		virtual ~MBroadcastMessage();

		unsigned getListLength() const { return m_listLength; }
		const unsigned *getDestList() const { return m_destList; }
		ChatAvatarCore *getSrcAvatar() { return &m_srcAvatar; }
		const Plat_Unicode::String &getSrcAddress() const { return m_srcAddress; }
		const Plat_Unicode::String &getMsg() const { return m_msg; }
		const Plat_Unicode::String &getOOB() const { return m_oob; }

	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_listLength;
		unsigned *m_destList;
		Plat_Unicode::String m_srcAddress;
		Plat_Unicode::String m_msg;
		Plat_Unicode::String m_oob;
	};

	class MFilterMessage : public GenericAPI::GenericMessage
	{
	public:
		MFilterMessage(Base::ByteStream::ReadIterator &iter);
		virtual ~MFilterMessage() {};

		const Plat_Unicode::String &getMsg() const { return m_inmsg; }

	private:
		Plat_Unicode::String m_inmsg;
	};

	class MFriendLogin : public GenericAPI::GenericMessage
	{
	public:
		MFriendLogin(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendLogin() {};

		const ChatAvatarCore *getFriendAvatar() const { return &m_friendAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
		const Plat_Unicode::String &getFriendAddress() const { return m_friendAddress; }
        const Plat_Unicode::String &getFriendStatusMessage() const { return m_friendStatus; }

	private:
		ChatAvatarCore m_friendAvatar;
		unsigned m_destAvatarID;
		Plat_Unicode::String m_friendAddress;
        Plat_Unicode::String m_friendStatus;
	};

	class MFriendLogout : public GenericAPI::GenericMessage
	{
	public:
		MFriendLogout(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendLogout() {};

		const ChatAvatarCore *getFriendAvatar() const { return &m_friendAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
		const Plat_Unicode::String &getFriendAddress() const { return m_friendAddress; }

	private:
		ChatAvatarCore m_friendAvatar;
		unsigned m_destAvatarID;
		Plat_Unicode::String m_friendAddress;
	};

    class MFriendStatus : public GenericAPI::GenericMessage
    {
    public:
        MFriendStatus(Base::ByteStream::ReadIterator &iter);
        virtual ~MFriendStatus() {};

        const ChatAvatarCore *getFriendAvatar() const { return &m_friendAvatar; }
        unsigned getDestAvatarID() const { return m_destAvatarID; }
        const Plat_Unicode::String &getFriendAddress() const { return m_friendAddress; }
        const Plat_Unicode::String &getFriendStatusMessage() const { return m_friendStatus; }

    private:
        ChatAvatarCore m_friendAvatar;
        unsigned m_destAvatarID;
        Plat_Unicode::String m_friendAddress;
        Plat_Unicode::String m_friendStatus;
    };

	class MKickRoom : public GenericAPI::GenericMessage
	{
	public:
		MKickRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MKickRoom() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;
		unsigned m_roomID;
	};

	class MKickAvatar : public GenericAPI::GenericMessage
	{
	public:
		MKickAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MKickAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MAddModeratorRoom : public GenericAPI::GenericMessage
	{
	public:
		MAddModeratorRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddModeratorRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		ChatAvatarCore *getDestAvatar() const { return m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		ChatAvatarCore *m_destAvatar;
		unsigned m_roomID;
	};

	class MAddModeratorAvatar : public GenericAPI::GenericMessage
	{
	public:
		MAddModeratorAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddModeratorAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MRemoveModeratorRoom : public GenericAPI::GenericMessage
	{
	public:
		MRemoveModeratorRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveModeratorRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_roomID;
	};

	class MRemoveModeratorAvatar : public GenericAPI::GenericMessage
	{
	public:
		MRemoveModeratorAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveModeratorAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MAddTemporaryModeratorRoom : public GenericAPI::GenericMessage
	{
	public:
		MAddTemporaryModeratorRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddTemporaryModeratorRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		ChatAvatarCore *getDestAvatar() const { return m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		ChatAvatarCore *m_destAvatar;
		unsigned m_roomID;
	};

	class MAddTemporaryModeratorAvatar : public GenericAPI::GenericMessage
	{
	public:
		MAddTemporaryModeratorAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddTemporaryModeratorAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MRemoveTemporaryModeratorRoom : public GenericAPI::GenericMessage
	{
	public:
		MRemoveTemporaryModeratorRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveTemporaryModeratorRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_roomID;
	};

	class MRemoveTemporaryModeratorAvatar : public GenericAPI::GenericMessage
	{
	public:
		MRemoveTemporaryModeratorAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveTemporaryModeratorAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MAddBanRoom : public GenericAPI::GenericMessage
	{
	public:
		MAddBanRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddBanRoom() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		ChatAvatarCore *getDestAvatar() const { return m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore *m_destAvatar;
		unsigned m_roomID;
	};

	class MAddBanAvatar : public GenericAPI::GenericMessage
	{
	public:
		MAddBanAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddBanAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MRemoveBanRoom : public GenericAPI::GenericMessage
	{
	public:
		MRemoveBanRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveBanRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_roomID;
	};

	class MRemoveBanAvatar : public GenericAPI::GenericMessage
	{
	public:
		MRemoveBanAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveBanAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }

		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MAddInviteRoom : public GenericAPI::GenericMessage
	{
	public:
		MAddInviteRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddInviteRoom() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		ChatAvatarCore *getDestAvatar() const { return m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore *m_destAvatar;
		unsigned m_roomID;
	};

	class MAddInviteAvatar : public GenericAPI::GenericMessage
	{
	public:
		MAddInviteAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddInviteAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }
		
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MRemoveInviteRoom : public GenericAPI::GenericMessage
	{
	public:
		MRemoveInviteRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveInviteRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_roomID;
	};

	class MRemoveInviteAvatar : public GenericAPI::GenericMessage
	{
	public:
		MRemoveInviteAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveInviteAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }
		
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MGrantVoiceRoom : public GenericAPI::GenericMessage
	{
	public:
		MGrantVoiceRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MGrantVoiceRoom() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		ChatAvatarCore *getDestAvatar() const { return m_destAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore *m_destAvatar;
		unsigned m_roomID;
	};

	class MGrantVoiceAvatar : public GenericAPI::GenericMessage
	{
	public:
		MGrantVoiceAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MGrantVoiceAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }
		
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MRevokeVoiceRoom : public GenericAPI::GenericMessage
	{
	public:
		MRevokeVoiceRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MRevokeVoiceRoom() {};

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_roomID;
	};

	class MRevokeVoiceAvatar : public GenericAPI::GenericMessage
	{
	public:
		MRevokeVoiceAvatar(Base::ByteStream::ReadIterator &iter);
		virtual ~MRevokeVoiceAvatar() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const ChatAvatarCore *getDestAvatar() const { return &m_destAvatar; }
		
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		ChatAvatarCore m_srcAvatar;
		ChatAvatarCore m_destAvatar;

		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomAddress;
	};

	class MEnterRoom : public GenericAPI::GenericMessage
	{
	public:
		MEnterRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MEnterRoom() {};

		ChatAvatarCore *getSrcAvatar() const { return m_srcAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore *m_srcAvatar;
		unsigned m_roomID;
	};

	class MLeaveRoom : public GenericAPI::GenericMessage
	{
	public:
		MLeaveRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MLeaveRoom() {};

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
	};

	class MDestroyRoom : public GenericAPI::GenericMessage
	{
	public:
		MDestroyRoom(Base::ByteStream::ReadIterator &iter);
		virtual ~MDestroyRoom() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_roomID;
	};

	class MSetRoomParams : public GenericAPI::GenericMessage
	{
	public:
		MSetRoomParams(Base::ByteStream::ReadIterator &iter);
		virtual ~MSetRoomParams() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		const Plat_Unicode::String &getRoomName() const { return m_roomName; }
		const Plat_Unicode::String &getRoomTopic() const { return m_roomTopic; }
		const Plat_Unicode::String &getRoomPassword() const { return m_roomPassword; }
		unsigned getRoomAttributes() const { return m_roomAttributes; }
		unsigned getRoomSize() const { return m_maxRoomSize; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore m_srcAvatar;
		Plat_Unicode::String m_roomName;
		Plat_Unicode::String m_roomTopic;
		Plat_Unicode::String m_roomPassword;
		unsigned m_roomAttributes;
		unsigned m_maxRoomSize;
		unsigned m_roomID;
	};

	class MPersistentMessage : public GenericAPI::GenericMessage
	{
	public:
		MPersistentMessage(Base::ByteStream::ReadIterator &iter);
		virtual ~MPersistentMessage();
		
		unsigned getDestAvatarID() const { return m_destAvatarID; }
		const PersistentHeader *getHeader() const { return m_header; }

	private:
		unsigned m_destAvatarID;
		PersistentHeaderCore *m_core;
		PersistentHeader *m_header;
	};

	class MForcedLogout : public GenericAPI::GenericMessage
	{
	public:
		MForcedLogout(Base::ByteStream::ReadIterator &iter);
		virtual ~MForcedLogout() {};

		unsigned getAvatarID() const { return m_avatarID; }

	private:
		unsigned m_avatarID;
	};

	class MUnregisterRoomReady : public GenericAPI::GenericMessage
	{
	public:
		MUnregisterRoomReady(Base::ByteStream::ReadIterator &iter);
		virtual ~MUnregisterRoomReady() {};

		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_roomID;
	};

	class MAddAdmin : public GenericAPI::GenericMessage
	{
	public:
		MAddAdmin(Base::ByteStream::ReadIterator &iter);
		virtual ~MAddAdmin() {};

		ChatAvatarCore *getAvatar() const { return m_avatar; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		ChatAvatarCore *m_avatar;
		unsigned m_roomID;
	};

	class MRemoveAdmin : public GenericAPI::GenericMessage
	{
	public:
		MRemoveAdmin(Base::ByteStream::ReadIterator &iter);
		virtual ~MRemoveAdmin() {};

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
	};

	class MFriendConfirmRequest : GenericAPI::GenericMessage
	{
	public:
		MFriendConfirmRequest(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendConfirmRequest() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_destAvatarID;
	};

	class MFriendConfirmResponse : GenericAPI::GenericMessage
	{
	public:
		MFriendConfirmResponse(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendConfirmResponse() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
		bool getConfirm() const { return m_confirm != 0; }
	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_destAvatarID;
		short m_confirm;
	};

	class MFriendConfirmReciprocateRequest : GenericAPI::GenericMessage
	{
	public:
		MFriendConfirmReciprocateRequest(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendConfirmReciprocateRequest() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_destAvatarID;
	};

	class MFriendConfirmReciprocateResponse : GenericAPI::GenericMessage
	{
	public:
		MFriendConfirmReciprocateResponse(Base::ByteStream::ReadIterator &iter);
		virtual ~MFriendConfirmReciprocateResponse() {};

		const ChatAvatarCore *getSrcAvatar() const { return &m_srcAvatar; }
		unsigned getDestAvatarID() const { return m_destAvatarID; }
		bool getConfirm() const { return m_confirm != 0; }
	private:
		ChatAvatarCore m_srcAvatar;
		unsigned m_destAvatarID;
		short m_confirm;
	};

	class MChangeRoomOwner: GenericAPI::GenericMessage
	{
	public:
		MChangeRoomOwner(Base::ByteStream::ReadIterator &iter);
		virtual ~MChangeRoomOwner() {};

		unsigned getDestRoomID() const { return m_destRoomID; }
		unsigned getNewRoomOwnerID() const { return m_newRoomOwnerID; }
		const Plat_Unicode::String &getNewRoomOwnerName() const { return m_newRoomOwnerName; }
		const Plat_Unicode::String &getNewRoomOwnerAddress() const { return m_newRoomOwnerAddress; }
		
	private:
		unsigned m_destRoomID;
		unsigned m_newRoomOwnerID;
		Plat_Unicode::String m_newRoomOwnerName;
		Plat_Unicode::String m_newRoomOwnerAddress;
	};

	class MRoomEntryRequest : public GenericAPI::GenericMessage
	{
	public: 
		MRoomEntryRequest(Base::ByteStream::ReadIterator &iter);
		virtual ~MRoomEntryRequest() {};

		unsigned getRoomOwnerID() const { return m_roomOwnerID; }
		const ChatAvatarCore *getRequestorAvatar() const { return &m_requestorAvatar; }
		const Plat_Unicode::String &getRequestedRoomAddress() const { return m_requestedRoomAddress; }

	private:
		unsigned m_roomOwnerID;
		ChatAvatarCore m_requestorAvatar;
		Plat_Unicode::String m_requestedRoomAddress;
	};

	class MDelayedRoomEntry : public GenericAPI::GenericMessage
	{
	public:
		MDelayedRoomEntry(Base::ByteStream::ReadIterator &iter);
		virtual ~MDelayedRoomEntry() {};
		
		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
		const Plat_Unicode::String &getEnteredAddress() { return m_enteredAddress; }
		ChatRoomCore *getRoom() const { return m_room; }
		const std::set<ChatRoomCore *> &getExtraRooms() { return m_setExtraRooms; }

	private:
		unsigned m_avatarID;
		unsigned m_roomID;
		Plat_Unicode::String m_enteredAddress;
		ChatRoomCore* m_room;
		std::set<ChatRoomCore *> m_setExtraRooms;
	};

	class MDeniedRoomEntry : public GenericAPI::GenericMessage
	{
	public:
		MDeniedRoomEntry(Base::ByteStream::ReadIterator &iter);
		virtual ~MDeniedRoomEntry() {};

		unsigned getRequestorID() const { return m_requestorAvatarID; }
		const Plat_Unicode::String &getRequestedRoomAddress() const { return m_requestedRoomAddress; }

	private:
		unsigned m_requestorAvatarID;
		Plat_Unicode::String m_requestedRoomAddress;
	};

	class MForceRoomFailover : GenericAPI::GenericMessage
	{
	public:
		MForceRoomFailover(Base::ByteStream::ReadIterator &iter);
		virtual ~MForceRoomFailover() {};

		const std::list<Plat_Unicode::String> &getAIDList() const { return m_aidList; }
	private:
		std::list <Plat_Unicode::String> m_aidList;
	};

	class MSnoop : GenericAPI::GenericMessage
	{
	public:
		MSnoop(Base::ByteStream::ReadIterator &iter);
		virtual ~MSnoop() {};

		unsigned getSnoopType() const { return m_snoopType; }
		const Plat_Unicode::String &getSnooperName() const { return m_snooperName; }
		const Plat_Unicode::String &getSnooperAddr() const { return m_snooperAddr; }
		const Plat_Unicode::String &getSrcName() const { return m_srcName; }
		const Plat_Unicode::String &getSrcAddr() const { return m_srcAddr; }
		const Plat_Unicode::String &getDestName() const { return m_destName; }
		const Plat_Unicode::String &getDestAddr() const { return m_destAddr; }
		const Plat_Unicode::String &getMessage() const { return m_message; }
	private:
		unsigned m_snoopType;
		Plat_Unicode::String m_snooperName;
		Plat_Unicode::String m_snooperAddr;
		Plat_Unicode::String m_srcName;
		Plat_Unicode::String m_srcAddr;
		Plat_Unicode::String m_destName;
		Plat_Unicode::String m_destAddr;
		Plat_Unicode::String m_message;
	};

	class MUIDList : public GenericAPI::GenericMessage
	{
	public:
		MUIDList(Base::ByteStream::ReadIterator &iter);
		virtual ~MUIDList() {};

		const std::set<Plat_Unicode::String> &getUIDList() const { return m_uidList; }
	private:
		std::set<Plat_Unicode::String> m_uidList;
	};

	class MNotifyFriendIsRemoved : public GenericAPI::GenericMessage
	{
	public:
		MNotifyFriendIsRemoved(Base::ByteStream::ReadIterator &iter);
		virtual ~MNotifyFriendIsRemoved() {};

		const unsigned &getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getDestName() const { return m_destName; }
		const Plat_Unicode::String &getDestAddress() const { return m_destAddress; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_destName;
		Plat_Unicode::String m_destAddress;
	};

	class MNotifyFriendsListChange : public GenericAPI::GenericMessage
	{
	public:
		MNotifyFriendsListChange(Base::ByteStream::ReadIterator &iter);
		virtual ~MNotifyFriendsListChange() {};

		const unsigned &getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getOriginalName() const { return m_originalName; }
		const Plat_Unicode::String &getOriginalAddress() const { return m_originalAddress; }
		const Plat_Unicode::String &getNewName() const { return m_newName; }
		const Plat_Unicode::String &getNewAddress() const { return m_newAddress; }
	private:

		unsigned m_avatarID;
		Plat_Unicode::String m_originalName;
		Plat_Unicode::String m_originalAddress;
		Plat_Unicode::String m_newName;
		Plat_Unicode::String m_newAddress;
	};


};

#endif

