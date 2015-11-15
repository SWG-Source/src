#if !defined (REQUEST_H_)
#define REQUEST_H_

#pragma warning (disable : 4786)

#include <Unicode/Unicode.h>
#include <Unicode/UnicodeUtils.h>
#include <GenericAPI/GenericMessage.h>

#include "ChatEnum.h"
#include "ChatRoom.h"

#include <set>

namespace ChatSystem {


class RoomParams;
class ChatAvatarCore;
enum PersistentStatus;

enum RequestType
{
	REQUEST_LOGINAVATAR = 0,
	REQUEST_LOGOUTAVATAR,
	REQUEST_DESTROYAVATAR,
	REQUEST_GETAVATAR,
	REQUEST_CREATEROOM,
	REQUEST_DESTROYROOM,			// 5
	REQUEST_SENDINSTANTMESSAGE,
	REQUEST_SENDROOMMESSAGE,
	REQUEST_SENDBROADCASTMESSAGE,
	REQUEST_ADDFRIEND,
	REQUEST_REMOVEFRIEND,			// 10
	REQUEST_FRIENDSTATUS,
	REQUEST_ADDIGNORE,
	REQUEST_REMOVEIGNORE,
	REQUEST_ENTERROOM,
	REQUEST_LEAVEROOM,				// 15
	REQUEST_ADDMODERATOR,
	REQUEST_REMOVEMODERATOR,
	REQUEST_ADDBAN,
	REQUEST_REMOVEBAN,
	REQUEST_ADDINVITE,				// 20 
	REQUEST_REMOVEINVITE,
	REQUEST_KICKAVATAR,
	REQUEST_SETROOMPARAMS,
	REQUEST_GETROOM,
	REQUEST_GETROOMSUMMARIES,		// 25
	REQUEST_SENDPERSISTENTMESSAGE,
	REQUEST_GETPERSISTENTHEADERS,
	REQUEST_GETPERSISTENTMESSAGE,
	REQUEST_UPDATEPERSISTENTMESSAGE,
	REQUEST_UNREGISTERROOM,			// 30
	REQUEST_IGNORESTATUS,
	REQUEST_FAILOVER_RELOGINAVATAR,
	REQUEST_FAILOVER_RECREATEROOM,
	REQUEST_CONFIRMFRIEND,
	REQUEST_GETAVATARKEYWORDS,		// 35
	REQUEST_SETAVATARKEYWORDS,
	REQUEST_SEARCHAVATARKEYWORDS,
	REQUEST_GETFANCLUBHANDLE,
	REQUEST_UPDATEPERSISTENTMESSAGES,
	REQUEST_FINDAVATARBYUID,		// 40
	REQUEST_CHANGEROOMOWNER,
	REQUEST_SETAPIVERSION,
	REQUEST_ADDTEMPORARYMODERATOR,
	REQUEST_REMOVETEMPORARYMODERATOR,
	REQUEST_GRANTVOICE,				// 45
	REQUEST_REVOKEVOICE,
	REQUEST_SETAVATARATTRIBUTES,
	REQUEST_ADDSNOOPAVATAR,
	REQUEST_REMOVESNOOPAVATAR,
	REQUEST_ADDSNOOPROOM,			// 50
	REQUEST_REMOVESNOOPROOM,
	REQUEST_GETSNOOPLIST,
	REQUEST_PARTIALPERSISTENTHEADERS,
	REQUEST_COUNTPERSISTENTMESSAGES,
	REQUEST_PURGEPERSISTENTMESSAGES,	// 55
	REQUEST_SETFRIENDCOMMENT,
	REQUEST_TRANSFERAVATAR,
	REQUEST_CHANGEPERSISTENTFOLDER,
	REQUEST_ALLOWROOMENTRY,
	REQUEST_SETAVATAREMAIL,				// 60
	REQUEST_SETAVATARINBOXLIMIT,
	REQUEST_SENDMULTIPLEPERSISTENTMESSAGES,
	REQUEST_GETMULTIPLEPERSISTENTMESSAGES,
	REQUEST_ALTERPERISTENTMESSAGE,    
    REQUEST_GETANYAVATAR,				// 65
    REQUEST_TEMPORARYAVATAR,
    REQUEST_AVATARLIST,					
    REQUEST_SETAVATARSTATUSMESSAGE,	
	REQUEST_CONFIRMFRIEND_RECIPROCATE,
	REQUEST_ADDFRIEND_RECIPROCATE,		// 70
	REQUEST_REMOVEFRIEND_RECIPROCATE,
	REQUEST_FILTERMESSAGE,
	REQUEST_FILTERMESSAGE_EX,

	REQUEST_REGISTRAR_GETCHATSERVER = 20001,
};

class RLoginAvatar : public GenericAPI::GenericRequest
{
public:
	RLoginAvatar(unsigned userID, const ChatUnicodeString &name, const ChatUnicodeString &address, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, int avatarLoginAttributes);
	virtual ~RLoginAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_userID;
	Plat_Unicode::String m_name;
	Plat_Unicode::String m_address;
	Plat_Unicode::String m_loginLocation;
	int m_loginPriority;
    int m_loginAttributes;
};

class RTemporaryAvatar : public GenericAPI::GenericRequest
{
public:
    RTemporaryAvatar(unsigned userID, const ChatUnicodeString& name, const ChatUnicodeString& address, const ChatUnicodeString& loginLocation );
    virtual ~RTemporaryAvatar() {}

    virtual void pack(Base::ByteStream& msg);

private:
    unsigned m_userID;
    Plat_Unicode::String m_name;
    Plat_Unicode::String m_address;
    Plat_Unicode::String m_loginLocation;
};

class RLogoutAvatar : public GenericAPI::GenericRequest
{
public:
	RLogoutAvatar(unsigned avatarID, const ChatUnicodeString & address);
	virtual ~RLogoutAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	Plat_Unicode::String m_address;
};

class RDestroyAvatar : public GenericAPI::GenericRequest
{
public:
	RDestroyAvatar(unsigned avatarID, const ChatUnicodeString & address);
	virtual ~RDestroyAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	Plat_Unicode::String m_address;
};

class RSetAvatarKeywords : public GenericAPI::GenericRequest
{
public:
	RSetAvatarKeywords(unsigned srcAvatarID, const ChatUnicodeString *keywordsList, unsigned keywordsLength);
	virtual ~RSetAvatarKeywords();

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_keywordsLength;
	Plat_Unicode::String *m_keywordsList;
};

class RGetAvatarKeywords : public GenericAPI::GenericRequest
{
public:
	RGetAvatarKeywords(unsigned srcAvatarID);
	virtual ~RGetAvatarKeywords() {};

	virtual void pack(Base::ByteStream &msg);
private:
	unsigned m_srcAvatarID;
};

class RSearchAvatarKeywords : public GenericAPI::GenericRequest
{
public:
	RSearchAvatarKeywords(const ChatUnicodeString &nodeAddress, const ChatUnicodeString *keywordsList, unsigned keywordsLength);
	virtual ~RSearchAvatarKeywords();

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_keywordsLength;
	Plat_Unicode::String *m_keywordsList;
	Plat_Unicode::String m_nodeAddress;
};

class RGetAvatar : public GenericAPI::GenericRequest
{
public:
	RGetAvatar(const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RGetAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
};

class RGetAnyAvatar : public GenericAPI::GenericRequest
{
public:
    RGetAnyAvatar( const ChatUnicodeString& destName, const ChatUnicodeString& destAddress );
    virtual ~RGetAnyAvatar() {}

    virtual void pack(Base::ByteStream& msg);

private:
    Plat_Unicode::String m_destName;
    Plat_Unicode::String m_destAddress;
};

class RAvatarList : public GenericAPI::GenericRequest
{
public:
    RAvatarList( unsigned userID );
    virtual ~RAvatarList() {}

    virtual void pack(Base::ByteStream& msg);

private:
    unsigned m_userID;
};

class RSetAvatarAttributes : public GenericAPI::GenericRequest
{
public:
	RSetAvatarAttributes(unsigned avatarID, const ChatUnicodeString &srcAddress, unsigned long avatarAttributes, bool persistent = true );
	virtual ~RSetAvatarAttributes() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	unsigned long m_avatarAttributes;
	unsigned m_persistent;
	Plat_Unicode::String m_srcAddress;
};

class RSetAvatarStatusMessage : public GenericAPI::GenericRequest
{
public:
    RSetAvatarStatusMessage(unsigned avatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString& statusMessage );
    virtual ~RSetAvatarStatusMessage() {};

    virtual void pack(Base::ByteStream &msg);

private:
    unsigned m_avatarID;
	Plat_Unicode::String m_statusMessage;
	Plat_Unicode::String m_srcAddress;
};

class RSetAvatarForwardingEmail : public GenericAPI::GenericRequest
{
public:
	RSetAvatarForwardingEmail(unsigned avatarID, const ChatUnicodeString &avatarForwardingEmail);
	virtual ~RSetAvatarForwardingEmail() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	Plat_Unicode::String m_avatarForwardingEmail;
};


class RSetAvatarInboxLimit : public GenericAPI::GenericRequest
{
public:
	RSetAvatarInboxLimit(unsigned avatarID, unsigned long avatarInboxLimit);
	virtual ~RSetAvatarInboxLimit() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	unsigned long m_avatarInboxLimit;
};

class RGetRoom : public GenericAPI::GenericRequest
{
public:
	RGetRoom(const ChatUnicodeString &roomAddress);
	virtual ~RGetRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_roomAddress;
};

class RCreateRoom : public GenericAPI::GenericRequest
{
public:
	RCreateRoom(unsigned avatarID, const ChatUnicodeString &srcAddress, const RoomParams &params, const ChatUnicodeString &roomAddress);

	virtual ~RCreateRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	Plat_Unicode::String m_roomName;
	Plat_Unicode::String m_roomTopic;
	Plat_Unicode::String m_roomPassword;
	unsigned m_roomAttributes;
	unsigned m_maxRoomSize;
	Plat_Unicode::String m_roomAddress;
	Plat_Unicode::String m_srcAddress;
};

class RDestroyRoom : public GenericAPI::GenericRequest
{
public:
	RDestroyRoom(unsigned avatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAddress);
	virtual ~RDestroyRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_avatarID;
	Plat_Unicode::String m_roomAddress;
	Plat_Unicode::String m_srcAddress;
};

class RSendInstantMessage : public GenericAPI::GenericRequest
{
public:
	RSendInstantMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob);
	virtual ~RSendInstantMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_msg;
	Plat_Unicode::String m_oob;
	Plat_Unicode::String m_srcAddress;
};

class RSendRoomMessage : public GenericAPI::GenericRequest
{
public:
	RSendRoomMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob);
	virtual ~RSendRoomMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destRoomAddress;
	Plat_Unicode::String m_msg;
	Plat_Unicode::String m_oob;
	Plat_Unicode::String m_srcAddress;
};

class RSendBroadcastMessage : public GenericAPI::GenericRequest
{
public:
	RSendBroadcastMessage(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob);
	virtual ~RSendBroadcastMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_msg;
	Plat_Unicode::String m_oob;
	Plat_Unicode::String m_srcAddress;
};

class RFilterMessage : public GenericAPI::GenericRequest
{
public:
	RFilterMessage(const ChatUnicodeString &msg);
	virtual ~RFilterMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_msg;
};

class RFilterMessageEx : public GenericAPI::GenericRequest
{
public:
	RFilterMessageEx(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &msg);
	virtual ~RFilterMessageEx() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_msg;
};

class RAddFriend : public GenericAPI::GenericRequest
{
public:
	RAddFriend(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, bool confirm);
	virtual ~RAddFriend() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_friendComment;
	bool m_confirm;
};

class RAddFriendReciprocate : public GenericAPI::GenericRequest
{
public:
	RAddFriendReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, bool confirm);
	virtual ~RAddFriendReciprocate() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_friendComment;
	bool m_confirm;
};


class RSetFriendComment : public GenericAPI::GenericRequest
{
public:
	RSetFriendComment(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment);
	virtual ~RSetFriendComment() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_friendComment;
};

class RRemoveFriend : public GenericAPI::GenericRequest
{
public:
	RRemoveFriend(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RRemoveFriend() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
};

class RRemoveFriendReciprocate : public GenericAPI::GenericRequest
{
public:
	RRemoveFriendReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RRemoveFriendReciprocate() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
};

class RFriendStatus : public GenericAPI::GenericRequest
{
public:
	RFriendStatus(unsigned srcAvatarID, const ChatUnicodeString &srcAddress);
	virtual ~RFriendStatus() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
};

class RAddIgnore : public GenericAPI::GenericRequest
{
public:
	RAddIgnore(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RAddIgnore() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
};

class RRemoveIgnore : public GenericAPI::GenericRequest
{
public:
	RRemoveIgnore(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RRemoveIgnore() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destName;
	Plat_Unicode::String m_destAddress;
};

class RIgnoreStatus : public GenericAPI::GenericRequest
{
public:
	RIgnoreStatus(unsigned srcAvatarID, const ChatUnicodeString &srcAddress);
	virtual ~RIgnoreStatus() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
};


class REnterRoom : public GenericAPI::GenericRequest
{
public:
	REnterRoom(unsigned srcAvatarID,  const ChatUnicodeString &srcAddress, const ChatUnicodeString &roomAddress, const ChatUnicodeString &roomPassword, const RoomParams *roomParams, bool requestingEntry);
	virtual ~REnterRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_roomAddress;
	Plat_Unicode::String m_roomPassword;
	bool m_passiveCreate;
	Plat_Unicode::String m_paramRoomTopic;
	unsigned m_paramRoomAttributes;
	unsigned m_paramRoomMaxSize;
	bool m_requestingEntry;
	Plat_Unicode::String m_srcAddress;
};

class RAllowRoomEntry : public GenericAPI::GenericRequest
{
public:
	RAllowRoomEntry(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, bool allow);
	virtual ~RAllowRoomEntry() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
	bool m_allow;
};

class RLeaveRoom : public GenericAPI::GenericRequest
{
public:
	RLeaveRoom(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &roomAddress);
	virtual ~RLeaveRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_roomAddress;
	Plat_Unicode::String m_srcAddress;
};

class RAddModerator : public GenericAPI::GenericRequest
{
public:
	RAddModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RAddModerator() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RRemoveModerator : public GenericAPI::GenericRequest
{
public:
	RRemoveModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RRemoveModerator() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RAddTemporaryModerator : public GenericAPI::GenericRequest
{
public:
	RAddTemporaryModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RAddTemporaryModerator() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RRemoveTemporaryModerator : public GenericAPI::GenericRequest
{
public:
	RRemoveTemporaryModerator(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RRemoveTemporaryModerator() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RAddBan : public GenericAPI::GenericRequest
{
public:
	RAddBan(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RAddBan() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RRemoveBan : public GenericAPI::GenericRequest
{
public:
	RRemoveBan(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RRemoveBan() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RAddInvite : public GenericAPI::GenericRequest
{
public:
	RAddInvite(unsigned srcAvatarID,  const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RAddInvite() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
	Plat_Unicode::String m_srcAddress;
};

class RRemoveInvite : public GenericAPI::GenericRequest
{
public:
	RRemoveInvite(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RRemoveInvite() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RGrantVoice : public GenericAPI::GenericRequest
{
public:
	RGrantVoice(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RGrantVoice() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RRevokeVoice : public GenericAPI::GenericRequest
{
public:
	RRevokeVoice(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RRevokeVoice() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RKickAvatar : public GenericAPI::GenericRequest
{
public:
	RKickAvatar(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress);
	virtual ~RKickAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	Plat_Unicode::String m_destRoomAddress;
};

class RSetRoomParams : public GenericAPI::GenericRequest
{
public:
	RSetRoomParams(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const RoomParams *params);
	virtual ~RSetRoomParams() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destRoomAddress;
	Plat_Unicode::String m_newRoomName;
	Plat_Unicode::String m_newRoomTopic;
	Plat_Unicode::String m_newRoomPassword;
	unsigned m_newRoomAttributes;
	unsigned m_newRoomSize;
};

class RChangeRoomOwner : public GenericAPI::GenericRequest
{
public:
	RChangeRoomOwner(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress);
	virtual ~RChangeRoomOwner() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcAddress;
	Plat_Unicode::String m_destRoomAddress;
	Plat_Unicode::String m_newRoomOwnerName;
	Plat_Unicode::String m_newRoomOwnerAddress;
};

class RGetRoomSummaries : public GenericAPI::GenericRequest
{
public:
	RGetRoomSummaries(const ChatUnicodeString &startNodeAddress, const ChatUnicodeString &roomFilter);
	virtual ~RGetRoomSummaries() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_startNodeAddress;
	Plat_Unicode::String m_roomFilter;
};

class RSendPersistentMessage : public GenericAPI::GenericRequest
{
public:
	RSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatar, const ChatUnicodeString &destAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit);
	RSendPersistentMessage(unsigned srcAvatarID, const ChatUnicodeString &destAvatar, const ChatUnicodeString &destAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit);

	virtual ~RSendPersistentMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	short m_avatarPresence;;
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcName;
	Plat_Unicode::String m_destAvatar;
	Plat_Unicode::String m_destAddress;
	Plat_Unicode::String m_subject;
	Plat_Unicode::String m_msg;
	Plat_Unicode::String m_oob;
	Plat_Unicode::String m_category;
	bool m_enforceInboxLimit;
	unsigned m_categoryLimit;
};

class RSendMultiplePersistentMessages : public GenericAPI::GenericRequest
{
public:
	RSendMultiplePersistentMessages(const ChatUnicodeString &src, unsigned numDestAvatars, const ChatUnicodeString *destAvatars, const ChatUnicodeString *destAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit);
	RSendMultiplePersistentMessages(unsigned srcAvatarID, unsigned numDestAvatars, const ChatUnicodeString *destAvatars, const ChatUnicodeString *destAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit);

	virtual ~RSendMultiplePersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	short m_avatarPresence;;
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_srcName;
	unsigned m_numDestAvatars;
	std::vector<Plat_Unicode::String> m_destAvatars;
	std::vector<Plat_Unicode::String> m_destAddresses;
	Plat_Unicode::String m_subject;
	Plat_Unicode::String m_msg;
	Plat_Unicode::String m_oob;
	Plat_Unicode::String m_category;
	bool m_enforceInboxLimit;
	unsigned m_categoryLimit;
};

class RAlterPersistentMessage : public GenericAPI::GenericRequest
{
public:
	RAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, unsigned alterationAttributes, const ChatUnicodeString &newSenderName, const ChatUnicodeString &newSenderAddress,  const ChatUnicodeString &newSubject, const ChatUnicodeString &newMsg, const ChatUnicodeString &newOOB, const ChatUnicodeString &newCategory, unsigned newSentTime);

	virtual ~RAlterPersistentMessage();

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
	unsigned m_messageID;
	unsigned m_alterationAttributes;
	Plat_Unicode::String m_newSenderName;
	Plat_Unicode::String m_newSenderAddress;
	Plat_Unicode::String m_newSubject;
	Plat_Unicode::String m_newMsg;
	Plat_Unicode::String m_newOOB;
	Plat_Unicode::String m_newCategory;
	unsigned m_newSentTime;
};

class RGetPersistentHeaders : public GenericAPI::GenericRequest
{
public:
	RGetPersistentHeaders(unsigned srcAvatarID, const ChatUnicodeString &category);
	virtual ~RGetPersistentHeaders() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_category;
};

class RGetPartialPersistentHeaders : public GenericAPI::GenericRequest
{
public:
	RGetPartialPersistentHeaders(unsigned srcAvatarID, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category);
	virtual ~RGetPartialPersistentHeaders() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_maxHeaders;
	bool m_inDescendingOrder;
	unsigned m_sentTimeStart;
	Plat_Unicode::String m_category;
};

class RCountPersistentMessages : public GenericAPI::GenericRequest
{
public:
	RCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category);
	virtual ~RCountPersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_avatarName;
	Plat_Unicode::String m_avatarAddress;
	Plat_Unicode::String m_category;
};

class RGetPersistentMessage : public GenericAPI::GenericRequest
{
public:
	RGetPersistentMessage(unsigned srcAvatarID, unsigned messageID);
	virtual ~RGetPersistentMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_messageID;
};

class RGetMultiplePersistentMessages : public GenericAPI::GenericRequest
{
public:
	RGetMultiplePersistentMessages(unsigned srcAvatarID, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category);
	virtual ~RGetMultiplePersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_maxHeaders;
	bool m_inDescendingOrder;
	unsigned m_sentTimeStart;
	Plat_Unicode::String m_category;
};

class RUpdatePersistentMessage : public GenericAPI::GenericRequest
{
public:
	RUpdatePersistentMessage(unsigned srcAvatarID, unsigned messageID, PersistentStatus status);
	virtual ~RUpdatePersistentMessage() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_messageID;
	PersistentStatus m_status;
};

class RUpdatePersistentMessages : public GenericAPI::GenericRequest
{
public:
	RUpdatePersistentMessages(unsigned srcAvatarID, PersistentStatus currentStatus, PersistentStatus newStatus, const ChatUnicodeString &category);
	virtual ~RUpdatePersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	PersistentStatus m_currentStatus;
	PersistentStatus m_newStatus;
	Plat_Unicode::String m_category;
};

class RClassifyPersistentMessages : public GenericAPI::GenericRequest
{
public:
	RClassifyPersistentMessages(unsigned srcAvatarID, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder);
	RClassifyPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder);
	virtual ~RClassifyPersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_avatarName;
	Plat_Unicode::String m_avatarAddress;
	std::set<unsigned> m_messageIDs;
	Plat_Unicode::String m_newFolder;
};

class RDeleteAllPersistentMessages : public GenericAPI::GenericRequest
{
public:
	RDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, unsigned olderThan);
	virtual ~RDeleteAllPersistentMessages() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_avatarName;
	Plat_Unicode::String m_avatarAddress;
	Plat_Unicode::String m_category;
	unsigned m_olderThan;
};


class RUnregisterRoom : public GenericAPI::GenericRequest
{
public:
	RUnregisterRoom(const ChatUnicodeString &destRoomAddress);
	virtual ~RUnregisterRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_destRoomAddress;
};

class RFailoverReloginAvatar : public GenericAPI::GenericRequest
{
public:
	RFailoverReloginAvatar(ChatAvatarCore *avatar);
	virtual ~RFailoverReloginAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	ChatAvatarCore *m_avatar;
};

class RFailoverRecreateRoom : public GenericAPI::GenericRequest
{
public:
	RFailoverRecreateRoom(ChatRoomCore *room);
	virtual ~RFailoverRecreateRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	ChatRoomCore *m_room;
};

class RFriendConfirm : public GenericAPI::GenericRequest
{
public:
	RFriendConfirm(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, unsigned destAvatarID, bool confirm);
	virtual ~RFriendConfirm() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_destAvatarID;
	short m_confirm;
	Plat_Unicode::String m_srcAddress;
};

class RFriendConfirmReciprocate : public GenericAPI::GenericRequest
{
public:
	RFriendConfirmReciprocate(unsigned srcAvatarID, const ChatUnicodeString &srcAddress, unsigned destAvatarID, bool confirm);
	virtual ~RFriendConfirmReciprocate() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	unsigned m_destAvatarID;
	short m_confirm;
	Plat_Unicode::String m_srcAddress;
};
class RGetFanClubHandle : public GenericAPI::GenericRequest
{
public:
	RGetFanClubHandle(unsigned stationID, unsigned fanClubCode);
	virtual ~RGetFanClubHandle() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_stationID; // i.e. userID
	unsigned m_fanClubCode;
};

class RFindAvatarByUID : public GenericAPI::GenericRequest
{
public:
	RFindAvatarByUID(unsigned userID);
	virtual ~RFindAvatarByUID() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_userID;
};

class RRegistrarGetChatServer : public GenericAPI::GenericRequest
{
public:
	RRegistrarGetChatServer(std::string &hostname, unsigned short port);
	virtual ~RRegistrarGetChatServer() {};

	virtual void pack(Base::ByteStream &msg);

private:
	Plat_Unicode::String m_hostname;
	unsigned short m_port;
};

class RSendApiVersion : public GenericAPI::GenericRequest
{
public:
	RSendApiVersion(unsigned long version);
	virtual ~RSendApiVersion() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned long m_version;
};

class RAddSnoopAvatar : public GenericAPI::GenericRequest
{
public:
	RAddSnoopAvatar(unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RAddSnoopAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
};

class RRemoveSnoopAvatar : public GenericAPI::GenericRequest
{
public:
	RRemoveSnoopAvatar(unsigned srcAvatarID, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress);
	virtual ~RRemoveSnoopAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_destAvatarName;
	Plat_Unicode::String m_destAvatarAddress;
};

class RAddSnoopRoom : public GenericAPI::GenericRequest
{
public:
	RAddSnoopRoom(unsigned srcAvatarID, const ChatUnicodeString &roomAddress);
	virtual ~RAddSnoopRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_roomAddress;
};

class RRemoveSnoopRoom : public GenericAPI::GenericRequest
{
public:
	RRemoveSnoopRoom(unsigned srcAvatarID, const ChatUnicodeString &roomAddress);
	virtual ~RRemoveSnoopRoom() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
	Plat_Unicode::String m_roomAddress;
};

class RGetSnoopList : public GenericAPI::GenericRequest
{
public:
	RGetSnoopList(unsigned srcAvatarID);
	virtual ~RGetSnoopList() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_srcAvatarID;
};

class RTransferAvatar : public GenericAPI::GenericRequest
{
public:
	RTransferAvatar(unsigned userID, unsigned newUserID, const ChatUnicodeString &avatarName, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &newAvatarAddress, bool transferPersistentMessages);
	virtual ~RTransferAvatar() {};

	virtual void pack(Base::ByteStream &msg);

private:
	unsigned m_userID;
	unsigned m_newUserID;
	Plat_Unicode::String m_avatarName;
	Plat_Unicode::String m_newAvatarName;
	Plat_Unicode::String m_avatarAddress;
	Plat_Unicode::String m_newAvatarAddress;
	bool m_transferPersistentMessages;
};


}; // end namespace


#endif

