#include <stdio.h>

#include "ChatAPICore.h"
#include "ChatAPI.h"
#include "Response.h"
#include "Request.h"
#include "ChatAvatar.h"
#include "ChatRoom.h"
#include "PersistentMessage.h"
#include "GenericAPI/GenericConnection.h"

namespace ChatSystem 
{

ChatAPI::ChatAPI(const char *registrar_host, short registrar_port, const char *server_host, short server_port)
: m_defaultRoomParams(nullptr), 
  m_defaultLoginPriority(0), 
  m_defaultEntryType(false)
{
	GenericAPI::GenericConnection::ms_crcBytes = 2;
	m_core = new ChatAPICore(registrar_host, registrar_port, server_host, server_port);
	m_core->setAPI(this);
}

ChatAPI::~ChatAPI()
{
	delete m_defaultRoomParams;
	m_defaultRoomParams = nullptr;
	delete m_core;
	m_core = nullptr;
}

ChatAPI::ChatAPI(ChatAPICore *core)
: m_defaultRoomParams(nullptr)
{
	m_core = core;
	m_core->setAPI(this);
}

void ChatAPI::setRequestTimeout(unsigned requestTimeout)
{
	m_core->setRequestTimeout(requestTimeout);
}

void ChatAPI::Process()
{
	m_core->processAPI();
	m_core->clearRequestCount();
}

unsigned ChatAPI::RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user)
{
	return RequestLoginAvatar(userID, avatarName, avatarAddress, m_defaultLoginLocation, m_defaultLoginPriority, 0, user);
}

unsigned ChatAPI::RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, void *user)
{
	return RequestLoginAvatar(userID, avatarName, avatarAddress, avatarLoginLocation, m_defaultLoginPriority, 0, user);
}

unsigned ChatAPI::RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, void *user)
{
    return RequestLoginAvatar(userID, avatarName, avatarAddress, avatarLoginLocation, avatarLoginPriority, 0, user);
}

unsigned ChatAPI::RequestLoginAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &avatarLoginLocation, int avatarLoginPriority, int avatarLoginAttributes, void *user)
{
    RLoginAvatar *req = new RLoginAvatar(userID, avatarName, avatarAddress, avatarLoginLocation, avatarLoginPriority, avatarLoginAttributes);
    ResLoginAvatar *res = new ResLoginAvatar(user, avatarLoginPriority);
    return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestTemporaryAvatar(unsigned userID, const ChatUnicodeString& avatarName, const ChatUnicodeString& avatarAddress, const ChatUnicodeString& avatarLoginLocation, void* user)
{
    RTemporaryAvatar* req = new RTemporaryAvatar(userID, avatarName, avatarAddress, avatarLoginLocation  );
    ResTemporaryAvatar* res = new ResTemporaryAvatar(user);
    return (m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestLogoutAvatar(const ChatAvatar *srcAvatar, void *user)
{
	RLogoutAvatar *req = new RLogoutAvatar(srcAvatar->getAvatarID(), srcAvatar->getAddress());
	ResLogoutAvatar *res = new ResLogoutAvatar(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestDestroyAvatar(const ChatAvatar *srcAvatar, void *user)
{
	RDestroyAvatar *req = new RDestroyAvatar(srcAvatar->getAvatarID(), srcAvatar->getAddress());
	ResDestroyAvatar *res = new ResDestroyAvatar(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSetAvatarKeywords(const ChatAvatar *srcAvatar, const ChatUnicodeString *keywordList, unsigned keywordLength, void *user)
{
	RSetAvatarKeywords *req = new RSetAvatarKeywords(srcAvatar->getAvatarID(), keywordList, keywordLength);
	ResSetAvatarKeywords *res = new ResSetAvatarKeywords(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}
		
unsigned ChatAPI::RequestGetAvatarKeywords(const ChatAvatar *srcAvatar, void *user)
{
	RGetAvatarKeywords *req = new RGetAvatarKeywords(srcAvatar->getAvatarID());
	ResGetAvatarKeywords *res = new ResGetAvatarKeywords(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSearchAvatarKeywords(const ChatUnicodeString &nodeAddress, const ChatUnicodeString *keywordList, unsigned keywordLength, void *user)
{
	RSearchAvatarKeywords *req = new RSearchAvatarKeywords(nodeAddress, keywordList, keywordLength);
	ResSearchAvatarKeywords *res = new ResSearchAvatarKeywords(user);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user)
{
	RGetAvatar *req = new RGetAvatar(avatarName, avatarAddress);
	ResGetAvatar *res = new ResGetAvatar(user);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestGetAnyAvatar(const ChatUnicodeString& avatarName, const ChatUnicodeString& avatarAddress, void* user )
{
    RGetAnyAvatar* req = new RGetAnyAvatar(avatarName, avatarAddress);
    ResGetAnyAvatar* res = new ResGetAnyAvatar(user);
    return (m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAvatarList(unsigned userID, void* user )
{
    RAvatarList* req = new RAvatarList(userID);
    ResAvatarList* res = new ResAvatarList(user);
    return (m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSetAvatarAttributes(const ChatAvatar *srcAvatar, unsigned long avatarAttributes, void *user)
{
	RSetAvatarAttributes *req = new RSetAvatarAttributes(srcAvatar->getAvatarID(), srcAvatar->getAddress(), avatarAttributes);
	ResSetAvatarAttributes *res = new ResSetAvatarAttributes(user);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestSetAvatarSessionAttributes(const ChatAvatar *srcAvatar, unsigned long avatarAttributes, void *user)
{
    RSetAvatarAttributes *req = new RSetAvatarAttributes(srcAvatar->getAvatarID(), srcAvatar->getAddress(), avatarAttributes, false);
    ResSetAvatarAttributes *res = new ResSetAvatarAttributes(user);
    return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestSetAvatarStatusMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString& statusMessage, void *user)
{
    RSetAvatarStatusMessage *req    = new RSetAvatarStatusMessage(srcAvatar->getAvatarID(), srcAvatar->getAddress(), statusMessage);
    ResSetAvatarStatusMessage* res  = new ResSetAvatarStatusMessage(user);
    return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestSetAvatarForwardingEmail(const ChatAvatar *srcAvatar, const ChatUnicodeString &avatarForwardingEmail, void *user)
{
	RSetAvatarForwardingEmail *req = new RSetAvatarForwardingEmail(srcAvatar->getAvatarID(), avatarForwardingEmail);
	ResSetAvatarForwardingEmail *res = new ResSetAvatarForwardingEmail(user);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestSetAvatarInboxLimit(const ChatAvatar *srcAvatar, unsigned long avatarInboxLimit, void *user)
{
	RSetAvatarInboxLimit *req = new RSetAvatarInboxLimit(srcAvatar->getAvatarID(), avatarInboxLimit);
	ResSetAvatarInboxLimit *res = new ResSetAvatarInboxLimit(user);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestFindAvatarByUID(unsigned userID, void *user)
{
	RFindAvatarByUID *req = new RFindAvatarByUID(userID);
	ResFindAvatarByUID *res = new ResFindAvatarByUID(user);
	return(m_core->submitRequest(req,res));
}

ChatAvatar *ChatAPI::getAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress)
{
	return m_core->getAvatar(avatarName, avatarAddress);
}

ChatAvatar *ChatAPI::getAvatar(unsigned avatarID)
{
	return m_core->getAvatar(avatarID);
}

unsigned ChatAPI::RequestTransferAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned newUserID, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &newAvatarAddress, void *user)
{
	return RequestTransferAvatar(userID, avatarName, avatarAddress, newUserID, newAvatarName, newAvatarAddress, false, user);
}

unsigned ChatAPI::RequestTransferAvatar(unsigned userID, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned newUserID, const ChatUnicodeString &newAvatarName, const ChatUnicodeString &newAvatarAddress, bool transferPersistentMessages, void *user)
{
	RTransferAvatar *req = new RTransferAvatar(userID, newUserID, avatarName, newAvatarName, avatarAddress, newAvatarAddress, transferPersistentMessages);
	ResTransferAvatar *res = new ResTransferAvatar(user, userID, newUserID, avatarName, newAvatarName, avatarAddress, newAvatarAddress);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestFanClubHandle(unsigned stationID, unsigned fanClubCode, void *user)
{
	RGetFanClubHandle *req = new RGetFanClubHandle(stationID, fanClubCode);
	ResGetFanClubHandle *res = new ResGetFanClubHandle(stationID, fanClubCode, user);
	return(m_core->submitRequest(req,res));
}

unsigned ChatAPI::RequestCreateRoom(const ChatAvatar *srcAvatar, const RoomParams &roomParams, const ChatUnicodeString &parentAddress, void *user)
{
	RCreateRoom *req = new RCreateRoom(srcAvatar->getAvatarID(), srcAvatar->getAddress(), roomParams, parentAddress);
	ResCreateRoom *res = new ResCreateRoom(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestDestroyRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user)
{
	RDestroyRoom *req = new RDestroyRoom(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress);
	ResDestroyRoom *res = new ResDestroyRoom(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetRoomSummaries(const ChatUnicodeString &startNodeAddress, const ChatUnicodeString &roomFilter, void *user)
{
	RGetRoomSummaries *req = new RGetRoomSummaries(startNodeAddress, roomFilter);
	ResGetRoomSummaries *res = new ResGetRoomSummaries(user);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendInstantMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	RSendInstantMessage *req = new RSendInstantMessage(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, msg, oob);
	ResSendInstantMessage *res = new ResSendInstantMessage(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendRoomMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	RSendRoomMessage *req = new RSendRoomMessage(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress, msg, oob);
	ResSendRoomMessage *res = new ResSendRoomMessage(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendBroadcastMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destNodeAddress, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	RSendBroadcastMessage *req = new RSendBroadcastMessage(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destNodeAddress, msg, oob);
	ResSendBroadcastMessage *res = new ResSendBroadcastMessage(user, srcAvatar->getAvatarID(), destNodeAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestFilterMessage(const ChatUnicodeString &in_msg, void *user)
{
	RFilterMessage *req = new RFilterMessage(in_msg);
	ResFilterMessage *res = new ResFilterMessage(user);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestFilterMessageEx(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &msg, void *user)
{
	RFilterMessageEx *req = new RFilterMessageEx(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, msg);
	ResFilterMessage *res = new ResFilterMessage(user, RESPONSE_FILTERMESSAGE_EX);
	return(m_core->submitRequest(req, res));

}

unsigned ChatAPI::RequestAddFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, bool confirm, void *user)
{
	return RequestAddFriend(srcAvatar, destAvatarName, destAvatarAddress, ChatUnicodeString(), confirm, user);
}

unsigned ChatAPI::RequestAddFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, bool confirm, void *user)
{
	return RequestAddFriendReciprocate(srcAvatar, destAvatarName, destAvatarAddress, ChatUnicodeString(), confirm, user);
}

unsigned ChatAPI::RequestAddFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, bool confirm, void *user)
{	
	RAddFriend *req = new RAddFriend(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, friendComment, confirm);
	ResAddFriend *res = new ResAddFriend(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress, friendComment);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAddFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, bool confirm, void *user)
{	
	RAddFriendReciprocate *req = new RAddFriendReciprocate(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, friendComment, confirm);
	ResAddFriendReciprocate *res = new ResAddFriendReciprocate(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress, friendComment);
	return(m_core->submitRequest(req, res));
}
unsigned ChatAPI::RequestSetFriendComment(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &friendComment, void *user)
{	
	RSetFriendComment *req = new RSetFriendComment(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, friendComment);
	ResSetFriendComment *res = new ResSetFriendComment(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress, friendComment);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveFriend(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RRemoveFriend *req = new RRemoveFriend(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress);
	ResRemoveFriend *res = new ResRemoveFriend(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveFriendReciprocate(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RRemoveFriendReciprocate *req = new RRemoveFriendReciprocate(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress);
	ResRemoveFriendReciprocate *res = new ResRemoveFriendReciprocate(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestFriendStatus(const ChatAvatar *srcAvatar, void *user)
{
	RFriendStatus *req = new RFriendStatus(srcAvatar->getAvatarID(), srcAvatar->getAddress());
	ResFriendStatus *res = new ResFriendStatus(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAddIgnore(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RAddIgnore *req = new RAddIgnore(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress);
	ResAddIgnore *res = new ResAddIgnore(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));

}

unsigned ChatAPI::RequestRemoveIgnore(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RRemoveIgnore *req = new RRemoveIgnore(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress);
	ResRemoveIgnore *res = new ResRemoveIgnore(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestIgnoreStatus(const ChatAvatar *srcAvatar, void *user)
{
	RIgnoreStatus *req = new RIgnoreStatus(srcAvatar->getAvatarID(), srcAvatar->getAddress());
	ResIgnoreStatus *res = new ResIgnoreStatus(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user)
{
	return RequestEnterRoom(srcAvatar, destRoomAddress, ChatUnicodeString(), user);
}

unsigned ChatAPI::RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destRoomPassword, void *user)
{
	return RequestEnterRoom(srcAvatar, destRoomAddress, destRoomPassword, m_defaultEntryType, user);
}

unsigned ChatAPI::RequestEnterRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destRoomPassword, bool requestEntry, void *user)
{
	REnterRoom *req = new REnterRoom(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress, destRoomPassword, m_defaultRoomParams, requestEntry);
	ResEnterRoom *res = new ResEnterRoom(user, srcAvatar->getAvatarID(), destRoomAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAllowRoomEntry(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, bool allow, void *user)
{
	RAllowRoomEntry *req = new RAllowRoomEntry(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress, allow);
	ResAllowRoomEntry *res = new ResAllowRoomEntry(user, srcAvatar->getAvatarID(), destRoomAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestLeaveRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, void *user)
{
	RLeaveRoom *req = new RLeaveRoom(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress);
	ResLeaveRoom *res = new ResLeaveRoom(user, srcAvatar->getAvatarID(), destRoomAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetRoom(const ChatUnicodeString &roomAddress, void *user)
{
	RGetRoom *req = new RGetRoom(roomAddress);
	ResGetRoom *res = new ResGetRoom(user);
	return(m_core->submitRequest(req, res));
}

ChatRoom *ChatAPI::getRoom(const ChatUnicodeString &roomAddress)
{
	return m_core->getRoom(roomAddress);
}

ChatRoom *ChatAPI::getRoom(unsigned roomID)
{
	return m_core->getRoom(roomID);
}

unsigned ChatAPI::RequestAddModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RAddModerator *req = new RAddModerator(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResAddModerator *res = new ResAddModerator(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RRemoveModerator *req = new RRemoveModerator(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResRemoveModerator *res = new ResRemoveModerator(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAddTemporaryModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RAddTemporaryModerator *req = new RAddTemporaryModerator(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResAddTemporaryModerator *res = new ResAddTemporaryModerator(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveTemporaryModerator(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{	
	RRemoveTemporaryModerator *req = new RRemoveTemporaryModerator(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResRemoveTemporaryModerator *res = new ResRemoveTemporaryModerator(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAddBan(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RAddBan *req = new RAddBan(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResAddBan *res = new ResAddBan(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveBan(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RRemoveBan *req = new RRemoveBan(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResRemoveBan *res = new ResRemoveBan(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAddInvite(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RAddInvite *req = new RAddInvite(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResAddInvite *res = new ResAddInvite(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRemoveInvite(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RRemoveInvite *req = new RRemoveInvite(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResRemoveInvite *res = new ResRemoveInvite(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}
	
unsigned ChatAPI::RequestGrantVoice(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RGrantVoice *req = new RGrantVoice(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResGrantVoice *res = new ResGrantVoice(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestRevokeVoice(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RRevokeVoice *req = new RRevokeVoice(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResRevokeVoice *res = new ResRevokeVoice(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestKickAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &destRoomAddress, void *user)
{
	RKickAvatar *req = new RKickAvatar(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatarName, destAvatarAddress, destRoomAddress);
	ResKickAvatar *res = new ResKickAvatar(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSetRoomParams(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const RoomParams *params, void *user)

{
	RSetRoomParams *req = new RSetRoomParams(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress, params);
	ResSetRoomParams *res = new ResSetRoomParams(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestChangeRoomOwner(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)

{
	RChangeRoomOwner *req = new RChangeRoomOwner(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destRoomAddress, destAvatarName, destAvatarAddress);
	ResChangeRoomOwner *res = new ResChangeRoomOwner(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	return RequestSendPersistentMessage(src, destAvatarName, destAvatarAddress, subject, msg, oob, ChatUnicodeString(), true, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user)
{
	return RequestSendPersistentMessage(src, destAvatarName, destAvatarAddress, subject, msg, oob, category, true, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user)
{
	return RequestSendPersistentMessage(src, destAvatarName, destAvatarAddress, subject, msg, oob, category, enforceInboxLimit, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatUnicodeString &src, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user)
{
	RSendPersistentMessage *req = new RSendPersistentMessage(src, destAvatarName, destAvatarAddress, subject, msg, oob, category, enforceInboxLimit, categoryLimit);
	ResSendPersistentMessage *res = new ResSendPersistentMessage(user, 0);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	return RequestSendPersistentMessage(srcAvatar, destAvatarName, destAvatarAddress, subject, msg, oob, ChatUnicodeString(), true, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user)
{
	return RequestSendPersistentMessage(srcAvatar, destAvatarName, destAvatarAddress, subject, msg, oob, category, true, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user)
{
	return RequestSendPersistentMessage(srcAvatar, destAvatarName, destAvatarAddress, subject, msg, oob, category, enforceInboxLimit, 0, user);
}

unsigned ChatAPI::RequestSendPersistentMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user)
{
	RSendPersistentMessage *req = new RSendPersistentMessage(srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress, subject, msg, oob, category, enforceInboxLimit, categoryLimit);
	ResSendPersistentMessage *res = new ResSendPersistentMessage(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user)
{
	return RequestSendMultiplePersistentMessages(srcAvatar, numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, true, 0, user);
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user)
{
	return RequestSendMultiplePersistentMessages(srcAvatar, numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, enforceInboxLimit, 0, user);
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses, const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user)
{
	RSendMultiplePersistentMessages *req = new RSendMultiplePersistentMessages(srcAvatar->getAvatarID(), numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, enforceInboxLimit, categoryLimit);
	ResSendMultiplePersistentMessages *res = new ResSendMultiplePersistentMessages(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, void *user)
{
	return RequestSendMultiplePersistentMessages(srcAvatar, numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, true, 0, user);
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, void *user)
{
	return RequestSendMultiplePersistentMessages(srcAvatar, numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, enforceInboxLimit, 0, user);
}

unsigned ChatAPI::RequestSendMultiplePersistentMessages(const ChatUnicodeString &srcAvatar, unsigned numDestAvatars, const ChatUnicodeString *destAvatarNames, const ChatUnicodeString *destAvatarAddresses,  const ChatUnicodeString &subject, const ChatUnicodeString &msg, const ChatUnicodeString &oob, const ChatUnicodeString &category, bool enforceInboxLimit, unsigned categoryLimit, void *user)
{
	RSendMultiplePersistentMessages *req = new RSendMultiplePersistentMessages(srcAvatar, numDestAvatars, destAvatarNames, destAvatarAddresses, subject, msg, oob, category, enforceInboxLimit, categoryLimit);
	ResSendMultiplePersistentMessages *res = new ResSendMultiplePersistentMessages(user, 0);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestAlterPersistentMessage(const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID, unsigned alterationAttributes, const ChatUnicodeString &newSenderName, const ChatUnicodeString &newSenderAddress,  const ChatUnicodeString &newSubject, const ChatUnicodeString &newMsg, const ChatUnicodeString &newOOB, const ChatUnicodeString &newCategory, unsigned newSentTime, void *user)
{
	RAlterPersistentMessage *req = new RAlterPersistentMessage(destAvatarName, destAvatarAddress, messageID, alterationAttributes, newSenderName, newSenderAddress,  newSubject, newMsg, newOOB, newCategory, newSentTime);
	ResAlterPersistentMessage *res = new ResAlterPersistentMessage(destAvatarName, destAvatarAddress, messageID, user);
	return (m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetPersistentHeaders(const ChatAvatar *srcAvatar, void *user)
{
	return RequestGetPersistentHeaders(srcAvatar, ChatUnicodeString(), user);
}

unsigned ChatAPI::RequestGetPersistentHeaders(const ChatAvatar *srcAvatar, const ChatUnicodeString &category, void *user)
{
	RGetPersistentHeaders *req = new RGetPersistentHeaders(srcAvatar->getAvatarID(), category);
	ResGetPersistentHeaders *res = new ResGetPersistentHeaders(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetPartialPersistentHeaders(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, void *user)
{
	return  RequestGetPartialPersistentHeaders(srcAvatar, maxHeaders, inDescendingOrder, sentTimeStart, ChatUnicodeString(), user);
}

unsigned ChatAPI::RequestGetPartialPersistentHeaders(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category, void *user)
{
	RGetPartialPersistentHeaders *req = new RGetPartialPersistentHeaders(srcAvatar->getAvatarID(), maxHeaders, inDescendingOrder, sentTimeStart, category);
	ResGetPartialPersistentHeaders *res = new ResGetPartialPersistentHeaders(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user)
{
	return RequestCountPersistentMessages(avatarName, avatarAddress, ChatUnicodeString(), user);
}

unsigned ChatAPI::RequestCountPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, void *user)
{
	RCountPersistentMessages *req = new RCountPersistentMessages(avatarName, avatarAddress, category);
	ResCountPersistentMessages *res = new ResCountPersistentMessages(user, avatarName, avatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetPersistentMessage(const ChatAvatar *srcAvatar, unsigned messageID, void *user)
{
	RGetPersistentMessage *req = new RGetPersistentMessage(srcAvatar->getAvatarID(), messageID);
	ResGetPersistentMessage *res = new ResGetPersistentMessage(user, srcAvatar->getAvatarID(), messageID);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetMultiplePersistentMessages(const ChatAvatar *srcAvatar, unsigned maxHeaders, bool inDescendingOrder, unsigned sentTimeStart, const ChatUnicodeString &category, void *user)
{
	RGetMultiplePersistentMessages *req = new RGetMultiplePersistentMessages(srcAvatar->getAvatarID(), maxHeaders, inDescendingOrder, sentTimeStart, category);
	ResGetMultiplePersistentMessages *res = new ResGetMultiplePersistentMessages(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestUpdatePersistentMessage(const ChatAvatar *srcAvatar, unsigned messageID, PersistentStatus status, void *user)
{
	RUpdatePersistentMessage *req = new RUpdatePersistentMessage(srcAvatar->getAvatarID(), messageID, status);
	ResUpdatePersistentMessage *res = new ResUpdatePersistentMessage(user, srcAvatar->getAvatarID(), messageID);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestUpdatePersistentMessages(const ChatAvatar *srcAvatar, PersistentStatus currentStatus, PersistentStatus newStatus, void *user)
{
	return RequestUpdatePersistentMessages(srcAvatar, currentStatus, newStatus, ChatUnicodeString(), user);
}

unsigned ChatAPI::RequestUpdatePersistentMessages(const ChatAvatar *srcAvatar, PersistentStatus currentStatus, PersistentStatus newStatus, const ChatUnicodeString &category, void *user)
{
	RUpdatePersistentMessages *req = new RUpdatePersistentMessages(srcAvatar->getAvatarID(), currentStatus, newStatus, category);
	ResUpdatePersistentMessages *res = new ResUpdatePersistentMessages(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestClassifyPersistentMessages(const ChatAvatar *srcAvatar, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder, void *user)
{
	RClassifyPersistentMessages *req = new RClassifyPersistentMessages(srcAvatar->getAvatarID(), numIDs, messageIDs, newFolder);
	ResClassifyPersistentMessages *res = new ResClassifyPersistentMessages(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestClassifyPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numIDs, const unsigned *messageIDs, const ChatUnicodeString &newFolder, void *user)
{
	RClassifyPersistentMessages *req = new RClassifyPersistentMessages(avatarName, avatarAddress, numIDs, messageIDs, newFolder);
	ResClassifyPersistentMessages *res = new ResClassifyPersistentMessages(user, 0);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, void *user)
{
	return RequestDeleteAllPersistentMessages(avatarName, avatarAddress, ChatUnicodeString(), 0, user);
}

unsigned ChatAPI::RequestDeleteAllPersistentMessages(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, const ChatUnicodeString &category, unsigned olderThan, void *user)
{
	RDeleteAllPersistentMessages *req = new RDeleteAllPersistentMessages(avatarName, avatarAddress, category, olderThan);
	ResDeleteAllPersistentMessages *res = new ResDeleteAllPersistentMessages(user, avatarName, avatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestUnregisterRoom(const ChatUnicodeString &roomAddress, void *user)
{
	RUnregisterRoom *req = new RUnregisterRoom(roomAddress);
	ResUnregisterRoom *res = new ResUnregisterRoom(user);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestFriendConfirm(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirm, void *user)
{
	RFriendConfirm *req = new RFriendConfirm(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatar->getAvatarID(), confirm);
	ResFriendConfirm *res = new ResFriendConfirm(srcAvatar->getAvatarID(), user);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestFriendConfirmReciprocate(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, bool confirm, void *user)
{
	RFriendConfirmReciprocate *req = new RFriendConfirmReciprocate(srcAvatar->getAvatarID(), srcAvatar->getAddress(), destAvatar->getAvatarID(), confirm);
	ResFriendConfirmReciprocate *res = new ResFriendConfirmReciprocate(srcAvatar->getAvatarID(), user);
	return(m_core->submitRequest(req, res));
}


unsigned ChatAPI::RequestBeginSnoopingAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RAddSnoopAvatar *req = new RAddSnoopAvatar(srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	ResAddSnoopAvatar *res = new ResAddSnoopAvatar(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestStopSnoopingAvatar(const ChatAvatar *srcAvatar, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, void *user)
{
	RRemoveSnoopAvatar *req = new RRemoveSnoopAvatar(srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	ResRemoveSnoopAvatar *res = new ResRemoveSnoopAvatar(user, srcAvatar->getAvatarID(), destAvatarName, destAvatarAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestBeginSnoopingRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user)
{
	RAddSnoopRoom *req = new RAddSnoopRoom(srcAvatar->getAvatarID(), roomAddress);
	ResAddSnoopRoom *res = new ResAddSnoopRoom(user, srcAvatar->getAvatarID(), roomAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestStopSnoopingRoom(const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user)
{
	RRemoveSnoopRoom *req = new RRemoveSnoopRoom(srcAvatar->getAvatarID(), roomAddress);
	ResRemoveSnoopRoom *res = new ResRemoveSnoopRoom(user, srcAvatar->getAvatarID(), roomAddress);
	return(m_core->submitRequest(req, res));
}

unsigned ChatAPI::RequestGetSnoopList(const ChatAvatar *srcAvatar, void *user)
{
	RGetSnoopList *req = new RGetSnoopList(srcAvatar->getAvatarID());
	ResGetSnoopList *res = new ResGetSnoopList(user, srcAvatar->getAvatarID());
	return(m_core->submitRequest(req, res));
}

const RoomParams *ChatAPI::getDefaultRoomParams(void)
{
	return m_defaultRoomParams;
}

void ChatAPI::setDefaultRoomParams(RoomParams *roomParams)
{
	if (roomParams)
	{
		if (m_defaultRoomParams == nullptr)
		{
			m_defaultRoomParams = new RoomParams;
		}

		// set new params
		(*m_defaultRoomParams) = (*roomParams);
	}
	else
	{
		delete m_defaultRoomParams;
		m_defaultRoomParams = nullptr;
	}
}
void ChatAPI::setDefaultLoginLocation(const ChatUnicodeString &defaultLoginLocation)
{
	m_defaultLoginLocation = defaultLoginLocation;
}

void ChatAPI::setDefaultLoginPriority(int defaultLoginPriority)
{
	m_defaultLoginPriority = defaultLoginPriority;
}

void ChatAPI::setDefaultEntryType(bool defaultEntryType)
{
	m_defaultEntryType = defaultEntryType;
}

bool ChatAPI::isUID(const ChatUnicodeString &uidNode)
{
	Plat_Unicode::String uid(uidNode.data(), uidNode.length());
	return ( m_core->isUID(uid) );
}


void ChatAPI::OnConnect()
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnConnect\n");
}

void ChatAPI::OnDisconnect()
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnDisconnect\n");
}

void ChatAPI::OnFailoverBegin()
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFailoverBegin\n");
}

void ChatAPI::OnFailoverComplete()
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFailoverComplete\n");
}

void ChatAPI::OnLoginAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnLoginAvatar\n");
}
void ChatAPI::OnLoginAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, int requiredLoginPriority, void *user)
{
	// //fprintf(stderr, "[ChatAPI.cpp] default OnLoginAvatar(with loginPriority) \n");
}

void ChatAPI::OnTemporaryAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, void *user)
{
    //fprintf(stderr, "[ChatAPI.cpp] default OnTemporaryAvatar\n");
}

void ChatAPI::OnLogoutAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnLogoutAvatar\n");
}

void ChatAPI::OnDestroyAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnDestroyAvatar\n");
}

void ChatAPI::OnGetAvatar(unsigned track, unsigned result, const ChatAvatar *foundAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetAvatar\n");
}

void ChatAPI::OnGetAnyAvatar(unsigned track, unsigned result, const ChatAvatar *foundAvatar, bool loggedIn, void *user)
{
    //fprintf(stderr, "[ChatAPI.cpp] default OnGetAnyAvatar\n");
}

void ChatAPI::OnAvatarList(unsigned track, unsigned result, unsigned listLength, const AvatarListItem* avatarList, void *user)
{
    //fprintf(stderr, "[ChatAPI.cpp] default OnAvatarList\n");
}

void ChatAPI::OnSetAvatarAttributes(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetAvatarAttributes\n");
}

void ChatAPI::OnSetAvatarStatusMessage(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user)
{
    //fprintf(stderr, "[ChatAPI.cpp] default OnSetAvatarStatusMessage\n");
}

void ChatAPI::OnSetAvatarForwardingEmail(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetAvatarForwardingEmail\n");
}

void ChatAPI::OnSetAvatarInboxLimit(unsigned track, unsigned result, const ChatAvatar *changedAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetAvatarInboxLimit\n");
}

void ChatAPI::OnTransferAvatar(unsigned track, unsigned result, unsigned oldUserID, unsigned newUserID, const ChatUnicodeString &oldName, const ChatUnicodeString &newName, const ChatUnicodeString &oldAddress, const ChatUnicodeString &newAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnTransferAvatar\n");
}

void ChatAPI::OnFanClubHandle(unsigned track, unsigned result, const ChatUnicodeString &handle, unsigned stationID, unsigned fanClubCode, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFanClubHandle\n");
}

void ChatAPI::OnCreateRoom(unsigned track, unsigned result, const ChatRoom *newRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnCreateRoom\n");
}

void ChatAPI::OnDestroyRoom(unsigned track, unsigned result, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnDestroyRoom\n");
}

void ChatAPI::OnGetRoom(unsigned track, unsigned result, const ChatRoom *room, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetRoom\n");
}

void ChatAPI::OnGetRoomSummaries(unsigned track, unsigned result, unsigned numFoundRooms, RoomSummary *foundRooms, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetRoomSummaries\n");
}

void ChatAPI::OnSendInstantMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSendInstantMessage\n");
}

void ChatAPI::OnSendRoomMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSendRoomMessage\n");
}

void ChatAPI::OnSendBroadcastMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &address, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSendBroadcastMessage\n");
}

void ChatAPI::OnFilterMessage(unsigned track, unsigned result, const ChatUnicodeString &filteredMsg, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFilterMessage\n");
}

void ChatAPI::OnFilterMessageEx(unsigned track, unsigned result, const ChatUnicodeString &filteredMsg, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFilterMessageEx\n");
}

void ChatAPI::OnAddFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddFriend\n");
}

void ChatAPI::OnAddFriendReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddFriendReciprocate\n");
}


void ChatAPI::OnSetFriendComment(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, const ChatUnicodeString &friendComment, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetFriendComment\n");
}

void ChatAPI::OnRemoveFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveFriend\n");
}

void ChatAPI::OnRemoveFriendReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveFriendReciprocate\n");
}

void ChatAPI::OnFriendStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatFriendStatus *friendList, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFriendStatus\n");
}

void ChatAPI::OnAddIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddIgnore\n");
}

void ChatAPI::OnRemoveIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveIgnore\n");
}

void ChatAPI::OnIgnoreStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatIgnoreStatus *ignoreList, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnIgnoreStatus\n");
}

void ChatAPI::OnEnterRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnEnterRoom\n");
}

void ChatAPI::OnAllowRoomEntry(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAllowRoomEntry\n");
}

void ChatAPI::OnLeaveRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnLeaveRoom\n");
}

void ChatAPI::OnReceiveDestroyRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveDestroyRoom\n");
}

void ChatAPI::OnAddModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddModerator\n");
}

void ChatAPI::OnRemoveModerator(unsigned track,unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveModerator\n");
}

void ChatAPI::OnAddTemporaryModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddTemporaryModerator\n");
}

void ChatAPI::OnRemoveTemporaryModerator(unsigned track,unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveTemporaryModerator\n");
}

void ChatAPI::OnAddBan(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddBan\n");
}

void ChatAPI::OnRemoveBan(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveBan\n");
}

void ChatAPI::OnAddInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAddInvite\n");
}

void ChatAPI::OnRemoveInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRemoveInvite\n");
}

void ChatAPI::OnGrantVoice(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGrantVoice\n");
}

void ChatAPI::OnRevokeVoice(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnRevokeVoice\n");
}

void ChatAPI::OnKickAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnKickAvatar\n");
}

void ChatAPI::OnSetRoomParams(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetRoomParams\n");
}

void ChatAPI::OnChangeRoomOwner(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnChangeRoomOwner\n");
}

void ChatAPI::OnSendPersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSendPersistentMessage\n");
}

void ChatAPI::OnSendPersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned messageID, void *user)
{
	// //fprintf(stderr, "[ChatAPI.cpp] default OnSendPersistentMessage (with message ID) \n");
}

void ChatAPI::OnSendMultiplePersistentMessages(unsigned track, unsigned result, unsigned numSubResults, const unsigned *subResults, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSendMultiplePersistentMessages\n");
}

void ChatAPI::OnSendMultiplePersistentMessages(unsigned track, unsigned result, unsigned numSubResults, const unsigned *subResults, const unsigned *messageIDs, const ChatAvatar *srcAvatar, void *user)
{
	// //fprintf(stderr, "[ChatAPI.cpp] default OnSendMultiplePersistentMessages (with message IDs)\n");
}

void ChatAPI::OnAlterPersistentMessage(unsigned track, unsigned result, const ChatUnicodeString &destAvatarName, const ChatUnicodeString &destAvatarAddress, unsigned messageID)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnAlterPersistentMessage\n");
}

void ChatAPI::OnGetPersistentHeaders(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, const PersistentHeader *list, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetPersistentHeaders\n");
}

void ChatAPI::OnGetPartialPersistentHeaders(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, const PersistentHeader *list, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetPartialPersistentHeaders\n");
}

void ChatAPI::OnCountPersistentMessages(unsigned track, unsigned result, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numberOfMessages, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnCountPersistentMessages\n");
}

void ChatAPI::OnGetPersistentMessage(unsigned track, unsigned result, ChatAvatar *destAvatar, const PersistentHeader *header, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetPersistentMessage\n");
}

void ChatAPI::OnGetMultiplePersistentMessages(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, PersistentMessage ** const list, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetMultiplePersistentMessages\n");
}

void ChatAPI::OnUpdatePersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnUpdatePersistentMessage\n");
}

void ChatAPI::OnUpdatePersistentMessages(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnUpdatePersistentMessages\n");
}

void ChatAPI::OnClassifyPersistentMessages(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnClassifyPersistentMessages\n");
}

void ChatAPI::OnDeleteAllPersistentMessages(unsigned track, unsigned result, const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress, unsigned numberDeleted, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnDeleteAllPersistentMessages\n");
}

void ChatAPI::OnUnregisterRoom(unsigned track, unsigned result, const ChatRoom *destRoom, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnUnregisterRoom\n");
}

void ChatAPI::OnSetAvatarKeywords(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSetAvatarKeywords\n");
}

void ChatAPI::OnGetAvatarKeywords(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString *keywordList, unsigned listLength, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetAvatarKeywords\n");
}

void ChatAPI::OnSearchAvatarKeywords(unsigned track, unsigned result, ChatAvatar **avatarMatches, unsigned numMatches, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnSearchAvatarKeywords\n");
}

void ChatAPI::OnFriendConfirm(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFriendConfirm\n");
}

void ChatAPI::OnFriendConfirmReciprocate(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFriendConfirmReciprocate\n");
}


void ChatAPI::OnFindAvatarByUID(unsigned track, unsigned result, unsigned numFoundAvatarsOnline, ChatAvatar **foundAvatars, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnFindAvatarByUID\n");
}

void ChatAPI::OnBeginSnoopingAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnBeginSnoopingAvatar\n");
}

void ChatAPI::OnStopSnoopingAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnStopSnoopingAvatar\n");
}

void ChatAPI::OnBeginSnoopingRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnBeginSnoopingRoom\n");
}

void ChatAPI::OnStopSnoopingRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &roomAddress, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnStopSnoopingRoom\n");
}

void ChatAPI::OnGetSnoopList(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned avatarSnoopListLength, AvatarSnoopPair **avatarSnoops, unsigned roomSnoopListLength, ChatUnicodeString **roomSnoops, void *user)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnGetSnoopList\n");
}

ChatUnicodeString ChatAPI::getErrorString(unsigned resultCode)
{
	return ChatAPICore::getErrorString(resultCode);
}

void ChatAPI::OnReceiveInstantMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveInstantMessage\n");
}

void ChatAPI::OnReceiveRoomMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRoomMessage\n");
}

void ChatAPI::OnReceiveRoomMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom, const ChatUnicodeString &msg, const ChatUnicodeString &oob, unsigned messageID)
{
	// //fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRoomMessage (with message ID)\n");
}

void ChatAPI::OnReceiveBroadcastMessage(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveBroadcastMessage\n");
}

void ChatAPI::OnReceiveFilterMessage(const ChatUnicodeString &msg)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFilterMessage\n");
}

void ChatAPI::OnReceiveFriendLogin(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendLogin\n");
}

void ChatAPI::OnReceiveFriendLogout(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendLogout\n");
}

void ChatAPI::OnReceiveFriendStatusChange(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar)
{
    //fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendStatusChange\n");
}

void ChatAPI::OnReceiveKickRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveKickRoom\n");
}

void ChatAPI::OnReceiveKickAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveKickAvatar\n");
}

void ChatAPI::OnReceiveRoomEntryRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &destRoomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRoomEntryRequest\n");
}

void ChatAPI::OnReceiveDelayedRoomEntry(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveDelayedRoomEntry\n");
}

void ChatAPI::OnReceiveDeniedRoomEntry(const ChatAvatar *srcAvatar, const ChatUnicodeString &destRoomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveDeniedRoomEntry\n");
}

void ChatAPI::OnReceiveEnterRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveEnterRoom\n");
}

void ChatAPI::OnReceiveLeaveRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveLeaveRoom\n");
}

void ChatAPI::OnReceiveAddModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddModeratorRoom\n");
}

void ChatAPI::OnReceiveAddModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddModeratorAvatar\n");
}

void ChatAPI::OnReceiveRemoveModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveModeratorRoom\n");
}

void ChatAPI::OnReceiveRemoveModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveModeratorAvatar\n");
}

void ChatAPI::OnReceiveAddTemporaryModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddTemporaryModeratorRoom\n");
}

void ChatAPI::OnReceiveAddTemporaryModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddTemporaryModeratorAvatar\n");
}

void ChatAPI::OnReceiveRemoveTemporaryModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveTemporaryModeratorRoom\n");
}

void ChatAPI::OnReceiveRemoveTemporaryModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveTemporaryModeratorAvatar\n");
}

void ChatAPI::OnReceiveAddBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddBanRoom\n");
}

void ChatAPI::OnReceiveAddBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddBanAvatar\n");
}

void ChatAPI::OnReceiveRemoveBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveBanRoom\n");
}

void ChatAPI::OnReceiveRemoveBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveBanAvatar\n");
}

void ChatAPI::OnReceiveAddInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddInviteRoom\n");
}

void ChatAPI::OnReceiveAddInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveAddInviteAvatar\n");
}

void ChatAPI::OnReceiveRemoveInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveInviteRoom\n");
}

void ChatAPI::OnReceiveRemoveInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRemoveInviteAvatar\n");
}

void ChatAPI::OnReceiveGrantVoiceRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveGrantVoiceRoom\n");
}

void ChatAPI::OnReceiveGrantVoiceAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveGrantVoiceAvatar\n");
}

void ChatAPI::OnReceiveRevokeVoiceRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRevokeVoiceRoom\n");
}

void ChatAPI::OnReceiveRevokeVoiceAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRevokeVoiceAvatar\n");
}

void ChatAPI::OnReceiveRoomParams(const ChatAvatar *srcAvatar, const ChatRoom *destRoom, const RoomParams *params, const RoomParams *oldParams)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRoomParams\n");
}

void ChatAPI::OnReceivePersistentMessage(const ChatAvatar *destAvatar, const PersistentHeader *header)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceivePersistentMessage\n");
}

void ChatAPI::OnReceiveForcedLogout(const ChatAvatar *oldAvatar)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveForcedLogout\n");
}

void ChatAPI::OnReceiveUnregisterRoomReady(const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveUnregisterRoomReady\n");
}

void ChatAPI::OnReceiveFriendConfirmRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendConfirmRequest\n");
}

void ChatAPI::OnReceiveFriendConfirmResponse(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar ,bool confirmed)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendConfirmResponse\n");
}

void ChatAPI::OnReceiveFriendConfirmReciprocateRequest(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendConfirmReciprocateRequest\n");
}

void ChatAPI::OnReceiveFriendConfirmReciprocateResponse(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar ,bool confirmed)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveFriendConfirmReciprocateResponse\n");
}

void ChatAPI::OnReceiveRoomOwnerChange(const ChatRoom *destRoom)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveRoomOwnerChange\n");
}

void ChatAPI::OnReceiveSnoopMessage(unsigned snoopType,
									const ChatUnicodeString &snooperName,
									const ChatUnicodeString &snooperAddress,
									const ChatUnicodeString &srcName,     // msg sender
									const ChatUnicodeString &srcAddress,  // msg sender
									const ChatUnicodeString &destName,    // dest avatar, invalid if room is destination
									const ChatUnicodeString &destAddress, // dest avatar or room address
									const ChatUnicodeString &message)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveSnoopMessage\n");
}

void ChatAPI::OnReceiveNotifyFriendIsRemoved(ChatAvatar * avatar, const ChatUnicodeString &friendName, const ChatUnicodeString &friendAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveNotifyFriendIsRemoved\n");
}

void ChatAPI::OnReceiveNotifyFriendsListChange(ChatAvatar * avatar,
												const ChatUnicodeString &originalName,     
												const ChatUnicodeString &originalAddress,  
												const ChatUnicodeString &newName,  
												const ChatUnicodeString &newAddress)
{
	//fprintf(stderr, "[ChatAPI.cpp] default OnReceiveNotifyFriendsListChange\n");
}


};

