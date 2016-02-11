//========================================================================
//
// TangibleController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleController.h"

#include "SwgGameServer/CombatEngine.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/TangibleObject.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "serverNetworkMessages/MessageQueueCommandQueueForceExecuteCommandFwd.h"
#include "serverNetworkMessages/MessageQueuePvpCommand.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ProsePackage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"

#include "sharedObject/NetworkIdManager.h"
#include "swgServerNetworkMessages/MessageQueueCombatDamageList.h"
#include "swgServerNetworkMessages/MessageQueueDirectDamage.h"
#include "swgServerNetworkMessages/MessageQueueResourceAdd.h"

//-----------------------------------------------------------------------

TangibleController::TangibleController(TangibleObject *newOwner) :
	ServerController(newOwner)
{
}

//-----------------------------------------------------------------------

TangibleController::~TangibleController()
{
}

//-----------------------------------------------------------------------

void TangibleController::setAuthoritative(bool newAuthoritative)
{
	ServerController::setAuthoritative(newAuthoritative);
	// pvp needs to be notified after an object is both initialized and made authoritative.
	if (newAuthoritative)
	{
		TangibleObject* owner = dynamic_cast<TangibleObject *>(getOwner());
		NOT_NULL(owner);
		if (owner->isInitialized())
			Pvp::handleAuthorityAcquire(*owner);
	}
}

//-----------------------------------------------------------------------

void TangibleController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	TangibleObject * const owner = getTangibleOwner();
	
	switch (message)
	{
	case CM_pvpMessage:
		{
			const MessageQueuePvpCommand *pvpMsg = dynamic_cast<const MessageQueuePvpCommand *>(data);
			NOT_NULL(pvpMsg);
			Pvp::handlePvpMessage(*owner, *pvpMsg);
		}
		break;
		
	case CM_combatDamageList:
		{
			if (owner->isAuthoritative())
			{
				const MessageQueueCombatDamageList *damageListMessage = 
					dynamic_cast<const MessageQueueCombatDamageList *>(data);
				NOT_NULL(damageListMessage);
				
				const std::vector<CombatEngineData::DamageData> & damageList = 
					damageListMessage->getDamageList();
				if (damageList.size() == 0)
					break;
				
				if (!owner->isInCombat())
				{
					WARNING(true, ("TangibleController setting owner %s to be in "
						"combat to apply damage; we should be able to avoid doing "
						"this!", owner->getNetworkId().getValueString().c_str()));
					owner->setInCombat(true);
				}
				
				// put the damage on the object's current damage list
				
				
				owner->createCombatData();

				CombatEngineData::CombatData * combatData = owner->getCombatData();

				for (std::vector<CombatEngineData::DamageData>::const_iterator 
					iter = damageList.begin(); iter != damageList.end(); ++iter)
				{
					combatData->defenseData.damage.push_back(*iter);
				}
			}
		}
		break;
	
	case CM_directDamage:
		{
			const MessageQueueDirectDamage *damageMessage = 
				dynamic_cast<const MessageQueueDirectDamage *>(data);
			NOT_NULL(damageMessage);
			CombatEngine::damage(*owner, 
				static_cast<ServerWeaponObjectTemplate::DamageType>(damageMessage->getDamageType()), 
				static_cast<uint16>(damageMessage->getHitLocation()),
				damageMessage->getDamageDone());
		}
		break;

	case CM_setOwnerId:
		{
			const MessageQueueNetworkId * const msg = safe_cast<const MessageQueueNetworkId *>(data);
			if(msg)
			{
				owner->setOwnerId(msg->getNetworkId());
			}
		}
		break;
	case CM_setMaxHitPoints:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setMaxHitPoints(msg->getValue());
			}
		}
		break;
	case CM_setVisible:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setVisible(msg->getValue());
			}
		}
		break;
	case CM_hideFromClient:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setHidden(msg->getValue());
			}
		}
		break;
	case CM_setInvulnerable:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setInvulnerable(msg->getValue());
			}
		}
		break;
	case CM_addLocationTarget:
		{
			const MessageQueueGenericValueType<LocationData> * const msg = safe_cast<const MessageQueueGenericValueType<LocationData> *>(data);
			if(msg)
			{
				owner->addLocationTarget(msg->getValue());
			}
		}
		break;
	case CM_removeLocationTarget:
		{
			const MessageQueueGenericValueType<Unicode::String> * const msg = safe_cast<const MessageQueueGenericValueType<Unicode::String> *>(data);
			if(msg)
			{
				owner->removeLocationTarget(msg->getValue());
			}
		}
		break;
	case CM_setInCombat:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setInCombat(msg->getValue());
			}
		}
		break;
	case CM_addAim:
		{
			owner->addAim();
		}
		break;
	case CM_clearAims:
		{
			owner->clearAims();
		}
		break;
	case CM_alterHitPoints:
		{
			const MessageQueueGenericValueType<std::pair<int, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, NetworkId> > *>(data);
			if(msg)
			{
				owner->alterHitPoints(msg->getValue().first, false, msg->getValue().second);
			}
		}
		break;
	case CM_addHate:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, float> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, float> > *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_addHate) The message data should never be nullptr"));

			if(msg)
			{
				owner->addHate(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setHate:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, float> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, float> > *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_setHate) The message data should never be nullptr"));

			if(msg)
			{
				owner->setHate(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_removeHateTarget:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_removeHateTarget) The message data should never be nullptr"));

			if(msg)
			{
				owner->removeHateTarget(msg->getValue());
			}
		}
		break;
	case CM_clearHateList:
		{
			owner->clearHateList();
		}
		break;
	case CM_resetHateTimer:
		{
			owner->resetHateTimer();
		}
		break;
	case CM_forceHateTarget:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_removeHateTarget) The message data should never be nullptr"));

			if(msg)
			{
				owner->forceHateTarget(msg->getValue());
			}
		}
		break;
	case CM_autoExpireHateListTargetEnabled:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_setHateListExpireTargetEnabled) The message data should never be nullptr"));

			if (msg != nullptr)
			{
				owner->setHateListAutoExpireTargetEnabled(msg->getValue());
			}
		}
		break;
	case CM_setCraftedId:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				owner->setCraftedId(msg->getValue());
			}
		}
		break;
	case CM_startCraftingSession:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				CreatureObject * crafter = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(msg->getValue()));
				DEBUG_FATAL(!crafter, ("%s:%s received startCraftingSession controller message for crafter %s, but crafter %s is not loaded on this server!", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str(), msg->getValue().getValueString().c_str(), msg->getValue().getValueString().c_str()));
				if(crafter)
					owner->startCraftingSession(*crafter);
			}
		}
		break;
	case CM_addObjectToOutputSlot:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *>(data);
			if(msg)
			{
				ServerObject * object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getValue().first));
				DEBUG_FATAL(! object, ("%s:%s received addObjectToOutputSlot controller message, but the ingredient %s is not loaded on this server!", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str(), msg->getValue().first.getValueString().c_str()));
				if(object)
				{
					ServerObject * transferer = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getValue().second));
					// transferer is allowed to be nullptr
					owner->addObjectToOutputSlot(*object, transferer);
				}
			}
		}
		break;
	case CM_setCraftingManufactureSchematic:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				ManufactureSchematicObject * schematic = safe_cast<ManufactureSchematicObject *>(NetworkIdManager::getObjectById(msg->getValue()));
				DEBUG_FATAL(! schematic, ("%s:%s received setCraftingManufactureSchematic controller message, but the ingredient %s is not loaded on this server!", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str(), msg->getValue().getValueString().c_str()));
				if(schematic)
				{
					owner->setCraftingManufactureSchematic(*schematic);
				}
			}
		}
		break;
	case CM_clearCraftingManufactureSchematic:
		{
			owner->clearCraftingManufactureSchematic();
		}
		break;
	case CM_setCraftingPrototype:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				ServerObject * prototype = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getValue()));
				DEBUG_FATAL(! prototype, ("%s:%s received setCraftingPrototype controller message, but the ingredient %s is not loaded on this server!", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str(), msg->getValue().getValueString().c_str()));
				if(prototype)
				{
					owner->setCraftingPrototype(*prototype);
				}
			}
		}
		break;
	case CM_clearCraftingPrototype:
		{
			owner->clearCraftingPrototype();
		}
		break;
	case CM_setCreatorId:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				owner->setCreatorId(msg->getValue());
			}
		}
		break;
	case CM_setCreatorXp:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setCreatorXp(msg->getValue());
			}
		}
		break;
	case CM_setCreatorXpType:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setCreatorXpType(msg->getValue());
			}
		}
		break;
	case CM_setVisibleComponents:
		{
			const MessageQueueGenericValueType<std::vector<int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::vector<int> > *>(data);
			if(msg)
			{
				owner->setVisibleComponents(msg->getValue());
			}
		}
		break;
	case CM_setCraftedType:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setCraftedType(msg->getValue());
			}
		}
		break;
	case CM_setCondition:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setCondition(msg->getValue());
			}
		}
		break;
	case CM_clearCondition:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->clearCondition(msg->getValue());
			}
		}
		break;
	case CM_npcConversationStart:
		{
			const MessageQueueStartNpcConversation * const msg = NON_NULL(dynamic_cast<const MessageQueueStartNpcConversation *>(data));
			
			const CachedNetworkId npcId (msg->getNpc ());
			TangibleObject * const npcObject = dynamic_cast<TangibleObject *>(npcId.getObject ());
			if (npcObject)
			{
				owner->startNpcConversation (*npcObject, msg->getConversationName().c_str(), msg->getStarter(), msg->getAppearanceOverrideTemplateCrc());
			}
		}
		break;
		
	case CM_npcConversationStop:
		{				
			owner->endNpcConversation ();
		}
		break;
		
	case CM_npcConversationSelect:
		{
			owner->respondToNpc (static_cast<int>(value));
		}
		break;
	case CM_forwardNpcConversationMessage:
		{
			typedef std::pair<std::pair<StringId, ProsePackage>, Unicode::String> Payload;
			const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
			if(msg)
			{
				const Payload & payload = msg->getValue ();
				owner->sendNpcConversationMessage(payload.first.first, payload.first.second, payload.second);
			}
		}
		break;
	case CM_addNpcConversationResponse:
		{
			typedef std::pair<StringId, ProsePackage> Payload;
			const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
			if(msg)
			{
				const Payload & payload = msg->getValue ();
				owner->addNpcConversationResponse(payload.first, payload.second);
			}
		}
		break;
	case CM_removeNpcConversationResponse:
		{
			typedef std::pair<StringId, ProsePackage> Payload;
			const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
			if(msg)
			{
				const Payload & payload = msg->getValue ();
				owner->removeNpcConversationResponse(payload.first, payload.second);
			}
		}
		break;
	case CM_sendNpcConversationResponses:
		{
			owner->sendNpcConversationResponses();
		}
		break;
	case CM_playerResponseToNpcConversation:
		{
			typedef std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > Payload;
			const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
			if(msg)
			{
				const Payload & payload = msg->getValue ();
				owner->handlePlayerResponseToNpcConversation(payload.first, payload.second.first, payload.second.second.first, payload.second.second.second);
			}
		}
		break;
	case CM_setBioLink:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				const NetworkId & linkId = msg->getValue();
				if (linkId != NetworkId::cms_invalid)
					owner->setBioLink(linkId);
				else
					owner->clearBioLink();
			}
		}
		break;
	case CM_commandQueueForceExecuteCommandFwd:
		{
			MessageQueueCommandQueueForceExecuteCommandFwd const * const cmdMsg = dynamic_cast<const MessageQueueCommandQueueForceExecuteCommandFwd *>(data);
			Command::ErrorCode status = static_cast<Command::ErrorCode>(cmdMsg->getStatus());

			owner->forceExecuteCommand(CommandTable::getCommand(cmdMsg->getCommandHash()), cmdMsg->getTargetId(), cmdMsg->getParams(), status, cmdMsg->getCommandIsFromCommandQueue());
		}
		break;
	case CM_commandQueueEnqueue:
		{
			handleCommandQueueEnqueue(dynamic_cast<const MessageQueueCommandQueueEnqueue *>(data));
		}
		break;
	case CM_commandQueueRemove:
		{
			MessageQueueCommandQueueRemove const *msg = dynamic_cast<MessageQueueCommandQueueRemove const *>(data);
			owner->commandQueueRemove(msg->getSequenceId());
		}
		break;
	case CM_commandQueueEnqueueFwd:
		{
			handleCommandQueueEnqueueFwd(dynamic_cast<const MessageQueueCommandQueueEnqueueFwd *>(data));
		}
		break;
	case CM_setAttackableOverride:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setAttackableOverride(msg->getValue());
			}
		}
		break;
	case CM_addPassiveReveal:
		{
			const MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > *>(data);
			if(msg)
			{
				owner->addPassiveReveal(msg->getValue().second, msg->getValue().first.first, (msg->getValue().first.second != 0));
			}
		}
		break;
	case CM_removePassiveReveal:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if(msg)
			{
				owner->removePassiveReveal(msg->getValue());
			}
		}
		break;
	case CM_removeAllPassiveReveal:
		{
			owner->removeAllPassiveReveal();
		}
		break;
	case CM_addHateOverTime:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > > *>(data);
			if(msg)
			{
				owner->addHateOverTime(msg->getValue().first, msg->getValue().second.second, msg->getValue().second.first);
			}
		}
		break;
	case CM_addUserToAccessList:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_addUserToAccessList) The message data should never be nullptr"));

			if(msg)
			{
				if(msg->getValue().first < 0)
					owner->addUserToAccessList(msg->getValue().second);
				else
					owner->addGuildToAccessList(msg->getValue().first);
			}
		}
		break;
	case CM_removeUserFromAccessList:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			WARNING((msg == nullptr), ("TangibleController::handleMessage(CM_removeUserFromAccessList) The message data should never be nullptr"));

			if(msg)
			{
				if(msg->getValue().first < 0)
					owner->removeUserFromAccessList(msg->getValue().second);
				else
					owner->removeGuildFromAccessList(msg->getValue().first);
			}
		}
		break;
	case CM_clearUserAccessList:
		{
			MessageQueueGenericValueType<bool> const *msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);
			if(msg)
			{
				if(msg->getValue())
					owner->clearGuildAccessList();
				else
					owner->clearUserAccessList();
			}
		}
		break;
	case CM_addObjectEffect:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float > > > > > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float > > > > > const *>(data);
			if(msg)
			{
				owner->addObjectEffect(msg->getValue().second.first, msg->getValue().second.second.first, msg->getValue().second.second.second.first, msg->getValue().second.second.second.second, msg->getValue().first);
			}
		}
		break;
	case CM_removeObjectEffect:
		{
			MessageQueueGenericValueType<std::string> const * msg = safe_cast<MessageQueueGenericValueType<std::string> const *>(data);
			if(msg)
			{
				owner->removeObjectEffect(msg->getValue());
			}
		}
		break;
	case CM_removeAllObjectEffect:
		{
			MessageQueueGenericValueType<bool> const *msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);
			if(msg)
			{
				if(msg->getValue())
					owner->removeAllObjectEffects();
			}
		}
		break;
	default:
		ServerController::handleMessage(message, value, data, flags);
		break;
	}
}

//----------------------------------------------------------------------

void TangibleController::handleCommandQueueEnqueue(MessageQueueCommandQueueEnqueue const *msg)
{
	if (!msg)
		return;

	TangibleObject * const owner = getTangibleOwner();

	if (owner->getClient())
		owner->getClient()->resetIdleTime();

	owner->commandQueueEnqueue(
		CommandTable::getCommand(msg->getCommandHash()),
		msg->getTargetId(),
		msg->getParams(),
		msg->getSequenceId());
}

//----------------------------------------------------------------------

void TangibleController::handleCommandQueueEnqueueFwd(MessageQueueCommandQueueEnqueueFwd const *msg)
{
	// this is an enqueue message forwarded from another server
	if (!msg)
		return;

	TangibleObject * const owner = getTangibleOwner();

	int priority = msg->getPriority();
	if (priority < 0 || priority >= Command::CP_NumberOfPriorities)
		priority = Command::CP_Default;
	
	owner->commandQueueEnqueue(
		CommandTable::getCommand(msg->getCommandHash()),
		msg->getTargetId(),
		msg->getParams(),
		msg->getSequenceId(),
		msg->getClearable(),
		static_cast<Command::Priority>(priority),
		msg->getFromServer());
}

//-----------------------------------------------------------------------

const TangibleObject * TangibleController::getTangibleOwner() const
{
	return getServerOwner()->asTangibleObject();
}

// ----------------------------------------------------------------------

TangibleObject * TangibleController::getTangibleOwner()
{
	return getServerOwner()->asTangibleObject();
}

//-----------------------------------------------------------------------

TangibleController * TangibleController::asTangibleController()
{
	return this;
}

//----------------------------------------------------------------------

TangibleController const * TangibleController::asTangibleController() const
{
	return this;
}

//----------------------------------------------------------------------

AiTurretController * TangibleController::asAiTurretController()
{
	return nullptr;
}

//----------------------------------------------------------------------

AiTurretController const * TangibleController::asAiTurretController() const
{
	return nullptr;
}

//========================================================================
