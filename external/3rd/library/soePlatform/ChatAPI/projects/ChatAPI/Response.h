#if !defined (RESPONSE_H_)
#define RESPONSE_H_

#include <GenericAPI/GenericMessage.h>

#include "ChatEnum.h"
#include "ChatAvatarCore.h"
#include "ChatRoomCore.h"
#include "ChatFriendStatus.h"
#include "ChatIgnoreStatus.h"
#include "AvatarListItem.h"

namespace ChatSystem 
{
	class ChatRoomCore;
	class ChatAvatarCore;
	class RoomSummary;
	class PersistentHeader;
	class PersistentHeaderCore;
	class PersistentMessage;
	class PersistentMessageCore;

	enum ResponseType
	{	
		RESPONSE_LOGINAVATAR = 0,
		RESPONSE_LOGOUTAVATAR,
		RESPONSE_DESTROYAVATAR,
		RESPONSE_GETAVATAR,
		RESPONSE_CREATEROOM,
		RESPONSE_DESTROYROOM,
		RESPONSE_SENDINSTANTMESSAGE,
		RESPONSE_SENDROOMMESSAGE,
		RESPONSE_SENDBROADCASTMESSAGE,
		RESPONSE_ADDFRIEND,
		RESPONSE_REMOVEFRIEND,
		RESPONSE_FRIENDSTATUS,
		RESPONSE_ADDIGNORE,
		RESPONSE_REMOVEIGNORE,
		RESPONSE_ENTERROOM,
		RESPONSE_LEAVEROOM,
		RESPONSE_ADDMODERATOR,
		RESPONSE_REMOVEMODERATOR,
		RESPONSE_ADDBAN,
		RESPONSE_REMOVEBAN,
		RESPONSE_ADDINVITE,
		RESPONSE_REMOVEINVITE,
		RESPONSE_KICKAVATAR,
		RESPONSE_SETROOMPARAMS,
		RESPONSE_GETROOM,
		RESPONSE_GETROOMSUMMARIES,
		RESPONSE_SENDPERSISTENTMESSAGE,
		RESPONSE_GETPERSISTENTHEADERS,
		RESPONSE_GETPERSISTENTMESSAGE,
		RESPONSE_UPDATEPERSISTENTMESSAGE,
		RESPONSE_UNREGISTERROOM,
		RESPONSE_IGNORESTATUS,
		RESPONSE_FAILOVER_RELOGINAVATAR,
		RESPONSE_FAILOVER_RECREATEROOM,
		RESPONSE_CONFIRMFRIEND,
		RESPONSE_GETAVATARKEYWORDS,
		RESPONSE_SETAVATARKEYWORDS,
		RESPONSE_SEARCHAVATARKEYWORDS,
		RESPONSE_GETFANCLUBHANDLE,
		RESPONSE_UPDATEPERSISTENTMESSAGES,
		RESPONSE_FINDAVATARBYUID,
		RESPONSE_CHANGEROOMOWNER,
		RESPONSE_SETAPIVERSION,
		RESPONSE_ADDTEMPORARYMODERATOR,
		RESPONSE_REMOVETEMPORARYMODERATOR,
		RESPONSE_GRANTVOICE,
		RESPONSE_REVOKEVOICE,
		RESPONSE_SETAVATARATTRIBUTES,
		RESPONSE_ADDSNOOPAVATAR,
		RESPONSE_REMOVESNOOPAVATAR,
		RESPONSE_ADDSNOOPROOM,
		RESPONSE_REMOVESNOOPROOM,
		RESPONSE_GETSNOOPLIST,
		RESPONSE_PARTIALPERSISTENTHEADERS,
		RESPONSE_COUNTPERSISTENTMESSAGES,
		RESPONSE_PURGEPERSISTENTMESSAGES,
		RESPONSE_SETFRIENDCOMMENT,
		RESPONSE_TRANSFERAVATAR,
		RESPONSE_CHANGEPERSISTENTFOLDER,
		RESPONSE_ALLOWROOMENTRY,
		RESPONSE_SETAVATAREMAIL,
		RESPONSE_SETAVATARINBOXLIMIT,
		RESPONSE_SENDMULTIPLEPERSISTENTMESSAGES,
		RESPONSE_GETMULTIPLEPERSISTENTMESSAGES,
		RESPONSE_ALTERPERSISTENTMESSAGE,
        RESPONSE_GETANYAVATAR,
        RESPONSE_TEMPORARYAVATAR,
        RESPONSE_AVATARLIST,
        RESPONSE_SETSTATUSMESSAGE,
		RESPONSE_CONFIRMFRIEND_RECIPROCATE,
		RESPONSE_ADDFRIEND_RECIPROCATE,
		RESPONSE_REMOVEFRIEND_RECIPROCATE,
		RESPONSE_FILTERMESSAGE,
		RESPONSE_FILTERMESSAGE_EX,

		RESPONSE_REGISTRAR_GETCHATSERVER = 20001,
	};

	class ResLoginAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResLoginAvatar(void *user, int avatarLoginPriority);
		virtual ~ResLoginAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);
		
		ChatAvatarCore *getAvatar() const { return m_avatar; }
		int getSubmittedLoginPriority() const { return m_submittedPriority; }
		int getRequiredLoginPriority() const { return m_requiredPriority; }
	private:
		ChatAvatarCore *m_avatar;
		int m_submittedPriority;
		int m_requiredPriority;
	};

    class ResTemporaryAvatar : public GenericAPI::GenericResponse
    {
    public:
        ResTemporaryAvatar(void *user);
        virtual ~ResTemporaryAvatar() {};

        virtual void unpack(Base::ByteStream::ReadIterator &iter);

        ChatAvatarCore *getAvatar() const { return m_avatar; }

    private:
        ChatAvatarCore *m_avatar;
    };

	class ResLogoutAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResLogoutAvatar(void *user, unsigned avatarID);
		virtual ~ResLogoutAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }

	private:
		unsigned m_avatarID;
	};

	class ResDestroyAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResDestroyAvatar(void *user, unsigned avatarID);
		virtual ~ResDestroyAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
	private:
		unsigned m_avatarID;
	};

	class ResSetAvatarKeywords : public GenericAPI::GenericResponse
	{
	public:
		ResSetAvatarKeywords(void *user, unsigned avatarID);
		virtual ~ResSetAvatarKeywords() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
	private:
		unsigned m_avatarID;
	};

	class ResGetAvatarKeywords : public GenericAPI::GenericResponse
	{
	public:
		ResGetAvatarKeywords(void *user, unsigned avatarID);
		virtual ~ResGetAvatarKeywords();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getListLength() const { return m_keywordLength; }
		const ChatUnicodeString *getList() const { return m_chatStrList; }
		
	private:
		unsigned m_avatarID;
		unsigned m_keywordLength;
		Plat_Unicode::String *m_keywordList;
		ChatUnicodeString *m_chatStrList;

	};

	class ResSearchAvatarKeywords : public GenericAPI::GenericResponse
	{
	public:
		ResSearchAvatarKeywords(void *user);
		virtual ~ResSearchAvatarKeywords();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getNumMatches() const { return m_numMatches; }
		ChatAvatarCore **getMatches() const { return m_avatarMatches; }

	private:
		unsigned m_numMatches;
		ChatAvatarCore **m_avatarMatches;
	};

	class ResGetAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResGetAvatar(void *user);
		virtual ~ResGetAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		ChatAvatarCore *getAvatar() const { return m_avatar; }
	private:
		ChatAvatarCore *m_avatar;
	};

    class ResGetAnyAvatar : public GenericAPI::GenericResponse
    {
    public:
        ResGetAnyAvatar(void *user);
        virtual ~ResGetAnyAvatar() {};

        virtual void unpack(Base::ByteStream::ReadIterator& iter);

        bool            isLoggedIn() const { return m_online; }
        ChatAvatarCore* getAvatar() const { return m_avatar; }
    private:
        bool            m_online;
        ChatAvatarCore* m_avatar;
    };

    class ResAvatarList : public GenericAPI::GenericResponse
    {
    public:
        ResAvatarList(void *user);
        virtual ~ResAvatarList();

        virtual void unpack(Base::ByteStream::ReadIterator& iter);

        // TODO: make this ignore status more generic, or make our
        // own datatype (e.g. UserAvatarInfo).
        unsigned getListLength() const { return m_listLength; }
        const AvatarListItem* getAvatarList() const { return m_avatarList; }

    private:
        unsigned m_listLength;
        AvatarListItem* m_avatarList;        
        AvatarListItemCore* m_cores;
    };

	class ResSetAvatarAttributes : public GenericAPI::GenericResponse
	{
	public:
		ResSetAvatarAttributes(void *user);
		virtual ~ResSetAvatarAttributes() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		ChatAvatarCore *getAvatar() const { return m_avatar; }
	private:
		ChatAvatarCore *m_avatar;
	};
	
    class ResSetAvatarStatusMessage: public GenericAPI::GenericResponse
    {
    public:
        ResSetAvatarStatusMessage(void *user);
        virtual ~ResSetAvatarStatusMessage() {};

        virtual void unpack(Base::ByteStream::ReadIterator &iter);

        ChatAvatarCore *getAvatar() const { return m_avatar; }
    private:
        ChatAvatarCore *m_avatar;
    };

	class ResSetAvatarForwardingEmail : public GenericAPI::GenericResponse
	{
	public:
		ResSetAvatarForwardingEmail(void *user);
		virtual ~ResSetAvatarForwardingEmail() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		ChatAvatarCore *getAvatar() const { return m_avatar; }
	private:
		ChatAvatarCore *m_avatar;
	};

	class ResSetAvatarInboxLimit : public GenericAPI::GenericResponse
	{
	public:
		ResSetAvatarInboxLimit(void *user);
		virtual ~ResSetAvatarInboxLimit() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		ChatAvatarCore *getAvatar() const { return m_avatar; }
	private:
		ChatAvatarCore *m_avatar;
	};

	class ResGetRoom : public GenericAPI::GenericResponse
	{
	public:
		ResGetRoom(void *user);
		virtual ~ResGetRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		ChatRoomCore *getRoom() const { return m_room; }
		unsigned getNumExtraRooms() const { return m_numExtraRooms; }
		const std::set<ChatRoomCore *> &getExtraRooms() { return m_setExtraRooms; }
	private:
		ChatRoomCore *m_room;
		unsigned m_numExtraRooms;
		std::set<ChatRoomCore *> m_setExtraRooms;
	};

	class ResCreateRoom : public GenericAPI::GenericResponse
	{
	public:
		ResCreateRoom(void *user, unsigned srcAvatarID);
		virtual ~ResCreateRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		ChatRoomCore *getRoom() const { return m_room; }
		unsigned getNumExtraRooms() const { return m_numExtraRooms; }
		const std::set<ChatRoomCore *> &getExtraRooms() { return m_setExtraRooms; }
	private:
		unsigned m_srcAvatarID;
		ChatRoomCore *m_room;
		unsigned m_numExtraRooms;
		std::set<ChatRoomCore *> m_setExtraRooms;
	};

	class ResDestroyRoom : public GenericAPI::GenericResponse
	{
	public:
		ResDestroyRoom(void *user, unsigned avatarID);
		virtual ~ResDestroyRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
	};

	class ResSendInstantMessage : public GenericAPI::GenericResponse
	{
	public:
		ResSendInstantMessage(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address);
		virtual ~ResSendInstantMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String getName() const { return m_name; }
		const Plat_Unicode::String getAddress() const { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

	class ResSendRoomMessage : public GenericAPI::GenericResponse
	{
	public:
		ResSendRoomMessage(void *user, unsigned avatarID);
		virtual ~ResSendRoomMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
	};

	class ResSendBroadcastMessage : public GenericAPI::GenericResponse
	{
	public:
		ResSendBroadcastMessage(void *user, unsigned avatarID, const ChatUnicodeString &address);
		virtual ~ResSendBroadcastMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_address;
	};

	class ResFilterMessage : public GenericAPI::GenericResponse
	{
	public:
		ResFilterMessage(void *user, unsigned version = RESPONSE_FILTERMESSAGE);
		virtual ~ResFilterMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		const Plat_Unicode::String getMsg() const { return m_msg; }
	private:
		Plat_Unicode::String m_msg;
		unsigned m_version;
	};

	class ResAddFriend : public GenericAPI::GenericResponse
	{
	public:
		ResAddFriend(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment);
		virtual ~ResAddFriend() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
		const Plat_Unicode::String &getComment() const { return m_comment; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
		Plat_Unicode::String m_comment;
	};

	class ResAddFriendReciprocate : public GenericAPI::GenericResponse
	{
	public:
		ResAddFriendReciprocate(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment);
		virtual ~ResAddFriendReciprocate() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
		const Plat_Unicode::String &getComment() const { return m_comment; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
		Plat_Unicode::String m_comment;
	};

	class ResSetFriendComment : public GenericAPI::GenericResponse
	{
	public:
		ResSetFriendComment(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &comment);
		virtual ~ResSetFriendComment() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
		const Plat_Unicode::String &getComment() const { return m_comment; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
		Plat_Unicode::String m_comment;
	};

	class ResRemoveFriend : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveFriend(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address);
		virtual ~ResRemoveFriend() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

	class ResRemoveFriendReciprocate : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveFriendReciprocate(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address);
		virtual ~ResRemoveFriendReciprocate() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() const { return m_name; }
		const Plat_Unicode::String &getAddress() const { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

	class ResFriendStatus : public GenericAPI::GenericResponse
	{
	public:
		ResFriendStatus(void *user, unsigned avatarID);
		virtual ~ResFriendStatus();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getListLength() const { return m_listLength; }
		const ChatFriendStatus *getFriendList() const { return m_friendList; }
	private:
		unsigned m_avatarID;
		unsigned m_listLength;
		ChatFriendStatus *m_friendList;
		ChatFriendStatusCore *m_cores;
	};

	class ResAddIgnore : public GenericAPI::GenericResponse
	{
	public:
		ResAddIgnore(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address);
		virtual ~ResAddIgnore() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() { return m_name; }
		const Plat_Unicode::String &getAddress() { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

	class ResRemoveIgnore : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveIgnore(void *user, unsigned avatarID, const ChatUnicodeString &name, const ChatUnicodeString &address);
		virtual ~ResRemoveIgnore() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		const Plat_Unicode::String &getName() { return m_name; }
		const Plat_Unicode::String &getAddress() { return m_address; }
	private:
		unsigned m_avatarID;
		Plat_Unicode::String m_name;
		Plat_Unicode::String m_address;
	};

	class ResIgnoreStatus : public GenericAPI::GenericResponse
	{
	public:
		ResIgnoreStatus(void *user, unsigned avatarID);
		virtual ~ResIgnoreStatus();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getListLength() const { return m_listLength; }
		const ChatIgnoreStatus *getIgnoreList() const { return m_ignoreList; }
	private:
		unsigned m_avatarID;
		unsigned m_listLength;
		ChatIgnoreStatus *m_ignoreList;
		ChatIgnoreStatusCore *m_cores;
	};


	class ResEnterRoom : public GenericAPI::GenericResponse
	{
	public:
		ResEnterRoom(void *user, unsigned avatarID, const ChatUnicodeString &destAddress);
		virtual ~ResEnterRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
		const Plat_Unicode::String &getDestAddress() { return m_destAddress; }
		ChatRoomCore *getRoom() const { return m_room; }
		unsigned getNumExtraRooms() const { return m_numExtraRooms; }
		const std::set<ChatRoomCore *> &getExtraRooms() { return m_setExtraRooms; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
		Plat_Unicode::String m_destAddress;
		bool m_gotRoomObj;
		ChatRoomCore* m_room;
		unsigned m_numExtraRooms;
		std::set<ChatRoomCore *> m_setExtraRooms;
	};

	class ResAllowRoomEntry : public GenericAPI::GenericResponse
	{
	public:
		ResAllowRoomEntry(void *user, unsigned srcAvatarID, const ChatUnicodeString &destRoomAddress);
		virtual ~ResAllowRoomEntry() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestRoomAddress() const { return m_destRoomAddress; } 

	private:
		unsigned m_srcAvatarID;
		unsigned m_roomID;
		Plat_Unicode::String m_destRoomAddress;
	};

	class ResLeaveRoom : public GenericAPI::GenericResponse
	{
	public:
		ResLeaveRoom(void *user, unsigned avatarID, const ChatUnicodeString &destAddress);
		virtual ~ResLeaveRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		unsigned getRoomID() const { return m_roomID; }
		const Plat_Unicode::String &getDestAddress() { return m_destAddress; }
	private:
		unsigned m_avatarID;
		unsigned m_roomID;
		Plat_Unicode::String m_destAddress;
	};

	class ResAddModerator : public GenericAPI::GenericResponse
	{
	public:
		ResAddModerator(void *user, unsigned srcAvatarID);
		virtual ~ResAddModerator() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResRemoveModerator : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveModerator(void *user, unsigned srcAvatarID);
		virtual ~ResRemoveModerator() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResAddTemporaryModerator : public GenericAPI::GenericResponse
	{
	public:
		ResAddTemporaryModerator(void *user, unsigned srcAvatarID);
		virtual ~ResAddTemporaryModerator() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResRemoveTemporaryModerator : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveTemporaryModerator(void *user, unsigned srcAvatarID);
		virtual ~ResRemoveTemporaryModerator() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResAddBan : public GenericAPI::GenericResponse
	{
	public:
		ResAddBan(void *user, unsigned srcAvatarID);
		virtual ~ResAddBan() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResRemoveBan : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveBan(void *user, unsigned srcAvatarID);
		virtual ~ResRemoveBan() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResAddInvite : public GenericAPI::GenericResponse
	{
	public:
		ResAddInvite(void *user, unsigned srcAvatarID);
		virtual ~ResAddInvite() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResRemoveInvite : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveInvite(void *user, unsigned srcAvatarID);
		virtual ~ResRemoveInvite() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResGrantVoice : public GenericAPI::GenericResponse
	{
	public:
		ResGrantVoice(void *user, unsigned srcAvatarID);
		virtual ~ResGrantVoice() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResRevokeVoice : public GenericAPI::GenericResponse
	{
	public:
		ResRevokeVoice(void *user, unsigned srcAvatarID);
		virtual ~ResRevokeVoice() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};


	class ResKickAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResKickAvatar(void *user, unsigned srcAvatarID);
		virtual ~ResKickAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResSetRoomParams : public GenericAPI::GenericResponse
	{
	public:
		ResSetRoomParams(void *user, unsigned srcAvatarID);
		virtual ~ResSetRoomParams() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResChangeRoomOwner : public GenericAPI::GenericResponse
	{
	public:
		ResChangeRoomOwner(void *user, unsigned srcAvatarID);
		virtual ~ResChangeRoomOwner() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_destRoomID;
	};

	class ResGetRoomSummaries : public GenericAPI::GenericResponse
	{
	public:
		ResGetRoomSummaries(void *user);
		virtual ~ResGetRoomSummaries();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getNumRooms() const { return m_numRooms; }
		RoomSummary *getRoomSummaries() const { return m_roomSummaries; }
	private:
		unsigned m_numRooms;
		RoomSummary *m_roomSummaries;
	};

	class ResSendPersistentMessage : public GenericAPI::GenericResponse
	{
	public:
		ResSendPersistentMessage(void *user, unsigned srcAvatarID);
		virtual ~ResSendPersistentMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getMessageID() const { return m_messageID; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_messageID;
	};

	class ResSendMultiplePersistentMessages : public GenericAPI::GenericResponse
	{
	public:
		ResSendMultiplePersistentMessages(void *user, unsigned srcAvatarID);
		virtual ~ResSendMultiplePersistentMessages() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getNumResults() const { return m_numResults; }
		const std::vector<unsigned> &getResultVector() const { return m_resultVector; }
		const std::vector<unsigned> &getMessageIDvector() const { return m_messageIDvector; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_numResults;
		std::vector<unsigned> m_resultVector;
		std::vector<unsigned> m_messageIDvector;
	};

	class ResAlterPersistentMessage : public GenericAPI::GenericResponse
	{
	public:
		ResAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, void *user);
		virtual ~ResAlterPersistentMessage();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
		unsigned getMessageID() const { return m_messageID; }

	private:
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
		unsigned m_messageID;
	};

	class ResGetPersistentHeaders : public GenericAPI::GenericResponse
	{
	public:
		ResGetPersistentHeaders(void *user, unsigned srcAvatarID);
		virtual ~ResGetPersistentHeaders();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const PersistentHeader *getList() const { return m_headers; }
		unsigned getListLength() const { return m_listLength; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_listLength;
		PersistentHeader *m_headers;
		PersistentHeaderCore *m_cores;
	};

	class ResGetPartialPersistentHeaders : public GenericAPI::GenericResponse
	{
	public:
		ResGetPartialPersistentHeaders(void *user, unsigned srcAvatarID);
		virtual ~ResGetPartialPersistentHeaders();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const PersistentHeader *getList() const { return m_headers; }
		unsigned getListLength() const { return m_listLength; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_listLength;
		PersistentHeader *m_headers;
		PersistentHeaderCore *m_cores;
	};

	class ResCountPersistentMessages : public GenericAPI::GenericResponse
	{
	public:
		ResCountPersistentMessages(void *user, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress);
		virtual ~ResCountPersistentMessages() { }

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		const Plat_Unicode::String &getAvatarName() const { return m_avatarName; }
		const Plat_Unicode::String &getAvatarAddress() const { return m_avatarAddress; }
		unsigned getNumberOfMessages() const { return m_numMessages; }

	private:
		Plat_Unicode::String m_avatarName;
		Plat_Unicode::String m_avatarAddress;
		unsigned m_numMessages;
	};

	class ResGetPersistentMessage : public GenericAPI::GenericResponse
	{
	public:
		ResGetPersistentMessage(void *user, unsigned srcAvatarID, unsigned messageID);
		virtual ~ResGetPersistentMessage();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const PersistentHeader *getHeader() const { return m_header; }
		const Plat_Unicode::String &getMsg() const { return m_msg; }
		const Plat_Unicode::String &getOOB() const { return m_oob; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_messageID;
		PersistentHeaderCore *m_core;
		PersistentHeader *m_header;
		Plat_Unicode::String m_msg;
		Plat_Unicode::String m_oob;
	};

	class ResGetMultiplePersistentMessages: public GenericAPI::GenericResponse
	{
	public:
		ResGetMultiplePersistentMessages(void *user, unsigned srcAvatarID);
		virtual ~ResGetMultiplePersistentMessages();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		PersistentMessage ** const getList() const;
		unsigned getListLength() const { return m_listLength; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_listLength;
		PersistentMessageCore **m_messages;
	};

	class ResUpdatePersistentMessage : public GenericAPI::GenericResponse
	{
	public:
		ResUpdatePersistentMessage(void *user, unsigned srcAvatarID, unsigned messageID);
		virtual ~ResUpdatePersistentMessage() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }

	private:
		unsigned m_srcAvatarID;
		unsigned m_messageID;
	};

	class ResUpdatePersistentMessages : public GenericAPI::GenericResponse
	{
	public:
		ResUpdatePersistentMessages(void *user, unsigned srcAvatarID);
		virtual ~ResUpdatePersistentMessages() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }

	private:
		unsigned m_srcAvatarID;
	};

	class ResClassifyPersistentMessages : public GenericAPI::GenericResponse
	{
	public:
		ResClassifyPersistentMessages(void *user, unsigned srcAvatarID);
		virtual ~ResClassifyPersistentMessages() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }

	private:
		unsigned m_srcAvatarID;
	};

	class ResDeleteAllPersistentMessages : public GenericAPI::GenericResponse
	{
	public:
		ResDeleteAllPersistentMessages(void *user, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress);
		virtual ~ResDeleteAllPersistentMessages() { }

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		const Plat_Unicode::String &getAvatarName() const { return m_avatarName; }
		const Plat_Unicode::String &getAvatarAddress() const { return m_avatarAddress; }
		unsigned getNumberDeleted() const { return m_numDeleted; }

	private:
		Plat_Unicode::String m_avatarName;
		Plat_Unicode::String m_avatarAddress;
		unsigned m_numDeleted;
	};

	class ResUnregisterRoom : public GenericAPI::GenericResponse
	{
	public:
		ResUnregisterRoom(void *user);
		virtual ~ResUnregisterRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getDestRoomID() const { return m_destRoomID; }
	private:
		unsigned m_destRoomID;
	};

	class ResFailoverReloginAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResFailoverReloginAvatar(unsigned avatarID);
		virtual ~ResFailoverReloginAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
		int getRequiredLoginPriority() const { return m_requiredPriority; }
	private:
		unsigned m_avatarID;
		int m_requiredPriority;
	};

	class ResFailoverRecreateRoom : public GenericAPI::GenericResponse
	{
	public:
		ResFailoverRecreateRoom(unsigned roomID, bool forced = false);
		virtual ~ResFailoverRecreateRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getRoomID() const { return m_roomID; }
		ChatRoomCore *getRoom() const { return m_room; }
		bool wasFailoverForcedByServer() { return m_forced; }
	private:
		unsigned m_roomID;
		ChatRoomCore *m_room;
		bool m_forced;
	};

	class ResFriendConfirm : public GenericAPI::GenericResponse
	{
	public:
		ResFriendConfirm(unsigned srcAvatarID, void *user);
		virtual ~ResFriendConfirm() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
	private:
		unsigned m_avatarID;
	};

	class ResFriendConfirmReciprocate : public GenericAPI::GenericResponse
	{
	public:
		ResFriendConfirmReciprocate(unsigned srcAvatarID, void *user);
		virtual ~ResFriendConfirmReciprocate() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getAvatarID() const { return m_avatarID; }
	private:
		unsigned m_avatarID;
	};

	class ResGetFanClubHandle : public GenericAPI::GenericResponse
	{
	public:
		ResGetFanClubHandle(unsigned stationID, unsigned fanClubHandle, void *user);
		virtual ~ResGetFanClubHandle() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getStationID() const { return m_stationID; }
		unsigned getFanClubCode() const { return m_fanClubCode; }
		const Plat_Unicode::String &getHandle() const { return m_handle; }
	private:
		unsigned m_stationID;
		unsigned m_fanClubCode;
		Plat_Unicode::String m_handle;
	};

	class ResFindAvatarByUID : public GenericAPI::GenericResponse
	{
	public:
		ResFindAvatarByUID(void *user);
		virtual ~ResFindAvatarByUID();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getNumAvatarsOnline() const { return m_numAvatarsOnline; }
		ChatAvatarCore **getAvatarsCore() const { return m_avatars; }
	private:
		unsigned m_numAvatarsOnline;
		ChatAvatarCore **m_avatars;
	};

	class ResRegistrarGetChatServer: public GenericAPI::GenericResponse
	{
	public:
		ResRegistrarGetChatServer();
		virtual ~ResRegistrarGetChatServer() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		const Plat_Unicode::String &getHostname() const { return m_hostname; }
		unsigned short getPort() const { return m_port; }
	private:
		Plat_Unicode::String m_hostname;
		unsigned short m_port;
	};

	class ResSendApiVersion: public GenericAPI::GenericResponse
	{
	public:
		ResSendApiVersion();
		virtual ~ResSendApiVersion() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned long getServerVersion() const { return m_serverVersion; }
	private:
		unsigned long m_serverVersion;
	};

	class ResAddSnoopAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResAddSnoopAvatar(void *user, unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
		virtual ~ResAddSnoopAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
	};

	class ResRemoveSnoopAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveSnoopAvatar(void *user, unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
		virtual ~ResRemoveSnoopAvatar() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getDestAvatarName() const { return m_destAvatarName; }
		const Plat_Unicode::String &getDestAvatarAddress() const { return m_destAvatarAddress; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_destAvatarName;
		Plat_Unicode::String m_destAvatarAddress;
	};

	class ResAddSnoopRoom : public GenericAPI::GenericResponse
	{
	public:
		ResAddSnoopRoom(void *user, unsigned srcAvatarID, const ChatUnicodeString &roomAddress);
		virtual ~ResAddSnoopRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_roomAddress;
	};

	class ResRemoveSnoopRoom : public GenericAPI::GenericResponse
	{
	public:
		ResRemoveSnoopRoom(void *user, unsigned srcAvatarID, const ChatUnicodeString &roomAddress);
		virtual ~ResRemoveSnoopRoom() {};

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		const Plat_Unicode::String &getRoomAddress() const { return m_roomAddress; }
	private:
		unsigned m_srcAvatarID;
		Plat_Unicode::String m_roomAddress;
	};

	class ResGetSnoopList : public GenericAPI::GenericResponse
	{
	public:
		ResGetSnoopList(void *user, unsigned srcAvatarID);
		virtual ~ResGetSnoopList();

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getSrcAvatarID() const { return m_srcAvatarID; }
		unsigned getAvatarSnoopListLength() const { return m_avatarSnoopListLength; }
		AvatarSnoopPair **getAvatarSnoopList() const { return m_avatarSnoops; }
		unsigned getRoomSnoopListLength() const { return m_roomSnoopListLength; }
		ChatUnicodeString **getRoomSnoopList() const { return m_roomSnoops; }
	private:
		unsigned m_srcAvatarID;
		unsigned m_avatarSnoopListLength;
		unsigned m_roomSnoopListLength;
		AvatarSnoopPair **m_avatarSnoops;
		ChatUnicodeString **m_roomSnoops;
	};

	class ResTransferAvatar : public GenericAPI::GenericResponse
	{
	public:
		ResTransferAvatar(void *user, unsigned userID, unsigned newUserID, const ChatUnicodeString &avatarName, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &newAvatarAddress);
		virtual ~ResTransferAvatar() {}

		virtual void unpack(Base::ByteStream::ReadIterator &iter);

		unsigned getUserID() const { return m_userID; }
		unsigned getNewUserID() const { return m_newUserID; }
		const Plat_Unicode::String &getAvatarName() const { return m_avatarName; }
		const Plat_Unicode::String &getNewAvatarName() const { return m_newAvatarName; }
		const Plat_Unicode::String &getAvatarAddress() const { return m_avatarAddress; }
		const Plat_Unicode::String &getNewAvatarAddress() const { return m_newAvatarAddress; }
	private:
		unsigned m_userID;
		unsigned m_newUserID;
		Plat_Unicode::String m_avatarName;
		Plat_Unicode::String m_newAvatarName;
		Plat_Unicode::String m_avatarAddress;
		Plat_Unicode::String m_newAvatarAddress;
	};

}; // end namespace

#endif

