// ChatServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "ChatServer.h"

#include "CentralServerConnection.h"
#include "ChatInterface.h"
#include "ChatServerMetricsData.h"
#include "ConfigChatServer.h"
#include "ConnectionServerConnection.h"
#include "CustomerServiceServerConnection.h"
#include "GameServerConnection.h"
#include "PlanetServerConnection.h"
#include "UnicodeUtils.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/ChatConnectAvatar.h"
#include "serverNetworkMessages/ChatServerOnline.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverUtility/ChatLogManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatInstantMessageToClient.h"
#include "sharedNetworkMessages/ChatOnAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatOnBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnCreateRoom.h"
#include "sharedNetworkMessages/ChatOnDestroyRoom.h"
#include "sharedNetworkMessages/ChatOnEnteredRoom.h"
#include "sharedNetworkMessages/ChatOnGetFriendsList.h"
#include "sharedNetworkMessages/ChatOnGetIgnoreList.h"
#include "sharedNetworkMessages/ChatOnInviteToRoom.h"
#include "sharedNetworkMessages/ChatOnInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatOnKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnLeaveRoom.h"
#include "sharedNetworkMessages/ChatOnRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/ChatOnSendInstantMessage.h"
#include "sharedNetworkMessages/ChatOnSendRoomMessage.h"
#include "sharedNetworkMessages/ChatOnUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnUninviteFromRoom.h"
#include "sharedNetworkMessages/ChatSystemMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/VoiceChatMiscMessages.h"
#include "serverUtility/ConfigServerUtility.h"
#include "ChatAPI/ChatAPI.h"

#include "VChatInterface.h"

#include <list>
#include <sstream>

//-----------------------------------------------------------------------

namespace ChatServerNamespace
{
	ChatServerMetricsData * s_chatServerMetricsData = 0;

	typedef std::list<std::pair<int, time_t> > MessageList;
	typedef std::unordered_map<Unicode::String, MessageList> PlayerMessageList;
	PlayerMessageList s_playerMessageList;

	void cleanChatLog();

	time_t const chatMessageCleanTime = 60;
	time_t chatMessageCleanTimer = 0;
	Unicode::String const s_channelTell = Unicode::narrowToWide("tell");
	Unicode::String const s_channelEmail = Unicode::narrowToWide("email");
	bool s_loggingEnabled = false;
	
	std::set<std::pair<std::string, ChatAvatarId> > s_pendingEntersByName;
	std::set<std::pair<NetworkId, std::pair<unsigned int, unsigned int> > > s_pendingEntersById;
	
	Unicode::String getRoomAddress(const std::string & roomName)
	{
		std::string fullName = roomName;
		size_t dotPos = 0;
		while(dotPos != std::string::npos)
		{
			dotPos = fullName.find('.');
			if(dotPos != std::string::npos)
			{
				fullName[dotPos] = '+';
			}
		}
		
		if(fullName.find(ConfigChatServer::getClusterName()) == std::string::npos)
		{
			fullName = ConfigChatServer::getClusterName() + '+' + fullName;
		}
		if(fullName.find(ConfigChatServer::getGameCode()) == std::string::npos)
		{
			fullName = ConfigChatServer::getGameCode() + '+' + fullName;
		}
		if(fullName.find("SOE") == std::string::npos)
		{
			fullName = "SOE+" + fullName;
		}
		return Unicode::narrowToWide(fullName);
	}
	
	ChatUnicodeString makeChatUnicodeString(const Unicode::String & source)
	{
		return ChatUnicodeString(source.c_str(), source.length());
	}

	std::set<NetworkId> s_godClients;
	std::set<NetworkId> s_notSubscribedClients;

	// Control log output for chat room related methods
	bool const s_enableChatRoomLogs = false;
}

using namespace ChatServerNamespace;


//-----------------------------------------------------------------------

void ChatServerNamespace::cleanChatLog()
{
	PlayerMessageList::iterator iterPlayerMessageList = s_playerMessageList.begin();

	time_t const spatialChatLogTime = static_cast<time_t>(ConfigServerUtility::getChatLogMinutes() * 60);
	time_t const currentTime = Os::getRealSystemTime();

	for (; iterPlayerMessageList != s_playerMessageList.end(); ++iterPlayerMessageList)
	{
		MessageList &messageList = iterPlayerMessageList->second;
		int chatLogCount = static_cast<int>(messageList.size());

		MessageList::iterator iterMessageList = messageList.begin();
		while (!messageList.empty())
		{
			time_t const messageTime = iterMessageList->second;

			if (messageTime < (currentTime - spatialChatLogTime) || (chatLogCount > ConfigServerUtility::getPlayerMaxChatLogLines()))
			{
				iterMessageList = messageList.erase(iterMessageList);
				--chatLogCount;
			}
			else
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------

ChatServer *ChatServer::m_instance = nullptr;

#include <stdio.h>

void splitRoomName(std::string roomName, Unicode::String &name, Unicode::String &address);

//-----------------------------------------------------------------------

std::string toLower(const std::string & source)
{
	std::string lower;
	size_t i;
	for(i = 0; i < source.length(); ++i)
	{
		char a = static_cast<char>(tolower(source[i]));
		lower += a;
	}
	return lower;
}

//-----------------------------------------------------------------------

std::string toUpper(const std::string & source)
{
	std::string upper;
	size_t i;
	for(i = 0; i < source.length(); ++i)
	{
		char a = static_cast<char>(toupper(source[i]));
		upper += a;
	}
	return upper;
}

//-----------------------------------------------------------------------

void ChatServer::update()
{
	PROFILER_BLOCK_DEFINE(profileBlockMainLoop, "main loop");
	PROFILER_BLOCK_ENTER(profileBlockMainLoop);
	static unsigned int lastTime = 0;
unsigned int t1 = Clock::timeMs();
	NetworkHandler::update();
unsigned int t2 = Clock::timeMs();

	chatInterface->checkQueuedLogins();
	chatInterface->sendQueuedHeadersToClient();

if ((t2 - t1) > (1000 * 5))
{
	DEBUG_WARNING(true, ("\n\n\n\n\nNetworkHandler::update() took %i ms\n\n\n\n\n", (t2 - t1)));
}

	NetworkHandler::dispatch();
unsigned int t3 = Clock::timeMs();
if ((t3 - t2) > (1000 * 5))
{
	DEBUG_WARNING(true, ("\n\n\n\n\nNetworkHandler::dispatch() took %i ms\n\n\n\n\n", (t3 - t2)));
}
	if (lastTime != 0)
	{
		unsigned int time = Clock::timeMs();
		if ((time - lastTime) > (1000))
		{
			//DEBUG_WARNING(true, ("\n\n\n\n\n\nLong Chat Frame (%i ms)\n\n\n\n", (time - lastTime)));
		}
	}
	lastTime = Clock::timeMs();
	chatInterface->Process();
	voiceChatInterface->Process();

	PROFILER_BLOCK_LEAVE(profileBlockMainLoop);

	if (!Os::update())
		done = true;

	Os::sleep(1);
	static int i = 0;
	if (i % 600 == 0)
	{
		instance().chatInterface->updateRooms();
	}
	if (i % 6000 == 0)
	{
		static Unicode::String wideSWG = Unicode::narrowToWide("SOE+SWG");
		static Unicode::String wideFilter = Unicode::narrowToWide("");
		static ChatUnicodeString swgNode(wideSWG.data(), wideSWG.size());
		static ChatUnicodeString filter(wideFilter.data(), wideFilter.size());
		instance().chatInterface->RequestGetRoomSummaries(swgNode, filter, nullptr);
	}

	++i;

	NetworkHandler::clearBytesThisFrame();
	if(s_chatServerMetricsData)
		MetricsManager::update(Clock::frameTime() * 1000);


	// Clear the chat log every once in a while

	if ((chatMessageCleanTimer + chatMessageCleanTime) > Os::getRealSystemTime())
	{
		chatMessageCleanTimer = Os::getRealSystemTime();
		cleanChatLog();
	}
}

//-----------------------------------------------------------------------

ChatServer::ChatServer() :
pendingRequests(),
callback(new MessageDispatch::Callback),
centralServerConnection(new CentralServerConnection(ConfigChatServer::getCentralServerAddress(), ConfigChatServer::getCentralServerPort())),
chatAvatars(),
chatInterface(0),
voiceChatInterface(0),
clientMap(),
gameServerConnectionMap(),
connectionServerConnections(),
done(false),
gameService(0),
planetService(),
ownerSystem(0),
systemAvatarId("SWG", ConfigChatServer::getClusterName(), "SYSTEM"),
customerServiceServerConnection(nullptr),
m_gameServerConnectionRegistry(),
m_voiceChatIdMap()
{
	s_loggingEnabled = ConfigChatServer::isLoggingEnabled();
	DebugFlags::registerFlag(s_loggingEnabled, "ChatServer", "loggingEnabled");
	SetupSharedLog::install("SwgChatServer");

	NetworkSetupData setup;
	setup.maxConnections = 100;
	setup.port = 0;
	setup.bindInterface = ConfigChatServer::getGameServiceBindInterface();
	gameService = new Service(ConnectionAllocator<GameServerConnection>(), setup);
	setup.bindInterface = ConfigChatServer::getPlanetServiceBindInterface();
	planetService = new Service(ConnectionAllocator<PlanetServerConnection>(), setup);

	const std::string	getGameCode = ConfigChatServer::getGameCode();
	const std::string	getGatewayServerIP = ConfigChatServer::getGatewayServerIP();
	chatInterface = new ChatInterface(getGameCode, getGatewayServerIP, ConfigChatServer::getGatewayServerPort(), ConfigChatServer::getRegistrarHost(), static_cast<unsigned short>(ConfigChatServer::getRegistrarPort()));


	std::string vChatHost;
	int numConnectionStrings = ConfigChatServer::getNumberOfVoiceChatGateways();
	for(int i = 0; i < numConnectionStrings; ++i)
	{
		if(i != 0)
		{
			vChatHost += " ";
		}
		vChatHost += ConfigChatServer::getVoiceChatGateway(i);
	}
	voiceChatInterface = new VChatInterface(vChatHost.c_str());

	callback->connect(*this, &ChatServer::onEnumerateServers); //lint !e1025 !e1703 !e1514 !e64 !e1058 !e118 // jrandall lint is confused, the compiler resolves this just fine //lint !e1703 // jrandall, really, this is fine!


	ChatServer::fileLog(true, "ChatServer", "ChatServer() VChat host: %s", vChatHost.c_str());
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigChatServer::getCentralServerAddress() %s", ConfigChatServer::getCentralServerAddress());
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigChatServer::getCentralServerPort() %d", ConfigChatServer::getCentralServerPort());

	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getReportUdpDisconnects() %s", ConfigSharedNetwork::getReportUdpDisconnects() ? "yes" : "no");
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getReportTcpDisconnects() %s", ConfigSharedNetwork::getReportTcpDisconnects() ? "yes" : "no");
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getLogConnectionConstructionDestruction() %s", ConfigSharedNetwork::getLogConnectionConstructionDestruction() ? "yes" : "no");
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getLogConnectionOpenedClosed() %s", ConfigSharedNetwork::getLogConnectionOpenedClosed() ? "yes" : "no");
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getLogConnectionDeferredMessagesWarning() %s", ConfigSharedNetwork::getLogConnectionDeferredMessagesWarning() ? "yes" : "no");
	ChatServer::fileLog(true, "ChatServer", "ChatServer() ConfigSharedNetwork::getLogConnectionDeferredMessagesWarningInterval() %d", ConfigSharedNetwork::getLogConnectionDeferredMessagesWarningInterval());

	s_chatServerMetricsData = new ChatServerMetricsData();
	MetricsManager::install(s_chatServerMetricsData, false, "ChatServer" , "", 0);
//	ownerSystem = new ChatAvatar("SWG", ConfigChatServer::getClusterName(), "SYSTEM");
}

//-----------------------------------------------------------------------

ChatServer::~ChatServer()
{
	std::set<ConnectionServerConnection *>::const_iterator i;
	for(i = connectionServerConnections.begin(); i != connectionServerConnections.end(); ++i)
	{
		ConnectionServerConnection * c = (*i);
		delete c;
	}
	if(centralServerConnection)
		centralServerConnection->disconnect();
	// need to delete these before NetworkHandler::remove() is called in ::run()
	//delete gameService;
	//delete planetService;
	delete callback;
	delete voiceChatInterface;
	delete chatInterface;
	ownerSystem = 0; //lint !e423 deletion is taken care of in the OnDestroy
	delete s_chatServerMetricsData;
	MetricsManager::remove();
}

//-----------------------------------------------------------------------

bool ChatServer::getChatLog(Unicode::String const &player, std::vector<ChatLogEntry> &chatLog)
{
	Unicode::String const lowerPlayer(Unicode::toLower(player));
	ChatServer::fileLog(false, "ChatServer", "ChatServer::getChatLog() player(%s)", Unicode::wideToNarrow(lowerPlayer).c_str());
	bool result = false;

	PlayerMessageList::const_iterator iterPlayerMessageList = s_playerMessageList.find(lowerPlayer);
	
	if (iterPlayerMessageList != s_playerMessageList.end())
	{
		result = true;
		cleanChatLog();

		MessageList::const_iterator iterMessageList = iterPlayerMessageList->second.begin();

		chatLog.clear();
	
		for (; iterMessageList != iterPlayerMessageList->second.end(); ++iterMessageList)
		{
			int const messageIndex = iterMessageList->first;
			ChatLogEntry chatLogEntry;

			if (ChatLogManager::getChatMessage(messageIndex, chatLogEntry.m_from, chatLogEntry.m_to, chatLogEntry.m_message, chatLogEntry.m_channel, chatLogEntry.m_time))
			{
				chatLog.push_back(chatLogEntry);
			}
			else
			{
				DEBUG_WARNING(true, ("Unable to find chat log message(%d)", messageIndex));
			}
		}
	}

	ChatServer::fileLog(false, "ChatServer", "ChatServer::getChatLog() Found %u chat log messages for player(%s)", chatLog.size(), Unicode::wideToNarrow(lowerPlayer).c_str());

	if (s_loggingEnabled)
	{
		ChatServer::fileLog(false, "ChatServer", "ChatServer::getChatLog() -- BEGIN Player List --");

		iterPlayerMessageList = s_playerMessageList.begin();
		int count = 0;

		for (; iterPlayerMessageList != s_playerMessageList.end(); ++iterPlayerMessageList)
		{
			ChatServer::fileLog(false, "ChatServer", "ChatServer::getChatLog() [%d] player(%s)", ++count, Unicode::wideToNarrow(iterPlayerMessageList->first).c_str());
		}

		ChatServer::fileLog(false, "ChatServer", "ChatServer::getChatLog() -- END Player List --");
	}

	return result;
}

//-----------------------------------------------------------------------

void ChatServer::deferChatMessageFor(const NetworkId & id, const Archive::ByteStream & bs)
{
	instance().chatInterface->deferChatMessageFor(id, bs);
}

//-----------------------------------------------------------------------

void ChatServer::sendToClient(const NetworkId & id, const GameNetworkMessage & message)
{
	ChatServer::fileLog(false, "ChatServer", "sendToClient() id(%s) message(%s)", id.getValueString().c_str(), message.getCmdName().c_str());

	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::const_iterator f = instance().clientMap.find(id);
	if(f != instance().clientMap.end())
	{
		(*f).second->sendToClient(id, message);
	}
}

//-----------------------------------------------------------------------

void ChatServer::setOwnerSystem(const ChatAvatar * o)
{
	ChatServer::fileLog(false, "ChatServer", "setOwnerSystem() ");

	instance().ownerSystem = o; //lint !e423 deletion is taken care of in the OnDestroy
	const Service * s = getGameService();
	if(s)
	{
		ChatServerOnline cso(s->getBindAddress(), s->getBindPort());
		Archive::ByteStream bs;
		cso.pack(bs);

		Connection *connection = safe_cast<Connection *>(instance().centralServerConnection);

		if (connection != nullptr)
		{
			connection->send(bs, true);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::onEnumerateServers(const EnumerateServers & e)
{
	if(e.getAdd())
	{
		switch(e.getServerType())
		{
		case EnumerateServers::CONNECTION_SERVER:
			{
//printf("!!!adding connection server, chat was told to connect to port %i!!!!\n",
//				e.getPort());

				static int count = 0;
				++count;

				ConnectionServerConnection * c = new ConnectionServerConnection(e.getAddress(), e.getPort());

				if (   !e.getAddress().empty()
					&& (e.getPort() != 0))
				{
					ChatServer::fileLog(true, "ChatServer", "onEnumerateServers() count(%d) address(%s)", count, getConnectionAddress(c).c_str());

					IGNORE_RETURN(connectionServerConnections.insert(c));
					s_chatServerMetricsData->setConnectionServerConnectionCount(instance().connectionServerConnections.size());
				}
				else
				{
					ChatServer::fileLog(true, "ChatServer", "onEnumerateServers() count(%d) Empty server address", count);
				}
			}//lint !e429  Custodial pointer 'c' (line 232) has not been freed or returned //jrandall tracked by connectionServerConnections
			break;
		default:
			break;
		}
	}
	else
	{
		// remove the server
	}
}

//-----------------------------------------------------------------------

ChatServer & ChatServer::instance()
{
	static ChatServer s;
	m_instance = &s;
	return *m_instance;
}

//-----------------------------------------------------------------------

void ChatServer::quit()
{
	instance().done = true;
}

//-----------------------------------------------------------------------

void ChatServer::reconnectToCentralServer()
{
	ChatServer::fileLog(true, "ChatServer", "reconnectToCentralServer()");

	if (instance().centralServerConnection)
		instance().centralServerConnection->disconnect();

	instance().centralServerConnection = new CentralServerConnection(ConfigChatServer::getCentralServerAddress(), ConfigChatServer::getCentralServerPort());
}

//-----------------------------------------------------------------------

void ChatServer::onCentralServerConnectionClosed()
{
	instance().centralServerConnection = 0;
}

//-----------------------------------------------------------------------

const ChatAvatarId & ChatServer::getSystemAvatarId()
{
	return instance().systemAvatarId;
}

//-----------------------------------------------------------------------

const Service * ChatServer::getGameService()
{
	return instance().gameService;
}

//-----------------------------------------------------------------------

void ChatServer::run()
{
	NetworkHandler::install();

	Clock::setFrameRateLimit(20.0f);

	// create default rooms

	NetworkId id = NetworkId::cms_invalid;

//	LOG("ServerStartup",("ChatServer starting"));

	int const numberOfCreateRooms = ConfigChatServer::getNumberOfCreateRooms();
	for (int i = 0; i < numberOfCreateRooms; ++i)
	{
		char const * const p = ConfigChatServer::getCreateRoom(i);
		if (p)
		{
			createRoom(id, 0, std::string(p), false, true, "");
		}
	}

	while(! instance().done)
	{
		instance().update();
	}

	instance().removeServices();
	NetworkHandler::remove();
}

void ChatServer::removeServices()
{
	delete gameService;
	gameService = 0;
	delete planetService;
	planetService = 0;
}

//-----------------------------------------------------------------------

ChatAvatarId ChatServer::getAvatarIdForTrackId(unsigned trackId)
{
	ChatServer & server = instance();
	std::unordered_map<unsigned, NetworkId>::iterator f = server.pendingRequests.find(trackId);
	if(f != server.pendingRequests.end())
	{
		const ChatAvatar *avatar = getAvatarByNetworkId((*f).second);
		if (avatar)
		{
			ChatAvatarId avatarId;
			makeAvatarId(*avatar, avatarId);
			return avatarId;
		}
		server.pendingRequests.erase(f);
	}
	static const ChatAvatarId badId;
	return badId;
}

//-----------------------------------------------------------------------

const ChatAvatar * ChatServer::getAvatarByNetworkId(const NetworkId & id)
{
	if (id.getValue() == 0)
	{
		return nullptr;
	}
	const ChatAvatar * result = 0;
	ChatAvatarList::const_iterator f = instance().chatAvatars.find(id);
	if(f != instance().chatAvatars.end())
	{
		result = &((*f).second.chatAvatar);
	}
	return result;
}

//-----------------------------------------------------------------------

ChatServer::AvatarExtendedData * ChatServer::getAvatarExtendedDataByNetworkId(const NetworkId & id)
{
	if (id.getValue() == 0)
	{
		return nullptr;
	}
	ChatServer::AvatarExtendedData * result = 0;
	ChatAvatarList::iterator f = instance().chatAvatars.find(id);
	if(f != instance().chatAvatars.end())
	{
		result = &((*f).second);
	}
	return result;
}

//-----------------------------------------------------------------------

const NetworkId &  ChatServer::getNetworkIdByAvatarId(const ChatAvatarId & id)
{
	PROFILER_AUTO_BLOCK_DEFINE("ChatServer - getNetworkIdByAvatarId");
	ChatAvatarList::const_iterator f;
	for (f = instance().chatAvatars.begin(); f != instance().chatAvatars.end(); ++f)
	{
		ChatAvatarId av;
		makeAvatarId((*f).second.chatAvatar, av);
		if (av == id)
		{
			return f->first;
		}
	}
	static NetworkId badId;
	return badId;
	
	return instance().chatInterface->getNetworkIdByAvatarId(id);
}

//-----------------------------------------------------------------------

NetworkId ChatServer::sendResponseForTrackId(unsigned trackId, const GameNetworkMessage & response)
{
	NetworkId id;
	ChatServer & server = instance();
	std::unordered_map<unsigned, NetworkId>::iterator f = server.pendingRequests.find(trackId);
	if(f != server.pendingRequests.end())
	{
		id = (*f).second;
		std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::const_iterator c = server.clientMap.find((*f).second);
		if(c != server.clientMap.end())
		{
			ConnectionServerConnection * conn = (*c).second;
			conn->sendToClient((*f).second, response);
		}
		server.pendingRequests.erase(f);
	}
	return id;
}

//-----------------------------------------------------------------------

GameServerConnection *ChatServer::getGameServerConnection(unsigned int sequence)
{
	GameServerConnection * result = 0;
	std::unordered_map<unsigned int, GameServerConnection *>::iterator f = instance().gameServerConnectionMap.find(sequence);
	if(f != instance().gameServerConnectionMap.end())
	{
		result = (*f).second;
		instance().gameServerConnectionMap.erase(f);
	}
	return result;

}

//-----------------------------------------------------------------------

void ChatServer::addGameServerConnection(unsigned int sequence, GameServerConnection *connection)
{
	IGNORE_RETURN(instance().gameServerConnectionMap.insert(std::make_pair(sequence, connection)));
	s_chatServerMetricsData->setGameServerConnectionCount(instance().gameServerConnectionMap.size());
}

//-----------------------------------------------------------------------

void ChatServer::clearGameServerConnection(const GameServerConnection *connection)
{
	std::vector<unsigned int> removeKeyList;
	std::unordered_map<unsigned int, GameServerConnection *>::const_iterator i;
	for(i = instance().gameServerConnectionMap.begin(); i != instance().gameServerConnectionMap.end(); ++i)
	{
		if ((*i).second == connection)
		{
			removeKeyList.push_back((*i).first);
		}
	}

	std::vector<unsigned int>::const_iterator j;
	for (j = removeKeyList.begin(); j != removeKeyList.end(); ++j)
	{
		IGNORE_RETURN(instance().gameServerConnectionMap.erase((*j)));
	}
	s_chatServerMetricsData->setGameServerConnectionCount(instance().gameServerConnectionMap.size());
}

//-----------------------------------------------------------------------

ConnectionServerConnection * ChatServer::getConnectionForCharacter(const NetworkId & characterId)
{
	ConnectionServerConnection * result = 0;
	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::iterator f = instance().clientMap.find(characterId);
	if(f != instance().clientMap.end())
	{
		result = (*f).second;
	}
	return result;
}


//-----------------------------------------------------------------------

void splitChatAvatarId(const ChatAvatarId &id, Unicode::String &name, Unicode::String &address)
{
	name = Unicode::narrowToWide(id.name);
	address = Unicode::narrowToWide("SOE+" + id.gameCode + "+" + id.cluster);
}

//-----------------------------------------------------------------------

void canonicalizeAvatarId(const ChatAvatarId &id)
{
	ChatAvatarId &hack = const_cast<ChatAvatarId &>(id);
	hack.gameCode = toUpper(hack.gameCode);
	hack.name = toLower(hack.name);
}

//-----------------------------------------------------------------------

void ChatServer::addFriend(const NetworkId & id, const unsigned int sequence, const ChatAvatarId & friendId)
{
	UNREF(sequence);
	ChatServer::fileLog(false, "ChatServer", "addFriend() id(%s) sequence(%u) friendId(%s)", id.getValueString().c_str(), sequence, friendId.getFullName().c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);

	if(from)
	{
		Unicode::String friendName;
		Unicode::String friendAddress;

		canonicalizeAvatarId(friendId);
		splitChatAvatarId(friendId, friendName, friendAddress);

		unsigned track = instance().chatInterface->RequestAddFriend(from, ChatUnicodeString(friendName.data(), friendName.length()),
			ChatUnicodeString(friendAddress.data(), friendAddress.length()),
			false, nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "addFriend() ERROR: avatar not found(%s)", id.getValueString().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeFriend(const NetworkId & id, const unsigned int sequence, const ChatAvatarId & friendId)
{
	UNREF(sequence);
	ChatServer::fileLog(false, "ChatServer", "removeFriend() id(%s) sequence(%u) friendId(%s)", id.getValueString().c_str(), sequence, friendId.getFullName().c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);

	if(from)
	{
		Unicode::String friendName;
		Unicode::String friendAddress;

		canonicalizeAvatarId(friendId);
		splitChatAvatarId(friendId, friendName, friendAddress);

		unsigned track = instance().chatInterface->RequestRemoveFriend(from, ChatUnicodeString(friendName.data(), friendName.length()),
			ChatUnicodeString(friendAddress.data(), friendAddress.length()), nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "removeFriend() ERROR: avatar not found(%s)", id.getValueString().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatServer::getFriendsList(const ChatAvatarId &characterName)
{
	ChatServer::fileLog(false, "ChatServer", "getFriendsList() characterName(%s)", characterName.getFullName().c_str());

	const NetworkId &id = getNetworkIdByAvatarId(characterName);
	const ChatAvatar *avatar = getAvatarByNetworkId(id);
	if (avatar)
	{
		unsigned track = instance().chatInterface->RequestFriendStatus(avatar, nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "getFriendsList() ERROR: avatar not found(%s)", characterName.getFullName().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatServer::addIgnore(const NetworkId & id, const unsigned int sequence, const ChatAvatarId & ignoreId)
{
	UNREF(sequence);
	ChatServer::fileLog(false, "ChatServer", "addIgnore() id(%s) sequence(%u) ignoreId(%s)", id.getValueString().c_str(), sequence, ignoreId.getFullName().c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);

	if(from)
	{
		Unicode::String ignoreName;
		Unicode::String ignoreAddress;

		canonicalizeAvatarId(ignoreId);
		splitChatAvatarId(ignoreId, ignoreName, ignoreAddress);

		unsigned track = instance().chatInterface->RequestAddIgnore(from, ChatUnicodeString(ignoreName.data(), ignoreName.length()),
			ChatUnicodeString(ignoreAddress.data(), ignoreAddress.length()),
			nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "addIgnore() ERROR: avatar not found(%s)", id.getValueString().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeIgnore(const NetworkId & id, const unsigned int sequence, const ChatAvatarId & ignoreId)
{
	UNREF(sequence);
	ChatServer::fileLog(false, "ChatServer", "removeIgnore() id(%s) sequence(%u) ignoreId(%s)", id.getValueString().c_str(), sequence, ignoreId.getFullName().c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);

	if(from)
	{
		Unicode::String ignoreName;
		Unicode::String ignoreAddress;

		canonicalizeAvatarId(ignoreId);
		splitChatAvatarId(ignoreId, ignoreName, ignoreAddress);

		unsigned track = instance().chatInterface->RequestRemoveIgnore(from, ChatUnicodeString(ignoreName.data(), ignoreName.length()),
			ChatUnicodeString(ignoreAddress.data(), ignoreAddress.length()), nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "removeIgnore() ERROR: avatar not found(%s)", id.getValueString().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatServer::getIgnoreList(const ChatAvatarId &characterName)
{
	canonicalizeAvatarId(characterName);
	ChatServer::fileLog(false, "ChatServer", "getIgnoreList() characterName(%s)", characterName.getFullName().c_str());

	const NetworkId &id = getNetworkIdByAvatarId(characterName);
	const ChatAvatar *avatar = getAvatarByNetworkId(id);

	if (avatar)
	{
		unsigned track = instance().chatInterface->RequestIgnoreStatus(avatar, nullptr);
		instance().pendingRequests[track] = id;
	}
	else
	{
		ChatServer::fileLog(true, "ChatServer", "getIgnoreList() ERROR: avatar not found(%s)", characterName.getFullName().c_str());
	}
}


//-----------------------------------------------------------------------

void ChatServer::addModeratorToRoom(const unsigned int sequenceId, const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName)
{
	canonicalizeAvatarId(avatarId);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "addModeratorToRoom() sequenceId(%u) id(%s) avatarId(%s) roomName(%s)", sequenceId, id.getValueString().c_str(), avatarId.getFullName().c_str(), roomName.c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);
	if(from)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		const ChatServerRoomOwner *roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String moderatorName;
			Unicode::String moderatorAddress;
			splitChatAvatarId(avatarId, moderatorName, moderatorAddress);

			// Remove the avatar from the ban list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(sequenceId, avatarId);

				instance().chatInterface->RequestRemoveBan(from, moderatorName, moderatorAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Try to make the avatar a moderator
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(sequenceId, avatarId);

				unsigned track = instance().chatInterface->RequestAddModerator(from, moderatorName, moderatorAddress, roomOwner->getAddress(), (void *)pair);
				instance().pendingRequests[track] = id;
			}
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId fromId;
			makeAvatarId(*from, fromId);

			ChatOnAddModeratorToRoom msg(errorCode, sequenceId, fromId, avatarId, roomName);
			instance().chatInterface->sendMessageToAvatar(fromId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeModeratorFromRoom(const unsigned int sequenceId, const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName)
{
	canonicalizeAvatarId(avatarId);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "removeModeratorFromRoom() sequenceId(%u) id(%s) avatarId(%s) roomName(%s)", sequenceId, id.getValueString().c_str(), avatarId.getFullName().c_str(), roomName.c_str());

	const ChatAvatar * from = getAvatarByNetworkId(id);
	if(from)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		const ChatServerRoomOwner *roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String moderatorName;
			Unicode::String moderatorAddress;
			splitChatAvatarId(avatarId, moderatorName, moderatorAddress);

			// User data so that we remember who is being removed as a moderator
			AvatarIdSequencePair *pair = new AvatarIdSequencePair(sequenceId, avatarId);

			unsigned track = instance().chatInterface->RequestRemoveModerator(from, moderatorName, moderatorAddress, roomOwner->getAddress(), (void *)pair);
			instance().pendingRequests[track] = id;
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId fromId;
			makeAvatarId(*from, fromId);

			ChatOnRemoveModeratorFromRoom msg(errorCode, sequenceId, fromId, avatarId, roomName);
			instance().chatInterface->sendMessageToAvatar(fromId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::chatConnectedAvatar(const NetworkId & id, const ChatAvatar & newAvatar)
{
	ChatServer::fileLog(false, "ChatServer", "chatConnectedAvatar() id(%s) newAvatar(%s)", id.getValueString().c_str(), getFullChatAvatarName(&newAvatar).c_str());

	AvatarExtendedData aed;
	aed.chatAvatar = newAvatar;
	aed.isSubscribed = (s_notSubscribedClients.count(id) == 0);
	aed.spatialCharCount = 0;
	aed.nonSpatialCharCount = 0;
	aed.chatSpamTimeEndInterval = 0;
	aed.chatSpamNextTimeToSyncWithGameServer = 0;
	aed.chatSpamNextTimeToNotifyPlayerWhenLimited = 0;
	aed.unsquelchTime = 0;

	IGNORE_RETURN(instance().chatAvatars.insert(std::make_pair(id, aed)));
}

//-----------------------------------------------------------------------

void ChatServer::connectPlayer(ConnectionServerConnection * connection, const unsigned int suid, const std::string & characterName, const NetworkId & networkId, bool isSecure, bool isSubscribed)
{
	ChatServer::fileLog(false, "ChatServer", "connectPlayer() address(%s) suid(%u) characterName(%s) networkId(%s)", getConnectionAddress(connection).c_str(), suid, characterName.c_str(), networkId.getValueString().c_str());

	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::iterator cf = instance().clientMap.find(networkId);
	if (cf != instance().clientMap.end())
	{
		DEBUG_WARNING(true, ("We received a chat login attempt for a player (%s) that we think is already logged in.  We're going to try to log out the old avatar then log in the new one.", characterName.c_str()));
		//instance().disconnectPlayer(networkId);
		instance().clientMap[networkId] = connection;
		return;
	}

	IGNORE_RETURN(instance().clientMap.insert(std::make_pair(networkId, connection)));
	instance().chatInterface->ConnectPlayer(suid, toLower(characterName), networkId);

	uint32 voiceStatus = VoiceChatStatus::SC_VoiceStatusUnknown;
	if(isSubscribed || isSecure)
	{
		getVChatInterface()->requestConnectPlayer(suid, characterName, networkId);
		voiceStatus = VoiceChatStatus::SC_VoiceEnabled;
	}
	else
	{
		voiceStatus = VoiceChatStatus::SC_VoiceClientIsTrial;
	}

	VoiceChatStatus msg(voiceStatus);
	sendToClient(networkId, msg);
	
	s_chatServerMetricsData->setClientCount(instance().clientMap.size());
	if(isSecure)
	{
		s_godClients.insert(networkId);
	}
	if (!isSubscribed)
	{
		s_notSubscribedClients.insert(networkId);
	}
}

int countDots(const std::string &str)
{
	int num = 0;
	for (unsigned i = 0; i < str.length(); ++i)
	{
		if (str[i] == '.')
		{
			++num;
		}
	}
	return num;
}

//-----------------------------------------------------------------------

void ChatServer::createRoom(const NetworkId & id, const unsigned int sequence, const std::string & roomName, const bool isModerated, const bool isPublic, const std::string & title)
{
	if(roomName.empty())
		return;

	if(roomName.find("..") != std::string::npos ||
		roomName.find(" ") != std::string::npos ||
		roomName[0] == '.')
	{
		return;
	}
	
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "createRoom() id(%s) sequence(%u) roomName(%s) isModerated(%s) isPublic(%s) title(%s)", id.getValueString().c_str(), sequence, roomName.c_str(), isModerated ? "yes" : "no", isPublic ? "yes" : "no", title.c_str());

	RoomParams roomParams;

	std::string baseName = std::string("SWG.") + ConfigChatServer::getClusterName();
	if (roomName == baseName || roomName == toLower(baseName))
	{
		return;
	}

	if(toLower(roomName).find(toLower(baseName)) != 0)
		return;
	
	int numDots = countDots(roomName);
	if (numDots > 1)
	{
		size_t lastDot = roomName.rfind('.');
		if (lastDot != std::string::npos)
		{
			std::string parent = roomName.substr(0, lastDot);
			const ChatServerRoomOwner *room =
				instance().chatInterface->getRoomOwner(parent);
			if (!room)
			{
				//REPORT_LOG(true, ("Creating parent room %s\n", parent.c_str()));
				createRoom(id, 0, parent, isModerated, isPublic, "");
			}

		}
	}

	//printf("Creating room %s\n", roomName.c_str());

	unsigned roomAttr = 0;

	static Archive::ByteStream a;
	const ChatAvatar * owner = 0;
	bool isSystemOwner = false;

	//REPORT_LOG(true, ("createRoom(%s, %s)\n", id.getValueString().c_str(), roomName.c_str()));
	if(id != NetworkId::cms_invalid)
	{
		owner = getAvatarByNetworkId(id);
	}
	else
	{
		isSystemOwner = true;
		owner = instance().ownerSystem;
		//REPORT_LOG(true, ("System is creating a room: %s\n", roomName.c_str()));
	}

	if (!owner)
	{
		return;
	}

	if (isModerated)
	{
		roomAttr |= ROOMATTR_MODERATED;
	}

	if (!isPublic)
	{
		roomAttr |= ROOMATTR_PRIVATE;
	}
	if (isSystemOwner && (strstr(roomName.c_str(), "system") != nullptr))
	{
		roomAttr |= ROOMATTR_PERSISTENT;
	}

	std::string tmpRoomName = roomName;

	Unicode::String wideRoomName;
	Unicode::String wideRoomAddress;
	splitRoomName(tmpRoomName, wideRoomName, wideRoomAddress);

	Unicode::String wideTitle    = Unicode::narrowToWide(title);

	roomParams.setRoomName(ChatUnicodeString(wideRoomName.data(), wideRoomName.size()));
	roomParams.setRoomTopic(ChatUnicodeString(wideTitle.data(), wideTitle.size()));
	roomParams.setRoomAttributes(roomAttr);


	//instance().voiceChatInterface->GetChannel(narrowRoomAddress,narrowRoomName,"text","",0,false,0);
		

	unsigned track = instance().chatInterface->RequestCreateRoom(owner, roomParams, ChatUnicodeString(wideRoomAddress.data(), wideRoomAddress.size()), (void *)sequence); //lint !e641 Converting enum 'ChatRoomType' to int
	if(id != NetworkId::cms_invalid)
		instance().pendingRequests[track] = id;
}

//-----------------------------------------------------------------------
void ChatServer::deleteAllPersistentMessages(const NetworkId &sourceNetworkId, const NetworkId &targetNetworkId)
{
	ChatServer::fileLog(false, "ChatServer", "deleteAllPersistentMessages() sourceNetworkId(%s) targetNetworkId(%s)", sourceNetworkId.getValueString().c_str(), targetNetworkId.getValueString().c_str());

	ChatAvatar const *avatar = getAvatarByNetworkId(targetNetworkId);

	if (avatar != nullptr)
	{
		{
			NetworkId *tmpNetworkId = new NetworkId(sourceNetworkId);
			instance().chatInterface->RequestUpdatePersistentMessages(avatar, PERSISTENT_NEW, PERSISTENT_DELETED, reinterpret_cast<void *>(tmpNetworkId));
		}

		{
			NetworkId *tmpNetworkId = new NetworkId(sourceNetworkId);
			instance().chatInterface->RequestUpdatePersistentMessages(avatar, PERSISTENT_UNREAD, PERSISTENT_DELETED, reinterpret_cast<void *>(tmpNetworkId));
		}

		{
			NetworkId *tmpNetworkId = new NetworkId(sourceNetworkId);
			instance().chatInterface->RequestUpdatePersistentMessages(avatar, PERSISTENT_READ, PERSISTENT_DELETED, reinterpret_cast<void *>(tmpNetworkId));
		}
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("ChatServer::deleteAllPersistentMessages() Unable to resolve target avatar.\n"));
	}
}

//-----------------------------------------------------------------------

void ChatServer::deletePersistentMessage(const NetworkId &id, const unsigned int messageId)
{
	ChatServer::fileLog(false, "ChatServer", "deletePersistentMessage() id(%s) messageId(%u)", id.getValueString().c_str(), messageId);

	const ChatAvatar *avatar = getAvatarByNetworkId(id);
	if (avatar)
	{
		instance().chatInterface->RequestUpdatePersistentMessage(avatar, messageId, PERSISTENT_DELETED, nullptr);
	}
}

//-----------------------------------------------------------------------

ChatInterface *ChatServer::getChatInterface()
{
	return instance().chatInterface;
}


//-----------------------------------------------------------------------

void ChatServer::destroyRoom(const NetworkId & id, const unsigned int sequence, const unsigned int roomId)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "destroyRoom() id(%s) sequence(%u) roomId(%u)", id.getValueString().c_str(), sequence, roomId);

/*	const ChatAvatar * avatar = getAvatarByNetworkId(id);
	if(avatar)
	{
		ChatAvatarId destroyer(avatar->GetGameCode(), avatar->GetGameServerName(), avatar->GetCharacterName());

		// can this avatar destroy the room?
		std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator i;
		for(i = instance().chatInterface->getRoomList().begin(); i != instance().chatInterface->getRoomList().end(); ++i)
		{
			if((*i).second.getRoomData().id == roomId)
			{
				bool removeRoom = false;
				if((*i).second.getRoomData().owner == destroyer)
					removeRoom = true;
				else if((*i).second.getRoomData().creator == destroyer)
					removeRoom = true;

				if(removeRoom)
				{
					std::string track = instance().chatInterface->DestroyRoom(sequence, roomId, avatar->GetCharacterName(), avatar->GetGameServerName(), avatar->GetGameCode());
					instance().pendingRequests[track] = id;
				}
				else
				{
					ChatOnDestroyRoom destroy(sequence, CHAT_RESULT_FAIL_INSUFFICIENT_PRIVILEGES, roomId, destroyer); //lint !e641 enum to int
					sendToClient(id, destroy);
				}
				break;
			}
		}
	}*/
	const ChatServerRoomOwner *owner = instance().chatInterface->getRoomOwner(roomId);
	const ChatAvatar *avatar = getAvatarByNetworkId(id);
	if (avatar && owner)
	{
		RoomOwnerSequencePair *pair = new RoomOwnerSequencePair(sequence, owner);
		unsigned track = instance().chatInterface->RequestDestroyRoom(avatar, owner->getAddress(), (void *)pair);
		instance().pendingRequests[track] = id;
	}
}

//-----------------------------------------------------------------------

void ChatServer::destroyRoom(const std::string & roomName)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "destroyRoom() roomName(%s)", roomName.c_str());

	if ((strstr(roomName.c_str(), ConfigChatServer::getClusterName()) != nullptr) ||
		(strstr(roomName.c_str(), toLower(std::string(ConfigChatServer::getClusterName() ) ).c_str() ) ))
	{
		if ((strstr(roomName.c_str(), "GroupChat") != nullptr) || (strstr(roomName.c_str(), "groupchat") != nullptr))
		{
			size_t pos = roomName.rfind(".");
			if (pos != roomName.npos)
			{
				std::string tmpRoomName = roomName.substr(0, pos);
				char lastChar = tmpRoomName[tmpRoomName.length() - 1];
				if (lastChar >= '0' && lastChar <= '9')
				{
					ChatServer::destroyRoom(tmpRoomName);
					return;
				}
			}
		}
	}
	const ChatServerRoomOwner *owner = instance().chatInterface->getRoomOwner(roomName);
	if (!instance().ownerSystem)
	{
		DEBUG_WARNING(true, ("Chat ownerSystem is nullptr"));
		return;
	}
	if (owner)
	{
		RoomOwnerSequencePair *pair = new RoomOwnerSequencePair(0, owner);
		static Unicode::String wideAddr = Unicode::narrowToWide(std::string("SWG") + "+" + ConfigChatServer::getClusterName());
		static ChatUnicodeString chatAddr(wideAddr.data(), wideAddr.size());
		instance().chatInterface->RequestDestroyRoom(instance().ownerSystem, owner->getAddress(), (void *)pair);
	}
}

//-----------------------------------------------------------------------

void ChatServer::disconnectAvatar(const ChatAvatar & avatar)
{
	ChatServer::fileLog(false, "ChatServer", "disconnectAvatar() avatar(%s)", ChatServer::getFullChatAvatarName(&avatar).c_str());

	if (&avatar == instance().ownerSystem)
	{
		instance().ownerSystem = nullptr;
	}
	ChatAvatarList::iterator i;
	for(i = instance().chatAvatars.begin(); i != instance().chatAvatars.end(); ++i)
	{
		if((*i).second.chatAvatar.getAvatarID() == avatar.getAvatarID())
		{
			disconnectPlayer((*i).first);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::disconnectPlayer(const NetworkId & id)
{
	ChatServer::fileLog(false, "ChatServer", "disconnectPlayer() id(%s)", id.getValueString().c_str());

	ChatAvatarList::iterator f = instance().chatAvatars.find(id);
	if(f != instance().chatAvatars.end())
	{
		ChatServerAvatarOwner *owner = instance().chatInterface->getAvatarOwner(&((*f).second.chatAvatar));

		ChatAvatarId avatarId;
		makeAvatarId((*f).second.chatAvatar, avatarId);
		instance().chatInterface->disconnectPlayer(avatarId);
		instance().chatInterface->RequestLogoutAvatar(&((*f).second.chatAvatar), (void *)owner);
		instance().chatAvatars.erase(f);
	}

	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::iterator cf = instance().clientMap.find(id);
	if(cf != instance().clientMap.end())
	{
		instance().clientMap.erase(cf);
	}
	s_chatServerMetricsData->setClientCount(instance().clientMap.size());
	
	VoiceChatAvatarList::iterator voiceMapItr = instance().m_voiceChatIdMap.find(id);
	if(voiceMapItr != instance().m_voiceChatIdMap.end())
	{
		std::map<std::string,NetworkId>::iterator nameMapItr = instance().m_voiceChatNameToIdMap.find(toLower(voiceMapItr->second.playerName));
		if(nameMapItr != instance().m_voiceChatNameToIdMap.end())
		{
			instance().m_voiceChatNameToIdMap.erase(nameMapItr);
		}
		
		nameMapItr = instance().m_voiceLoginNameToIdMap.find(toLower(voiceMapItr->second.loginName));
		if(nameMapItr != instance().m_voiceLoginNameToIdMap.end())
		{
			instance().m_voiceLoginNameToIdMap.erase(nameMapItr);
		}

		instance().m_voiceChatIdMap.erase(voiceMapItr);
	}

	s_godClients.erase(id);
	s_notSubscribedClients.erase(id);
}

//-----------------------------------------------------------------------

void ChatServer::enterRoom(const NetworkId & id, const unsigned int sequence, const unsigned int roomId)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "enterRoom() id(%s) sequence(%u) roomId(%u)", id.getValueString().c_str(), sequence, roomId);

	//REPORT_LOG(true, ("EnterRoom(%s, %u)\n", id.getValueString().c_str(), roomId));
	const ChatAvatar * avatar = getAvatarByNetworkId(id);
	const ChatServerRoomOwner *room = instance().chatInterface->getRoomOwner(roomId);

	if(avatar && room)
	{
		unsigned track = instance().chatInterface->RequestEnterRoom(avatar, room->getAddress(), (void *)sequence);
		instance().pendingRequests[track] = id;
	}
	else
	{
		s_pendingEntersById.insert(std::make_pair(id, std::make_pair(sequence, roomId)));
	}
}

//-----------------------------------------------------------------------

void ChatServer::enterRoom(const ChatAvatarId & id, const std::string & roomName, bool forceCreate, bool createPrivate)
{
	canonicalizeAvatarId(id);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "enterRoom() id(%s) roomName(%s)", id.getFullName().c_str(), roomName.c_str());

	const ChatServerRoomOwner *room = instance().chatInterface->getRoomOwner(roomName);
	const ChatAvatar *avatar = getAvatarByNetworkId(getNetworkIdByAvatarId(id));
	if (room && avatar)
	{
		instance().chatInterface->RequestEnterRoom(avatar, room->getAddress(), nullptr);
	}
	else
	{
		if(! room)
		{
			if(forceCreate)
			{
				// create a room for the avatar and wait for the OnCreateRoom to return
				createRoom(NetworkId::cms_invalid, 0, roomName, false, !createPrivate, "");
				s_pendingEntersByName.insert(std::make_pair(toLower(roomName), id)); // for game server generated requests
			}
		}
		//REPORT_LOG(!room, ("room is not found\n"));
		//REPORT_LOG(!avatar, ("avatar is not found\n"));
	}
}

//-----------------------------------------------------------------------

void ChatServer::putSystemAvatarInRoom(const std::string & roomName)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "putSystemAvatarInRoom() roomName(%s)", roomName.c_str());

	const ChatServerRoomOwner *room = instance().chatInterface->getRoomOwner(roomName);
	if (room && instance().ownerSystem)
	{
		instance().chatInterface->RequestEnterRoom(instance().ownerSystem, room->getAddress(), nullptr);
	}
}

//-----------------------------------------------------------------------

void ChatServer::enterRoom(const NetworkId & id, const unsigned int sequence, const std::string & roomName)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "enterRoom() id(%s) sequence(%u) roomName(%s)", id.getValueString().c_str(), sequence, roomName.c_str());

	//REPORT_LOG(true, ("enterRoom3(%s, %s)\n", id.getValueString().c_str(), roomName.c_str()));
	const ChatAvatar * avatar = getAvatarByNetworkId(id);
	if(avatar)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;
		unsigned roomId    = 0;

		const ChatServerRoomOwner *roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			// We can now get a room ID	
			roomId = roomOwner->getRoomData().id;

			// prevent players from requesting to enter/leave system rooms
			if((roomName.rfind(".") != std::string::npos))
			{              
				std::string leaf = roomName.substr(0, roomName.rfind("."));
				std::string lowerLeaf = toLower(leaf);
				if(lowerLeaf != toLower(ChatRoomTypes::ROOM_SYSTEM))
				{
					const ChatServerRoomOwner *room = instance().chatInterface->getRoomOwner(roomName);
					if (room)
					{
						unsigned track = instance().chatInterface->RequestEnterRoom(avatar, room->getAddress(), (void *)sequence);
						instance().pendingRequests[track] = id;

						return;
					}
				}
			}

			errorCode = CHATRESULT_ROOM_UNKNOWNFAILURE;
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId avatarId;
			makeAvatarId(*avatar, avatarId);

			ChatOnEnteredRoom fail(sequence, errorCode, roomId, avatarId); //lint !e641 enum to int
			sendToClient(id, fail);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::unbanFromRoom(const unsigned sequence, const NetworkId &banner, const ChatAvatarId &bannee, const std::string &roomName)
{
	canonicalizeAvatarId(bannee);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "unbanFromRoom() sequence(%u) banner(%s) bannee(%s) roomName(%s)", sequence, banner.getValueString().c_str(), bannee.getFullName().c_str(), roomName.c_str());

	const ChatAvatar *bannerAvatar = getAvatarByNetworkId(banner);
	if (bannerAvatar)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		const ChatServerRoomOwner *roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String banName;
			Unicode::String banAddress;
			splitChatAvatarId(bannee, banName, banAddress);

			AvatarIdSequencePair *pair = new AvatarIdSequencePair(sequence, bannee);

			unsigned track = instance().chatInterface->RequestRemoveBan(bannerAvatar, banName, banAddress, roomOwner->getAddress(), (void *)pair);
			instance().pendingRequests[track] = banner;
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId bannerAvatarId;
			makeAvatarId(*bannerAvatar, bannerAvatarId);

			ChatOnUnbanAvatarFromRoom msg(sequence, errorCode, roomName, bannerAvatarId, bannee);
			instance().chatInterface->sendMessageToAvatar(bannerAvatarId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::banFromRoom(const unsigned sequence, const NetworkId &banner, const ChatAvatarId &bannee, const std::string &roomName)
{
	canonicalizeAvatarId(bannee);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "banFromRoom() sequence(%u) banner(%s) bannee(%s) roomName(%s)", sequence, banner.getValueString().c_str(), bannee.getFullName().c_str(), roomName.c_str());

	// can't ban god accounts
	NetworkId networkId = getNetworkIdByAvatarId(bannee);
	if(isGod(networkId))
	{
		return;
	}

	const ChatAvatar *bannerAvatar = getAvatarByNetworkId(banner);
	if (bannerAvatar)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		const ChatServerRoomOwner *roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String banName;
			Unicode::String banAddress;
			splitChatAvatarId(bannee, banName, banAddress);

			// Remove the bannee from the invited list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, bannee);
				instance().chatInterface->RequestRemoveInvite(bannerAvatar, banName, banAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Remove the bannee from the moderator list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, bannee);
				instance().chatInterface->RequestRemoveModerator(bannerAvatar, banName, banAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Ban the avatar
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(sequence, bannee);

				unsigned track = instance().chatInterface->RequestAddBan(bannerAvatar, banName, banAddress, roomOwner->getAddress(), (void *)pair);
				instance().pendingRequests[track] = banner;
			}
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId bannerAvatarId;
			makeAvatarId(*bannerAvatar, bannerAvatarId);

			ChatOnBanAvatarFromRoom msg(sequence, errorCode, roomName, bannerAvatarId, bannee);
			instance().chatInterface->sendMessageToAvatar(bannerAvatarId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::invite(const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName)
{
	canonicalizeAvatarId(avatarId);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "invite() id(%s) avatar(%s) roomName(%s)", id.getValueString().c_str(), avatarId.getFullName().c_str(), roomName.c_str());

	// Try to get an invitor
	ChatAvatar const * invitor = nullptr;
	if (id != NetworkId::cms_invalid)
	{
		invitor = getAvatarByNetworkId(id);
	}
	else
	{
		invitor = instance().ownerSystem;
	}

	if (invitor)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		ChatServerRoomOwner const * const roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String inviteName;
			Unicode::String inviteAddress;
			splitChatAvatarId(avatarId, inviteName, inviteAddress);

			// Remove the avatar from the banned list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, avatarId);

				instance().chatInterface->RequestRemoveBan(invitor, inviteName, inviteAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Invite the avatar
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, avatarId);

				unsigned track = instance().chatInterface->RequestAddInvite(invitor, inviteName, inviteAddress, roomOwner->getAddress(), (void *)pair);
				instance().pendingRequests[track] = id;
			}
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId invitorId;
			makeAvatarId(*invitor, invitorId);

			ChatOnInviteToRoom msg(errorCode, roomName, invitorId, avatarId);
			instance().chatInterface->sendMessageToAvatar(invitorId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::inviteGroupMembers(const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName, const stdvector<NetworkId>::fwd & members)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "inviteGroupMembers() id(%s) avatarId(%s) roomName(%s) number of members (%u)", id.getValueString().c_str(), avatarId.getFullName().c_str(), roomName.c_str(), members.size());

	// Try to get an invitor
	ChatAvatar const * const invitor = getAvatarByNetworkId(id);
	if (invitor)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		ChatServerRoomOwner const * const roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			// Invite each member
			for (unsigned i = 0; i < members.size(); ++i)
			{
				NetworkId const &        memberNetworkId = members[i];
				ChatAvatar const * const memberAvatar    = getAvatarByNetworkId(memberNetworkId);
				if (memberAvatar)
				{
					ChatAvatarId memberId;
					makeAvatarId(*memberAvatar, memberId);

					// Break up the member name
					Unicode::String inviteName;
					Unicode::String inviteAddress;
					splitChatAvatarId(memberId, inviteName, inviteAddress);

					// Remove the avatar from the banned list
					{
						AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, memberId);

						instance().chatInterface->RequestRemoveBan(invitor, inviteName, inviteAddress, roomOwner->getAddress(), (void *)pair);
					}

					// Invite the avatar
					{
						AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, memberId);

						unsigned track = instance().chatInterface->RequestAddInvite(invitor, inviteName, inviteAddress, roomOwner->getAddress(), (void *)pair);
						instance().pendingRequests[track] = id;
					}
				}
			}
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId invitorId;
			makeAvatarId(*invitor, invitorId);

			ChatOnInviteGroupToRoom msg(errorCode, roomName, invitorId, avatarId);
			instance().chatInterface->sendMessageToAvatar(invitorId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeSystemAvatarFromRoom(const ChatRoom *room)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "removeSystemAvatarFromRoom() room(%s)", (room != nullptr) ? toNarrowString(room->getRoomName()).c_str() : "nullptr");

	if (!room || !instance().ownerSystem)
	{
		return;
	}

	AvatarIterator i = room->getFirstAvatar();
	if (!i.outOfBounds())
	{
		if ((*i) != instance().ownerSystem)
		{
			return;
		}
	}

	instance().chatInterface->RequestLeaveRoom(instance().ownerSystem, room->getAddress(), nullptr);
}

//-----------------------------------------------------------------------

void ChatServer::leaveRoom(const NetworkId & id, const unsigned int sequence, const unsigned int roomId)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "leaveRoom() id(%s) sequence(%u) roomId(%u)", id.getValueString().c_str(), sequence, roomId);

	const ChatAvatar * avatar = getAvatarByNetworkId(id);
	if (avatar)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		// Try to get the room
		ChatServerRoomOwner const * const roomOwner = instance().chatInterface->getRoomOwner(roomId);
		if (roomOwner)
		{
			unsigned track = instance().chatInterface->RequestLeaveRoom(avatar, roomOwner->getAddress(), (void *)sequence);
			instance().pendingRequests[track] = id;
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId avatarId;
			makeAvatarId(*avatar, avatarId);

			ChatOnLeaveRoom msg(sequence, errorCode, roomId, avatarId);
			instance().chatInterface->sendMessageToAvatar(avatarId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeAvatarFromRoom(const NetworkId & id, const ChatAvatarId & avatarName, const std::string & roomName)
{
	canonicalizeAvatarId(avatarName);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "removeAvatarFromRoom() id(%s) avatarName(%s) roomName(%s)", id.getValueString().c_str(), avatarName.getFullName().c_str(), roomName.c_str());

	const ChatAvatar * avatar = getAvatarByNetworkId(id);

	if (avatar != nullptr) {
		ChatAvatarId removerId;
		makeAvatarId(*avatar, removerId);
		size_t pos = roomName.rfind(".");
		std::string leaf;
		if (pos != std::string::npos)
		{
			leaf = toLower(roomName.substr(pos));
		}
		//REPORT_LOG(true, ("removeAvatarFromRoom()\n"));

		if (avatar)
		{
			bool removeAvatar = false;
			if (avatarName == removerId)
			{
				// avatar is attempting to remove self from the room
				if (leaf != ".system")
					removeAvatar = true;
			}
			else if (isGod(id))
			{
				removeAvatar = true;
			}
			else
			{
				const ChatServerRoomOwner * r = instance().chatInterface->getRoomByName(roomName);
				if (r)
				{
					// if the avatar is the room owner, remove the target
					if (r->getRoomData().owner == removerId)
					{
						removeAvatar = true;
					}
					// if the avatar is the room creator, remove the target
					else if (r->getRoomData().creator == removerId)
					{
						removeAvatar = true;
					}
					// if the avatar is a moderator, remove the target
					else
					{
						/*MLSTODO					if(r->getRoom())
											{
											AvatarIterator i = r->getRoom()->getFirstModerator();
											for(; !i.outOfBounds(); ++i)
											{
											ChatAvatarId tmpAvatarId;
											makeAvatarId(*(*i), tmpAvatarId);
											if(tmpAvatarId == removerId)
											{
											removeAvatar = true;
											break;
											}
											}
											}
											*/
					}
				}
			}
			if (removeAvatar)
			{
				const ChatServerRoomOwner * r = instance().chatInterface->getRoomByName(roomName);
				const NetworkId &id = getNetworkIdByAvatarId(avatarName);
				if (r)
				{
					leaveRoom(id, 0, r->getRoomData().id);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::removeAvatarFromRoom(const ChatAvatarId & avatarName, const std::string & roomName)
{
	canonicalizeAvatarId(avatarName);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "removeAvatarFromRoom() avatarName(%s) roomName(%s)", avatarName.getFullName().c_str(), roomName.c_str());

	const ChatServerRoomOwner * r = instance().chatInterface->getRoomByName(roomName);
	const NetworkId &id = getNetworkIdByAvatarId(avatarName);
	if (r)
	{
		leaveRoom(id, 0, r->getRoomData().id);
	}
}

//-----------------------------------------------------------------------

void ChatServer::kickAvatarFromRoom(const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName)
{
	canonicalizeAvatarId(avatarId);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "kickAvatarFromRoom() moderator(%s) avatarName(%s) roomName(%s)", id.getValueString().c_str(), avatarId.getFullName().c_str(), roomName.c_str());

	unsigned errorCode = CHATRESULT_SUCCESS;

	ChatAvatar const * const moderatorChatAvatar = getAvatarByNetworkId(id);
	if (moderatorChatAvatar)
	{
		ChatServerRoomOwner const * const roomOwner = instance().chatInterface->getRoomByName(roomName);
		if (roomOwner)
		{
			Unicode::String kickName;
			Unicode::String kickAddress;
			splitChatAvatarId(avatarId, kickName, kickAddress);

			// Remove the avatar from the invited list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, avatarId);
				instance().chatInterface->RequestRemoveInvite(moderatorChatAvatar, kickName, kickAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Remove the avatar from the moderator list
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, avatarId);
				instance().chatInterface->RequestRemoveModerator(moderatorChatAvatar, kickName, kickAddress, roomOwner->getAddress(), (void *)pair);
			}

			// Kick the avatar from the room
			{
				AvatarIdSequencePair *pair = new AvatarIdSequencePair(0, avatarId);

				unsigned track = instance().chatInterface->RequestKickAvatar(moderatorChatAvatar, kickName, kickAddress, roomOwner->getAddress(), (void*)pair);
				instance().pendingRequests[track] = id;
			}
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId moderatorAvatarId;
			makeAvatarId(*moderatorChatAvatar, moderatorAvatarId);

			ChatOnKickAvatarFromRoom msg(errorCode, moderatorAvatarId, avatarId, roomName);
			instance().chatInterface->sendMessageToAvatar(moderatorAvatarId, msg);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::queryRoom(const NetworkId & id, ConnectionServerConnection * connection, const unsigned int sequence, const std::string & roomName)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "queryRoom() id(%s) roomName(%s)", id.getValueString().c_str(), roomName.c_str());

	instance().chatInterface->queryRoom(id, connection, sequence, roomName);
}

//-----------------------------------------------------------------------

void ChatServer::requestPersistentMessage(const NetworkId &id, const unsigned int sequence, const unsigned int messageId)
{
	ChatServer::fileLog(false, "ChatServer", "requestPersistentMessage() id(%s) sequence(%u) messageId(%u)", id.getValueString().c_str(), sequence, messageId);

	const ChatAvatar * avatar = getAvatarByNetworkId(id);

	if (avatar)
	{
		IGNORE_RETURN(instance().chatInterface->RequestGetPersistentMessage(avatar, messageId, (void *)sequence));
	}
	else
	{
		LOG("ChatServer", ("requestPersistentMessage() id(%s) Unable to resolve network id to ChatAvatar, request failed", id.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void ChatServer::requestRoomList(const NetworkId & id, ConnectionServerConnection * connection)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "requestRoomList() id(%s) connection(%s)", id.getValueString().c_str(), getConnectionAddress(connection).c_str());

	instance().chatInterface->requestRoomList(id, connection);
}

//-----------------------------------------------------------------------

//This is for system messages only
void ChatServer::sendInstantMessage(const ChatAvatarId & from, const ChatAvatarId & to, const Unicode::String & message, const Unicode::String & outOfBand)
{
	canonicalizeAvatarId(from);
	canonicalizeAvatarId(to);
	ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() from(%s) to(%s)", from.getFullName().c_str(), to.getFullName().c_str());

	const NetworkId &toNetworkId = getNetworkIdByAvatarId(to);
	const ChatAvatar * toAvatar = getAvatarByNetworkId(toNetworkId);

	if (toAvatar != nullptr)
	{
		ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() resolved tell locally");

		ChatAvatarId toId;
		makeAvatarId(*toAvatar, toId);
		ChatSystemMessage chat(ChatSystemMessage::PERSONAL, message, outOfBand); //lint !e641 enum to int
		instance().chatInterface->sendMessageToAvatar(toId, chat);
	}
	else
	{
		ChatSystemMessage chat(ChatSystemMessage::PERSONAL, message, outOfBand); //lint !e641 enum to int

		bool success = instance().chatInterface->sendMessageToPendingAvatar(to, chat);

		if (!success)
		{
			//I don't know who this is, send it to the chat api
			const NetworkId &fromNetworkId = getNetworkIdByAvatarId(from);
			const ChatAvatar * fromAvatar = getAvatarByNetworkId(fromNetworkId);
			if (fromAvatar)
			{
				ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() sending tell through chat API");

				Unicode::String friendName;
				Unicode::String friendAddress;

				splitChatAvatarId(to, friendName, friendAddress);
				IGNORE_RETURN(instance().chatInterface->RequestSendInstantMessage(fromAvatar,
					ChatUnicodeString(friendName.data(), friendName.size()),
					ChatUnicodeString(friendAddress.data(), friendAddress.size()),
					ChatUnicodeString(message.data(), message.size()), ChatUnicodeString(outOfBand.data(), outOfBand.size()), nullptr));
			}
		}
	}

	Unicode::String const wideTo(Unicode::narrowToWide(to.getFullName()));
	Unicode::String const wideFrom(Unicode::narrowToWide(from.getFullName()));

	if(from != getSystemAvatarId())
	{
		ChatServer::instance().logChatMessage(wideTo, wideFrom, wideTo, message, s_channelTell);
		ChatServer::instance().logChatMessage(wideFrom, wideFrom, wideTo, message, s_channelTell);
	}
}

//-----------------------------------------------------------------------

void ChatServer::sendInstantMessage(const NetworkId & fromId, const unsigned int sequence, const ChatAvatarId & n, const Unicode::String & message, const Unicode::String & oob)
{
	canonicalizeAvatarId(n);
	ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() fromId(%s) sequence(%u) toName(%s)", fromId.getValueString().c_str(), sequence, n.getFullName().c_str());

	ChatServer::AvatarExtendedData * aed = getAvatarExtendedDataByNetworkId(fromId);
	if (aed)
	{
		// see if player is squelched
		if (aed->unsquelchTime != 0)
		{
			if (aed->unsquelchTime < 0) // squelched indefinitely
			{
				return;
			}
			else if (::time(nullptr) < aed->unsquelchTime) // still in squelch period
			{
				return;
			}
			else // squelch has expired
			{
				aed->unsquelchTime = 0;
			}
		}

		unsigned int const maxTellLength = 512;

		if(message.length() < maxTellLength)
		{
			const ChatAvatar * from = &(aed->chatAvatar);
			const NetworkId &toNetworkId = getNetworkIdByAvatarId(n);
			const ChatAvatar * toAvatar = getAvatarByNetworkId(toNetworkId);
			ChatAvatarId fromAvatarId;

			Unicode::String wideFrom;
			Unicode::String wideTo;

			if (from != nullptr)
			{
				makeAvatarId(*from, fromAvatarId);
				wideFrom = (Unicode::narrowToWide(fromAvatarId.getFullName()));
			}

			ChatAvatarId toAvatarId;
			if (toAvatar != nullptr)
			{
				ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() resolved tell locally");

				makeAvatarId(*toAvatar, toAvatarId);
				wideTo = Unicode::narrowToWide(toAvatarId.getFullName());

				if (from)
				{
					ChatInstantMessageToClient chat(fromAvatarId, message, oob);
					instance().chatInterface->sendMessageToAvatar(toAvatarId, chat);

					ChatOnSendInstantMessage chat2(sequence, 0);
					IGNORE_RETURN(instance().chatInterface->sendMessageToAvatar(fromAvatarId, chat2));
				}

				if(fromAvatarId != getSystemAvatarId())
				{
					ChatServer::instance().logChatMessage(wideTo, wideFrom, wideTo, message, s_channelTell);
				}
			}
			else
			{
				ChatServer::fileLog(false, "ChatServer", "sendInstantMessage() sending tell through chat API");

				wideTo = Unicode::narrowToWide(n.getFullName());
				Unicode::String friendName;
				Unicode::String friendAddress;

				splitChatAvatarId(n, friendName, friendAddress);

				unsigned track = instance().chatInterface->RequestSendInstantMessage(from,
					ChatUnicodeString(friendName.data(), friendName.size()),
					ChatUnicodeString(friendAddress.data(), friendAddress.size()),
					ChatUnicodeString(message.data(), message.size()), ChatUnicodeString(oob.data(), oob.size()), (void *)sequence);

				instance().pendingRequests[track] = fromId;
			}

			if(fromAvatarId != getSystemAvatarId())
			{
				ChatServer::instance().logChatMessage(wideFrom, wideFrom, wideTo, message, s_channelTell);
			}
		}
		else
		{
			ChatServer::fileLog(true, "ChatServer", "sendInstantMessage() ERROR: Unable to send tell longer than %u", maxTellLength);

			//ChatOnSendInstantMessage chat(sequence, CHAT_RESULT_FAIL_INSUFFICIENT_PRIVILEGES); //lint !e641 enum to int
			//sendToClient(fromId, chat);
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::sendPersistentMessage(const ChatAvatarId & from, const ChatAvatarId & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob)
{
	canonicalizeAvatarId(from);
	canonicalizeAvatarId(to);

	ChatServer::fileLog(false, "ChatServer", "sendPersistentMessage() from(%s) to(%s) subject(%s)", from.getFullName().c_str(), to.getFullName().c_str(), Unicode::wideToNarrow(subject).c_str());

	Unicode::String wideFromName(Unicode::narrowToWide(from.name));
	Unicode::String friendName;
	Unicode::String friendAddress;

	splitChatAvatarId(to, friendName, friendAddress);
	instance().chatInterface->RequestSendPersistentMessage(
		ChatUnicodeString(wideFromName.data(), wideFromName.size()),
		ChatUnicodeString(friendName.data(), friendName.size()),
		ChatUnicodeString(friendAddress.data(), friendAddress.size()),
		ChatUnicodeString(subject.data(), subject.size()),
		ChatUnicodeString(message.data(), message.size()),
		ChatUnicodeString(oob.data(), oob.size()),
		nullptr
		);

	Unicode::String log;
	log.append(Unicode::narrowToWide("[subject] "));
	log.append(subject);
	log.append(Unicode::narrowToWide(" "));
	log.append(Unicode::narrowToWide("[message] "));
	log.append(message);

	Unicode::String const wideTo(Unicode::narrowToWide(to.getFullName()));
	Unicode::String const wideFrom(Unicode::narrowToWide(from.getFullName()));

	ChatServer::instance().logChatMessage(wideTo, wideFrom, wideTo, log, ChatServer::getChannelEmail());
	ChatServer::instance().logChatMessage(wideFrom, wideFrom, wideTo, log, ChatServer::getChannelEmail());
}

//-----------------------------------------------------------------------

void ChatServer::sendPersistentMessage(const NetworkId & fromId, const unsigned int sequenceId, const ChatAvatarId & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob)
{
	canonicalizeAvatarId(to);

	ChatServer::fileLog(false, "ChatServer", "sendPersistentMessage() fromId(%s) sequence(%u) to(%s) subject(%s)", fromId.getValueString().c_str(), sequenceId, to.getFullName().c_str(), Unicode::wideToNarrow(subject).c_str());

//	printf("Sending a message to %s.%s.%s\n", to.gameCode.c_str(),
//		to.cluster.c_str(), to.name.c_str());
	UNREF(sequenceId);
	ChatServer::AvatarExtendedData * aed = getAvatarExtendedDataByNetworkId(fromId);
	const ChatAvatar * from = (aed ? &(aed->chatAvatar) : nullptr);
	if(from)
	{
		// see if player is squelched
		if (aed->unsquelchTime != 0)
		{
			if (aed->unsquelchTime < 0) // squelched indefinitely
			{
				return;
			}
			else if (::time(nullptr) < aed->unsquelchTime) // still in squelch period
			{
				return;
			}
			else // squelch has expired
			{
				aed->unsquelchTime = 0;
			}
		}

		Unicode::String friendName;
		Unicode::String friendAddress;

		splitChatAvatarId(to, friendName, friendAddress);
		unsigned track = instance().chatInterface->RequestSendPersistentMessage(
			from,
			ChatUnicodeString(friendName.data(), friendName.size()),
			ChatUnicodeString(friendAddress.data(), friendAddress.size()),
			ChatUnicodeString(subject.data(), subject.size()),
			ChatUnicodeString(message.data(), message.size()),
			ChatUnicodeString(oob.data(), oob.size()),
			(void *)sequenceId
			);
		instance().pendingRequests[track] = fromId;
	}

	ChatAvatarId fromAvatarId;

	if (from != nullptr)
	{
		makeAvatarId(*from, fromAvatarId);
	}

	Unicode::String log;
	log.append(Unicode::narrowToWide("[subject] "));
	log.append(subject);
	log.append(Unicode::narrowToWide(" "));
	log.append(Unicode::narrowToWide("[message] "));
	log.append(message);

	Unicode::String const wideTo(Unicode::narrowToWide(to.getFullName()));
	Unicode::String const wideFrom(Unicode::narrowToWide(fromAvatarId.getFullName()));

	ChatServer::instance().logChatMessage(wideTo, wideFrom, wideTo, log, ChatServer::getChannelEmail());
	ChatServer::instance().logChatMessage(wideFrom, wideFrom, wideTo, log, ChatServer::getChannelEmail());
}

//-----------------------------------------------------------------------
//for system avatars
void ChatServer::sendRoomMessage(const ChatAvatarId &id, const std::string & roomName, const Unicode::String & msg, const Unicode::String & oob)
{
	if(! msg.empty() || !oob.empty())
	{
		UNREF(id);
		ChatServer::fileLog(false, "ChatServer", "sendRoomMessage() id(%s) roomName(%s)", id.getFullName().c_str(), roomName.c_str());
	
		if (!instance().ownerSystem)
		{
			DEBUG_WARNING(true, ("Chat ownerSystem is nullptr"));
			return;
		}
		// get room id
		std::string lowerName = toLower(roomName);
		std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator f = instance().chatInterface->getRoomList().find(lowerName);
		if(f != instance().chatInterface->getRoomList().end())
		{
			const ChatAvatar *sender = instance().ownerSystem;
			
			IGNORE_RETURN(instance().chatInterface->RequestSendRoomMessage(sender, (*f).second.getAddress(), ChatUnicodeString(msg.data(), msg.size()), ChatUnicodeString(oob.data(), oob.size()), nullptr));
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::sendRoomMessage(const NetworkId & id, const unsigned int sequence, const unsigned int roomId, const Unicode::String &msg, const Unicode::String & oob)
{
	if(!msg.empty() || !oob.empty())
	{
		ChatServer::fileLog(false, "ChatServer", "sendRoomMessage() id(%s) sequence(%u) roomId(%u)", id.getValueString().c_str(), sequence, roomId);
	
		UNREF(sequence);
		ChatServer::AvatarExtendedData * aed = getAvatarExtendedDataByNetworkId(id);
		const ChatAvatar * sender = (aed ? &(aed->chatAvatar) : nullptr);
		const ChatServerRoomOwner *room = instance().chatInterface->getRoomOwner(roomId);
		if(sender && room)
		{
			if(msg.length() < 512)
			{
				// track amount of room chat for the character, except for group chat
				time_t timeNow = 0;
				bool allowToSpeak = true;
				bool squelched = false;
				if (!room->isGroupChatRoom())
				{
					// update chat character count
					aed->nonSpatialCharCount += msg.size();

					// sync chat character count with game server
					timeNow = ::time(nullptr);
					if ((timeNow > aed->chatSpamNextTimeToSyncWithGameServer) || (timeNow > aed->chatSpamTimeEndInterval))
					{
						GenericValueTypeMessage<std::pair<std::pair<NetworkId, int>, std::pair<int, int> > > chatStatistics("ChatStatisticsCS", std::make_pair(std::make_pair(id, static_cast<int>(aed->chatSpamTimeEndInterval)), std::make_pair(aed->spatialCharCount, aed->nonSpatialCharCount)));
						sendToClient(id, chatStatistics);
						aed->chatSpamNextTimeToSyncWithGameServer = timeNow + ConfigChatServer::getChatStatisticsReportIntervalSeconds();
					}

					// see if player has exceeded chat limit
					if (ConfigChatServer::getChatSpamLimiterEnabledForFreeTrial() &&
						!aed->isSubscribed &&
						(aed->chatSpamTimeEndInterval > timeNow) &&
						((aed->spatialCharCount + aed->nonSpatialCharCount) > ConfigChatServer::getChatSpamLimiterNumCharacters()))
					{
						allowToSpeak = false;
					}
				}

				// see if player is squelched
				if (allowToSpeak && (aed->unsquelchTime != 0))
				{
					if (aed->unsquelchTime < 0) // squelched indefinitely
					{
						allowToSpeak = false;
						squelched = true;
					}
					else if (::time(nullptr) < aed->unsquelchTime) // still in squelch period
					{
						allowToSpeak = false;
						squelched = true;
					}
					else // squelch has expired
					{
						aed->unsquelchTime = 0;
					}
				}

				if (allowToSpeak)
				{
					// character allowed to talk
					unsigned track = instance().chatInterface->RequestSendRoomMessage(sender, room->getAddress(), ChatUnicodeString(msg.data(), msg.size()), ChatUnicodeString(oob.data(), oob.size()), (void  *)sequence);
					instance().pendingRequests[track] = id;
				}
				else if (!squelched && (ConfigChatServer::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds() > 0) && (timeNow >= aed->chatSpamNextTimeToNotifyPlayerWhenLimited))
				{
					// send message telling character he can no longer talk
					GenericValueTypeMessage<int> csl("ChatSpamLimited", static_cast<int>(aed->chatSpamTimeEndInterval - timeNow));
					sendToClient(id, csl);

					aed->chatSpamNextTimeToNotifyPlayerWhenLimited = timeNow + ConfigChatServer::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds();
				}
			}
			else
			{
				//ChatOnSendRoomMessage chat(sequence, CHAT_RESULT_FAIL_INSUFFICIENT_PRIVILEGES); //lint !e641 enum to int
				//sendToClient(id, chat);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatServer::sendStandardRoomMessage(const ChatAvatarId &senderId, const std::string & roomName, const Unicode::String & msg, const Unicode::String & oob)
{
	if(!msg.empty() || !oob.empty())
	{
		canonicalizeAvatarId(senderId);
		ChatServer::fileLog(false, "ChatServer", "sendStandardRoomMessage() senderId(%s) roomName(%s)", senderId.getFullName().c_str(), roomName.c_str());
	
		std::string systemRoom = toLower(std::string("SWG.") + ConfigChatServer::getClusterName() + std::string(".system"));
	
		// get room id
		std::string lowerName = toLower(roomName);
		if (lowerName == systemRoom)
		{
			ChatSystemMessage chat(ChatSystemMessage::BROADCAST, msg,
				oob); //lint !e641 enum to int
			instance().chatInterface->sendMessageToAllAvatars(chat);
			return;
		}
	
		std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator f = instance().chatInterface->getRoomList().find(lowerName);
		if(f != instance().chatInterface->getRoomList().end())
		{
			const ChatAvatar *sender;
	
			if (senderId.name == "SYSTEM" || senderId.name == "system")
			{
				sender = instance().ownerSystem;
			}
			else
			{
				sender = getAvatarByNetworkId(getNetworkIdByAvatarId(senderId));
			}
			if (sender)
			{
				IGNORE_RETURN(instance().chatInterface->RequestSendRoomMessage(sender, (*f).second.getAddress(), ChatUnicodeString(msg.data(), msg.size()), ChatUnicodeString(oob.data(), oob.size()),  nullptr));
			}
		}
	}
}


//-----------------------------------------------------------------------

void ChatServer::uninvite(const NetworkId &id , const unsigned int sequence, const ChatAvatarId &avatarId , const std::string &roomName )
{
	canonicalizeAvatarId(avatarId);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatServer", "uninvite() id(%s) sequence(%u) avatarId(%s) roomName(%s)", id.getValueString().c_str(), sequence, avatarId.getFullName().c_str(), roomName.c_str());

	// Try to get the invitor
	ChatAvatar const * invitor = getAvatarByNetworkId(id);
	if (invitor)
	{
		unsigned errorCode = CHATRESULT_SUCCESS;

		// Try to get the room
		ChatServerRoomOwner const * const roomOwner = instance().chatInterface->getRoomOwner(roomName);
		if (roomOwner)
		{
			Unicode::String inviteName;
			Unicode::String inviteAddress;
			splitChatAvatarId(avatarId, inviteName, inviteAddress);

			AvatarIdSequencePair * pair = new AvatarIdSequencePair(sequence, avatarId);

			unsigned track = instance().chatInterface->RequestRemoveInvite(invitor, inviteName, inviteAddress, roomOwner->getAddress(), (void *)pair);
			instance().pendingRequests[track] = id;
		}
		else
		{
			errorCode = CHATRESULT_ADDRESSNOTROOM;
		}

		if (errorCode != CHATRESULT_SUCCESS)
		{
			ChatAvatarId invitorId;
			makeAvatarId(*invitor, invitorId);

			ChatOnUninviteFromRoom msg(sequence, errorCode, roomName, invitorId, avatarId);
			instance().chatInterface->sendMessageToAvatar(invitorId, msg);
		}
	}
}


//-----------------------------------------------------------------------

void ChatServer::removeConnectionServerConnection(ConnectionServerConnection * target)
{
	ChatServer::fileLog(false, "ChatServer", "removeConnectionServerConnection() target(%s)", getConnectionAddress(target).c_str());

	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash> tmpList = instance().clientMap;
	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::const_iterator i;
	for(i = tmpList.begin(); i != tmpList.end(); ++i)
	{
		if((*i).second == target)
			disconnectPlayer((*i).first);
		std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>::iterator f = instance().clientMap.find((*i).first);
		if (f != instance().clientMap.end())
		{
			instance().clientMap.erase(f);
		}
	}

	std::set<ConnectionServerConnection *>::iterator ci = instance().connectionServerConnections.find(target);
	if (ci != instance().connectionServerConnections.end())
	{
		instance().connectionServerConnections.erase(ci);
	}
	s_chatServerMetricsData->setConnectionServerConnectionCount(instance().connectionServerConnections.size());
}

//-----------------------------------------------------------------------

void  ChatServer::sendToAllConnectionServers(const GameNetworkMessage &message)
{
	ChatServer::fileLog(false, "ChatServer", "sendToAllConnectionServers() message(%s)", message.getCmdName().c_str());

	std::set<ConnectionServerConnection *>::const_iterator i;
	for(i = instance().connectionServerConnections.begin(); i != instance().connectionServerConnections.end(); ++i)
	{
		ConnectionServerConnection * c = (*i);
		c->sendToClient(NetworkId::cms_invalid, message);
	}
}

// ----------------------------------------------------------------------

void ChatServer::broadcastToGameServers(const GameNetworkMessage &message)
{
	ChatServer::fileLog(false, "ChatServer", "broadcastToGameServers() message(%s)", message.getCmdName().c_str());

	if (instance().gameService)
	{
		static Archive::ByteStream bs;
		bs.clear();
		message.pack(bs);
		instance().gameService->broadcast(bs, true);
	}
}

// ----------------------------------------------------------------------

std::string ChatServer::getFullChatAvatarName(ChatAvatar const *chatAvatar)
{
	std::string result;

	if (chatAvatar != nullptr)
	{
		result += toNarrowString(chatAvatar->getName());
		result += '.';
		result += toNarrowString(chatAvatar->getAddress());
	}
	else
	{
		result = "nullptr";
	}

	return result;
}

// ----------------------------------------------------------------------

std::string ChatServer::toNarrowString(ChatUnicodeString const &chatUnicodeString)
{
	std::string result;

	for (unsigned int index = 0; index < chatUnicodeString.string_length; ++index)
	{
		result += static_cast<char>(chatUnicodeString.string_data[index]);
	}

	return result;
}

// ----------------------------------------------------------------------

std::string ChatServer::getConnectionAddress(Connection const *connection)
{
	std::string result;

	if (connection != nullptr)
	{
		char text[256];
		snprintf(text, sizeof(text), "%s:%d", connection->getRemoteAddress().c_str(), connection->getRemotePort());
		result = text;
	}
	else
	{
		result = "nullptr";
	}

	return result;
}

// ----------------------------------------------------------------------

std::string ChatServer::getChatRoomNameNarrow(ChatRoom const *chatRoom)
{
	return Unicode::wideToNarrow(getChatRoomName(chatRoom));
}

// ----------------------------------------------------------------------

Unicode::String ChatServer::getChatRoomName(ChatRoom const *chatRoom)
{
	Unicode::String result;

	if (chatRoom != nullptr)
	{
		result = toUnicodeString(chatRoom->getRoomName());
	}
	else
	{
		result = Unicode::narrowToWide("nullptr");
	}

	return result;
}

// ----------------------------------------------------------------------

void ChatServer::clearCustomerServiceServerConnection()
{
	if (instance().customerServiceServerConnection != nullptr)
	{
		instance().customerServiceServerConnection->disconnect();
	}

	instance().customerServiceServerConnection = nullptr;
}

// ----------------------------------------------------------------------

void ChatServer::connectToCustomerServiceServer(const std::string &address, const unsigned short port)
{
	//DEBUG_REPORT_LOG(true, ("***ChatServer::connectToCustomerServiceServer() address(%s) port(%d)\n", address.c_str(), port));

	if (instance().customerServiceServerConnection != nullptr)
	{
		instance().customerServiceServerConnection->disconnect();
	}

	instance().customerServiceServerConnection = new CustomerServiceServerConnection(address, port);
}

// ----------------------------------------------------------------------

bool ChatServer::isValidChatAvatarName(Unicode::String const &chatAvatarName)
{
	// This function is retarded. We need to get an API call to determine if a chat avatar name is valid

	std::string lowerNarrowName(Unicode::toLower(Unicode::wideToNarrow(chatAvatarName)));

	//DEBUG_REPORT_LOG(true, ("ChatServer::isValidChatAvatarName() chatAvatarName(%s)\n", lowerNarrowName.c_str()));

	bool result = false;
	ChatAvatarList::const_iterator iterChatAvatars = instance().chatAvatars.begin();

	for (; iterChatAvatars != instance().chatAvatars.end(); ++iterChatAvatars)
	{
		ChatAvatar const *chatAvatar = &(iterChatAvatars->second.chatAvatar);

		if (toNarrowString(chatAvatar->getName()) == lowerNarrowName)
		{
			result = true;
			break;
		}
	}

	if (result == false)
	{
		// How many dots are in this name

		int dotCount = 0;

		for (unsigned int i = 0; i < lowerNarrowName.length(); ++i)
		{
			if (lowerNarrowName[i] == '.')
			{
				++dotCount;
			}
		}

		// Tack on extra code info for the name match
		
		Unicode::String const wideDot(Unicode::narrowToWide("."));
		Unicode::String searchAvatarName;

		if (dotCount == 0)
		{
			searchAvatarName = Unicode::narrowToWide(instance().systemAvatarId.gameCode) + wideDot + Unicode::narrowToWide(instance().systemAvatarId.cluster) + wideDot + chatAvatarName;
		}
		else if (dotCount == 1)
		{
			searchAvatarName = Unicode::narrowToWide(instance().systemAvatarId.gameCode) + wideDot + chatAvatarName;
		}
		else
		{
			searchAvatarName = chatAvatarName;
		}

		result = ChatLogManager::isPlayerInLogs(searchAvatarName);
	}

	return result;
}

// ----------------------------------------------------------------------

void ChatServer::logChatMessage(Unicode::String const &logPlayer, Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &text, Unicode::String const &channel)
{
	DEBUG_WARNING(toPlayer.empty(), ("toPlayer is nullptr"));

	Unicode::String lowerLogPlayer(Unicode::toLower(logPlayer));
	Unicode::String lowerFromPlayer(Unicode::toLower(fromPlayer));
	Unicode::String lowerToPlayer(Unicode::toLower(toPlayer));

	ChatServer::fileLog(false, "ChatServer", "logChatMessage() logPlayer(%s) fromPlayer(%s) toPlayer(%s) channel(%s) text(%s)", Unicode::wideToNarrow(lowerLogPlayer).c_str(), Unicode::wideToNarrow(lowerFromPlayer).c_str(), Unicode::wideToNarrow(lowerToPlayer).c_str(), Unicode::wideToNarrow(channel).c_str(), Unicode::wideToNarrow(text).c_str());

	int const messageIndex = ChatLogManager::getNextMessageIndex();
	time_t const time = Os::getRealSystemTime();
	
	ChatLogManager::logChat(lowerFromPlayer, lowerToPlayer, text, channel, messageIndex, time);

	PlayerMessageList::iterator iterPlayerMessageList = s_playerMessageList.find(lowerLogPlayer);
	
	if (iterPlayerMessageList != s_playerMessageList.end())
	{
		// The player already has some messages logged, append the message

		iterPlayerMessageList->second.push_back(std::make_pair(messageIndex, time));
	
		ChatServer::fileLog(false, "ChatServer", "logChatMessage() player(%s) messageIndex(%d) time(%u) messageCount(%u)", Unicode::wideToNarrow(lowerLogPlayer).c_str(), messageIndex, time, iterPlayerMessageList->second.size());
	}
	else
	{
		// The player does not have any message logged, add the first one

		MessageList messageList;
		messageList.push_back(std::make_pair(messageIndex, time));

		s_playerMessageList.insert(std::make_pair(lowerLogPlayer, messageList));

		ChatServer::fileLog(false, "ChatServer", "logChatMessage() player(%s) messageIndex(%d) time(%u) first message entered", Unicode::wideToNarrow(lowerLogPlayer).c_str(), messageIndex, time);
	}
}

// ----------------------------------------------------------------------

Unicode::String ChatServer::toUnicodeString(ChatUnicodeString const &chatUnicodeString)
{
	return Unicode::String(chatUnicodeString.string_data, chatUnicodeString.string_length);
}

// ----------------------------------------------------------------------

Unicode::String const &ChatServer::getChannelTell()
{
	return s_channelTell;
}

// ----------------------------------------------------------------------

Unicode::String const &ChatServer::getChannelEmail()
{
	return s_channelEmail;
}

// ----------------------------------------------------------------------

void ChatServer::fileLog(bool const forceLog, char const * const label, char const * const format, ...)
{
	if (   forceLog
	    || s_loggingEnabled)
	{
		char text[8192];

		va_list va;
		va_start(va, format);
			_vsnprintf(text, sizeof(text), format, va);
			text[sizeof(text) - 1] = '\0';
		va_end(va);

		LOG(label, (text));
	}
}

// ----------------------------------------------------------------------

void ChatServer::requestTransferAvatar(const TransferCharacterData & request)
{
	if(instance().chatInterface)
	{
		ChatAvatarId sourceAvatar("SWG", request.getSourceGalaxy(), request.getSourceCharacterName());
		
		std::string destination = request.getDestinationGalaxy();
		if(destination.empty())
			destination = request.getSourceGalaxy();
			
		ChatAvatarId destinationAvatar("SWG", destination, request.getDestinationCharacterName());
		
		instance().chatInterface->RequestTransferAvatar(request.getSourceStationId(), makeChatUnicodeString(sourceAvatar.getName()), makeChatUnicodeString(sourceAvatar.getAPIAddress()), request.getDestinationStationId(), makeChatUnicodeString(destinationAvatar.getName()), makeChatUnicodeString(destinationAvatar.getAPIAddress()), true, nullptr);
	}
}

// ----------------------------------------------------------------------

void ChatServer::onCreateRoomSuccess(const std::string & lowerName, const unsigned int roomId)
{
	std::set<std::pair<std::string, ChatAvatarId> >::iterator nameIter;
	for(nameIter = s_pendingEntersByName.begin(); nameIter != s_pendingEntersByName.end();)
	{
		if(nameIter->first == lowerName)
		{
			enterRoom(nameIter->second, nameIter->first, false, false);
			s_pendingEntersByName.erase(nameIter++);
		}
		else
		{
			++nameIter;
		}
	}
	
	std::set<std::pair<NetworkId, std::pair<unsigned int, unsigned int> > >::iterator idIter;
	for(idIter = s_pendingEntersById.begin(); idIter != s_pendingEntersById.end();)
	{
		if(idIter->second.second == roomId)
		{
			enterRoom(idIter->first, idIter->second.first, idIter->second.second);
			s_pendingEntersById.erase(idIter++);
		}
		else
		{
			++idIter;
		}
	}
}

// ----------------------------------------------------------------------

bool ChatServer::isGod(const NetworkId & networkId)
{
	bool result = false;
	if(s_godClients.find(networkId) != s_godClients.end())
	{
		result = true;
	}
	return result;
}

// ----------------------------------------------------------------------

void ChatServer::setUnsquelchTime(NetworkId const & character, time_t unsquelchTime)
{
	ChatServer::AvatarExtendedData * aed = getAvatarExtendedDataByNetworkId(character);
	if (aed)
	{
		aed->unsquelchTime = unsquelchTime;
	}
}

// ----------------------------------------------------------------------

void ChatServer::handleChatStatisticsFromGameServer(NetworkId const & character, time_t unsquelchTime, time_t chatSpamTimeEndInterval, int spatialNumCharacters, int nonSpatialNumCharacters)
{
	// update character chat data with chat data from game server
	ChatServer::AvatarExtendedData * aed = getAvatarExtendedDataByNetworkId(character);
	if (aed)
	{
		// squelch state is tracked by the game server
		aed->unsquelchTime = unsquelchTime;

		// spatial chat is tracked by the game server
		aed->spatialCharCount = spatialNumCharacters;
		
		// we are out of sync with the game server, so use
		// all information passed from game server
		if (aed->chatSpamTimeEndInterval != chatSpamTimeEndInterval)
		{
			aed->nonSpatialCharCount = nonSpatialNumCharacters;
			aed->chatSpamTimeEndInterval = chatSpamTimeEndInterval;
		}
		// we are in sync with the game server, and we have additional
		// non-spatial chat to report to the game server
		else if (aed->nonSpatialCharCount != nonSpatialNumCharacters)
		{
			time_t const timeNow = ::time(nullptr);
			if (timeNow > aed->chatSpamNextTimeToSyncWithGameServer)
			{
				GenericValueTypeMessage<std::pair<std::pair<NetworkId, int>, std::pair<int, int> > > chatStatistics("ChatStatisticsCS", std::make_pair(std::make_pair(character, static_cast<int>(aed->chatSpamTimeEndInterval)), std::make_pair(aed->spatialCharCount, aed->nonSpatialCharCount)));
				sendToClient(character, chatStatistics);
				aed->chatSpamNextTimeToSyncWithGameServer = timeNow + ConfigChatServer::getChatStatisticsReportIntervalSeconds();
			}
		}
	}
}

// ======================================================================

VChatInterface* ChatServer::getVChatInterface()
{
	return instance().voiceChatInterface;
}

// ----------------------------------------------------------------------

unsigned ChatServer::registerGameServerConnection(GameServerConnection *connection)
{
	static unsigned nextId = 0;

	++nextId;
	m_gameServerConnectionRegistry.insert(std::make_pair(nextId, connection));
	return nextId;
}

//-----------------------------------------------------------------------

void ChatServer::unregisterGameServerConnection(unsigned const connectionId)
{
	GameServerMap::iterator i = m_gameServerConnectionRegistry.find(connectionId);
	if(i != m_gameServerConnectionRegistry.end())
	{
		m_gameServerConnectionRegistry.erase(i);
	}
}

// ----------------------------------------------------------------------

void ChatServer::sendToGameServerById(unsigned const connectionId, GameNetworkMessage const & message)
{
	GameServerConnection* connection = getGameServerConnectionFromId(connectionId);
	if(connection)
	{
		connection->send(message, true);
	}
	else
	{
		DEBUG_WARNING(true, ("VoiceChatServer::sendToGameServer could not find connection"));
	}
}

//-----------------------------------------------------------------------

GameServerConnection *ChatServer::getGameServerConnectionFromId(unsigned int sequence)
{
	GameServerConnection * result = nullptr;
	std::unordered_map<unsigned int, GameServerConnection *>::iterator f = m_gameServerConnectionRegistry.find(sequence);
	if(f != m_gameServerConnectionRegistry.end())
	{
		result = (*f).second;
	}
	return result;

}

//-----------------------------------------------------------------------

void ChatServer::sendResponse(ReturnAddress const & requester, const GameNetworkMessage & response)
{
	fileLog(false, "ChatServer", "sendResponse() target(%s) responseType(%s)", requester.debugString().c_str(), response.getCmdName().c_str());

	switch(requester.type)
	{
	case ReturnAddress::RAT_invalid: break;
	case ReturnAddress::RAT_client:
		{
			sendToClient(requester.clientId, response);
		}
		break;
	case ReturnAddress::RAT_gameserver:
		{
			sendToGameServerById(requester.gameServerId, response);
		}
		break;
	default:
		DEBUG_WARNING(true, ("ChatServer: Trying to send a response to a return address of unknown type."));
	}
}

//-----------------------------------------------------------------------
//voice chat actions
//TODO: remove these and integrate their functionality where it should rightly go
void ChatServer::requestGetChannel(ReturnAddress const & requester, std::string const &roomName, bool isPublic, bool isPersistant, uint32 limit, std::list<std::string> const & moderators)
{
	fileLog(false, "ChatServer", "request to create room from(%s) name(%s)", requester.debugString().c_str(), roomName.c_str());

	//TODO: real passwords and descriptions?
	const std::string description = "";
	const std::string password = "";

	getVChatInterface()->requestGetChannel(roomName, description, password, limit, isPublic, isPersistant, moderators, requester);
}


void ChatServer::requestChannelInfo(ReturnAddress const & requester, std::string const &roomName)
{
	fileLog(false, "ChatServer", "request for channel info from (%s) channel (%s)", requester.debugString().c_str(), roomName.c_str());
	getVChatInterface()->requestChannelInfo(roomName, requester);
}


//-----------------------------------------------------------------------

void ChatServer::requestDeleteChannel(ReturnAddress const & requester, std::string const & roomName)
{
	getVChatInterface()->requestDeleteChannel(roomName, requester);
}

//-----------------------------------------------------------------------

void ChatServer::requestAddClientToChannel(const NetworkId & id, std::string const & playerName, std::string const &roomName, bool forceShortlist)
{
	fileLog(false, "ChatServer", "requestAddClientToChannel id(%s) name(%s) roomName(%s) forceShortlist(%i)", id.getValueString().c_str(), playerName.c_str(), roomName.c_str(), forceShortlist ? 1 : 0);
	getVChatInterface()->addClientToChannel(roomName, id, playerName, forceShortlist);
}

//-----------------------------------------------------------------------

void ChatServer::requestRemoveClientFromChannel(const NetworkId & id, std::string const & playerName, std::string const &roomName)
{
	fileLog(false, "ChatServer", "requestRemoveClientFromChannel id(%s) name(%s) roomName(%s)", id.getValueString().c_str(), playerName.c_str(), roomName.c_str());
	getVChatInterface()->removeClientFromChannel(roomName, id, playerName);
}

//-----------------------------------------------------------------------

void ChatServer::requestChannelCommand(ReturnAddress const & requester,
	const std::string &srcUserName, const std::string &destUserName, const std::string &destChannelAddress, 
	unsigned command,unsigned banTimeout)
{
	getVChatInterface()->requestChannelCommand(requester, srcUserName, destUserName, destChannelAddress, command, banTimeout);
}

//-----------------------------------------------------------------------

void ChatServer::requestBroadcastChannelMessage(std::string const & channelName, std::string const & textMessage, bool isRemove)
{
	LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer::requestBroadcastGlobalChannel chan(%s) text(%s)", channelName.c_str(), textMessage.c_str()));
	getVChatInterface()->broadcastGlobalChannelMessage(channelName, textMessage, isRemove);
}

//-----------------------------------------------------------------------

bool ChatServer::getVoiceChatLoginInfoFromId(NetworkId const & id, std::string & userName, std::string & playerName)
{
	if(id.getValue() != 0)
	{
		VoiceChatAvatarList::const_iterator i = instance().m_voiceChatIdMap.find(id);
		if(i != instance().m_voiceChatIdMap.end())
		{
			userName = i->second.loginName;
			playerName = i->second.playerName;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------

bool ChatServer::getVoiceChatLoginInfoFromName(std::string const & playerName, NetworkId & id)
{
	std::map<std::string,NetworkId>::const_iterator i = instance().m_voiceChatNameToIdMap.find(toLower(playerName));
	if(i != instance().m_voiceChatNameToIdMap.end())
	{
		id = i->second;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool ChatServer::getVoiceChatLoginInfoFromLoginName(std::string const & loginName, NetworkId & id)
{
	std::map<std::string,NetworkId>::const_iterator i = instance().m_voiceLoginNameToIdMap.find(toLower(loginName));
	if(i != instance().m_voiceLoginNameToIdMap.end())
	{
		id = i->second;
		return true;
	}

	return false;
}
//-----------------------------------------------------------------------

void ChatServer::voiceChatGotLoginInfo(NetworkId const & id, std::string const & userName, std::string const & playerName)
{
	if(id.getValue() != 0)
	{
		VoiceChatAvatarData data;
		data.loginName = userName;
		data.playerName = playerName;
		instance().m_voiceChatIdMap.insert(std::make_pair(id, data));
		instance().m_voiceChatNameToIdMap.insert(std::make_pair(toLower(playerName), id));
		instance().m_voiceLoginNameToIdMap.insert(std::make_pair(toLower(userName), id));
	}
}

//-----------------------------------------------------------------------

void ChatServer::requestInvitePlayerToChannel(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName)
{
	if(channelName == getVChatInterface()->buildPersonalChannelName(sourceId))
	{
		getVChatInterface()->requestInvite(sourceId, targetId, channelName);
	}
}

void ChatServer::requestKickPlayerFromChannel(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName)
{
	if(channelName == getVChatInterface()->buildPersonalChannelName(sourceId))
	{
		getVChatInterface()->requestKick(sourceId, targetId, channelName);
	}
}

