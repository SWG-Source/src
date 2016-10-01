// ConnectionServerConnection.cpp
// copyright 2001 Verant Interactive

// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConnectionServerConnection.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/Chat.h"
#include "serverGame/CommunityManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverScript/GameScriptObject.h"
#include "serverUtility/FreeCtsDataTable.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/PlayerCreationManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatOnChangeFriendStatus.h"
#include "sharedNetworkMessages/ChatOnGetFriendsList.h"
#include "sharedNetworkMessages/ChatOnChangeIgnoreStatus.h"
#include "sharedNetworkMessages/ChatOnGetIgnoreList.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/SkillObject.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

namespace ConnectionServerConnectionNamespace
{
	class ConnectionServerNetworkSetupData: public NetworkSetupData
	{
	public:
		ConnectionServerNetworkSetupData();
	};
}

using namespace ConnectionServerConnectionNamespace;

// ======================================================================

ConnectionServerConnectionNamespace::ConnectionServerNetworkSetupData::ConnectionServerNetworkSetupData():
	NetworkSetupData()
{
	//-- The server connection from gameserver (or any other server) to connection server should sync to the connection server's time stamp.
	clockSyncDelay = 45000;
}

// ======================================================================

ConnectionServerConnection::ConnectionServerConnection(const std::string & a, const unsigned short p) :
	ServerConnection(a, p, ConnectionServerNetworkSetupData()),
	m_syncStampShort(0),
	m_syncStampLong(0)
{
}

// ----------------------------------------------------------------------

ConnectionServerConnection::~ConnectionServerConnection()
{
	static MessageConnectionCallback m("ConnectionServerConnectionDestroyed");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("ConnectionServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

            //Send info about ourself.
        std::string sceneName = ConfigServerGame::getSceneID();
        NewGameServer msg(sceneName, GameServer::getInstance().getProcessId());
        send(msg, true);

	static MessageConnectionCallback m("ConnectionServerConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onReceive (const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin();
	
	const uint32 messageType = m.getType();
	
	switch (messageType) {
		case constcrc("GameClientMessage") :
		{
			GameClientMessage c(ri);
			const std::vector<NetworkId> & idList = c.getDistributionList();
			std::vector<NetworkId>::const_iterator i;
			for(i = idList.begin(); i != idList.end(); ++i)
			{
				GameServer::getInstance().deliverMessageToClientObject((*i), c.getByteStream());
			}
			break;
		}
		case constcrc("SetSyncStamp") :
		{
			Archive::ReadIterator readIterator = message.begin();
			GenericValueTypeMessage<std::pair<uint16, uint32> > syncStampMessage(readIterator);
			setSyncStamps(syncStampMessage.getValue().first, syncStampMessage.getValue().second);
			break;
		}
		case constcrc("ChatOnChangeFriendStatus") :
		{
			ChatOnChangeFriendStatus chat(ri);

			CommunityManager::handleMessage(chat);
			break;
		}
		case constcrc("ChatOnGetFriendsList") :
		{
			ChatOnGetFriendsList chat(ri);

			CommunityManager::handleMessage(chat);
			break;
		}
		case constcrc("ChatOnChangeIgnoreStatus") :
		{
			ChatOnChangeIgnoreStatus chat(ri);

			CommunityManager::handleMessage(chat);
			break;
		}
		case constcrc("ChatOnGetIgnoreList") :
		{
			ChatOnGetIgnoreList chat(ri);

			CommunityManager::handleMessage(chat);
			break;
		}
		case constcrc("ChatStatisticsCS") :
		{
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, int>, std::pair<int, int> > > chatStatistics(ri);
			ServerObject * const so = ServerWorld::findObjectByNetworkId(chatStatistics.getValue().first.first);
			if (so)
			{
				CreatureObject * const character = so->asCreatureObject();
				if (character)
				{
					PlayerObject * const player = PlayerCreatureController::getPlayerObject(character);
					if (player)
					{
						player->handleChatStatisticsFromChatServer(chatStatistics.getValue().first.first, chatStatistics.getValue().first.second, chatStatistics.getValue().second.first, chatStatistics.getValue().second.second);
					}
				}
			}
			break;
		}
		case constcrc("RequestLoadPackedHouses") :
		{
			GenericValueTypeMessage<NetworkId> characterId(ri);
			LOG("CustomerService", ("CharacterTransfer: ConnectionServerConnection received RequestLoadPackedHouses for %s", characterId.getValue().getValueString().c_str()));

			CreatureObject * character = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(characterId.getValue()));
			if(character)
			{
				int numHouses = 0;
				numHouses = character->loadPackedHouses();
				if (numHouses > 0)
				{
					character->setLoadCTSPackedHouses(true);
					// add the objvar.
					character->setObjVarItem("cts.packed_house_count", numHouses);
					LOG("CustomerService", ("CharacterTransfer: starting %d packed house load for CTS character %s", numHouses, PlayerObject::getAccountDescription(character).c_str()));
				}
				else
				{
					// houses are loaded, go ahead and send the reply message
					GenericValueTypeMessage<NetworkId> loadedMsg("PackedHousesLoaded", characterId.getValue());
					send(loadedMsg, true);
					LOG("CustomerService", ("CharacterTransfer: CTS character %s does not have any packed houses, so telling the connection server", PlayerObject::getAccountDescription(character).c_str()));
				}
			}
			else
				LOG("CustomerService", ("CharacterTransfer: character object was nullptr so unable to check if bank is loaded"));

			break;
		}
		case constcrc("RequestLoadCTSBank") :
		{
			GenericValueTypeMessage<NetworkId> characterId(ri);
			LOG("CustomerService", ("CharacterTransfer: ConnectionServerConnection received RequestLoadCTSBank for %s", characterId.getValue().getValueString().c_str()));
		
			CreatureObject * character = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(characterId.getValue()));
			if(character)
			{
				ServerObject *bankContainer = character->getBankContainer();
				if (bankContainer)
				{
					if (!bankContainer->areContentsLoaded())
					{
						LOG("CustomerService", ("CharacterTransfer: setting LoadCTSBank flag and request bank to be loaded for %s", PlayerObject::getAccountDescription(character).c_str()));
						character->setLoadCTSBank(true);
						bankContainer->loadAllContentsFromDB();
					}
					else
					{
						LOG("CustomerService", ("CharacterTransfer: bank contents already loaded for %s, sending ReplyBankCTSLoaded back to client", PlayerObject::getAccountDescription(character).c_str()));
						//Bank contents are already loaded
						GenericValueTypeMessage<NetworkId> reply("ReplyBankCTSLoaded", characterId.getValue());
						send(reply, true);
					}
				}
				else
					LOG("CustomerService", ("CharacterTransfer: bank container object was nullptr for getBankContainer call"));
			}
			else
				LOG("CustomerService", ("CharacterTransfer: character object was nullptr so unable to check if bank is loaded"));
			
			break;
		}
		case constcrc("RequestTransferData") :
		{
			GenericValueTypeMessage<TransferCharacterData> requestTransferData(ri);
			const TransferCharacterData & transferCharacterData = requestTransferData.getValue();

			CreatureObject * const character = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(requestTransferData.getValue().getCharacterId()));
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(character);
			time_t characterCreateTime = -1;
			FreeCtsDataTable::FreeCtsInfo const * freeCtsInfo = nullptr;
			bool freeCtsBypassTimeRestriction = false;
			if (character && playerObject)
			{
				characterCreateTime = static_cast<time_t>(NameManager::getInstance().getPlayerCreateTime(character->getNetworkId()));
				
				freeCtsInfo = FreeCtsDataTable::wouldCharacterTransferBeFree(characterCreateTime, static_cast<uint32>(transferCharacterData.getSourceStationId()), transferCharacterData.getSourceGalaxy(), static_cast<uint32>(transferCharacterData.getDestinationStationId()), transferCharacterData.getDestinationGalaxy(), false);

				// see if we can/should bypass the free CTS time restriction
				if (!freeCtsInfo && ConfigServerGame::getAllowIgnoreFreeCtsTimeRestriction() && (transferCharacterData.getTransferRequestSource() == TransferRequestMoveValidation::TRS_ingame_freects_command_transfer))
				{
					// if we got this far and the CTS request came from TRS_ingame_freects_command_transfer, then it
					// should be allowed as a free CTS, so do another check bypassing the free CTS time restriction
					// check to find any matching free CTS rule, even if the time restriction on the rule has expired
					freeCtsInfo = FreeCtsDataTable::wouldCharacterTransferBeFree(characterCreateTime, static_cast<uint32>(transferCharacterData.getSourceStationId()), transferCharacterData.getSourceGalaxy(), static_cast<uint32>(transferCharacterData.getDestinationStationId()), transferCharacterData.getDestinationGalaxy(), true);
					if (freeCtsInfo)
					{
						freeCtsBypassTimeRestriction = true;
					}
				}
			}

			LOG("CustomerService", ("CharacterTransfer: Received RequestTransferData for character [character create time (%s) free transfer (%s)] %s", ((characterCreateTime > 0) ? CalendarTime::convertEpochToTimeStringLocal(characterCreateTime).c_str() : "NA"), (freeCtsInfo ? (freeCtsBypassTimeRestriction ? "yes - bypass" : "yes") : "no "), transferCharacterData.toString().c_str()));

			bool succeeded = false;
			if(ServerWorld::getSceneId() != NewbieTutorial::getSceneId())
			{
				if(character)
				{
					// get hair
					std::string hairObjectTemplateName;
					std::string hairObjectAppearanceData;
					SlottedContainer * const container = ContainerInterface::getSlottedContainer(*character);
					if(container)
					{
						SlotId hairSlot = SlotIdManager::findSlotId(ConstCharCrcString("hair"));
						if(hairSlot != SlotId::invalid)
						{
							Container::ContainerErrorCode containerErrorCode = Container::CEC_Success;
							Container::ContainedItem hairItem = container->getObjectInSlot(hairSlot, containerErrorCode);
							const TangibleObject * hairObject = dynamic_cast<const TangibleObject *>(hairItem.getObject());
							if(hairObject)
							{
								hairObjectTemplateName = hairObject->getObjectTemplateName();
								hairObjectAppearanceData = hairObject->getAppearanceData();
							}
						}
					}
					std::string professionName;
					const CreatureObject::SkillList & skills = character->getSkillList();
					if (!skills.empty())
					{
						for (CreatureObject::SkillList::const_iterator i = skills.begin(); i != skills.end(); ++i)
						{
							const SkillObject * profession = (*i)->findProfessionForSkill();
							if (profession != nullptr)
								professionName = profession->getSkillName();
						}
					}
					if (professionName.empty())
					{
						// just get the 1st profession available
						PlayerCreationManager::StringVector professions;
						PlayerCreationManager::getProfessionVector (professions, "");
						if (! professions.empty())
						{
							professionName = professions[0];
						}
					}

					if (playerObject)
					{
						// set objvar on the creature indicating that transfer will be free,
						// so that script can include the information in the packed dictionary
						// so that the information can be transferred to the destination galaxy
						// indicating that it's a free transfer
						character->removeObjVarItem("freeCtsTransactionRuleName");

						if (freeCtsInfo)
							IGNORE_RETURN(character->setObjVarItem("freeCtsTransactionRuleName", (freeCtsInfo->ruleName.empty() ? "freeCts" : freeCtsInfo->ruleName)));

						std::vector<unsigned char> dictionaryData = character->uploadCharacterData(transferCharacterData.getWithItems(), transferCharacterData.getAllowOverride());

						character->removeObjVarItem("freeCtsTransactionRuleName");

						if(! dictionaryData.empty())
						{
							// send the response to the Connection server
							TransferCharacterData replyData(transferCharacterData);
							replyData.setCustomizationData(character->getAppearanceData());
							replyData.setScriptDictionaryData(dictionaryData);
							replyData.setObjectTemplateName(character->getObjectTemplateName());
							replyData.setObjectTemplateCrc(character->getTemplateCrc());
							replyData.setScaleFactor(character->getScaleFactor());
							replyData.setHairTemplateName(hairObjectTemplateName);
							replyData.setHairAppearanceData(hairObjectAppearanceData);
							replyData.setProfession(professionName);
							replyData.setSkillTemplate(playerObject->getSkillTemplate());
							replyData.setWorkingSkill(playerObject->getWorkingSkill());
						
							GenericValueTypeMessage<TransferCharacterData> reply("ReplyTransferData", replyData);
							send(reply, true);
							succeeded = true;
						}
						else
						{
							LOG("CustomerService", ("CharacterTransfer: Transfer failed: Could not retrieve dictionary for character object (%s) for transfer request. %s", requestTransferData.getValue().getCharacterId().getValueString().c_str(), requestTransferData.getValue().toString().c_str()));
						}
					}
					else
					{
						LOG("CustomerService", ("CharacterTransfer: Transfer failed: Could not load player object (%s) for transfer request. %s", requestTransferData.getValue().getCharacterId().getValueString().c_str(), requestTransferData.getValue().toString().c_str()));
					}
				}
				else
				{
					LOG("CustomerService", ("CharacterTransfer: Transfer failed: Could not load character object (%s) for transfer request. %s", requestTransferData.getValue().getCharacterId().getValueString().c_str(), requestTransferData.getValue().toString().c_str()));
				}
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Transfer failed: transfers from the Tutorial are not allowed. %s", transferCharacterData.toString().c_str()));
			}
			if(! succeeded)
			{
				GenericValueTypeMessage<TransferCharacterData> reply("ReplyTransferDataFail", requestTransferData.getValue());
				send(reply, true);
			}
			
			break;
		}
		case constcrc("ApplyTransferData") :
		{
			GenericValueTypeMessage<TransferCharacterData> applyTransferData(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferData for character %s", applyTransferData.getValue().toString().c_str()));
			const TransferCharacterData & transferCharacterData = applyTransferData.getValue();

			CreatureObject * character = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(transferCharacterData.getDestinationCharacterId()));
			if(character)
			{
				if(character->receiveCharacterTransferMessage(transferCharacterData.getScriptDictionaryData()))
				{
					// force CTS failure if the source character has this objvar (which has now been
					// transferred to the target character); this is for testing CTS failure handing
					if (character->getObjVars().hasItem("testing_only_force_cts_failure_testing_only"))
					{
						GenericValueTypeMessage<TransferCharacterData> applyTransferDataFail("ApplyTransferDataFail", transferCharacterData);
						send(applyTransferDataFail, true);
						LOG("CustomerService", ("CharacterTransfer: *** ERROR: Character has testing_only_force_cts_failure_testing_only objvar, sending ApplyTransferDataFail to central %s", applyTransferData.getValue().toString().c_str()));
					}
					else
					{
						GenericValueTypeMessage<TransferCharacterData> applyTransferDataSuccess("ApplyTransferDataSuccess", transferCharacterData);
						send(applyTransferDataSuccess, true);
						LOG("CustomerService", ("CharacterTransfer: Sending ApplyTransferDataSuccess for character %s", applyTransferData.getValue().toString().c_str()));
					}
				}
				else
				{
					GenericValueTypeMessage<TransferCharacterData> applyTransferDataFail("ApplyTransferDataFail", transferCharacterData);
					send(applyTransferDataFail, true);
					LOG("CustomerService", ("CharacterTransfer: *** ERROR: Sending ApplyTransferDataFail for character %s", applyTransferData.getValue().toString().c_str()));
				}
			}
			else
			{
				GenericValueTypeMessage<TransferCharacterData> applyTransferDataFail("ApplyTransferDataFail", transferCharacterData);
				send(applyTransferDataFail, true);
				LOG("CustomerService", ("CharacterTransfer: *** ERROR: Could not find object for character, sending ApplyTransferDataFail to central %s", applyTransferData.getValue().toString().c_str()));
			}
			
			break;
		}
		case constcrc("ChatEnterRoomValidationRequest") :
		{
			Archive::ReadIterator readIterator = message.begin();
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, unsigned int> > const cervreq(readIterator);

			unsigned int result = CHATRESULT_SUCCESS;

			ServerObject const * const o = ServerWorld::findObjectByNetworkId(cervreq.getValue().first.first);
			CreatureObject const * const co = (o ? o->asCreatureObject() : nullptr);
			if (co)
			{
				result = Chat::isAllowedToEnterRoom(*co, cervreq.getValue().first.second);
			}
			else
			{
				result = SWG_CHAT_ERR_INVALID_OBJECT;
			}

			GenericValueTypeMessage<std::pair<std::pair<NetworkId, unsigned int>, unsigned int> > const cervresp(
				"ChatEnterRoomValidationResponse",
				std::make_pair(
				std::make_pair(cervreq.getValue().first.first, result),
				cervreq.getValue().second));

			send(cervresp, true);
			
			break;
		}
		case constcrc("ChatQueryRoomValidationRequest") :
		{
			Archive::ReadIterator readIterator = message.begin();
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, unsigned int> > const cqrvreq(readIterator);

			bool success = false;

			ServerObject const * const o = ServerWorld::findObjectByNetworkId(cqrvreq.getValue().first.first);
			CreatureObject const * const co = (o ? o->asCreatureObject() : nullptr);
			if (co)
			{
				success = (CHATRESULT_SUCCESS == Chat::isAllowedToEnterRoom(*co, cqrvreq.getValue().first.second));
			}

			GenericValueTypeMessage<std::pair<std::pair<NetworkId, bool>, unsigned int> > const cqrvresp(
				"ChatQueryRoomValidationResponse",
				std::make_pair(
				std::make_pair(cqrvreq.getValue().first.first, success),
				cqrvreq.getValue().second));

			send(cqrvresp, true);
			
			break;
		}
		case constcrc("PlayerDroppedFromGameServerCrash") :
		{
			Archive::ReadIterator readIterator = message.begin();
			GenericValueTypeMessage<std::set<NetworkId> > const m(readIterator);

			std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
			std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();

			std::set<NetworkId>::const_iterator const end = m.getValue().end();
			for (std::set<NetworkId>::const_iterator iter = m.getValue().begin(); iter != end; ++iter)
			{
				if (connectedCharacterLfgData.count(*iter) > 0)
					ServerUniverse::removeConnectedCharacterData(*iter);

				if (connectedCharacterBiographyData.count(*iter) > 0)
					ServerUniverse::removeConnectedCharacterBiographyData(*iter);
			}
			
			break;
		}
		default :
		{
			ServerConnection::emitMessage(m);
			break;
		}
	}
}

// ----------------------------------------------------------------------

void ConnectionServerConnection::setSyncStamps(uint16 syncStampShort, uint32 syncStampLong)
{
	m_syncStampShort = syncStampShort;
	m_syncStampLong = syncStampLong;
}

// ----------------------------------------------------------------------

uint16 ConnectionServerConnection::getSyncStampShort() const
{
	return m_syncStampShort;
}

// ----------------------------------------------------------------------

uint32 ConnectionServerConnection::getSyncStampLong() const
{
	return m_syncStampLong;
}

// ======================================================================

