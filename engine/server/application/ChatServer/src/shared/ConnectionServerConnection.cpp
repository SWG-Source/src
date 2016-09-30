// ConnectionServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "FirstChatServer.h"

#include "ChatInterface.h"
#include "ChatServer.h"
#include "ChatServerAvatarOwner.h"
#include "ConfigChatServer.h"
#include "ConnectionServerConnection.h"
#include "VChatInterface.h"
#include "serverNetworkMessages/ChatConnectAvatar.h"
#include "serverNetworkMessages/ChatDisconnectAvatar.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ChatAddFriend.h"
#include "sharedNetworkMessages/ChatAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatCreateRoom.h"
#include "sharedNetworkMessages/ChatDeletePersistentMessage.h"
#include "sharedNetworkMessages/ChatDeleteAllPersistentMessages.h"
#include "sharedNetworkMessages/ChatDestroyRoom.h"
#include "sharedNetworkMessages/ChatEnterRoom.h"
#include "sharedNetworkMessages/ChatEnterRoomById.h"
#include "sharedNetworkMessages/ChatInstantMessageToCharacter.h"
#include "sharedNetworkMessages/ChatInstantMessageToClient.h"
#include "sharedNetworkMessages/ChatInviteAvatarToRoom.h"
#include "sharedNetworkMessages/ChatKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnSendInstantMessage.h"
#include "sharedNetworkMessages/ChatQueryRoom.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClient.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatRequestPersistentMessage.h"
#include "sharedNetworkMessages/ChatSendToRoom.h"
#include "sharedNetworkMessages/ChatUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatUninviteFromRoom.h"
#include "sharedNetworkMessages/VerifyPlayerNameMessage.h"
#include "sharedNetworkMessages/VerifyPlayerNameResponseMessage.h"
#include "UnicodeUtils.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

ConnectionServerConnection::ConnectionServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData()),
m_avatars()
{
	ChatServer::fileLog(true, "ConnectionServerConnection", "Connection created...listening on (%s:%d)", a.c_str(), static_cast<int>(p));
}

//-----------------------------------------------------------------------

ConnectionServerConnection::~ConnectionServerConnection()
{
	ChatServer::fileLog(true, "ConnectionServerConnection", "~ConnectionServerConnection() - %s:%d", getRemoteAddress().c_str(), getRemotePort());

	std::set<ChatServerAvatarOwner *>::iterator i;
	for(i = m_avatars.begin(); i != m_avatars.end(); ++i)
	{
		(*i)->setPlayerConnection(0);
	}
	
	ChatServer::removeConnectionServerConnection(this);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionClosed()
{
	ChatServer::fileLog(true, "ConnectionServerConnection", "onConnectionClosed()");
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionOpened()
{
	ChatServer::fileLog(true, "ConnectionServerConnection", "onConnectionOpened()");
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	
	static int count = 0;
	if ((count % 10) == 0)
	{
		ChatServer::getChatInterface()->Process();
	}
	++count;

	ri = message.begin();
	
	const uint32 messageType = m.getType();

	//LOG("ConnectionServerConnection", ("onReceive() message(%s)", m.getCmdName().c_str()));

	switch(messageType) {
		case constcrc("ChatConnectAvatar") :
		{
			//printf("ConnectionServerConnection -- ChatConnectAvatar\n");		
			PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatConnectAvatar");
			ChatConnectAvatar c(ri);
			// break surname from last name
			std::string name = c.getCharacterName();
			size_t pos = name.find_first_of(" ");
			if(pos != std::string::npos)
				name = name.substr(0, pos);
			ChatServer::connectPlayer(this, c.getStationId(), name, c.getCharacterId(), c.getIsSecure(), c.getIsSubscribed());
			break;
		}
		case constcrc("ChatDisconnectAvatar") :
		{
			PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatDisconnectAvatar");
			//printf("ConnectionServerConnection -- ChatDisconnectAvatar\n");		
			static MessageDispatch::Transceiver<const ChatDisconnectAvatar &> disconn;
			ChatDisconnectAvatar d(ri);
			disconn.emitMessage(d);
			ChatServer::disconnectPlayer(d.getCharacterId());
			break;
		} 
		case constcrc("GameClientMessage") :
		{
			// chat message forwarded by the connection server
			GameClientMessage c(ri);
			//deliverMessageToClientObject(c.getNetworkId(), c.getByteStream());
			Archive::ReadIterator cri = c.getByteStream().begin();
			GameNetworkMessage cm(cri);
			std::vector<NetworkId>::const_iterator i;
			for(i = c.getDistributionList().begin(); i != c.getDistributionList().end(); ++i)
			{
				const ChatAvatar * avatar = ChatServer::getAvatarByNetworkId(*i);
				const uint32 cmtype = cm.getType();
				
				if (!avatar && cmtype == constcrc("ChatInstantMessageToCharacter"))
				{
					PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatInstantMessageToCharacter");
					//printf("!Sending via the shortcut\n");
					cri = c.getByteStream().begin();

					// deliver IM to character
					ChatInstantMessageToCharacter chat(cri);
					ChatAvatarId characterName = chat.getCharacterName();
					if(characterName.gameCode.empty())
						characterName.gameCode = "SWG";
					if(characterName.cluster.empty())
						characterName.cluster = ConfigChatServer::getClusterName();

					ChatAvatarId fromName;
					fromName.gameCode = "SWG";
					fromName.cluster = ConfigChatServer::getClusterName();
					fromName.name = ChatServer::getChatInterface()->getChatName((*i));

					ChatInstantMessageToClient msg(fromName, chat.getMessage(), chat.getOutOfBand());
					bool sent = ChatServer::getChatInterface()->sendMessageToPendingAvatar(characterName, msg);
					if (!sent)
					{
						ChatServer::getChatInterface()->sendMessageToAvatar(characterName, msg);
					}

					ChatOnSendInstantMessage response(chat.getSequence(), 0);	
					ChatServer::getChatInterface()->sendMessageToPendingAvatar(fromName, response); 
					return;
				}
				
				if(avatar)
				{
					cri = c.getByteStream().begin();

					switch(cmtype) {
						case constcrc("ChatDeleteAllPersistentMessages") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatDeleteAllPersistentMessages");
							ChatDeleteAllPersistentMessages chatDeleteAllPersistentMessages(cri);
							ChatServer::deleteAllPersistentMessages(chatDeleteAllPersistentMessages.getSourceNetworkId(), chatDeleteAllPersistentMessages.getTargetNetworkId());
							break;
						}
						case constcrc("ChatInstantMessageToCharacter") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatInstantMessageToCharacter");
							//printf("ConnectionServerConnection -- ChatInstantMessageToCharacter\n");		
							// deliver IM to character
							ChatInstantMessageToCharacter chat(cri);
							ChatAvatarId characterName = chat.getCharacterName();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();

							ChatServer::sendInstantMessage(*i, chat.getSequence(), characterName, chat.getMessage(), chat.getOutOfBand());
							break;						
						}
						case constcrc("ChatQueryRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatQueryRoom");
							//printf("ConnectionServerConnection -- ChatQueryRoom\n");		
							ChatQueryRoom chat(cri);
							ChatServer::queryRoom((*i), this, chat.getSequence(), chat.getRoomName());
							break;
						}
						case constcrc("ChatPersistentMessageToServer") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatPeristentMessageToServer");
							//printf("ConnectionServerConnection -- ChatPersistentMessageToServer\n");		
							ChatPersistentMessageToServer chat(cri);
							ChatAvatarId characterName = chat.getToCharacterName();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
					
							ChatServer::sendPersistentMessage((*i), chat.getSequence(), characterName, chat.getSubject(), chat.getMessage(), chat.getOutOfBand());
							break;
						}
						case constcrc("ChatRequestPersistentMessage") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatRequestPersistentMessage");
							//printf("ConnectionServerConnection -- ChatRequestPersistentMessage\n");		
							ChatRequestPersistentMessage chat(cri);
							ChatServer::requestPersistentMessage((*i), chat.getSequence(), chat.getMessageId());
							break;
						}
						case constcrc("ChatDeletePersistentMessage") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatDeletePersistentMessage");
							//printf("ConnectionServerConnection -- ChatDeletePersistentMessage\n");		
							ChatDeletePersistentMessage chat(cri);
							ChatServer::deletePersistentMessage((*i), chat.getMessageId());
							break;
						}
						case constcrc("ChatCreateRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatCreateRoom");
							//printf("ConnectionServerConnection -- ChatCreateRoom\n");		
							ChatCreateRoom chat(cri);
							ChatServer::createRoom((*i), chat.getSequence(), chat.getRoomName(), chat.getIsModerated(), chat.getIsPublic(), chat.getRoomTitle());
							break;
						}
						case constcrc("ChatEnterRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatEnterRoom");
							//printf("ConnectionServerConnection -- ChatEnterRoom\n");		
							// entering a room by name
							ChatEnterRoom chat(cri);
							ChatServer::enterRoom((*i), chat.getSequence(), chat.getRoomName());
							break;
						}
						case constcrc("ChatEnterRoomById") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatEnterRoomById");
							//printf("ConnectionServerConnection -- ChatEnterRoomById\n");		
							// entering a room by room id
							ChatEnterRoomById chat(cri);
							ChatServer::enterRoom((*i), chat.getSequence(), chat.getRoomId());
							break;

						}
						case constcrc("ChatSendToRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatSendToRoom");
							//printf("ConnectionServerConnection -- ChatSendToRoom\n");		
							ChatSendToRoom chat(cri);
							ChatServer::sendRoomMessage((*i), chat.getSequence(), chat.getRoomId(), chat.getMessage(), chat.getOutOfBand());
							break;
						}
						case constcrc("ChatRequestRoomList") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatRequestRoomList");
							//printf("ConnectionServerConnection -- ChatRequestRoomList\n");		
							ChatServer::requestRoomList((*i), this);
							break;
						}
						case constcrc("ChatDestroyRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatDestroyRoom");
							//printf("ConnectionServerConnection -- ChatDestroyRoom\n");		
							ChatDestroyRoom chat(cri);
							ChatServer::destroyRoom((*i), chat.getSequence(), chat.getRoomId());
							break;						
						}
						case constcrc("ChatAddModeratorToRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatAddModeratorToRoom");
							//printf("ConnectionServerConnection -- ChatAddModeratorToRoom\n");		
							ChatAddModeratorToRoom chat(cri);
							ChatServer::addModeratorToRoom(chat.getSequenceId(), (*i), chat.getAvatarId(), chat.getRoomName());
							break;
						}
						case constcrc("ChatRemoveModeratorFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatRemoveModeratorFromRoom");
							//printf("ConnectionServerConnection -- ChatREmoveModeratorFromRoom\n");		
							ChatRemoveModeratorFromRoom chat(cri);
							ChatAvatarId characterName = chat.getAvatarId();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
							ChatServer::removeModeratorFromRoom(chat.getSequenceId(), (*i), characterName, chat.getRoomName());
							break;
						}
						case constcrc("ChatRemoveAvatarFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatRemoveAvatarFromRoom");
							//printf("ConnectionServerConnection -- ChatREmoveAvatarFromRoom\n");		
							ChatRemoveAvatarFromRoom chat(cri);
							ChatServer::removeAvatarFromRoom((*i), chat.getAvatarId(), chat.getRoomName());
							break;
						}
						case constcrc("ChatKickAvatarFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatKickAvatarFromRoom");
							//printf("ConnectionServerConnection -- ChatKickAvatarFromRoom\n");		
							ChatKickAvatarFromRoom chat(cri);
							ChatServer::kickAvatarFromRoom((*i), chat.getAvatarId(), chat.getRoomName());
							break;
						}
						case constcrc("ChatAddFriend") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatAddFriend");
							//printf("ConnectionServerConnection -- ChatAddFriend\n");		
							ChatAddFriend chat(ri);
							ChatServer::addFriend((*i), chat.getSequence(), chat.getCharacterName());
							break;
						}
						case constcrc("ChatInviteAvatarToRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatInviteAvatarToRoom");
							//printf("ConnectionServerConnection -- ChatInviteAvatarToRoom\n");		
							ChatInviteAvatarToRoom chat(cri);
							ChatAvatarId characterName = chat.getAvatarId();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
							ChatServer::invite((*i), characterName, chat.getRoomName());
							break;
						}
						case constcrc("ChatUninviteFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatUninviteFromRoom");
							//printf("ConnectionServerConnection -- ChatUninviteAvatarToRoom\n");		
							ChatUninviteFromRoom chat(cri);
							ChatAvatarId characterName = chat.getAvatar();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
							ChatServer::uninvite((*i), chat.getSequence(), characterName, chat.getRoomName());
							break;
						}
						case constcrc("ChatBanAvatarFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatBanAvatarFromRoom");
							//printf("ConnectionServerConnection -- ChatBanAvatarFromRoom\n");		
							ChatBanAvatarFromRoom chat(cri);
							ChatAvatarId characterName = chat.getAvatarId();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
							ChatServer::banFromRoom(chat.getSequence(), (*i), characterName, chat.getRoomName());
							break;
						}
						case constcrc("ChatUnbanAvatarFromRoom") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - ChatUnbanAvatarFromRoom");
							//printf("ConnectionServerConnection -- ChatUnbanAvatarFromRoom\n");		
							ChatUnbanAvatarFromRoom chat(cri);
							ChatAvatarId characterName = chat.getAvatarId();
							if(characterName.gameCode.empty())
								characterName.gameCode = "SWG";
							if(characterName.cluster.empty())
								characterName.cluster = ConfigChatServer::getClusterName();
							ChatServer::unbanFromRoom(chat.getSequence(), (*i), characterName, chat.getRoomName());
							break;
						}
						case constcrc("VerifyPlayerNameMessage") :
						{
							PROFILER_AUTO_BLOCK_DEFINE("ConnectionServer - VerifyPlayerNameMessage");
							VerifyPlayerNameMessage message(cri);

							bool const valid = ChatServer::isValidChatAvatarName(message.getPlayerName());

							VerifyPlayerNameResponseMessage response(valid, message.getPlayerName());

							ChatServer::fileLog(true, "ConnectionServerConnection", "onReceive() message(VerifyPlayerNameMessage) name(%s) valid(%s)", Unicode::wideToNarrow(message.getPlayerName()).c_str(), (valid ? "yes" : "no"));

							sendToClient(message.getSourceNetworkId(), response);
							break;
						}
					}					
				}

				// @todo : how is a message sent to anyone with a station account?
				
				else
				{
					// defer until avatar is connected to chat backend
					ChatServer::deferChatMessageFor((*i), message);
				}
				
			}
			break;
		}
	}

}

//-----------------------------------------------------------------------

void ConnectionServerConnection::sendToClient(const NetworkId & clientId, const GameNetworkMessage & message)
{
	static std::vector<NetworkId> v;
	v.clear();
	v.push_back(clientId);
	GameClientMessage msg(v, true, message);
	ServerConnection::send(msg, true);
}


//-----------------------------------------------------------------------

void ConnectionServerConnection::addAvatar(ChatServerAvatarOwner * a)
{
	m_avatars.insert(a);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::removeAvatar(ChatServerAvatarOwner * a)
{
	std::set<ChatServerAvatarOwner *>::iterator f = m_avatars.find(a);
	if(f != m_avatars.end())
		m_avatars.erase(f);
}

//-----------------------------------------------------------------------

