#include "ChatAPICore.h"
#include "ChatAPI.h"
#include "Response.h"
#include "Request.h"
#include "Message.h"
#include "ChatAvatar.h"
#include "ChatAvatarCore.h"
#include "ChatRoomCore.h"
#include "ChatEnum.h"
#include "PersistentMessage.h"

#include <GenericAPI/GenericConnection.h>

const unsigned long kChatApiVersion = 2;

namespace ChatSystem
{
	using namespace std;
	using namespace Base;
	using namespace Plat_Unicode;
	using namespace GenericAPI;

	const char * const ChatAPICore::ms_errorStringsEnglish[] =
	{
		"Succeeded",	// 0
		"Timed Out",
		"Duplicate login",
		"Source avatar does not exist",
		"Destination avatar does not exist",
		"Address does not exist",				// 5
		"Address is not a room",
		"Address is not an AID node",
		"Friend not found",
		"Unknown failure",
		"Source avatar not in room",			// 10
		"Destination avatar not in room",
		"Avatar is banned from room",
		"Room is private",
		"Room is moderated",
		"Not in room",							// 15
		"Insufficient room privileges",
		"Database error",
		"Cannot get avatar ID",
		"Cannot get node ID",
		"Cannot get persistent message ID",		// 20
		"Persistent message not found",
		"Maximum number of avatars already in room",
		"Destination avatar is ignoring source avatar",
		"Room already exists",
		"Nothing to confirm",					// 25
		"Duplicate friend",
		"Ignore not found",
		"Duplicate ignore",
		"Database error",
		"Destination avatar is not a moderator",	// 30
		"Destination avatar is not a invited",
		"Destination avatar has not been banned",
		"Duplicate ban",
		"Duplicate moderator",
		"Duplicate invite",						// 35
		"Already in room",
		"Parent room is not persistent",
		"Parent node is of wrong type",
		"No fan club handle",
		"AID node already exists",				// 40
		"UID node already exists",
		"Wrong chat server for request",
		"Succeeded, but local data is invalid",
		"Login with nullptr name",
		"No server assigned to this identity",			// 45
		"Another server already assumed this identity",
		"Remote server is down",
		"Node ID conflict",
		"Invalid node name",
		"Insufficient message privileges",		// 50
		"Snoop already added",
		"Not snooping",
		"Destination avatar is not a temporary moderator",
		"Destination avatar does not have voice",
		"Duplicate temporary moderator",		// 55
		"Duplicate voice",
		"Chat avatar must first be logged out",
		"No work to do",
		"Cannot perform rename to nullptr avatar name",
		"Cannot perform station acct transfer to stationID = 0",	// 60
		"Cannot perform avatar move to nullptr avatar address",
		"Failed to obtain an ID for a new room or avatar",
		"Room is local to namespace/world; cannot enter from other worlds",
		"Room is local to game; cannot enter from other game namespaces",
		"Destination avatar has not submitted entry request for room",		// 65
		"Insufficient login priority to force logout of current avatar",
		"Avatar must wait to be allowed into specified room.",
		"Persistent message would exceed inbox limit for this avatar",
		"Duplicate destination specified for multiple persistent send",
		"Persistent message would exceed given category limit",				// 70
		"Add friend request is pending due to destination avatar is offline",
	};

	unsigned ChatAPICore::ms_numErrorStrings = sizeof(ChatAPICore::ms_errorStringsEnglish) / sizeof(const char *);

	ChatAPICore::ChatAPICore(const char *registrar_host, short registrar_port, const char *server_host, short server_port)
		: GenericAPICore(registrar_host,
			registrar_port,
			180 /*request timeout*/,
			10 /* reconnect timeout */,
			60 /*nodata timeout*/,
			60 /*noack timeout*/,
			2048 /*in buffer in KB*/,
			2048 /*out buffer in KB*/,
			15 /*keepalives in secs*/),
		m_connected(false),
		m_sentVersion(false),
		m_inFailoverMode(false),
		m_failoverAvatarResRemain(0),
		m_failoverRoomResRemain(0),
		m_requestCount(0),
		m_setToRegistrar(true),
		m_registrarHost(registrar_host),
		m_defaultServerHost(server_host),
		m_assignedServerHost(server_host),
		m_registrarPort(registrar_port),
		m_defaultServerPort(server_port),
		m_assignedServerPort(server_port),
		m_timeSinceLastDisconnect(time(nullptr)),
		m_rcvdRegistrarResponse(false),
		m_shouldSendVersion(true),
		m_api(nullptr)
	{
		// prevent user requests from going through
		suspendProcessing();
	}

	ChatAPICore::~ChatAPICore()
	{
		m_api = nullptr;

		std::map<unsigned, ChatAvatarCore*>::iterator iter = m_avatarCoreCache.begin();
		for (; iter != m_avatarCoreCache.end(); ++iter)
		{
			delete (*iter).second;
		}

		std::map<unsigned, ChatAvatar*>::iterator iter2 = m_avatarCache.begin();
		for (; iter2 != m_avatarCache.end(); ++iter2)
		{
			delete (*iter2).second;
		}

		std::map<unsigned, ChatRoomCore*>::iterator iter3 = m_roomCoreCache.begin();
		for (; iter3 != m_roomCoreCache.end(); ++iter3)
		{
			delete (*iter3).second;
		}

		std::map<unsigned, ChatRoom*>::iterator iter4 = m_roomCache.begin();
		for (; iter4 != m_roomCache.end(); ++iter4)
		{
			delete (*iter4).second;
		}
	}

	ChatUnicodeString ChatAPICore::getErrorString(unsigned resultCode)
	{
		ChatUnicodeString errorString;

		if (resultCode < ms_numErrorStrings)
		{
			errorString = ms_errorStringsEnglish[resultCode];
		}

		return errorString;
	}

	void ChatAPICore::cacheAvatar(ChatAvatarCore *avatarCore)
	{
		if (avatarCore)
		{
			pair < map<unsigned, ChatAvatarCore*>::iterator, bool > result;

			result = m_avatarCoreCache.insert(pair<unsigned, ChatAvatarCore *>(avatarCore->getAvatarID(), avatarCore));

			if (result.second == true)
			{
				// create m_avatarCache entry as well
				ChatAvatar *newAvatar = new ChatAvatar(avatarCore->getAvatarID(),
					avatarCore->getUserID(),
					ChatUnicodeString(avatarCore->getName().data(), avatarCore->getName().size()),
					ChatUnicodeString(avatarCore->getAddress().data(), avatarCore->getAddress().size()),
					ChatUnicodeString(avatarCore->getGateway().data(), avatarCore->getGateway().size()),
					ChatUnicodeString(avatarCore->getServer().data(), avatarCore->getServer().size()),
					avatarCore->getGatewayID(),
					avatarCore->getServerID(),
					ChatUnicodeString(avatarCore->getLoginLocation().data(), avatarCore->getLoginLocation().size()),
					avatarCore->getAttributes());

				newAvatar->setLoginPriority(avatarCore->getLoginPriority());
				newAvatar->setForwardingEmail(avatarCore->getEmail());
				newAvatar->setInboxLimit(avatarCore->getInboxLimit());

				m_avatarCache.insert(pair<unsigned, ChatAvatar *>(newAvatar->getAvatarID(), newAvatar));
			}
			else
			{
				// insert failed; we have to clean up the ChatAvatarCore
				delete avatarCore;
			}
		}
	}

	ChatAvatarCore *ChatAPICore::getAvatarCore(unsigned avatarID)
	{
		ChatAvatarCore *returnAvatar = nullptr;

		std::map<unsigned, ChatAvatarCore *>::iterator iter = m_avatarCoreCache.find(avatarID);
		if (iter != m_avatarCoreCache.end())
		{
			returnAvatar = (*iter).second;
		}
		return(returnAvatar);
	}

	ChatAvatar *ChatAPICore::getAvatar(unsigned avatarID)
	{
		ChatAvatar *returnAvatar = nullptr;

		std::map<unsigned, ChatAvatar *>::iterator iter = m_avatarCache.find(avatarID);
		if (iter != m_avatarCache.end())
		{
			returnAvatar = (*iter).second;
		}
		return(returnAvatar);
	}

	ChatAvatarCore *ChatAPICore::decacheAvatar(unsigned avatarID)
	{
		ChatAvatarCore *returnAvatar = nullptr;
		std::map<unsigned, ChatAvatarCore *>::iterator iterCore = m_avatarCoreCache.find(avatarID);
		if (iterCore != m_avatarCoreCache.end())
		{
			returnAvatar = (*iterCore).second;
			m_avatarCoreCache.erase(iterCore);

			// erase and delete from m_avatarCache as well
			std::map<unsigned, ChatAvatar *>::iterator iter = m_avatarCache.find(avatarID);
			delete (*iter).second;
			m_avatarCache.erase(iter);
		}
		return(returnAvatar);
	}

	void ChatAPICore::cacheRoom(ChatRoomCore *roomCore)
	{
		if (roomCore)
		{
			pair < map<unsigned, ChatRoomCore*>::iterator, bool > result;
			result = m_roomCoreCache.insert(pair<unsigned, ChatRoomCore *>(roomCore->getRoomID(), roomCore));

			if (result.second == true)
			{
				// create m_avatarCache entry as well
				ChatRoom *newRoom = new ChatRoom(roomCore);

				m_roomCache.insert(pair<unsigned, ChatRoom *>(roomCore->getRoomID(), newRoom));
			}
		}
	}

	ChatRoomCore *ChatAPICore::getRoomCore(unsigned roomID)
	{
		ChatRoomCore *returnRoom = nullptr;
		std::map<unsigned, ChatRoomCore *>::iterator iter = m_roomCoreCache.find(roomID);
		if (iter != m_roomCoreCache.end())
		{
			returnRoom = (*iter).second;
		}

		return(returnRoom);
	}

	ChatRoom *ChatAPICore::getRoom(unsigned roomID)
	{
		ChatRoom *returnRoom = nullptr;
		std::map<unsigned, ChatRoom *>::iterator iter = m_roomCache.find(roomID);
		if (iter != m_roomCache.end())
		{
			returnRoom = (*iter).second;
		}

		return(returnRoom);
	}

	ChatRoomCore *ChatAPICore::decacheRoom(unsigned roomID)
	{
		ChatRoomCore *returnRoom = nullptr;

		std::map<unsigned, ChatRoomCore *>::iterator iterCore = m_roomCoreCache.find(roomID);
		if (iterCore != m_roomCoreCache.end())
		{
			returnRoom = (*iterCore).second;
			m_roomCoreCache.erase(iterCore);

			// erase and delete from m_roomCache as well
			std::map<unsigned, ChatRoom *>::iterator iter = m_roomCache.find(roomID);
			delete (*iter).second;
			m_roomCache.erase(iter);
		}

		return(returnRoom);
	}

	void ChatAPICore::responseCallback(GenericResponse *res)
	{
		switch (res->getType())
		{
		case RESPONSE_LOGINAVATAR:
		{
			ResLoginAvatar *R = static_cast<ResLoginAvatar *>(res);
			ChatAvatar *avatar = nullptr;

			if (R->getAvatar())
			{
				cacheAvatar(R->getAvatar());
				avatar = getAvatar(R->getAvatar()->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_LOGINAVATAR: avatar=%p\n", avatar);
			}
			if (avatar)
			{
				avatar->setLoginPriority(R->getSubmittedLoginPriority());
			}
			m_api->OnLoginAvatar(R->getTrack(), R->getResult(), avatar, R->getUser());
			m_api->OnLoginAvatar(R->getTrack(), R->getResult(), avatar, R->getRequiredLoginPriority(), R->getUser());
		}
		break;
		case RESPONSE_TEMPORARYAVATAR:
		{
			ResTemporaryAvatar* R = static_cast<ResTemporaryAvatar*>(res);
			ChatAvatar* avatar = nullptr;

			if (R->getAvatar())
			{
				cacheAvatar(R->getAvatar());
				avatar = getAvatar(R->getAvatar()->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_TEMPORARYAVATAR: avatar=%p\n", avatar);
			}

			m_api->OnTemporaryAvatar(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_LOGOUTAVATAR:
		{
			ResLogoutAvatar *R = static_cast<ResLogoutAvatar *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_LOGOUTAVATAR: avatar=%p\n", avatar);
			}
			m_api->OnLogoutAvatar(R->getTrack(), R->getResult(), avatar, R->getUser());
			if (R->getResult() == 0)
			{
				ChatAvatarCore *avatarCore = decacheAvatar(R->getAvatarID());
				delete avatarCore;
			}
		}
		break;
		case RESPONSE_DESTROYAVATAR:
		{
			ResDestroyAvatar *R = static_cast<ResDestroyAvatar *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_DESTROYAVATAR: avatar=%p\n", avatar);
			}
			m_api->OnDestroyAvatar(R->getTrack(), R->getResult(), avatar, R->getUser());
			ChatAvatarCore *avatarCore = decacheAvatar(R->getAvatarID());
			delete avatarCore;
		}
		break;
		case RESPONSE_GETFANCLUBHANDLE:
		{
			ResGetFanClubHandle *R = static_cast<ResGetFanClubHandle *>(res);
			m_api->OnFanClubHandle(R->getTrack(), R->getResult(), ChatUnicodeString(R->getHandle().data(), R->getHandle().size()), R->getStationID(), R->getFanClubCode(), R->getUser());
		}
		break;
		case RESPONSE_GETAVATAR:
		{
			ResGetAvatar *R = static_cast<ResGetAvatar *>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				// attempt to return locally cached avatar, if available
				cachedAvatar = getAvatar(avatarCore->getAvatarID());

				if (cachedAvatar != nullptr)
				{
					// update cached information with returned data
					cachedAvatar->setAttributes(avatarCore->getAttributes());
					cachedAvatar->setForwardingEmail(avatarCore->getEmail());
					cachedAvatar->setInboxLimit(avatarCore->getInboxLimit());

					avatar = cachedAvatar;
				}
				else
				{
					// don't cache uncached avatars
					avatar = avatarCore->getNewChatAvatar();
				}
			}

			if ((!avatar || !avatarCore) && (R->getResult() == CHATRESULT_SUCCESS))
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETAVATAR: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
			}

			m_api->OnGetAvatar(R->getTrack(), R->getResult(), avatar, R->getUser());

			if (cachedAvatar == nullptr)
			{
				delete avatar;
			}

			delete avatarCore;
		}
		break;
		case RESPONSE_GETANYAVATAR:
		{
			ResGetAnyAvatar *R = static_cast<ResGetAnyAvatar *>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				// attempt to return locally cached avatar, if available
				cachedAvatar = getAvatar(avatarCore->getAvatarID());

				if (cachedAvatar != nullptr)
				{
					// update cached information with returned data
					cachedAvatar->setAttributes(avatarCore->getAttributes());
					cachedAvatar->setForwardingEmail(avatarCore->getEmail());
					cachedAvatar->setInboxLimit(avatarCore->getInboxLimit());

					avatar = cachedAvatar;
				}
				else
				{
					// don't cache uncached avatars
					avatar = avatarCore->getNewChatAvatar();
				}
			}

			if ((!avatar || !avatarCore) && (R->getResult() == CHATRESULT_SUCCESS))
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETANYAVATAR: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
			}

			m_api->OnGetAnyAvatar(R->getTrack(), R->getResult(), avatar, R->isLoggedIn(), R->getUser());

			if (cachedAvatar == nullptr)
			{
				delete avatar;
			}

			delete avatarCore;
		}
		break;
		case RESPONSE_AVATARLIST:
		{
			ResAvatarList *R = static_cast<ResAvatarList *>(res);

			m_api->OnAvatarList(R->getTrack(), R->getResult(), R->getListLength(), R->getAvatarList(), R->getUser());

			break;
		}
		case RESPONSE_SETAVATARATTRIBUTES:
		{
			ResSetAvatarAttributes *R = static_cast<ResSetAvatarAttributes *>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				avatar = avatarCore->getNewChatAvatar();
				if (avatar)
				{
					// attempt to return locally cached avatar, if available
					cachedAvatar = getAvatar(avatar->getAvatarID());

					if (cachedAvatar != nullptr)
					{
						cachedAvatar->setAttributes(avatar->getAttributes());
					}
				}
			}

			if (cachedAvatar != nullptr)
			{
				cachedAvatar = avatar;
				m_api->OnSetAvatarAttributes(R->getTrack(), R->getResult(), cachedAvatar, R->getUser());
			}
			else
			{
				if ((!avatar || !avatarCore) && R->getResult() == CHATRESULT_SUCCESS)
				{
					R->setResult(CHATRESULT_SUCCESSBADDATA);
					_chatdebug_("ChatAPI:BadData: RESPONSE_SETAVATARATTRIBUTES: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
				}
				m_api->OnSetAvatarAttributes(R->getTrack(), R->getResult(), avatar, R->getUser());
			}

			delete avatar;
			delete avatarCore;
		}
		break;
		case RESPONSE_SETSTATUSMESSAGE:
		{
			ResSetAvatarStatusMessage *R = static_cast<ResSetAvatarStatusMessage*>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				avatar = avatarCore->getNewChatAvatar();
				if (avatar)
				{
					// attempt to return locally cached avatar, if available
					cachedAvatar = getAvatar(avatar->getAvatarID());

					if (cachedAvatar != nullptr)
					{
						cachedAvatar->setStatusMessage(avatar->getStatusMessage());
					}
				}
			}

			if (cachedAvatar != nullptr)
			{
				cachedAvatar = avatar;
				m_api->OnSetAvatarStatusMessage(R->getTrack(), R->getResult(), cachedAvatar, R->getUser());
			}
			else
			{
				if ((!avatar || !avatarCore) && R->getResult() == CHATRESULT_SUCCESS)
				{
					R->setResult(CHATRESULT_SUCCESSBADDATA);
					_chatdebug_("ChatAPI:BadData: RESPONSE_SETAVATARATTRIBUTES: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
				}
				m_api->OnSetAvatarAttributes(R->getTrack(), R->getResult(), avatar, R->getUser());
			}

			delete avatar;
			delete avatarCore;
		}
		break;
		case RESPONSE_SETAVATAREMAIL:
		{
			ResSetAvatarForwardingEmail*R = static_cast<ResSetAvatarForwardingEmail*>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				avatar = avatarCore->getNewChatAvatar();
				if (avatar)
				{
					// attempt to return locally cached avatar, if available
					cachedAvatar = getAvatar(avatar->getAvatarID());

					if (cachedAvatar != nullptr)
					{
						cachedAvatar->setForwardingEmail(avatar->getForwardingEmail());
					}
				}
			}

			if (cachedAvatar != nullptr)
			{
				cachedAvatar = avatar;
				m_api->OnSetAvatarForwardingEmail(R->getTrack(), R->getResult(), cachedAvatar, R->getUser());
			}
			else
			{
				if ((!avatar || !avatarCore) && R->getResult() == CHATRESULT_SUCCESS)
				{
					R->setResult(CHATRESULT_SUCCESSBADDATA);
					_chatdebug_("ChatAPI:BadData: RESPONSE_SETAVATAREMAIL: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
				}
				m_api->OnSetAvatarForwardingEmail(R->getTrack(), R->getResult(), avatar, R->getUser());
			}

			delete avatar;
			delete avatarCore;
		}
		break;
		case RESPONSE_SETAVATARINBOXLIMIT:
		{
			ResSetAvatarInboxLimit *R = static_cast<ResSetAvatarInboxLimit*>(res);
			ChatAvatar *cachedAvatar = nullptr;
			ChatAvatar *avatar = nullptr;
			ChatAvatarCore *avatarCore = R->getAvatar();

			if (R->getResult() == 0 && avatarCore) // if success
			{
				avatar = avatarCore->getNewChatAvatar();
				if (avatar)
				{
					// attempt to return locally cached avatar, if available
					cachedAvatar = getAvatar(avatar->getAvatarID());

					if (cachedAvatar != nullptr)
					{
						cachedAvatar->setInboxLimit(avatar->getInboxLimit());
					}
				}
			}

			if (cachedAvatar != nullptr)
			{
				cachedAvatar = avatar;
				m_api->OnSetAvatarInboxLimit(R->getTrack(), R->getResult(), cachedAvatar, R->getUser());
			}
			else
			{
				if ((!avatar || !avatarCore) && R->getResult() == CHATRESULT_SUCCESS)
				{
					R->setResult(CHATRESULT_SUCCESSBADDATA);
					_chatdebug_("ChatAPI:BadData: RESPONSE_SETAVATARINBOXLIMIT: avatarCore=%p, avatar=%p\n", avatarCore, avatar);
				}
				m_api->OnSetAvatarInboxLimit(R->getTrack(), R->getResult(), avatar, R->getUser());
			}

			delete avatar;
			delete avatarCore;
		}
		break;
		case RESPONSE_GETROOM:
		{
			ResGetRoom *R = static_cast<ResGetRoom *>(res);
			ChatRoom *room = nullptr;
			ChatRoomCore *roomCore = R->getRoom();

			if (R->getResult() == 0 && roomCore) // if success
			{
				unsigned roomid = roomCore->getRoomID();
				if (getRoomCore(roomid) == nullptr)
				{
					// we need to cache this room first
					cacheRoom(roomCore);

					// are there any parent rooms to the one we
					// created that we need to cache as well?
					const set<ChatRoomCore *> &extraRooms = R->getExtraRooms();
					set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
					for (; iter != extraRooms.end(); ++iter)
					{
						cacheRoom(*iter);
					}
				}
				else
				{
					// room was already cached, so we must
					// recache it
					ChatRoomCore *oldCore = decacheRoom(roomid);

					delete oldCore;
					cacheRoom(roomCore);

					// if we were sent parent rooms, we need to delete
					// the ChatRoomCore objects because we shouldn't need them
					const set<ChatRoomCore *> &extraRooms = R->getExtraRooms();
					set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
					for (; iter != extraRooms.end(); ++iter)
					{
						delete (*iter);
					}
				}

				room = getRoom(roomid);
			}

			if ((!room || !roomCore) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETROOM: roomCore=%p, room=%p\n", roomCore, room);
			}
			m_api->OnGetRoom(R->getTrack(), R->getResult(), room, R->getUser());
		}
		break;
		case RESPONSE_CREATEROOM:
		{
			ResCreateRoom *R = static_cast<ResCreateRoom *>(res);

			ChatRoom *room = nullptr;
			ChatRoomCore* roomCore = nullptr;

			if (R->getResult() == 0) // if success
			{
				roomCore = R->getRoom();

				if (roomCore)
				{
					// cache this room
					cacheRoom(roomCore);
					room = getRoom(roomCore->getRoomID());

					if (room)
					{
						// are there any parent rooms to the one we
						// created that we need to cache as well?
						const set<ChatRoomCore *> &extraRooms = R->getExtraRooms();
						set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
						for (; iter != extraRooms.end(); ++iter)
						{
							cacheRoom(*iter);
						}
					}
				}
			}

			if ((!room || !roomCore) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_CREATEROOM: roomCore=%p, room=%p\n", roomCore, room);
			}
			m_api->OnCreateRoom(R->getTrack(), R->getResult(), room, R->getUser());
		}
		break;
		case RESPONSE_DESTROYROOM:
		{
			ResDestroyRoom *R = static_cast<ResDestroyRoom *>(res);
			m_api->OnDestroyRoom(R->getTrack(), R->getResult(), R->getUser());
			// decache occurs on MESSAGE_DESTROYROOM
		}
		break;
		case RESPONSE_SENDINSTANTMESSAGE:
		{
			ResSendInstantMessage *R = static_cast<ResSendInstantMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SENDINSTANTMESSAGE: avatar=%p\n", avatar);
			}
			m_api->OnSendInstantMessage(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_SENDROOMMESSAGE:
		{
			ResSendRoomMessage *R = static_cast<ResSendRoomMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			ChatRoom *room = getRoom(R->getRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SENDROOMMESSAGE: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnSendRoomMessage(R->getTrack(), R->getResult(), avatar, room, R->getUser());
		}
		break;
		case RESPONSE_SENDBROADCASTMESSAGE:
		{
			ResSendBroadcastMessage *R = static_cast<ResSendBroadcastMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SENDBROADCASTMESSAGE: avatar=%p\n", avatar);
			}
			m_api->OnSendBroadcastMessage(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_FILTERMESSAGE:
		{
			ResFilterMessage *R = static_cast<ResFilterMessage *>(res);
			m_api->OnFilterMessage(R->getTrack(), R->getResult(), ChatUnicodeString(R->getMsg().data(), R->getMsg().size()), R->getUser());
		}
		break;
		case RESPONSE_FILTERMESSAGE_EX:
		{
			ResFilterMessage *R = static_cast<ResFilterMessage *>(res);
			m_api->OnFilterMessageEx(R->getTrack(), R->getResult(), ChatUnicodeString(R->getMsg().data(), R->getMsg().size()), R->getUser());
		}
		break;
		case RESPONSE_ADDFRIEND:
		{
			ResAddFriend *R = static_cast<ResAddFriend *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDFRIEND: avatar=%p\n", avatar);
			}
			m_api->OnAddFriend(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_ADDFRIEND_RECIPROCATE:
		{
			ResAddFriendReciprocate *R = static_cast<ResAddFriendReciprocate *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDFRIEND_RECIPROCATE: avatar=%p\n", avatar);
			}
			m_api->OnAddFriendReciprocate(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_SETFRIENDCOMMENT:
		{
			ResSetFriendComment *R = static_cast<ResSetFriendComment *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDFRIEND: avatar=%p\n", avatar);
			}
			m_api->OnSetFriendComment(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), ChatUnicodeString(R->getComment().data(), R->getComment().size()), R->getUser());
		}
		break;
		case RESPONSE_REMOVEFRIEND:
		{
			ResRemoveFriend *R = static_cast<ResRemoveFriend *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEFRIEND: avatar=%p\n", avatar);
			}
			m_api->OnRemoveFriend(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;

		case RESPONSE_REMOVEFRIEND_RECIPROCATE:
		{
			ResRemoveFriendReciprocate *R = static_cast<ResRemoveFriendReciprocate *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEFRIEND_RECIPROCATE: avatar=%p\n", avatar);
			}
			m_api->OnRemoveFriendReciprocate(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_FRIENDSTATUS:
		{
			ResFriendStatus *R = static_cast<ResFriendStatus *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_FRIENDSTATUS: avatar=%p\n", avatar);
			}
			m_api->OnFriendStatus(R->getTrack(), R->getResult(), avatar, R->getListLength(), R->getFriendList(), R->getUser());
		}
		break;
		case RESPONSE_ADDIGNORE:
		{
			ResAddIgnore *R = static_cast<ResAddIgnore *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDIGNORE: avatar=%p\n", avatar);
			}
			m_api->OnAddIgnore(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_REMOVEIGNORE:
		{
			ResRemoveIgnore *R = static_cast<ResRemoveIgnore *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEIGNORE: avatar=%p\n", avatar);
			}
			m_api->OnRemoveIgnore(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getName().data(), R->getName().size()), ChatUnicodeString(R->getAddress().data(), R->getAddress().size()), R->getUser());
		}
		break;
		case RESPONSE_ENTERROOM:
		{
			ResEnterRoom *R = static_cast<ResEnterRoom *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			ChatRoomCore *roomCore = R->getRoom();
			ChatRoom *room = getRoom(R->getRoomID());

			if (R->getResult() == 0)
			{
				if (!room && roomCore)
				{
					// we didn't have this room cached already
					cacheRoom(roomCore);
					room = getRoom(roomCore->getRoomID());

					// are there any parent rooms to the one we
					// created that we need to cache as well?
					const set<ChatRoomCore *> &extraRooms = R->getExtraRooms();
					set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
					for (; iter != extraRooms.end(); ++iter)
					{
						cacheRoom(*iter);
					}
				}
				else if (room && roomCore)
				{
					// We already had room (and parents) cached, and our
					// ResEnterRoom object has created a new ChatRoomCore,
					// so we delete it because we don't need to cache it.
					delete roomCore;

					// if we were sent parent rooms, we need to delete
					// the ChatRoomCore objects because we shouldn't need them
					const set<ChatRoomCore *> &extraRooms = R->getExtraRooms();
					set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
					for (; iter != extraRooms.end(); ++iter)
					{
						delete (*iter);
					}
				}
				else if (!room && !roomCore)
				{
					// we didn't have room cached, and we didn't get one to cache,
					// thus we'll have trouble giving a callback with a nullptr pointer.
					_chatdebug_("ChatAPI:BadData: RESPONSE_ENTERROOM: avatar=%p, room=%p , roomCore=%p\n", avatar, room, roomCore);
				}
			}
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ENTERROOM: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnEnterRoom(R->getTrack(), R->getResult(), avatar, room, R->getUser());
		}
		break;
		case RESPONSE_ALLOWROOMENTRY:
		{
			ResAllowRoomEntry *R = static_cast<ResAllowRoomEntry *>(res);
			ChatAvatar *srcAvatar = getAvatar(R->getSrcAvatarID());

			if (!srcAvatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ALLOWROOMENTRY: srcAvatar=%p\n", srcAvatar);
			}

			m_api->OnAllowRoomEntry(R->getTrack(), R->getResult(), srcAvatar, R->getDestRoomAddress(), R->getUser());
		}
		break;
		case RESPONSE_LEAVEROOM:
		{
			ResLeaveRoom *R = static_cast<ResLeaveRoom *>(res);
			ChatRoom *room = getRoom(R->getRoomID());

			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_LEAVEROOM: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnLeaveRoom(R->getTrack(), R->getResult(), avatar, room, R->getUser());
		}
		break;
		case RESPONSE_ADDMODERATOR:
		{
			ResAddModerator *R = static_cast<ResAddModerator *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDMODERATOR: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnAddModerator(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_ADDMODERATOR
		}
		break;
		case RESPONSE_REMOVEMODERATOR:
		{
			ResRemoveModerator *R = static_cast<ResRemoveModerator *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEMODERATOR: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnRemoveModerator(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_REMOVEMODERATOR
		}
		break;
		case RESPONSE_ADDTEMPORARYMODERATOR:
		{
			ResAddTemporaryModerator *R = static_cast<ResAddTemporaryModerator *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDTEMPORARYMODERATOR: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnAddTemporaryModerator(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_ADDMTEMPORARYODERATOR
		}
		break;
		case RESPONSE_REMOVETEMPORARYMODERATOR:
		{
			ResRemoveTemporaryModerator *R = static_cast<ResRemoveTemporaryModerator *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVETEMPORARYMODERATOR: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnRemoveTemporaryModerator(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_REMOVETEMPORARYMODERATOR
		}
		break;
		case RESPONSE_ADDBAN:
		{
			ResAddBan *R = static_cast<ResAddBan *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDBAN: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnAddBan(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_ADDBAN
		}
		break;
		case RESPONSE_REMOVEBAN:
		{
			ResRemoveBan *R = static_cast<ResRemoveBan *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEBAN: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnRemoveBan(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_REMOVEBAN
		}
		break;
		case RESPONSE_ADDINVITE:
		{
			ResAddInvite *R = static_cast<ResAddInvite *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDINVITE: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnAddInvite(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_ADDINVITE
		}
		break;
		case RESPONSE_REMOVEINVITE:
		{
			ResRemoveInvite *R = static_cast<ResRemoveInvite *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVEINVITE: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnRemoveInvite(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_REMOVEINVITE
		}
		break;
		case RESPONSE_GRANTVOICE:
		{
			ResGrantVoice *R = static_cast<ResGrantVoice *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GRANTVOICE: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnGrantVoice(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_GRANTVOICE
		}
		break;
		case RESPONSE_REVOKEVOICE:
		{
			ResRevokeVoice *R = static_cast<ResRevokeVoice *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REVOKEVOICE: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnRevokeVoice(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_REVOKEVOICE
		}
		break;
		case RESPONSE_KICKAVATAR:
		{
			ResKickAvatar *R = static_cast<ResKickAvatar *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_KICKAVATAR: avatar=%p, room=%p\n", avatar, room);
			}
			m_api->OnKickAvatar(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_KICK
		}
		break;
		case RESPONSE_SETROOMPARAMS:
		{
			ResSetRoomParams *R = static_cast<ResSetRoomParams *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SETROOMPARAMS: avatar=%p, room=%p\n", avatar, room);
			}

			m_api->OnSetRoomParams(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_SETROOMPARAMS
		}
		break;
		case RESPONSE_CHANGEROOMOWNER:
		{
			ResChangeRoomOwner *R = static_cast<ResChangeRoomOwner *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatRoom *room = getRoom(R->getDestRoomID());
			if ((!avatar || !room) && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_CHANGEROOMOWNER: avatar=%p, room=%p\n", avatar, room);
			}

			m_api->OnChangeRoomOwner(R->getTrack(), R->getResult(), avatar, room, R->getUser());
			// cached room gets updated by MESSAGE_CHANGEROOMOWNER
		}
		break;
		case RESPONSE_GETROOMSUMMARIES:
		{
			ResGetRoomSummaries *R = static_cast<ResGetRoomSummaries *>(res);
			unsigned numRooms = R->getNumRooms();
			RoomSummary *foundRooms = R->getRoomSummaries();

			m_api->OnGetRoomSummaries(R->getTrack(), R->getResult(), numRooms, foundRooms, R->getUser());
		}
		break;
		case RESPONSE_FINDAVATARBYUID:
		{
			ResFindAvatarByUID *R = static_cast<ResFindAvatarByUID *>(res);
			ChatAvatar **avatarMatches = nullptr;
			ChatAvatarCore **avatarCoreMatches = nullptr;

			unsigned numAvatarsOnline = R->getNumAvatarsOnline();

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatarCoreMatches = R->getAvatarsCore();
				avatarMatches = new ChatAvatar*[numAvatarsOnline];
				for (unsigned i(0); i < numAvatarsOnline; i++)
				{
					avatarMatches[i] = avatarCoreMatches[i]->getNewChatAvatar();
				}
			}

			m_api->OnFindAvatarByUID(R->getTrack(), R->getResult(), numAvatarsOnline, avatarMatches, R->getUser());

			delete[] avatarMatches;
		}
		break;
		case RESPONSE_SENDPERSISTENTMESSAGE:
		{
			ResSendPersistentMessage *R = static_cast<ResSendPersistentMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SENDPERSISTENTMESSAGE: avatar=%p\n", avatar);
			}
			m_api->OnSendPersistentMessage(R->getTrack(), R->getResult(), avatar, R->getUser());
			m_api->OnSendPersistentMessage(R->getTrack(), R->getResult(), avatar, R->getMessageID(), R->getUser());
		}
		break;
		case RESPONSE_SENDMULTIPLEPERSISTENTMESSAGES:
		{
			ResSendMultiplePersistentMessages *R = static_cast<ResSendMultiplePersistentMessages *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SENDMULTIPLEPERSISTENTMESSAGES: avatar=%p\n", avatar);
			}
			m_api->OnSendMultiplePersistentMessages(R->getTrack(), R->getResult(), R->getNumResults(), &R->getResultVector()[0], avatar, R->getUser());
			m_api->OnSendMultiplePersistentMessages(R->getTrack(), R->getResult(), R->getNumResults(), &R->getResultVector()[0], &R->getMessageIDvector()[0], avatar, R->getUser());
		}
		break;
		case RESPONSE_ALTERPERSISTENTMESSAGE:
		{
			ResAlterPersistentMessage *R = static_cast<ResAlterPersistentMessage *>(res);
			ChatUnicodeString destAvatarName(R->getDestAvatarName().data(), R->getDestAvatarName().length());
			ChatUnicodeString destAvatarAddress(R->getDestAvatarAddress().data(), R->getDestAvatarAddress().length());

			m_api->OnAlterPersistentMessage(R->getTrack(), R->getResult(), destAvatarName, destAvatarAddress, R->getMessageID());
		}
		break;
		case RESPONSE_GETPERSISTENTMESSAGE:
		{
			ResGetPersistentMessage *R = static_cast<ResGetPersistentMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			ChatUnicodeString message(R->getMsg().data(), R->getMsg().size());
			ChatUnicodeString oobdata(R->getOOB().data(), R->getOOB().size());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETPERSISTENTMESSAGE: avatar=%p\n", avatar);
			}
			m_api->OnGetPersistentMessage(R->getTrack(), R->getResult(), avatar, R->getHeader(), message, oobdata, R->getUser());
		}
		break;
		case RESPONSE_GETMULTIPLEPERSISTENTMESSAGES:
		{
			ResGetMultiplePersistentMessages *R = static_cast<ResGetMultiplePersistentMessages *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETMULTIPLEPERSISTENTMESSAGES: avatar=%p\n", avatar);
			}
			m_api->OnGetMultiplePersistentMessages(R->getTrack(), R->getResult(), avatar, R->getListLength(), R->getList(), R->getUser());
		}
		break;
		case RESPONSE_UPDATEPERSISTENTMESSAGE:
		{
			ResUpdatePersistentMessage *R = static_cast<ResUpdatePersistentMessage *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_UPDATEPERSISTENTMESSAGE: avatar=%p\n", avatar);
			}
			m_api->OnUpdatePersistentMessage(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_UPDATEPERSISTENTMESSAGES:
		{
			ResUpdatePersistentMessages *R = static_cast<ResUpdatePersistentMessages *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_UPDATEPERSISTENTMESSAGES: avatar=%p\n", avatar);
			}
			m_api->OnUpdatePersistentMessages(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_CHANGEPERSISTENTFOLDER:
		{
			ResClassifyPersistentMessages *R = static_cast<ResClassifyPersistentMessages *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_CHANGEPERSISTENTFOLDER: avatar=%p\n", avatar);
			}
			m_api->OnClassifyPersistentMessages(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_PURGEPERSISTENTMESSAGES:
		{
			ResDeleteAllPersistentMessages *R = static_cast<ResDeleteAllPersistentMessages *>(res);

			m_api->OnDeleteAllPersistentMessages(R->getTrack(), R->getResult(), R->getAvatarName(), R->getAvatarAddress(), R->getNumberDeleted(), R->getUser());
		}
		break;
		case RESPONSE_GETPERSISTENTHEADERS:
		{
			ResGetPersistentHeaders *R = static_cast<ResGetPersistentHeaders *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETPERSISTENTHEADERS: avatar=%p\n", avatar);
			}
			m_api->OnGetPersistentHeaders(R->getTrack(), R->getResult(), avatar, R->getListLength(), R->getList(), R->getUser());
		}
		break;
		case RESPONSE_PARTIALPERSISTENTHEADERS:
		{
			ResGetPartialPersistentHeaders *R = static_cast<ResGetPartialPersistentHeaders *>(res);
			ChatAvatar *avatar = getAvatar(R->getSrcAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETPARTIALPERSISTENTHEADERS: avatar=%p\n", avatar);
			}
			m_api->OnGetPartialPersistentHeaders(R->getTrack(), R->getResult(), avatar, R->getListLength(), R->getList(), R->getUser());
		}
		break;
		case RESPONSE_COUNTPERSISTENTMESSAGES:
		{
			ResCountPersistentMessages *R = static_cast<ResCountPersistentMessages *>(res);

			m_api->OnCountPersistentMessages(R->getTrack(), R->getResult(), R->getAvatarName(), R->getAvatarAddress(), R->getNumberOfMessages(), R->getUser());
		}
		break;
		case RESPONSE_UNREGISTERROOM:
		{
			ResUnregisterRoom *R = static_cast<ResUnregisterRoom *>(res);
			ChatRoom *room = getRoom(R->getDestRoomID());
			if (!room && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_UNREGISTERROOM: room=%p\n", room);
			}
			m_api->OnUnregisterRoom(R->getTrack(), R->getResult(), room, R->getUser());

			if (R->getResult() == 0 && room)
			{
				ChatRoomCore *roomCore = decacheRoom(R->getDestRoomID());
				delete roomCore;
			}
		}
		break;
		case RESPONSE_IGNORESTATUS:
		{
			ResIgnoreStatus *R = static_cast<ResIgnoreStatus *>(res);
			ChatAvatar *avatar = getAvatar(R->getAvatarID());
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_IGNORESTATUS: avatar=%p\n", avatar);
			}
			m_api->OnIgnoreStatus(R->getTrack(), R->getResult(), avatar, R->getListLength(), R->getIgnoreList(), R->getUser());
		}
		break;
		case RESPONSE_FAILOVER_RELOGINAVATAR:
		{
			ResFailoverReloginAvatar *R = static_cast<ResFailoverReloginAvatar *>(res);

			unsigned result = R->getResult();
			if (result != 0 && result != CHATRESULT_DUPLICATELOGIN)
			{
				ChatAvatar *avatar = getAvatar(R->getAvatarID());
				if (avatar)
				{
					// if we can't relogin the avatar for reason other than
					// duplicate login from this API, then effectively
					// our API must consider him logged out because he's now
					// no longer logged in to his AID controller.
					m_api->OnLogoutAvatar(0, 0, avatar, nullptr);
				}
			}

			m_failoverAvatarResRemain--;

			if (!m_failoverAvatarResRemain)
			{
				failoverRecreateRooms();
			}
		}
		break;
		case RESPONSE_FAILOVER_RECREATEROOM:
		{
			ResFailoverRecreateRoom *R = static_cast<ResFailoverRecreateRoom *>(res);

			if (R->getResult() == 0)
			{
				ChatRoomCore *rcvdRoom = R->getRoom();

				if (rcvdRoom)
				{
					ChatRoomCore *roomCore = getRoomCore(R->getRoomID());

					if (!roomCore)
					{
						// probably shouldn't ever get to this point, but protection
						cacheRoom(rcvdRoom);
					}
					else
					{
						// MAKE APPROPRIATE CHANGES to the cached roomCore object

						// creator values should NEVER change

						// CHECK room values (attribs, topic, maxsize, id)
						//   (name, address, createtime should NEVER change)
						if (roomCore->getRoomID() != rcvdRoom->getRoomID())
						{
							roomCore->setID(rcvdRoom->getRoomID());
						}
						if (roomCore->getRoomAttributes() != rcvdRoom->getRoomAttributes())
						{
							roomCore->setAttributes(rcvdRoom->getRoomAttributes());
						}
						if (roomCore->getRoomTopic() != rcvdRoom->getRoomTopic())
						{
							roomCore->setTopic(rcvdRoom->getRoomTopic());
						}
						if (roomCore->getMaxRoomSize() != rcvdRoom->getMaxRoomSize())
						{
							roomCore->setMaxRoomSize(rcvdRoom->getMaxRoomSize());
						}
					}
				}
			}
			else
			{
				// destroy room from cache and send OnDestroyRoom() callback
				ChatRoomCore *destRoomCore = getRoomCore(R->getRoomID());

				if (destRoomCore)
				{
					Plat_Unicode::String pus1(narrowToWide("ChatSystem"));
					Plat_Unicode::String pus2(narrowToWide(""));
					ChatUnicodeString cus1(pus1.data(), pus1.size());
					ChatUnicodeString cus2(pus2.data(), pus2.size());

					ChatAvatar dummySrcAvatar(0, 0, cus1, cus1, cus2, cus2, 0, 0);

					ChatRoom *destRoom = getRoom(R->getRoomID());

					if (!destRoom && R->getResult() == CHATRESULT_SUCCESS)
					{
						R->setResult(CHATRESULT_SUCCESSBADDATA);
						_chatdebug_("ChatAPI:BadData: RESPONSE_FAILOVER_RECREATEROOM: destRoom=%p\n", destRoom);
					}
					m_api->OnReceiveDestroyRoom(&dummySrcAvatar, destRoom);

					destRoomCore = decacheRoom(R->getRoomID());
					delete destRoomCore;
				}
			}

			if (R->wasFailoverForcedByServer() == false)
			{
				m_failoverRoomResRemain--;

				if (!m_failoverRoomResRemain)
				{
					// resume processing and indicate it's ok to send new requests.
					m_inFailoverMode = false;
					m_api->OnFailoverComplete();
					resumeProcessing();
				}
			}
		}
		break;
		case RESPONSE_GETAVATARKEYWORDS:
		{
			ResGetAvatarKeywords *R = static_cast<ResGetAvatarKeywords *>(res);
			ChatAvatar *avatar = nullptr;

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_GETAVATARKEYWORDS: avatar=%p\n", avatar);
			}
			m_api->OnGetAvatarKeywords(R->getTrack(), R->getResult(), avatar, R->getList(), R->getListLength(), R->getUser());
		}
		break;

		case RESPONSE_SETAVATARKEYWORDS:
		{
			ResSetAvatarKeywords *R = static_cast<ResSetAvatarKeywords *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_SETAVATARKEYWORDS: avatar=%p\n", avatar);
			}
			m_api->OnSetAvatarKeywords(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_SEARCHAVATARKEYWORDS:
		{
			ResSearchAvatarKeywords *R = static_cast<ResSearchAvatarKeywords *>(res);
			ChatAvatar **avatarMatches = nullptr;
			ChatAvatarCore **avatarCoreMatches = nullptr;

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatarCoreMatches = R->getMatches();
				avatarMatches = new ChatAvatar*[R->getNumMatches()];
				for (unsigned i(0); i < R->getNumMatches(); i++)
				{
					avatarMatches[i] = avatarCoreMatches[i]->getNewChatAvatar();
				}
			}
			m_api->OnSearchAvatarKeywords(R->getTrack(), R->getResult(), avatarMatches, R->getNumMatches(), R->getUser());

			delete[] avatarMatches;
		}
		break;
		case RESPONSE_CONFIRMFRIEND:
		{
			ResFriendConfirm *R = static_cast<ResFriendConfirm *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_CONFIRMFRIEND: avatar=%p\n", avatar);
			}
			m_api->OnFriendConfirm(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_CONFIRMFRIEND_RECIPROCATE:
		{
			ResFriendConfirmReciprocate *R = static_cast<ResFriendConfirmReciprocate *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_CONFIRMFRIEND_RECIPROCATE: avatar=%p\n", avatar);
			}
			m_api->OnFriendConfirmReciprocate(R->getTrack(), R->getResult(), avatar, R->getUser());
		}
		break;
		case RESPONSE_REGISTRAR_GETCHATSERVER:
		{
			ResRegistrarGetChatServer *R = static_cast<ResRegistrarGetChatServer *>(res);

			m_rcvdRegistrarResponse = true;

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				m_assignedServerHost = wideToNarrow(R->getHostname()).c_str();
				m_assignedServerPort = (short)R->getPort();
			}
		}
		break;
		case RESPONSE_SETAPIVERSION:
		{
			ResSendApiVersion *R = static_cast<ResSendApiVersion *>(res);

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				m_sentVersion = true;

				if (m_inFailoverMode)
				{
					// reconnect to server succeeded, proceed with failover procedures.
					// The failover procs will make the resumeProcessing() call.
					m_api->OnFailoverBegin();
					failoverReloginAvatars();
				}
				else
				{
					// first time connecting, allow user requests to go through
					resumeProcessing();
					m_api->OnConnect();
				}
			}
			else if (R->getResult() == CHATRESULT_TIMEOUT)
			{
				// force resubmit request
				if (m_connected)
				{
					OnConnect(m_assignedServerHost.c_str(), m_assignedServerPort);
				}
			}
			else
			{
				// server is wrong version, so we can't work with it.
				if (R->getResult() == CHATRESULT_WRONGCHATSERVERFORREQUEST)
				{
					fprintf(stderr, "The client API is a higher version than the server version.\n");
					assert(true);
				}
			}
		}
		break;
		case RESPONSE_ADDSNOOPAVATAR:
		{
			ResAddSnoopAvatar *R = static_cast<ResAddSnoopAvatar *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getSrcAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDSNOOPAVATAR: avatar=%p\n", avatar);
			}
			m_api->OnBeginSnoopingAvatar(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getDestAvatarName()), ChatUnicodeString(R->getDestAvatarAddress()), R->getUser());
		}
		break;
		case RESPONSE_REMOVESNOOPAVATAR:
		{
			ResRemoveSnoopAvatar *R = static_cast<ResRemoveSnoopAvatar *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getSrcAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVESNOOPAVATAR: avatar=%p\n", avatar);
			}
			m_api->OnStopSnoopingAvatar(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getDestAvatarName()), ChatUnicodeString(R->getDestAvatarAddress()), R->getUser());
		}
		break;
		case RESPONSE_ADDSNOOPROOM:
		{
			ResAddSnoopRoom *R = static_cast<ResAddSnoopRoom *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getSrcAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_ADDSNOOPROOM: avatar=%p\n", avatar);
			}
			m_api->OnBeginSnoopingRoom(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getRoomAddress()), R->getUser());
		}
		break;
		case RESPONSE_REMOVESNOOPROOM:
		{
			ResRemoveSnoopRoom *R = static_cast<ResRemoveSnoopRoom *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getSrcAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVESNOOPROOM: avatar=%p\n", avatar);
			}
			m_api->OnStopSnoopingRoom(R->getTrack(), R->getResult(), avatar, ChatUnicodeString(R->getRoomAddress()), R->getUser());
		}
		break;
		case RESPONSE_GETSNOOPLIST:
		{
			ResGetSnoopList *R = static_cast<ResGetSnoopList *>(res);
			ChatAvatar *avatar = nullptr;
			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatar = getAvatar(R->getSrcAvatarID());
			}
			if (!avatar && R->getResult() == CHATRESULT_SUCCESS)
			{
				R->setResult(CHATRESULT_SUCCESSBADDATA);
				_chatdebug_("ChatAPI:BadData: RESPONSE_REMOVESNOOPROOM: avatar=%p\n", avatar);
			}

			AvatarSnoopPair **avatarList = nullptr;
			ChatUnicodeString **roomList = nullptr;

			unsigned numAvatars = R->getAvatarSnoopListLength();
			unsigned numRooms = R->getRoomSnoopListLength();

			if (R->getResult() == CHATRESULT_SUCCESS)
			{
				avatarList = R->getAvatarSnoopList();
				roomList = R->getRoomSnoopList();
			}

			m_api->OnGetSnoopList(R->getTrack(), R->getResult(), avatar, numAvatars, avatarList, numRooms, roomList, R->getUser());
		}
		break;
		case RESPONSE_TRANSFERAVATAR:
		{
			ResTransferAvatar *R = static_cast<ResTransferAvatar *>(res);

			m_api->OnTransferAvatar(R->getTrack(), R->getResult(), R->getUserID(), R->getNewUserID(), R->getAvatarName(), R->getNewAvatarName(), R->getAvatarAddress(), R->getNewAvatarAddress(), R->getUser());
		}
		break;

		default:
			fprintf(stderr, "[ChatAPICore.cpp] default (response) responseCallback(GenericResponse *), type = %u\n", res->getType());
		}
	}

	void ChatAPICore::responseCallback(short type, ByteStream::ReadIterator &iter)
	{
		switch (type)
		{
		case MESSAGE_INSTANTMESSAGE:
		{
			MInstantMessage M(iter);
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());
			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_INSTANTMESSAGE: M.getSrcAvatar()=%p\n", M.getSrcAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			if (!destAvatar || !srcAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_INSTANTMESSAGE: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveInstantMessage(srcAvatar, destAvatar, ChatUnicodeString(M.getMsg().data(), M.getMsg().size()), ChatUnicodeString(M.getOOB().data(), M.getOOB().size()));
			delete srcAvatar;
		}
		break;
		case MESSAGE_ROOMMESSAGE:
		{
			MRoomMessage M(iter);
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!M.getSrcAvatar() || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ROOMMESSAGE: M.getSrcAvatar()=%p, destRoom=%p\n", M.getSrcAvatar(), destRoom);
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			if (!srcAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ROOMMESSAGE: srcAvatar=%p\n", srcAvatar);
				break;
			}
			for (unsigned i = 0; i < M.getListLength(); i++)
			{
				ChatAvatar *destAvatar = getAvatar(M.getDestList()[i]);
				if (!destAvatar)
				{
					_chatdebug_("ChatAPI:BadData: MESSAGE_ROOMMESSAGE: destAvatar[%i]=nullptr\n", i);
					continue;
				}
				m_api->OnReceiveRoomMessage(srcAvatar, destAvatar, destRoom, ChatUnicodeString(M.getMsg().data(), M.getMsg().size()), ChatUnicodeString(M.getOOB().data(), M.getOOB().size()));
				m_api->OnReceiveRoomMessage(srcAvatar, destAvatar, destRoom, ChatUnicodeString(M.getMsg().data(), M.getMsg().size()), ChatUnicodeString(M.getOOB().data(), M.getOOB().size()), M.getMessageID());
			}

			if (destRoom && (M.getMessageID() > 0))
			{
				destRoom->setRoomMessageID(M.getMessageID());
			}

			delete srcAvatar;
		}
		break;
		case MESSAGE_BROADCASTMESSAGE:
		{
			MBroadcastMessage M(iter);
			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_BROADCASTMESSAGE: M.getSrcAvatar()=%p\n", M.getSrcAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			if (!srcAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_BROADCASTMESSAGE: srcAvatar=%p\n", srcAvatar);
				break;
			}
			for (unsigned i = 0; i < M.getListLength(); i++)
			{
				ChatAvatar *destAvatar = getAvatar(M.getDestList()[i]);
				if (!destAvatar)
				{
					_chatdebug_("ChatAPI:BadData: MESSAGE_BROADCASTMESSAGE: destAvatar[%i]=nullptr\n", i);
					continue;
				}
				m_api->OnReceiveBroadcastMessage(srcAvatar, ChatUnicodeString(M.getSrcAddress().data(), M.getSrcAddress().size()), destAvatar, ChatUnicodeString(M.getMsg().data(), M.getMsg().size()), ChatUnicodeString(M.getOOB().data(), M.getOOB().size()));
			}
			delete srcAvatar;
		}
		break;
		case MESSAGE_FILTERMESSAGE:
		{
			MFilterMessage M(iter);
			m_api->OnReceiveFilterMessage(ChatUnicodeString(M.getMsg().data(), M.getMsg().size()));
		}
		break;
		case MESSAGE_FRIENDLOGIN:
		{
			MFriendLogin M(iter);
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());
			if (!M.getFriendAvatar() || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDLOGIN: M.getFriendAvatar()=%p, destAvatar=%p\n", M.getFriendAvatar(), destAvatar);
				break;
			}
			ChatAvatar *friendAvatar = M.getFriendAvatar()->getNewChatAvatar();
			if (!friendAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDLOGIN: friendAvatar=%p\n", friendAvatar);
				break;
			}

			friendAvatar->setStatusMessage(M.getFriendStatusMessage());
			m_api->OnReceiveFriendLogin(friendAvatar, ChatUnicodeString(M.getFriendAddress().data(), M.getFriendAddress().size()), destAvatar);
			delete friendAvatar;
		}
		break;
		case MESSAGE_FRIENDLOGOUT:
		{
			MFriendLogout M(iter);
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());
			if (!M.getFriendAvatar() || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDLOGOUT: M.getFriendAvatar()=%p, destAvatar=%p\n", M.getFriendAvatar(), destAvatar);
				break;
			}
			ChatAvatar *friendAvatar = M.getFriendAvatar()->getNewChatAvatar();
			if (!friendAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDLOGOUT: friendAvatar=%p\n", friendAvatar);
				break;
			}
			m_api->OnReceiveFriendLogout(friendAvatar, ChatUnicodeString(M.getFriendAddress().data(), M.getFriendAddress().size()), destAvatar);
			delete friendAvatar;
		}
		break;
		case MESSAGE_FRIENDSTATUS:
		{
			MFriendStatus M(iter);
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());
			if (!M.getFriendAvatar() || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDSTATUS: M.getFriendAvatar()=%p, destAvatar=%p\n", M.getFriendAvatar(), destAvatar);
				break;
			}
			ChatAvatar *friendAvatar = M.getFriendAvatar()->getNewChatAvatar();
			if (!friendAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDSTATUS: friendAvatar=%p\n", friendAvatar);
				break;
			}
			friendAvatar->setStatusMessage(M.getFriendStatusMessage());
			m_api->OnReceiveFriendStatusChange(friendAvatar, ChatUnicodeString(M.getFriendAddress().data(), M.getFriendAddress().size()), destAvatar);
			delete friendAvatar;
		}
		break;
		case MESSAGE_KICKROOM:
		{
			MKickRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());

			if (!destRoomCore) {
				_chatdebug_("ChatAPI:destroomCore is nullptr!");
				break;
			}

			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_KICKROOM: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}

			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatarCore *kickedAvatar = nullptr;

			if (destAvatar != nullptr) {
				kickedAvatar = destRoomCore ? destRoomCore->removeAvatar(destAvatar->getAvatarID()) : nullptr;
			}

			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!srcAvatar || !destAvatar || !kickedAvatar || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_KICKROOM: srcAvatar=%p, destAvatar=%p, kickedAvatar=%p, destRoom=%p\n", srcAvatar, destAvatar, kickedAvatar, destRoom);
				delete srcAvatar;
				delete destAvatar;
				delete kickedAvatar;
				break;
			}

			m_api->OnReceiveKickRoom(srcAvatar, destAvatar, destRoom);
			delete kickedAvatar;
			delete destAvatar;
			delete srcAvatar;
		}
		break;
		case MESSAGE_ADDMODERATORROOM:
		{
			MAddModeratorRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDMODERATORROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				delete M.getDestAvatar();
				break;
			}

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			ChatAvatarCore *destAvatarCore = M.getDestAvatar();
			if (!srcAvatar || !destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDMODERATORROOM: srcAvatar=%p, destAvatarCore=%p\n", srcAvatar, destAvatarCore);
				delete M.getDestAvatar();
				break;
			}
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();

			if (!destRoomCore->addModerator(destAvatarCore))
			{
				delete M.getDestAvatar();
			}

			if (!destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDMODERATORROOM: destAvatar=%p\n", destAvatar);
				break;
			}
			m_api->OnReceiveAddModeratorRoom(srcAvatar, destAvatar, destRoom);
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVEMODERATORROOM:
		{
			MRemoveModeratorRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEMODERATORROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			ChatAvatarCore *destAvatarCore = destRoomCore->removeModerator(M.getDestAvatarName(), M.getDestAvatarAddress());

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			if (!destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEMODERATORROOM: destAvatarCore=%p\n", destAvatarCore);
				delete destAvatarCore;
				break;
			}
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();
			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEMODERATORROOM: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveModeratorRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatarCore;
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVEMODERATORAVATAR:
		{
			MRemoveModeratorAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEMODERATORAVATAR: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEMODERATORAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveModeratorAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDTEMPORARYMODERATORROOM:
		{
			MAddTemporaryModeratorRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDTEMPORARYMODERATORROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				delete M.getDestAvatar();
				break;
			}

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			ChatAvatarCore *destAvatarCore = M.getDestAvatar();
			if (!srcAvatar || !destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDTEMPORARYMODERATORROOM: srcAvatar=%p, destAvatarCore=%p\n", srcAvatar, destAvatarCore);
				delete M.getDestAvatar();
				break;
			}
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();

			if (!destRoomCore->addTempModerator(destAvatarCore))
			{
				delete M.getDestAvatar();
			}

			if (!destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDTEMPORARYMODERATORROOM: destAvatar=%p\n", destAvatar);
				break;
			}
			m_api->OnReceiveAddTemporaryModeratorRoom(srcAvatar, destAvatar, destRoom);
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVETEMPORARYMODERATORROOM:
		{
			MRemoveTemporaryModeratorRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVETEMPORARYMODERATORROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			ChatAvatarCore *destAvatarCore = destRoomCore->removeTempModerator(M.getDestAvatarName(), M.getDestAvatarAddress());

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			if (!destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVETEMPORARYMODERATORROOM: destAvatarCore=%p\n", destAvatarCore);
				delete destAvatarCore;
				break;
			}
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();
			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVETEMPORARYMODERATORROOM: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveTemporaryModeratorRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatarCore;
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVETEMPORARYMODERATORAVATAR:
		{
			MRemoveTemporaryModeratorAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVETEMPORARYMODERATORAVATAR: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVETEMPORARYMODERATORAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveTemporaryModeratorAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDBANROOM:
		{
			MAddBanRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());

			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDBANAVATAR: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			if (!M.getDestAvatar() || !M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDBANAVATAR: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				delete M.getSrcAvatar();
				break;
			}
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();

			if (destRoomCore && (!destRoomCore->addBan(M.getDestAvatar())))
			{
				delete M.getSrcAvatar();
			}

			if (destRoomCore && (!destRoomCore || !destRoom))
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDBANAVATAR: destRoom=%p, destRoomCore=%p\n", destRoom, destRoomCore);
				delete srcAvatar;
				delete destAvatar;
				break;
			}

			ChatAvatarCore *removedAvatar = destRoomCore->removeAvatar(destAvatar->getAvatarID());
			delete removedAvatar;

			m_api->OnReceiveAddBanRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatar;
			delete srcAvatar;
		}
		break;
		case MESSAGE_REMOVEBANROOM:
		{
			MRemoveBanRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());

			if (!destRoom || !destRoomCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEBANROOM: destRoom=%p, destRoomCore=%p\n", destRoom, destRoomCore);
				break;
			}
			ChatAvatarCore *destAvatarCore = destRoomCore->removeBan(M.getDestAvatarName(), M.getDestAvatarAddress());
			if (!destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEBANROOM: destAvatarCore=%p\n", destAvatarCore);
				break;
			}

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();
			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEBANROOM: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete destAvatar;
				break;
			}

			m_api->OnReceiveRemoveBanRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatarCore;
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVEBANAVATAR:
		{
			MRemoveBanAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEBANAVATAR: M.getSrcAvatar=()%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEBANAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveBanAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDINVITEROOM:
		{
			MAddInviteRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());

			if (!destRoomCore || !M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDINVITEROOM: destRoomCore=%p, M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", destRoomCore, M.getSrcAvatar(), M.getDestAvatar());
				delete M.getDestAvatar();
				break;
			}

			if (!destRoomCore->addInvite(M.getDestAvatar()))
			{
				delete M.getDestAvatar();
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			ChatRoom *destRoom = getRoom(M.getRoomID());

			if (!srcAvatar || !destAvatar || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDINVITEROOM: srcAvatar=%p, destAvatar=%p, destRoom=%p\n", srcAvatar, destAvatar, destRoom);
				delete srcAvatar;
				delete destAvatar;
				break;
			}

			m_api->OnReceiveAddInviteRoom(srcAvatar, destAvatar, destRoom);
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDINVITEAVATAR:
		{
			MAddInviteAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDINVITEAVATAR: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDINVITEAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveAddInviteAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVEINVITEROOM:
		{
			MRemoveInviteRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEINVITEROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			ChatAvatarCore *destAvatarCore = destRoomCore->removeInvite(M.getDestAvatarName(), M.getDestAvatarAddress());
			if (!destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEINVITEROOM: destAvatarCore=%p\n", destAvatarCore);
				break;
			}

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEINVITEROOM: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete destAvatarCore;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveInviteRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatarCore;
			delete destAvatar;
		}
		break;
		case MESSAGE_REMOVEINVITEAVATAR:
		{
			MRemoveInviteAvatar M(iter);

			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEINVITEAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REMOVEINVITEAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRemoveInviteAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_GRANTVOICEROOM:
		{
			MGrantVoiceRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());

			if (!destRoomCore || !M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_GRANTVOICEROOM: destRoomCore=%p, M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", destRoomCore, M.getSrcAvatar(), M.getDestAvatar());
				delete M.getDestAvatar();
				break;
			}

			if (!destRoomCore->addVoice(M.getDestAvatar()))
			{
				delete M.getDestAvatar();
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();
			ChatRoom *destRoom = getRoom(M.getRoomID());

			if (!srcAvatar || !destAvatar || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_GRANTVOICEROOM: srcAvatar=%p, destAvatar=%p, destRoom=%p\n", srcAvatar, destAvatar, destRoom);
				delete srcAvatar;
				delete destAvatar;
				break;
			}

			m_api->OnReceiveGrantVoiceRoom(srcAvatar, destAvatar, destRoom);
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_GRANTVOICEAVATAR:
		{
			MGrantVoiceAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_GRANTVOICEAVATAR: M.getSrcAvatar()=%p, M.getDestAvatar()=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_GRANTVOICEAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveGrantVoiceAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_REVOKEVOICEROOM:
		{
			MRevokeVoiceRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REVOKEVOICEROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			ChatAvatarCore *destAvatarCore = destRoomCore->removeVoice(M.getDestAvatarName(), M.getDestAvatarAddress());
			if (!destAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REVOKEVOICEROOM: destAvatarCore=%p\n", destAvatarCore);
				break;
			}

			const ChatAvatar *srcAvatar = destRoomCore->getAvatar(M.getSrcAvatarID());
			ChatAvatar *destAvatar = destAvatarCore->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REVOKEVOICEROOM: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete destAvatarCore;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRevokeVoiceRoom(srcAvatar, destAvatar, destRoom);

			delete destAvatarCore;
			delete destAvatar;
		}
		break;
		case MESSAGE_REVOKEVOICEAVATAR:
		{
			MRevokeVoiceAvatar M(iter);

			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REVOKEVOICEAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REVOKEVOICEAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveRevokeVoiceAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ENTERROOM:
		{
			MEnterRoom M(iter);

			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());

			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom || !M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ENTERROOM: destRoomCore=%p, destRoom=%p, M.getSrcAvatar=%p\n", destRoomCore, destRoom, M.getSrcAvatar());
				delete M.getSrcAvatar();
				break;
			}

			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			if (!srcAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ENTERROOM: srcAvatar=nullptr\n");
				delete M.getSrcAvatar();
				break;
			}

			bool isLocalAvatar = (getAvatar(srcAvatar->getAvatarID()) != nullptr);
			if (!destRoomCore->addAvatar(M.getSrcAvatar(), isLocalAvatar))
			{
				delete M.getSrcAvatar();
			}

			m_api->OnReceiveEnterRoom(srcAvatar, destRoom);
			delete srcAvatar;
		}
		break;
		case MESSAGE_LEAVEROOM:
		{
			MLeaveRoom M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatRoom *destRoom = getRoom(M.getRoomID());
			if (!destRoomCore || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_LEAVEROOM: destRoomCore=%p, destRoom=%p\n", destRoomCore, destRoom);
				break;
			}

			ChatAvatarCore *srcAvatarCore = destRoomCore->removeAvatar(M.getAvatarID());
			if (!srcAvatarCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_LEAVEROOM: srcAvatarCore=%p\n", srcAvatarCore);
				break;
			}
			ChatAvatar *srcAvatar = srcAvatarCore->getNewChatAvatar();
			if (!srcAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_LEAVEROOM: srcAvatar=%p\n", srcAvatar);
				break;
			}

			m_api->OnReceiveLeaveRoom(srcAvatar, destRoom);
			delete srcAvatarCore;
			delete srcAvatar;
		}
		break;
		case MESSAGE_DESTROYROOM:
		{
			MDestroyRoom M(iter);

			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());

			if (destRoomCore)
			{
				ChatRoom *destRoom = getRoom(M.getRoomID());
				ChatAvatar *srcAvatar = nullptr;
				if (!M.getSrcAvatar())
				{
					_chatdebug_("ChatAPI:BadData: MESSAGE_DESTROYROOM: M.getSrcAvatar=nullptr\n");
				}
				else
				{
					srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
				}

				if (destRoom)
				{
					m_api->OnReceiveDestroyRoom(srcAvatar, destRoom);
				}
				else
				{
					_chatdebug_("ChatAPI:BadData: MESSAGE_DESTROYROOM: srcAvatar=%p, destRoom=%p\n", srcAvatar, destRoom);
				}
				delete srcAvatar;

				ChatRoomCore *destRoomCore = decacheRoom(M.getRoomID());
				delete destRoomCore;
			}
			else
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_DESTROYROOM: destRoomCore=%p\n", destRoomCore);
			}
		}
		break;
		case MESSAGE_SETROOMPARAMS:
		{
			MSetRoomParams M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			if (!destRoomCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_SETROOMPARAMS: destRoomCore=nullptr\n");
				break;
			}

			// build RoomParams object for OnReceiveRoomParams call
			RoomParams oldParams(ChatUnicodeString(destRoomCore->getRoomName().data(), destRoomCore->getRoomName().size()), ChatUnicodeString(destRoomCore->getRoomTopic().data(), destRoomCore->getRoomTopic().size()), ChatUnicodeString(destRoomCore->getRoomPassword().data(), destRoomCore->getRoomPassword().size()), destRoomCore->getRoomAttributes(), destRoomCore->getMaxRoomSize());
			RoomParams params(ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomTopic().data(), M.getRoomTopic().size()), ChatUnicodeString(M.getRoomPassword().data(), M.getRoomPassword().size()), M.getRoomAttributes(), M.getRoomSize());

			// update cached room
			destRoomCore->setAttributes(M.getRoomAttributes());
			destRoomCore->setMaxRoomSize(M.getRoomSize());
			destRoomCore->setName(M.getRoomName());
			destRoomCore->setTopic(M.getRoomTopic());
			destRoomCore->setPassword(M.getRoomPassword());

			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_SETROOMPARAMS: M.getSrcAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatRoom *destRoom = getRoom(M.getRoomID());

			if (!srcAvatar || !destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_SETROOMPARAMS: srcAvatar=%p, destRoom=%p\n", srcAvatar, destRoom);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveRoomParams(srcAvatar, destRoom, &params, &oldParams);
			delete srcAvatar;
		}
		break;
		case MESSAGE_PERSISTENTMESSAGE:
		{
			MPersistentMessage M(iter);
			ChatAvatar *avatar = getAvatar(M.getDestAvatarID());
			if (avatar)
				m_api->OnReceivePersistentMessage(avatar, M.getHeader());
		}
		break;
		case MESSAGE_FORCEDLOGOUT:
		{
			MForcedLogout M(iter);
			ChatAvatar *avatar = getAvatar(M.getAvatarID());
			if (avatar)
				m_api->OnReceiveForcedLogout(avatar);
			ChatAvatarCore *core = decacheAvatar(M.getAvatarID());
			delete core;
		}
		break;
		case MESSAGE_UNREGISTERROOMREADY:
		{
			MUnregisterRoomReady M(iter);

			ChatRoom *room = getRoom(M.getRoomID());

			if (room)
			{
				m_api->OnReceiveUnregisterRoomReady(room);
			}
		}
		break;
		case MESSAGE_KICKAVATAR:
		{
			MKickAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_KICKAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_KICKAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveKickAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDMODERATORAVATAR:
		{
			MAddModeratorAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDMODERATORAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDMODERATORAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveAddModeratorAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDTEMPORARYMODERATORAVATAR:
		{
			MAddTemporaryModeratorAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDTEMPORARYMODERATORAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDTEMPORARYMODERATORAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveAddTemporaryModeratorAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDBANAVATAR:
		{
			MAddBanAvatar M(iter);
			if (!M.getSrcAvatar() || !M.getDestAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDBANAVATAR: M.getSrcAvatar=%p, M.getDestAvatar=%p\n", M.getSrcAvatar(), M.getDestAvatar());
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = M.getDestAvatar()->getNewChatAvatar();

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_ADDBANAVATAR: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				delete destAvatar;
				break;
			}
			m_api->OnReceiveAddBanAvatar(srcAvatar, destAvatar, ChatUnicodeString(M.getRoomName().data(), M.getRoomName().size()), ChatUnicodeString(M.getRoomAddress().data(), M.getRoomAddress().size()));
			delete srcAvatar;
			delete destAvatar;
		}
		break;
		case MESSAGE_ADDADMIN:
		{
			MAddAdmin M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			if (destRoomCore && (destRoomCore->addAdministrator(M.getAvatar()) == nullptr))
			{
				delete (M.getAvatar());
			}
		}
		break;
		case MESSAGE_REMOVEADMIN:
		{
			MRemoveAdmin M(iter);
			ChatRoomCore *destRoomCore = getRoomCore(M.getRoomID());
			ChatAvatarCore *admin = nullptr;
			if (destRoomCore)
				admin = destRoomCore->removeAdministrator(M.getAvatarID());
			delete admin;
		}
		break;
		case MESSAGE_FRIENDCONFIRMREQUEST:
		{
			MFriendConfirmRequest M(iter);
			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMREQUEST: M.getSrcAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMREQUEST: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveFriendConfirmRequest(srcAvatar, destAvatar);

			delete srcAvatar;
		}
		break;
		case MESSAGE_FRIENDCONFIRMRESPONSE:
		{
			MFriendConfirmResponse M(iter);

			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRESPONSE: M.getSrcAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRESPONSE: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveFriendConfirmResponse(srcAvatar, destAvatar, M.getConfirm());

			delete srcAvatar;
		}
		break;
		case MESSAGE_FRIENDCONFIRMRECIPROCATE_REQUEST:
		{
			MFriendConfirmReciprocateRequest M(iter);
			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRECIPROCATE_REQUEST: M.getSrcAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRECIPROCATE_REQUEST: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveFriendConfirmReciprocateRequest(srcAvatar, destAvatar);

			delete srcAvatar;
		}
		break;
		case MESSAGE_FRIENDCONFIRMRECIPROCATE_RESPONSE:
		{
			MFriendConfirmReciprocateResponse M(iter);

			if (!M.getSrcAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRECIPROCATE_RESPONSE: M.getSrcAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getSrcAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = getAvatar(M.getDestAvatarID());

			if (!srcAvatar || !destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_FRIENDCONFIRMRECIPROCATE_RESPONSE: srcAvatar=%p, destAvatar=%p\n", srcAvatar, destAvatar);
				delete srcAvatar;
				break;
			}
			m_api->OnReceiveFriendConfirmReciprocateResponse(srcAvatar, destAvatar, M.getConfirm());

			delete srcAvatar;
		}
		break;
		case MESSAGE_CHANGEROOMOWNER:
		{
			MChangeRoomOwner M(iter);

			ChatRoomCore *destRoomCore = getRoomCore(M.getDestRoomID());
			if (!destRoomCore)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_CHANGEROOMOWNER: destRoomCore=nullptr\n");
				break;
			}

			// update cached room
			destRoomCore->setCreator(M.getNewRoomOwnerID(), M.getNewRoomOwnerName(), M.getNewRoomOwnerAddress());
			ChatRoom *destRoom = getRoom(M.getDestRoomID());

			if (!destRoom)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_CHANGEROOMOWNER: destRoom=%p\n", destRoom);
				break;
			}

			m_api->OnReceiveRoomOwnerChange(destRoom);
		}
		break;
		case MESSAGE_REQUESTROOMENTRY:
		{
			MRoomEntryRequest M(iter);

			if (!M.getRequestorAvatar())
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REQUESTROOMENTRY: M.getRequestorAvatar=nullptr\n");
				break;
			}
			ChatAvatar *srcAvatar = M.getRequestorAvatar()->getNewChatAvatar();
			ChatAvatar *destAvatar = getAvatar(M.getRoomOwnerID());

			if (!destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_REQUESTROOMENTRY: destAvatar=%p\n", destAvatar);
				delete srcAvatar;
				break;
			}

			m_api->OnReceiveRoomEntryRequest(srcAvatar, destAvatar, M.getRequestedRoomAddress());

			delete srcAvatar;
		}
		break;
		case MESSAGE_DELAYEDROOMENTRY:
		{
			MDelayedRoomEntry M(iter);

			ChatAvatar *avatar = getAvatar(M.getAvatarID());
			ChatRoom *room = getRoom(M.getRoomID());

			const set<ChatRoomCore *> &extraRooms = M.getExtraRooms();
			set<ChatRoomCore *>::const_iterator iter = extraRooms.begin();
			for (; iter != extraRooms.end(); ++iter)
			{
				if (room)
				{
					// we don't need to cache the rooms, so delete them
					delete (*iter);
				}
				else
				{
					cacheRoom(*iter);
				}
			}

			room = getRoom(M.getRoomID());

			if (!avatar || !room)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_DELAYEDROOMENTRY: avatar=%p, room=%p\n", avatar, room);
				break;
			}

			m_api->OnReceiveDelayedRoomEntry(avatar, room);
		}
		break;
		case MESSAGE_DENIEDROOMENTRY:
		{
			MDeniedRoomEntry M(iter);

			ChatAvatar *destAvatar = getAvatar(M.getRequestorID());

			if (!destAvatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_DENIEDROOMENTRY: destAvatar=%p\n", destAvatar);
				break;
			}

			m_api->OnReceiveDeniedRoomEntry(destAvatar, M.getRequestedRoomAddress());
		}
		break;
		case MESSAGE_FORCEROOMFAILOVER:
		{
			MForceRoomFailover M(iter);

			// get list of AIDs for which we need to send room failover requests
			const list<String> &aidList = M.getAIDList();
			if (aidList.size() > 0)
			{
				// iterate through rooms, checking if we must fail them over
				map<unsigned, ChatRoomCore *>::const_iterator roomIter = m_roomCoreCache.begin();
				for (; roomIter != m_roomCoreCache.end(); ++roomIter)
				{
					ChatRoomCore *room = (*roomIter).second;
					const String &roomAddress = room->getAddress();

					// compare room address against aid string, it is a match
					// if aid string is found at index 0 (start of roomAddr string)
					String::size_type index;
					list<String>::const_iterator aidIter = aidList.begin();
					for (; aidIter != aidList.end(); ++aidIter)
					{
						index = roomAddress.find(*aidIter);
						if (index == 0)
						{
							// we need to failover this room
							RFailoverRecreateRoom *req = new RFailoverRecreateRoom(room);
							ResFailoverRecreateRoom *res = new ResFailoverRecreateRoom(room->getRoomID(), true);
							submitRequest(req, res);
							break;
						}
					}
				}
			}
		}
		break;
		case MESSAGE_FAILOVER_AVATAR_LIST:
		{
			unsigned numWorlds = 0;

			get(iter, numWorlds);

			std::vector<Plat_Unicode::String> worldVec;
			unsigned i = 0;
			for (i = 0; i < numWorlds; ++i)
			{
				Plat_Unicode::String world;
				get(iter, world);
				worldVec.push_back(world);
			}

			map<unsigned, ChatAvatarCore *>::iterator avatarIter;

			if (m_avatarCoreCache.size() > 0)
			{
				for (avatarIter = m_avatarCoreCache.begin(); avatarIter != m_avatarCoreCache.end(); ++avatarIter)
				{
					for (i = 0; i < worldVec.size(); ++i)
					{
						if (caseInsensitiveCompare((*avatarIter).second->getAddress(), worldVec[i]))
						{
							failoverReloginOneAvatar((*avatarIter).second);
							break; // break out of world loop into avatar loop
						}
					}
				}
			}
		}
		break;
		case MESSAGE_SNOOP:
		{
			MSnoop M(iter);

			m_api->OnReceiveSnoopMessage(M.getSnoopType(),
				ChatUnicodeString(M.getSnooperName()),
				ChatUnicodeString(M.getSnooperAddr()),
				ChatUnicodeString(M.getSrcName()),
				ChatUnicodeString(M.getSrcAddr()),
				ChatUnicodeString(M.getDestName()),
				ChatUnicodeString(M.getDestAddr()),
				ChatUnicodeString(M.getMessage()));
		}
		break;
		case MESSAGE_UIDLIST:
		{
			MUIDList M(iter);

			m_uidSet = M.getUIDList();
		}
		break;
		case MESSAGE_NOTIFY_FRIEND_IS_REMOVED:
		{
			MNotifyFriendIsRemoved M(iter);

			ChatAvatar *avatar = getAvatar(M.getAvatarID());

			if (!avatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_NOTIFY_FRIEND_IS_REMOVED: avatar=%p avatarID(%d)\n", avatar, M.getAvatarID());
				break;
			}

			m_api->OnReceiveNotifyFriendIsRemoved(avatar, M.getDestName(), M.getDestAddress());
		}
		break;
		case MESSAGE_NOTIFY_FRIENDS_LIST_CHANGE:
		{
			MNotifyFriendsListChange M(iter);
			ChatAvatar *avatar = getAvatar(M.getAvatarID());

			if (!avatar)
			{
				_chatdebug_("ChatAPI:BadData: MESSAGE_NOTIFY_FRIENDS_LIST_CHANGE: avatar=%p avatarID(%d)\n", avatar, M.getAvatarID());
				break;
			}

			m_api->OnReceiveNotifyFriendsListChange(avatar, M.getOriginalName(), M.getOriginalAddress(), M.getNewName(), M.getNewAddress());
		}
		break;
		default:
			fprintf(stderr, "[ChatAPICore.cpp] default (message) responseCallback(ByteStream::ReadIterator &), type = %u\n", type);
		}
	}

	void ChatAPICore::processAPI()
	{
		// if we are connected to the registrar and we got the
		// ResRegistrarGetChatServer callback, then we disconnect
		// and connect to the assigned ChatServer
		if (m_connected &&
			m_setToRegistrar &&
			m_rcvdRegistrarResponse)
		{
			// disconnect from the Registrar
			m_serverConnections[0]->disconnect();
		}
		// if we've been disconnected for at least a minute...
		else if (!m_connected &&
			time(nullptr) - m_timeSinceLastDisconnect >= 60)
		{
			if (m_setToRegistrar)
			{
				// ...and we can't connect to the registrar, then
				// connect to lastAssigned Chat Server host and port
				// (which on the first attempt would be the same as default)
				changeHostPort(0, m_assignedServerHost.c_str(), m_assignedServerPort);
				m_setToRegistrar = false;
			}
			else
			{
				// ...and we can't connect to the Chat Server, then
				// connect to the registrar and ask again for direction
				changeHostPort(0, m_registrarHost.c_str(), m_registrarPort);
				m_setToRegistrar = true;
			}

			m_timeSinceLastDisconnect = time(nullptr);
		}

		// call GenericAPICore::process()
		process();
	}

	void ChatAPICore::OnConnect(const char *host, short port)
	{
		m_connected = true;

		// act on connection depending on if we are connected to
		// registrar or chatserver
		if (m_setToRegistrar)
		{
			// negotiate chat server host/port with the Registrar
			Base::ByteStream msg;
			RRegistrarGetChatServer *req = new RRegistrarGetChatServer(m_defaultServerHost, m_defaultServerPort);
			ResRegistrarGetChatServer *res = new ResRegistrarGetChatServer();

			if (m_currTrack == 0)
			{
				m_currTrack++;
			}
			req->setTrack(m_currTrack);
			res->setTrack(m_currTrack);
			m_currTrack++;

			time_t timeout = time(nullptr) + m_requestTimeout;
			req->setTimeout(timeout);
			res->setTimeout(timeout);

			m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
			m_pendingCount++;

			// submit request directly (the exception, not using submitRequest())
			req->pack(msg);
			m_serverConnections[0]->Send(msg);
			delete req;

			m_rcvdRegistrarResponse = false;
			m_serverConnections[0]->process();
		}
		else if (!m_sentVersion &&
			m_shouldSendVersion) // always true, except ChatAdminAPICore can override
		{
			// send the version to the chat server
			Base::ByteStream msg;
			RSendApiVersion *req = new RSendApiVersion(kChatApiVersion);
			ResSendApiVersion *res = new ResSendApiVersion();

			if (m_currTrack == 0)
			{
				m_currTrack++;
			}
			req->setTrack(m_currTrack);
			res->setTrack(m_currTrack);
			m_currTrack++;

			time_t timeout = time(nullptr) + m_requestTimeout;
			req->setTimeout(timeout);
			res->setTimeout(timeout);

			m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
			m_pendingCount++;

			// submit request directly (the exception, not using submitRequest())
			req->pack(msg);
			m_serverConnections[0]->Send(msg);
			delete req;

			m_serverConnections[0]->process();
		}
		else if (m_shouldSendVersion == false)
		{
			if (m_inFailoverMode)
			{
				// reconnect to server succeeded, proceed with failover procedures.
				// The failover procs will make the resumeProcessing() call.
				m_api->OnFailoverBegin();
				failoverReloginAvatars();
			}
			else
			{
				// first time connecting, allow user requests to go through
				resumeProcessing();
				m_api->OnConnect();
			}
		}
	}

	void ChatAPICore::OnDisconnect(const char *host, short port)
	{
		// we may get OnDisconnect even when we're already disconnected,
		// so protect against unnecessary OnDisconnects by checking m_connected
		if (m_connected)
		{
			m_connected = false;
			m_sentVersion = false;

			// stop processing immediately
			suspendProcessing();

			m_timeSinceLastDisconnect = time(nullptr);

			// determine who we disconnected from and take appropriate action
			if (strcmp(host, m_registrarHost.c_str()) == 0 &&
				port == m_registrarPort)
			{
				// we finished communication to the registrar
				// change connection to the assigned host and port
				changeHostPort(0, m_assignedServerHost.c_str(), m_assignedServerPort);
				m_setToRegistrar = false;
			}
			else
			{
				// we disconnected from a Chat Server
				// consider ourselves in failover mode, meaning next connection to
				// a Chat Server will induce API failure procedure.  First we will
				// attempt to connect to Registrar again.
				m_inFailoverMode = true;
				m_failoverAvatarResRemain = 0;
				m_failoverRoomResRemain = 0;

				changeHostPort(0, m_registrarHost.c_str(), m_registrarPort);
				m_setToRegistrar = true;

				// indicate disconnect state
				m_api->OnDisconnect();
			}
		}
	}

	void ChatAPICore::failoverReloginAvatars()
	{
		map<unsigned, ChatAvatarCore *>::iterator avatarIter;

		if (m_avatarCoreCache.size() > 0)
		{
			for (avatarIter = m_avatarCoreCache.begin(); avatarIter != m_avatarCoreCache.end(); ++avatarIter)
			{
				// build failover-login request for avatar
				failoverReloginOneAvatar(avatarIter->second);
			}
		}
		else
		{
			// no avatars to relogin, so directly proceed to failover rooms
			failoverRecreateRooms();
		}
	}

	void ChatAPICore::failoverReloginOneAvatar(ChatAvatarCore * avatarCore)
	{
		if (avatarCore)
		{
			// build failover-login request for avatar
			Base::ByteStream msg;
			RFailoverReloginAvatar *req = new RFailoverReloginAvatar(avatarCore);
			ResFailoverReloginAvatar *res = new ResFailoverReloginAvatar(avatarCore->getAvatarID());

			if (m_currTrack == 0)
			{
				m_currTrack++;
			}
			req->setTrack(m_currTrack);
			res->setTrack(m_currTrack);
			m_currTrack++;

			time_t timeout = time(nullptr) + m_requestTimeout;
			req->setTimeout(timeout);
			res->setTimeout(timeout);

			m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
			m_pendingCount++;

			// submit request directly (the exception, not using submitRequest())
			req->pack(msg);
			m_serverConnections[0]->Send(msg);
			delete req;

			m_failoverAvatarResRemain++;
			m_serverConnections[0]->process(false);
		}
	}

	void ChatAPICore::failoverRecreateRooms()
	{
		map<unsigned, ChatRoomCore *>::iterator roomIter;
		multimap<unsigned, ChatRoomCore *>::iterator roomMultiIter;

		if (m_roomCoreCache.size() > 0)
		{
			// first build multimap of rooms keyed by their node level (ascending order is default).
			multimap<unsigned, ChatRoomCore *> levelMap;
			ChatRoomCore *roomCore = nullptr;
			for (roomIter = m_roomCoreCache.begin(); roomIter != m_roomCoreCache.end(); ++roomIter)
			{
				roomCore = (*roomIter).second;
				if (roomCore->getNodeLevel() != 0)
				{
					levelMap.insert(pair<unsigned, ChatRoomCore *>(roomCore->getNodeLevel(), roomCore));
				}
			}

			for (roomMultiIter = levelMap.begin(); roomMultiIter != levelMap.end(); ++roomMultiIter)
			{
				roomCore = (*roomMultiIter).second;

				// build failover-room-create request for room
				Base::ByteStream msg;
				RFailoverRecreateRoom *req = new RFailoverRecreateRoom(roomCore);
				ResFailoverRecreateRoom *res = new ResFailoverRecreateRoom(roomCore->getRoomID());

				if (m_currTrack == 0)
				{
					m_currTrack++;
				}
				req->setTrack(m_currTrack);
				res->setTrack(m_currTrack);
				m_currTrack++;

				time_t timeout = time(nullptr) + m_requestTimeout;
				req->setTimeout(timeout);
				res->setTimeout(timeout);

				m_pending.insert(pair<unsigned, GenericResponse *>(res->getTrack(), res));
				m_pendingCount++;

				// submit request directly (the exception, not using submitRequest())
				req->pack(msg);
				m_serverConnections[0]->Send(msg);
				delete req;

				m_failoverRoomResRemain++;
				m_serverConnections[0]->process(false);
			}
		}
		else
		{
			// no rooms to recreate, so resume processing and
			// indicate it's ok to send new requests.
			m_inFailoverMode = false;
			m_api->OnFailoverComplete();
			resumeProcessing();
		}
	}

	ChatAvatar *ChatAPICore::getAvatar(const ChatUnicodeString &avatarName, const ChatUnicodeString &avatarAddress)
	{
		ChatAvatar *returnVal = nullptr;

		map<unsigned, ChatAvatar *>::iterator iter = m_avatarCache.begin();

		for (; iter != m_avatarCache.end(); ++iter)
		{
			ChatAvatar *avatar = (*iter).second;

			if (areEqualChatStrings(avatarName, avatar->getName()) &&
				areEqualChatStrings(avatarAddress, avatar->getAddress()))
			{
				returnVal = avatar;
				break;
			}
		}

		return returnVal;
	}

	ChatRoom *ChatAPICore::getRoom(const ChatUnicodeString &roomAddress)
	{
		ChatRoom *returnVal = nullptr;

		map<unsigned, ChatRoom *>::iterator iter = m_roomCache.begin();

		for (; iter != m_roomCache.end(); ++iter)
		{
			ChatRoom *room = (*iter).second;

			if (areEqualChatStrings(roomAddress, room->getAddress()))
			{
				returnVal = room;
				break;
			}
		}

		return returnVal;
	}

	bool ChatAPICore::areEqualChatStrings(const ChatUnicodeString &str1, const ChatUnicodeString &str2)
	{
		bool returnVal = true;

		if (str1.string_length != str2.string_length)
		{
			returnVal = false;
		}
		else
		{
			for (unsigned i = 0; i != str1.string_length; i++)
			{
				if (trueLower(str1.string_data[i]) != trueLower(str2.string_data[i]))
				{
					returnVal = false;
					break;
				}
			}
		}

		return returnVal;
	}
};
