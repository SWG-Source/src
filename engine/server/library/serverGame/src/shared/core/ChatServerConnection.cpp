// ChatServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/ChatServerConnection.h"

#include "serverGame/Chat.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ReportManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedNetworkMessages/ChatOnCreateRoom.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedObject/NetworkIdManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

ChatServerConnection::ChatServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

ChatServerConnection::~ChatServerConnection()
{
	if (this == Chat::getChatServer())
	{
		Chat::setChatServer(0);
	}
}

//-----------------------------------------------------------------------

void ChatServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("ChatServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ChatServerConnection::onReceive(const Archive::ByteStream &message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);

	Archive::ReadIterator cri = message.begin();
	
	const uint32 messageType = msg.getType();

	switch(messageType) {
		case constcrc("ChatOnCreateRoom") :
		{
			ChatOnCreateRoom chat(cri);
			GroupObject::onChatRoomCreate(chat.getRoomData().path);
			GuildInterface::onChatRoomCreate(chat.getRoomData().path);
			CityInterface::onChatRoomCreate(chat.getRoomData().path);
			break;
		}
		case constcrc("ChatOnRequestLog") :
		{
			ChatOnRequestLog chatOnRequestLog(cri);

			ReportManager::handleMessage(chatOnRequestLog);
			break;
		}
		case constcrc("ChatAvatarConnected") :
		{
			GenericValueTypeMessage<NetworkId> m(cri);
			Object *o = NetworkIdManager::getObjectById(m.getValue());
			if (o)
			{
				ServerObject *so = o->asServerObject();
				if (so && so->isAuthoritative())
				{
					CreatureObject *co = so->asCreatureObject();
					if (co)
					{
						if (co->getGuildId())
						{
							GuildInterface::enterGuildChatRoom(*co);
							GuildInterface::enterGuildVoiceChatRoom(*co);
						}

						std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(co->getNetworkId());
						if (!cityIds.empty())
							CityInterface::enterCityChatRoom(cityIds.front(), *co);
					}
					GameScriptObject * gso = so->getScriptObject();
					if(gso)
					{
						ScriptParams params;
						gso->trigAllScripts(Scripting::TRIG_CHAT_ON_LOGIN, params);
					}
				}
			}
			break;
		}
		case constcrc("ChatAvatarRenamed") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::string> > const m(cri);

			// refresh the friends and/or ignore list of any character that
			// has the renamed avatar in his friends and/or ignore list
			std::string const oldName(NameManager::normalizeName(m.getValue().first));
			std::string const newName(NameManager::normalizeName(m.getValue().second));

			std::set<PlayerObject const *> const &players = PlayerObject::getAllPlayerObjects();
			if (!players.empty())
			{
				std::set<PlayerObject const *>::const_iterator i;
				for (i = players.begin(); i != players.end(); ++i)
				{
					PlayerObject const * const   playerObject   = *i;
					if (!playerObject->isAuthoritative())
						continue;

					CreatureObject const * const creatureObject = playerObject->getCreatureObject();
					if (!creatureObject || !creatureObject->isAuthoritative() || !creatureObject->getClient())
						continue;

					if (playerObject->isFriend(oldName))
					{
						// refresh friends list in 30 seconds
						MessageToQueue::getInstance().sendMessageToC(playerObject->getNetworkId(),
							"C++RequestChatFriendsList",
							"",
							30,
							false);

						Chat::sendSystemMessage(*creatureObject, Unicode::narrowToWide(FormattedString<2048>().sprintf("%s on your friends list has been renamed to %s.  If your friends list has not been updated with the new name in about 30 seconds, you can relog to refresh your friends list.", oldName.c_str(), newName.c_str())), Unicode::emptyString);
					}

					if (playerObject->isIgnoring(oldName))
					{
						// refresh ignore list in 30 seconds
						MessageToQueue::getInstance().sendMessageToC(playerObject->getNetworkId(),
							"C++RequestChatIgnoreList",
							"",
							30,
							false);

						Chat::sendSystemMessage(*creatureObject, Unicode::narrowToWide(FormattedString<2048>().sprintf("%s on your ignore list has been renamed to %s.  If your ignore list has not been updated with the new name in about 30 seconds, you can relog to refresh your ignore list.", oldName.c_str(), newName.c_str())), Unicode::emptyString);
					}
				}
			}
			break;
		}
	}
}