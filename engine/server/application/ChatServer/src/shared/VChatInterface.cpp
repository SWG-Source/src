//VChatInterface.cpp

#include "FirstChatServer.h"
#include "VChatInterface.h"

#include "ChatInterface.h" //included for toLower...
#include "ChatServer.h"
#include "ConfigChatServer.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/VoiceChatChannelInfo.h"
#include "sharedNetworkMessages/VoiceChatOnGetAccount.h"
#include "sharedNetworkMessages/VoiceChatMiscMessages.h"

char const * const ms_logLable = "VChatInterface";
std::string const ms_systemAvatarName("SYSTEM");
static bool ms_voiceLoggingEnabled = false;
static unsigned const timeoutRetryLimit = 1;


//user structures

//used for GetChannel calls
struct GetChannelInfoStruct
{
	ReturnAddress requester;
	std::string channelName;
	std::string password;
	bool isPublic;
	bool persistant;
	std::list<std::string> moderators;
};

struct DeleteChannelInfo
{
	ReturnAddress requester;
	std::string channelName;
};

struct ChannelCommandInfo
{
	ReturnAddress requester;
	std::string srcUserName;
	std::string destUserName;
	std::string destChannelAddress;
	uint32 command;
	uint32 banTimeout;
};

struct GetAccountInfo
{
	std::string avatarName;
	NetworkId id;
	unsigned suid;
	unsigned failedAttempts;
};

//used for GetChannelInfo calls
struct GetChannelInfoInfo
{
	ReturnAddress requester;
	std::string channelName;
	bool isGlobalBroadcast;
	std::string textMessage;
};


VChatInterface::VChatInterface(const char * hostList)
: VChatAPI(hostList),
  m_channelData(),
  m_globalChannels(),
  m_connected(false),
  m_connectionFailedCount(0)
{
	ChatServer::fileLog(true, ms_logLable, "Creating interface hostlist=%s", hostList);
	ms_voiceLoggingEnabled = ConfigChatServer::getVoiceChatLoggingEnabled();
}

VChatInterface::~VChatInterface()
{
	ChatServer::fileLog(true, ms_logLable, "Destroying interface");
}

std::string systemNameHelper()
{
	std::string temp;
	VChatSystem::CreateUserName(ms_systemAvatarName, ConfigChatServer::getClusterName(), temp);
	return temp;
}

std::string const & VChatInterface::getSystemLoginName()
{
	static std::string const adminName (systemNameHelper());
	return adminName;
}

std::string VChatInterface::buildPersonalChannelName(NetworkId const & ownerId)
{
	std::string ownerLogin, ownerPlayerName;
	if(!ChatServer::getVoiceChatLoginInfoFromId(ownerId, ownerLogin, ownerPlayerName))
	{
		ownerPlayerName = ownerId.getValueString();
	}

	std::string outstr = ConfigChatServer::getGameCode();
	outstr += ".";
	outstr += ConfigChatServer::getClusterName();
	outstr += ".private.";
	outstr += ownerPlayerName;

	return outstr;
}


bool VChatInterface::findChannelDataByName(std::string const & name, VChatSystem::Channel & data)
{
	ChannelDataMap::iterator i = m_channelData.find(toLower(name));
	if(i != m_channelData.end())
	{
		data = i->second;
		return true;
	}
	return false;
}

void VChatInterface::setChannelData(VChatSystem::Channel const & data)
{
	m_channelData[toLower(data.m_channelName)] = data;
}

bool VChatInterface::eraseChannelData(std::string const & name)
{
	ChannelDataMap::iterator i = m_channelData.find(toLower(name));
	if(i != m_channelData.end())
	{
		m_channelData.erase(i);
		return true;
	}
	return false;
}

void VChatInterface::requestInvite(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestInvite roomName(%s) sourceId(%s) targetId(%s)",
		channelName.c_str(), sourceId.getValueString().c_str(), targetId.getValueString().c_str());

	std::string srcUserName, targetUserName, tmp;
	if(ChatServer::getVoiceChatLoginInfoFromId(sourceId, srcUserName, tmp) && ChatServer::getVoiceChatLoginInfoFromId(targetId, targetUserName, tmp))
	{
		//@TODO: if this fails clients will be confused
		requestChannelCommand(ReturnAddress(), srcUserName, targetUserName, channelName, VChatSystem::COMMAND_ADD_ACL, 0);
	}

	VChatSystem::Channel data;
	if(findChannelDataByName(channelName,data))
	{
		VoiceChatInvite msg(sourceId, targetId, channelName, data.m_channelURI);
		ChatServer::sendToClient(targetId, msg);
	}
}

void VChatInterface::requestKick(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestKick roomName(%s) sourceId(%s) targetId(%s)",
		channelName.c_str(), sourceId.getValueString().c_str(), targetId.getValueString().c_str());

	std::string srcUserName, targetUserName, tmp;
	if(ChatServer::getVoiceChatLoginInfoFromId(sourceId, srcUserName, tmp) && ChatServer::getVoiceChatLoginInfoFromId(targetId, targetUserName, tmp))
	{
		//@TODO: if this fails clients will be confused
		requestChannelCommand(ReturnAddress(), srcUserName, targetUserName, channelName, VChatSystem::COMMAND_DELETE_ACL, 0);
		requestChannelCommand(ReturnAddress(), srcUserName, targetUserName, channelName, VChatSystem::COMMAND_HANGUP, 0);
	}

	VChatSystem::Channel data;
	if(findChannelDataByName(channelName,data))
	{
		VoiceChatKick msg(sourceId, targetId, channelName);
		ChatServer::sendToClient(targetId, msg);
	}
}


bool VChatInterface::addClientToChannel(std::string const & roomName, NetworkId const & id, std::string const & playerName, bool forceShortlist)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "AddClientToChannel roomName(%s) clientId(%s) playerName(%s) forceShortlist(%i)",
		roomName.c_str(), id.getValueString().c_str(), playerName.c_str(), forceShortlist ? 1 : 0);

	std::string userName, tmp;
	if(!ChatServer::getVoiceChatLoginInfoFromId(id, userName, tmp))
	{
		//try to recover with the passed in player name
		VChatSystem::CreateUserName(playerName, ConfigChatServer::getClusterName(), userName);
	}
	requestChannelCommand(ReturnAddress(), "", userName, roomName, VChatSystem::COMMAND_ADD_ACL, 0);

	VChatSystem::Channel data;
	if(findChannelDataByName(roomName,data))
	{
		uint32 flags = forceShortlist ? VoiceChatChannelInfo::CIF_ForcedShortlist : VoiceChatChannelInfo::CIF_None;
		VoiceChatChannelInfo msg(data.m_channelName, data.m_channelName, data.m_channelURI, data.m_channelPassword, "", flags);
		ChatServer::sendToClient(id, msg);

		return true;
	}
	else
	{
		//no channel data...
		//TODO: get the channel and add the client?
	}
	return false;
}

bool VChatInterface::removeClientFromChannel(std::string const & roomName, NetworkId const & id, std::string const & playerName)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "removeClientFromChannel roomName(%s) clientId(%s) playerName(%s)",
		roomName.c_str(), id.getValueString().c_str(), playerName.c_str());

	std::string userName, tmp;
	if(!ChatServer::getVoiceChatLoginInfoFromId(id, userName, tmp))
	{
		//try to recover with the passed in player name
		VChatSystem::CreateUserName(playerName, ConfigChatServer::getClusterName(), userName);
	}
	requestChannelCommand(ReturnAddress(), "", userName, roomName, VChatSystem::COMMAND_DELETE_ACL, 0);
	requestChannelCommand(ReturnAddress(), "", userName, roomName, VChatSystem::COMMAND_HANGUP, 0);

	VChatSystem::Channel data;
	if(findChannelDataByName(roomName,data))
	{
		uint32 flags = VoiceChatChannelInfo::CIF_AutoJoin | VoiceChatChannelInfo::CIF_LeaveChannel;
		VoiceChatChannelInfo msg(data.m_channelName, data.m_channelName, data.m_channelURI, data.m_channelPassword, "", flags);
		ChatServer::sendToClient(id, msg);

		return true;
	}
	else
	{
		//we couldn't find the channel, but for a removal we just need the name anyhow
		uint32 flags = VoiceChatChannelInfo::CIF_AutoJoin | VoiceChatChannelInfo::CIF_LeaveChannel | VoiceChatChannelInfo::CIF_ChannelDoesNotExist;
		VoiceChatChannelInfo msg(roomName, roomName, "", "", "", flags);
		ChatServer::sendToClient(id, msg);
	}
	return false;
}

void VChatInterface::sendChannelData(ReturnAddress const & toWhom, std::string const & channelName, std::string const & messageText, bool targetIsModerator, bool notifyIfNoChannel, bool doGetChannelInfo)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "sendChannelData roomName(%s) target(%s)",
		channelName.c_str(), toWhom.debugString().c_str());

	VChatSystem::Channel data;
	if(findChannelDataByName(channelName, data))
	{
		uint32 flags = targetIsModerator ? VoiceChatChannelInfo::CIF_TargetModerator : VoiceChatChannelInfo::CIF_None;
		VoiceChatChannelInfo msg(data.m_channelName, data.m_channelName, data.m_channelURI, data.m_channelPassword, messageText, flags);
		ChatServer::instance().sendResponse(toWhom, msg);
	}
	else if (doGetChannelInfo)
	{
		requestChannelInfo(channelName, toWhom, false, messageText);
	}
	else if (notifyIfNoChannel)
	{
		uint32 flags = VoiceChatChannelInfo::CIF_ChannelDoesNotExist;
		if(targetIsModerator) flags |= VoiceChatChannelInfo::CIF_TargetModerator;
		VoiceChatChannelInfo msg(channelName, channelName, "", "", messageText, flags);
		ChatServer::instance().sendResponse(toWhom, msg);
	}
}


unsigned VChatInterface::requestGetChannel(const std::string &channelName,
					const std::string &description,
					const std::string &password,
					unsigned limit,
					bool isPublic,
					bool persistent,
					std::list<std::string> const & moderators,
					ReturnAddress const & requester)
{
	if(requester.type == ReturnAddress::RAT_gameserver && requester.gameServerId == 0)
	{
		ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestGetChannel called with a bad return address(%s)", requester.debugString().c_str());
	}

	PendingGetChannelRequest newReq;
	newReq.m_channelName = channelName;
	newReq.m_description = description;
	newReq.m_password = password;
	newReq.m_limit = limit;
	newReq.m_isPublic = isPublic;
	newReq.m_persistent = persistent;
	newReq.m_moderators = moderators;
	newReq.m_requester = requester;

	PendingGetChannelRequestMap::iterator itr = m_pendingGetChannelRequests.find(channelName);
	if(itr != m_pendingGetChannelRequests.end() && !itr->second.empty())
	{
		itr->second.push_back(newReq);
		return 0;
	}
	else
	{
		m_pendingGetChannelRequests[channelName].push_back(newReq);
	}

	return internalGetChannel(channelName, description, password, limit, isPublic, persistent, moderators, requester);
}

unsigned VChatInterface::internalGetChannel(const std::string &channelName,
							const std::string &description,
							const std::string &password,
							unsigned limit,
							bool isPublic,
							bool persistent,
							std::list<std::string> const & moderators,
							ReturnAddress const & requester)
{
	GetChannelInfoStruct * info = new GetChannelInfoStruct;
	info->requester = requester;
	info->channelName = channelName;
	info->password = password;
	info->isPublic = isPublic;
	info->persistant = persistent;
	info->moderators = moderators;

	//This is dumb. vchat should be consistant about qualifying channel names.
	std::string shortName, server, game;
	VChatSystem::GetChannelComponents(channelName,shortName,server,game);
	DEBUG_WARNING(game != ConfigChatServer::getGameCode(),("Channel create request with mismatched game code %s (should be %s)", game.c_str(), ConfigChatServer::getGameCode()));
	DEBUG_WARNING(server != ConfigChatServer::getClusterName(),("Channel create request with mismatched cluster name %s (should be %s)", server.c_str(), ConfigChatServer::getClusterName()));

	unsigned track = VChatAPI::GetChannelV2(shortName, game, server, description, password, limit, persistent, (void*)info);

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestGetChannel track(%d) requester(%s) name(%s) desc(%s) pass(%s) public(%s) persist(%s)",
		track, requester.debugString().c_str(), channelName.c_str(), description.c_str(), password.c_str(), isPublic?"true":"false", persistent?"true":"false");

	return track;
}

unsigned VChatInterface::requestDeleteChannel(std::string const &channelName, ReturnAddress const & requester)
{
	if(requester.type == ReturnAddress::RAT_gameserver && requester.gameServerId == 0)
	{
		ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestDeleteChannel called with a bad return address");
	}

	DeleteChannelInfo * info = new DeleteChannelInfo;
	info->requester = requester;
	info->channelName = channelName;

	//dumb
	std::string shortName, server, game;
	VChatSystem::GetChannelComponents(channelName,shortName,server,game);
	DEBUG_WARNING(game != ConfigChatServer::getGameCode(),("Channel delete request with mismatched game code %s (should be %s)", game.c_str(), ConfigChatServer::getGameCode()));
	DEBUG_WARNING(server != ConfigChatServer::getClusterName(),("Channel delete request with mismatched cluster name %s (should be %s)", server.c_str(), ConfigChatServer::getClusterName()));

	unsigned track = VChatAPI::DeleteChannel(shortName, game, server, info);

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestDeleteChannel track(%d) requester(%s) name(%s)",
		track, requester.debugString().c_str(), channelName.c_str());

	return track;
}

unsigned VChatInterface::requestChannelCommand(ReturnAddress const & requester,
							   const std::string &srcUserName,
							   const std::string &destUserName,
							   const std::string &destChannelAddress,
							   unsigned command,
							   unsigned banTimeout)
{
	ChannelCommandInfo * info = new ChannelCommandInfo;
	info->requester = requester;
	info->srcUserName = srcUserName;
	info->destUserName = destUserName;
	info->destChannelAddress = destChannelAddress;
	info->command = command;
	info->banTimeout = banTimeout;

	if(command == VChatSystem::COMMAND_ADD_ACL)
		checkForCharacterChannelAdd(destUserName, destChannelAddress);
	else if (command == VChatSystem::COMMAND_DELETE_ACL)
		checkForCharacterChannelRemove(destUserName, destChannelAddress);

	uint32 track = VChatAPI::ChannelCommand(srcUserName, destUserName, destChannelAddress, command, banTimeout, info);

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestChannelCommand: track(%u) returnAddress(%s) src(%s) dest(%s) chan(%s) cmd(%u) banTimeout(%u)",
		track, requester.debugString().c_str(), srcUserName.c_str(), destUserName.c_str(), destChannelAddress.c_str(), command, banTimeout);

	return track;
}

unsigned VChatInterface::requestConnectPlayer(unsigned suid, std::string const & characterName, NetworkId const & netId, unsigned previousAttempts)
{
	GetAccountInfo *info = new GetAccountInfo;
	info->avatarName = characterName;
	info->id = netId;
	info->suid = suid;
	info->failedAttempts = previousAttempts;

	uint32 track = VChatAPI::GetAccount(characterName, ConfigChatServer::getGameCode(), ConfigChatServer::getClusterName(), suid, 0,(void*)info);
	ChatServer::fileLog(ms_voiceLoggingEnabled,ms_logLable, "requestConnectPlayer: track(%d) suid(%u) name(%s) netId(%s) failedAttempts(%u)",
		track, suid, characterName.c_str(), netId.getValueString().c_str(), previousAttempts);
	return track;
}

unsigned VChatInterface::requestChannelInfo(std::string const & channelName, ReturnAddress const & requester, bool isGlobalBroadcast, std::string const & broadcastMessage)
{
	GetChannelInfoInfo *info = new GetChannelInfoInfo;
	info->requester = requester;
	info->channelName = channelName;
	info->isGlobalBroadcast = isGlobalBroadcast;
	info->textMessage = broadcastMessage;

	//split the name since we can't request any random channel...
	std::string shortName, server, game;
	VChatSystem::GetChannelComponents(info->channelName, shortName, server, game);

	uint32 track = VChatAPI::GetChannelInfo(shortName, game, server, (void*)info);

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "requestChannelInfo: track(%d) channel(%s) requester(%s)",
		track, info->channelName.c_str(), info->requester.debugString().c_str());

	return track;
}

void VChatInterface::OnConnectionOpened( const char * address )
{
	m_connected = true;
	m_connectionFailedCount = 0;
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnConnectionOpened: %s", address);


	uint32 track = VChatAPI::GetAccount(ms_systemAvatarName, ConfigChatServer::getGameCode(), ConfigChatServer::getClusterName(), 0, 0, nullptr);
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "Creating system avatar: %s track(%u)", getSystemLoginName().c_str(), track);
}

void VChatInterface::OnConnectionFailed( const char * address )
{
	m_connected = false;
	++m_connectionFailedCount;
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnConnectionFailed: %s failedCount(%d)",
		address, m_connectionFailedCount);
}

void VChatInterface::OnConnectionClosed( const char * address, const char * reason )
{
	m_connected = false;
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnConnectionClosed: %s, %s", address, reason);
}

void VChatInterface::OnConnectionShutdownNotified( const char * address, unsigned outstandingRequests )
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnConnectionShutdownNotified: address: %s, outstanding requests: %u", address, outstandingRequests);
}

void VChatInterface::OnGetAccount(unsigned track, unsigned result, unsigned userID, unsigned accountID,
						  const std::string &voicePassword, const std::string &encodedVoiceAccount,
						  const std::string &URI, void *user)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnGetAccount() track(%u) result(%u) userID(%u) accountID(%u) password(%s) encoded(%s) uri(%s)",
		track, result, userID, accountID, voicePassword.c_str(), encodedVoiceAccount.c_str(), URI.c_str());

	GetAccountInfo *info = (GetAccountInfo *)user;
	if(!info)
	{
		return;
	}

	if (result == VChatSystem::RESULT_TIMEOUT)
	{
		if(info->failedAttempts < timeoutRetryLimit)
		{
			requestConnectPlayer(info->suid,info->avatarName,info->id, info->failedAttempts+1);
			delete info;
			info = nullptr;
			return;
		}
	}

	unsigned msgResult = result == VChatSystem::RESULT_SUCCESS ? VoiceChatOnGetAccount::GAR_SUCCESS : VoiceChatOnGetAccount::GAR_FAILURE;

	ChatServer::voiceChatGotLoginInfo(info->id, encodedVoiceAccount, info->avatarName);

	VoiceChatOnGetAccount const msg(msgResult, encodedVoiceAccount, voicePassword, URI);
	ChatServer::sendToClient(info->id, msg);

	//send the private channel if it exists (but don't create it at this point)
	std::list<std::string> modlist;
	modlist.push_back(encodedVoiceAccount);
	sendChannelData(info->id, buildPersonalChannelName(info->id), "", true, false, false);

	//send the current global channels
	for(GlobalChannelMap::const_iterator i = m_globalChannels.begin(); i != m_globalChannels.end(); ++i)
	{
		sendChannelData(info->id, (*i).first, (*i).second, false, false, true);
	}

	delete info;
	info = nullptr;
}

void VChatInterface::OnGetChannelV2(unsigned track, unsigned result,
				const std::string &channelName, const std::string &channelURI,
				unsigned channelID, unsigned isNewChannel, void *user)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnGetChannel: track(%u) result(%u) chanName(%s) chanURI(%s) chanID(%d)",
				track, result, channelName.c_str(), channelURI.c_str(), channelID);

	GetChannelInfoStruct * info = (GetChannelInfoStruct*)user;
	if (info)
	{
		bool success = (result == VChatSystem::RESULT_SUCCESS);

		//we might have gotten good data even if creation failed...interesting...
		VChatSystem::Channel data;
		if (!channelURI.empty())
		{

			// If a channel is persisted, we want to add it to our character channel list. Currently the only persisted channel in SWG is guilds.
			if(info->persistant)
			{
				// Do we already have this channel on record?
				VoiceChannelIdMap::iterator iter = m_voiceChatChannelNameToNetworkIdMap.find(toLower(channelName));
				if(iter == m_voiceChatChannelNameToNetworkIdMap.end())
				{
					// Create a new record for this channel.
					m_voiceChatChannelNameToNetworkIdMap.insert(std::make_pair(toLower(channelName), std::vector<NetworkId>()));
				}
			}

			//if this is a newly created channel add the system avatar to it
			if (isNewChannel)
			{
				requestChannelCommand(ReturnAddress(), "", getSystemLoginName(), channelName, VChatSystem::COMMAND_ADD_ACL, 0);
			}

			//Always add the moderators just in case something failed in the past and we are now in a bad state
			for(std::list<std::string>::const_iterator i = info->moderators.begin(); i != info->moderators.end(); ++i)
			{
				requestChannelCommand(ReturnAddress(), "", *i, channelName, VChatSystem::COMMAND_ADD_MODERATOR, 0);
				requestChannelCommand(ReturnAddress(), "", *i, channelName, VChatSystem::COMMAND_ADD_ACL, 0);
			}

			data.m_channelName = channelName;
			data.m_channelPassword = info->password;
			data.m_channelURI = channelURI;
			data.m_channelID = channelID;
			setChannelData(data);
		}

		VoiceChatOnGetChannel response(channelName, channelURI, info->password, info->isPublic, info->persistant, success);

		ChatServer::instance().sendResponse(info->requester, response);

		delete info;
	}

	//check for any queued requests for this channel so we can fire them off now
	PendingGetChannelRequestMap::iterator itr = m_pendingGetChannelRequests.find(channelName);
	if(itr != m_pendingGetChannelRequests.end())
	{
		//If we don't make it in here then something is very messed up in the request list as
		// any request that is outstanding should be the front of the list.

		std::list<PendingGetChannelRequest> & thelist = itr->second;

		if(!thelist.empty())
		{
			//We should always hit this block. If we didn't, something is VERY wrong.
			thelist.pop_front();
		}
		else
			WARNING(true, ("OnGetChannelV2: Received a response for a channel that has no pending request! This is bad!!!"));

		std::list<PendingGetChannelRequest>::iterator nextItr = thelist.begin();
		if(nextItr != thelist.end())
		{
			PendingGetChannelRequest const & req = *nextItr;
			internalGetChannel(req.m_channelName, req.m_description, req.m_password, req.m_limit, req.m_isPublic, req.m_persistent, req.m_moderators, req.m_requester);
		}

		if(thelist.empty())
		{
			m_pendingGetChannelRequests.erase(itr);
		}
	}
	else
		WARNING(true, ("OnGetChannelV2: Received a response for a channel that isn't in our pending request map! BAD!"));
}

void VChatInterface::OnChannelCommand(unsigned track, unsigned result, void *user)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnChannelCommand: track(%u) result(%u)", track, result);


	ChannelCommandInfo * info = (ChannelCommandInfo*)user;
	if(info)
	{
		VoiceChatOnChannelCommand response(info->srcUserName,info->destUserName,info->destChannelAddress,info->command,info->banTimeout,result);
		ChatServer::instance().sendResponse(info->requester, response);

		delete info;
	}
}

void VChatInterface::OnGetAllChannels(unsigned track, unsigned result, const VChatSystem::ChannelVec_t & channels, void *user)
{
	UNREF(user);

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "====OnGetAllChannels==== track(%u) count(%u)", track, channels.size());

	static uint32 failedCount = 0;

	if(result == VChatSystem::RESULT_SUCCESS)
	{
		failedCount = 0;
		m_channelData.clear();

		for(VChatSystem::ChannelVec_t::const_iterator i = channels.begin(); i != channels.end(); ++i)
		{
			VChatSystem::Channel const & data = *i;
			setChannelData(data);
			ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "name(%s) uri(%s) pass(%s) id(%d) type(%d)",
				data.m_channelName.c_str(), data.m_channelURI.c_str(), data.m_channelPassword.c_str(), data.m_channelID, data.m_channelType);
		}
	}
	else
	{
		bool shouldRetry = result == VChatSystem::RESULT_TIMEOUT && failedCount < timeoutRetryLimit;

		++failedCount;

		ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "failed, code(%u) count(%d) retry(%s)", result, failedCount, shouldRetry?"yes":"no");

		if(shouldRetry)
		{
			GetAllChannels(nullptr);
		}
	}

	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "========================");
}

void VChatInterface::OnDeleteChannel(unsigned track, unsigned result, void *user)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnDeleteChannel: track(%u) result(%u)", track, result);

	DeleteChannelInfo * info = (DeleteChannelInfo*)user;
	if(info)
	{
		//TODO: send the response
		eraseChannelData(info->channelName);

		delete info;
	}
}

void VChatInterface::OnGetChannelInfo(unsigned track, unsigned result, const std::string & channelName,
	const std::string & channelURI, unsigned channelID, void * user)
{
	ChatServer::fileLog(ms_voiceLoggingEnabled, ms_logLable, "OnGetChannelInfo: track(%u) result(%u) chanName(%s) chanURI(%s) chanID(%d)",
		track, result, channelName.c_str(), channelURI.c_str(), channelID);

	GetChannelInfoInfo * info = (GetChannelInfoInfo*)user;
	if(info)
	{
		bool const channelExists = result == VChatSystem::RESULT_SUCCESS; //?

		uint32 flags = channelExists ? VoiceChatChannelInfo::CIF_None : VoiceChatChannelInfo::CIF_ChannelDoesNotExist;

		VoiceChatChannelInfo msg (channelName, channelName, channelURI, "", info->textMessage, flags);

		if(info->isGlobalBroadcast)
		{
			LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer: sending to all clients chan(%s) text(%s)", channelName.c_str(), info->textMessage.c_str()));
			ChatServer::instance().sendToAllConnectionServers(msg);
		}
		else
		{
			ChatServer::instance().sendResponse(info->requester, msg);
		}

		// cache the channel data
		if(channelExists)
		{
			VChatSystem::Channel data;
			data.m_channelName = channelName;
			data.m_channelPassword.clear(); // none of the channels have passwords at this point so this is a safe assumption
			data.m_channelURI = channelURI;
			data.m_channelID = channelID;
			setChannelData(data);
		}

		delete info;
		info = 0;
	}
}


void VChatInterface::broadcastGlobalChannelMessage(std::string const & channelName, std::string const & textMessage, bool isRemove)
{
	if(channelName.empty())
	{
		LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer: no channel, sending text only message chan(%s) text(%s) remove(%d)",
			channelName.c_str(), textMessage.c_str(),(isRemove?1:0)));
		GenericValueTypeMessage<std::string> msg("VCBroadcastMessage",textMessage);
		ChatServer::instance().sendToAllConnectionServers(msg);
	}
	else
	{
		GlobalChannelMap::iterator entryItr = m_globalChannels.find(channelName);

		if(entryItr == m_globalChannels.end())
		{
			if(!isRemove)
			{
				m_globalChannels.insert(std::make_pair(channelName, textMessage));
			}
		}
		else
		{
			if(isRemove)
			{
				m_globalChannels.erase(entryItr);
			}
		}

		VChatSystem::Channel data;
		if(findChannelDataByName(channelName, data))
		{
			uint32 flags = isRemove ? VoiceChatChannelInfo::CIF_LeaveChannel | VoiceChatChannelInfo::CIF_AutoJoin : VoiceChatChannelInfo::CIF_None;
			VoiceChatChannelInfo msg (data.m_channelName, data.m_channelName, data.m_channelURI, data.m_channelPassword, textMessage, flags);

			LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer found channel data. sending to all clients chan(%s) text(%s) remove(%d)",
				channelName.c_str(), textMessage.c_str(),(isRemove?1:0)));
			ChatServer::instance().sendToAllConnectionServers(msg);
		}
		else
		{
			if(isRemove)
			{
				uint32 flags = VoiceChatChannelInfo::CIF_LeaveChannel | VoiceChatChannelInfo::CIF_AutoJoin;
				VoiceChatChannelInfo msg (channelName, channelName, "", "", textMessage, flags);
				LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer: didn't find channel data sending remove to all clients chan(%s) text(%s)", channelName.c_str(), textMessage.c_str()));
				ChatServer::instance().sendToAllConnectionServers(msg);
			}
			else
			{
				LOG("CustomerService", ("BroadcastVoiceChannel: ChatServer: didn't find channel data asking vchat for channel chan(%s) text(%s)", channelName.c_str(), textMessage.c_str()));
				requestChannelInfo(channelName, ReturnAddress(), true, textMessage);
			}

		}
	}
}

void VChatInterface::OnAddCharacterChannel(unsigned int /*track*/, unsigned int /*result*/, void * /*user*/)
{

}

void VChatInterface::OnRemoveCharacterChannel(unsigned int /*track*/, unsigned int /*result*/, void * /*user*/)
{

}

void VChatInterface::OnGetCharacterChannel(unsigned int /*track*/, unsigned int /*result*/, const VChatSystem::CharacterChannelVec_t & /*characterChannels*/, void * /*user*/)
{

}

void VChatInterface::OnUpdateCharacterChannel(unsigned int /*track*/, unsigned int /*result*/, void * /*user*/)
{

}

void VChatInterface::checkForCharacterChannelAdd(std::string const & name, std::string const & channelName)
{

	// No persistant channels right now. Skip the add check.
	if(m_voiceChatChannelNameToNetworkIdMap.empty())
		return;

	// Can we get a valid player OID?
	NetworkId playerOID = NetworkId::cms_invalid;
	if(ChatServer::getVoiceChatLoginInfoFromLoginName(name, playerOID) && playerOID != NetworkId::cms_invalid)
	{
		// Are we tracking this channel?
		if(m_voiceChatChannelNameToNetworkIdMap.find(toLower(channelName)) == m_voiceChatChannelNameToNetworkIdMap.end())
			return;

		VChatSystem::Channel data;
		if(findChannelDataByName(channelName, data)) // See if we can get valid channel data.
		{
			const ChatAvatar * avatar = ChatServer::getAvatarByNetworkId(playerOID);
			if(avatar)
			{
				// Store the OIDs of players in our channel.
				bool insert = true;
				VoiceChannelIdMap::iterator iter = m_voiceChatChannelNameToNetworkIdMap.find(toLower(channelName));
				if(iter != m_voiceChatChannelNameToNetworkIdMap.end())
				{
					// do we have the OID on record?
					std::vector<NetworkId>::iterator OIDIter = (*iter).second.begin();
					for(; OIDIter != (*iter).second.end(); ++OIDIter) // Holy crap does this suck.
					{
						if((*OIDIter) == playerOID)
						{
							// Yup.
							insert = false;
							break;
						}
					}

					if((*iter).second.empty())
						insert = true;

					if(insert) // Nope, add it and send off the create character channel message.
					{
						AddCharacterChannel(avatar->getUserID(),
							avatar->getAvatarID(),
							std::string(avatar->getName().c_str()),
							parseWorldName(std::string(avatar->getServer().c_str())),
							"SWG",
							"guild",
							data.m_channelName,  //"A Guild chat channel."
							data.m_channelPassword,
							data.m_channelURI,
							"en_US",
							nullptr);

						(*iter).second.push_back(playerOID);

						return;
					}


				}
			}
		}
	}
}

void VChatInterface::checkForCharacterChannelRemove(std::string const & name, std::string const & channelName)
{
	// No persistant channels right now. Skip the clean up.
	if(m_voiceChatChannelNameToNetworkIdMap.empty())
		return;

	NetworkId playerOID = NetworkId::cms_invalid;
	// Verify we can get an OID for this user.
	if(ChatServer::getVoiceChatLoginInfoFromLoginName(name, playerOID) && playerOID != NetworkId::cms_invalid)
	{
		// Clean up any character channels this player may have.

		// Do we have a record for this channel?
		VoiceChannelIdMap::iterator chanIter = m_voiceChatChannelNameToNetworkIdMap.find(toLower(channelName));
		if(chanIter != m_voiceChatChannelNameToNetworkIdMap.end())
		{
			// Is this user in our OID list?
			std::vector<NetworkId>::iterator iter = (*chanIter).second.begin();
			for(; iter != (*chanIter).second.end(); )
			{
				if((*iter) == playerOID) // Yup, let's remove him.
				{
					const ChatAvatar * avatar = ChatServer::getAvatarByNetworkId(playerOID);
					if(avatar)
					{
						RemoveCharacterChannel(avatar->getUserID(),
							avatar->getAvatarID(),
							std::string(avatar->getName().c_str()),
							parseWorldName(std::string(avatar->getServer().c_str())),
							"SWG",
							"guild",
							nullptr);
					}

					iter = (*chanIter).second.erase(iter);

					if((*chanIter).second.empty())
					{
						// No more OIDs to track, clean this channel up.
						m_voiceChatChannelNameToNetworkIdMap.erase(chanIter);
						return;
					}
					break;
				}
				else
					++iter;
			}
		}
	}
}

std::string VChatInterface::parseWorldName(std::string const & input)
{
	static std::string const worldCode = "SWG+";
	std::string output;

	std::string::size_type findIndex = input.find(worldCode);
	if(findIndex != std::string::npos)
	{
		findIndex += worldCode.length();
		output = input.substr(findIndex);
	}
	else
		output = input;

	return output;
}
