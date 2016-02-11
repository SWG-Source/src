// ============================================================================
// 
// CommunityManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommunityManager.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageParticipant.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatOnChangeFriendStatus.h"
#include "sharedNetworkMessages/ChatOnGetFriendsList.h"
#include "sharedNetworkMessages/ChatOnChangeIgnoreStatus.h"
#include "sharedNetworkMessages/ChatOnGetIgnoreList.h"
#include "sharedObject/NetworkIdManager.h"

// ============================================================================
//
// CommunityManagerNameSpace
//
// ============================================================================

namespace CommunityManagerNameSpace
{
	PlayerObject *getPlayerObject(NetworkId const &networkId);
	ServerObject *getServerObject(NetworkId const &networkId);

	void sendProseChatMessage(NetworkId const &networkId, StringId const &stringId, std::string const &name);
}

using namespace CommunityManagerNameSpace;

//-----------------------------------------------------------------------------
PlayerObject *CommunityManagerNameSpace::getPlayerObject(NetworkId const &networkId)
{
	PlayerObject *result = nullptr;
	Object *object = NetworkIdManager::getObjectById(networkId);
	
	if (object != nullptr)
	{
		CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(object);
		
		if (creatureObject != nullptr)
		{
			result = PlayerCreatureController::getPlayerObject(creatureObject);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
ServerObject *CommunityManagerNameSpace::getServerObject(NetworkId const &networkId)
{
	ServerObject *result = nullptr;
	Object *object = NetworkIdManager::getObjectById(networkId);
	
	if (object != nullptr)
	{
		result = dynamic_cast<ServerObject *>(object);
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManagerNameSpace::sendProseChatMessage(NetworkId const &networkId, StringId const &stringId, std::string const &name)
{
	Object *object = NetworkIdManager::getObjectById(networkId);

	if (object != nullptr)
	{
		ServerObject *serverObject = dynamic_cast<ServerObject *>(object);
		
		if (serverObject != nullptr)
		{
			ProsePackage prosePackage;
			prosePackage.stringId = stringId;

			ProsePackageParticipant target;
			target.str = Unicode::narrowToWide(name);

			prosePackage.target = target;

			Chat::sendSystemMessage(*serverObject, prosePackage);
		}
	}
}

// ============================================================================
//
// CommunityManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void CommunityManager::handleMessage(ChatOnChangeFriendStatus const &message)
{
	unsigned int resultCode = message.getResultCode();

	switch (resultCode)
	{
		case CHATRESULT_SUCCESS:
			{
				//DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() <ChatOnChangeFriendStatus, CHATRESULT_SUCCESS> networkId: %s friend: %s\n", message.getCharacter().getValueString().c_str(), message.getFriendName().getFullName().c_str()));

				// Make sure the player still exists on the server
	
				PlayerObject *playerObject = getPlayerObject(message.getCharacter());
	
				if (playerObject != nullptr)
				{
					if (message.getAdd())
					{
						// Friend was added
	
						//DEBUG_REPORT_LOG(true, ("  CommunityManager::handleMessage() <FRIEND_ADDED>\n"));

						sendProseChatMessage(message.getCharacter(), StringId("cmnty", "friend_added"), message.getFriendName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
					}
					else
					{
						// Friend was removed
	
						//DEBUG_REPORT_LOG(true, ("  CommunityManager::handleMessage() <FRIEND_REMOVED>\n"));

						sendProseChatMessage(message.getCharacter(), StringId("cmnty", "friend_removed"), message.getFriendName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
					}

					playerObject->requestFriendList();
				}
			}
			break;
		case ERR_DESTAVATARDOESNTEXIST:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "friend_not_found"), message.getFriendName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		case ERR_DUPLICATEFRIEND:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "friend_duplicate"), message.getFriendName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		case ERR_FRIENDNOTFOUND:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "friend_not_found"), message.getFriendName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		default:
			{
				DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() <ChatOnChangeFriendStatus, ERROR NOT HANDLED> networkId: %s result code: %d\n", message.getCharacter().getValueString().c_str(), resultCode));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::handleMessage(ChatOnGetFriendsList const &message)
{
	//DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() - <ChatOnGetFriendsList> networkId: %s friend count: %d\n", message.getCharacter().getValueString().c_str(), message.getFriends().size()));

	PlayerObject *playerObject = getPlayerObject(message.getCharacter());

	if (playerObject != nullptr)
	{
		typedef std::vector<std::string> StringVector;
		StringVector friendList;
		friendList.reserve(message.getFriends().size());

		std::vector<ChatAvatarId>::const_iterator iterFriends = message.getFriends().begin();

#ifdef _DEBUG
		int count = 1;
#endif // _DEBUG

		std::string const & thisGameCode = Chat::getGameCode();
		std::string const & thisClusterName = GameServer::getInstance().getClusterName();
		for (; iterFriends != message.getFriends().end(); ++iterFriends)
		{
			if (!iterFriends->name.empty())
			{
				if (_stricmp(iterFriends->gameCode.c_str(), thisGameCode.c_str()))
				{
					// friend is from a different game, so store both the game and cluster name
					friendList.push_back(iterFriends->gameCode + std::string(".") + iterFriends->cluster + std::string(".") + iterFriends->name);
				}
				else if (_stricmp(iterFriends->cluster.c_str(), thisClusterName.c_str()))
				{
					// friend is from a different cluster, so store cluster name
					friendList.push_back(iterFriends->cluster + std::string(".") + iterFriends->name);
				}
				else
				{
					// friend is from the same cluster, so don't need to store game name
					friendList.push_back(iterFriends->name);
				}
			}

			//DEBUG_REPORT_LOG(true, ("  %2d CommunityManager::handleMessage() - <ChatOnGetFriendsList> networkId: %s friend: %s\n", count, message.getCharacter().getValueString().c_str(), iterFriends->getFullName().c_str()));

#ifdef _DEBUG
			++count;
#endif // _DEBUG
		}

		playerObject->setFriendList(friendList);
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::handleMessage(ChatOnChangeIgnoreStatus const &message)
{
	unsigned int resultCode = message.getResultCode();

	switch (resultCode)
	{
		case CHATRESULT_SUCCESS:
			{
				//DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() - <ChatOnChangeIgnoreStatus, CHATRESULT_SUCCESS> networkId: %s friend: %s\n", message.getCharacter().getValueString().c_str(), message.getIgnoreName().getFullName().c_str()));

				// Make sure the player still exists on the server
	
				PlayerObject *playerObject = getPlayerObject(message.getCharacter());
	
				if (playerObject != nullptr)
				{
					if (message.getIgnore())
					{
						// Ignore person was added
	
						//DEBUG_REPORT_LOG(true, ("  CommunityManager::handleMessage() <IGNORE_ADDED>\n"));

						sendProseChatMessage(message.getCharacter(), StringId("cmnty", "ignore_added"), message.getIgnoreName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
					}
					else
					{
						// Ignore person was removed
	
						//DEBUG_REPORT_LOG(true, ("  CommunityManager::handleMessage() <IGNORE_REMOVED>\n"));

						sendProseChatMessage(message.getCharacter(), StringId("cmnty", "ignore_removed"), message.getIgnoreName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
					}

					playerObject->requestIgnoreList();
				}
			}
			break;
		case ERR_DESTAVATARDOESNTEXIST:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "ignore_not_found"), message.getIgnoreName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		case ERR_DUPLICATEIGNORE:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "ignore_duplicate"), message.getIgnoreName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		case ERR_IGNORENOTFOUND:
			{
				sendProseChatMessage(message.getCharacter(), StringId("cmnty", "ignore_not_found"), message.getIgnoreName().getNameWithNecessaryPrefix(Chat::getGameCode(), GameServer::getInstance().getClusterName()));
			}
			break;
		default:
			{
				DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() - <ChatOnChangeIgnoreStatus, ERROR NOT HANDLED> networkId: %s result code: %d\n", message.getCharacter().getValueString().c_str(), resultCode));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::handleMessage(ChatOnGetIgnoreList const &message)
{
	//DEBUG_REPORT_LOG(true, ("CommunityManager::handleMessage() - <ChatOnGetIgnoreList> networkId: %s ignore count: %d\n", message.getCharacter().getValueString().c_str(), message.getIgnoreList().size()));

	PlayerObject *playerObject = getPlayerObject(message.getCharacter());

	if (playerObject != nullptr)
	{
		typedef std::vector<std::string> StringVector;
		StringVector ignoreList;
		ignoreList.reserve(message.getIgnoreList().size());

		std::vector<ChatAvatarId>::const_iterator iterIgnoreList = message.getIgnoreList().begin();

#ifdef _DEBUG
		int count = 1;
#endif // _DEBUG

		std::string const & thisGameCode = Chat::getGameCode();
		std::string const & thisClusterName = GameServer::getInstance().getClusterName();
		for (; iterIgnoreList != message.getIgnoreList().end(); ++iterIgnoreList)
		{
			if (!iterIgnoreList->name.empty())
			{
				if (_stricmp(iterIgnoreList->gameCode.c_str(), thisGameCode.c_str()))
				{
					// ignore is from a different game, so store both the game and cluster name
					ignoreList.push_back(iterIgnoreList->gameCode + std::string(".") + iterIgnoreList->cluster + std::string(".") + iterIgnoreList->name);
				}
				else if (_stricmp(iterIgnoreList->cluster.c_str(), thisClusterName.c_str()))
				{
					// ignore is from a different cluster, so store cluster name
					ignoreList.push_back(iterIgnoreList->cluster + std::string(".") + iterIgnoreList->name);
				}
				else
				{
					// ignore is from the same cluster, so don't need to store game name
					ignoreList.push_back(iterIgnoreList->name);
				}
			}

			//DEBUG_REPORT_LOG(true, ("  %2d CommunityManager::handleMessage() - <ChatOnGetIgnoreList> networkId: %s friend: %s\n", ++count, message.getCharacter().getValueString().c_str(), iterIgnoreList->getFullName().c_str()));

			//playerObject->addIgnore(iterIgnoreList->name);
#ifdef _DEBUG
			++count;
#endif // _DEBUG
		}

		playerObject->setIgnoreList(ignoreList);
	}
}

// ============================================================================
