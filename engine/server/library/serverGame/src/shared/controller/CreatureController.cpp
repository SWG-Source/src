//========================================================================
//
// CreatureController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CreatureController.h"

#include "SwgGameServer/CombatEngine.h"
#include "SwgGameServer/ConfigCombatEngine.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/InstallationObject.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerSecureTradeManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/WeaponObject.h"
#include "serverNetworkMessages/MessageQueueAlterAttribute.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "serverNetworkMessages/MessageQueueSetState.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedLog/Log.h"
#include "sharedGame/CommandTable.h"
#include "sharedUtility/Location.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/Waypoint.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/IncubatorCommitMessage.h"
#include "sharedNetworkMessages/MessageQueueCombatCommand.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsChangeRequest.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericString.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdPair.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueuePushCreature.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"
#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueSlowDownEffect.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/UpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateTransformWithParentMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/MovementTable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

namespace CreatureControllerNamespace
{
	template<typename ObjectType>
	ObjectType & getOwner(Controller * controller)
	{
		NOT_NULL(controller);
		NOT_NULL(controller->getOwner());
		ObjectType * owner = static_cast<ObjectType *>(controller->getOwner());
		return *owner;
	}
}

//-----------------------------------------------------------------------

CreatureController::CreatureController(CreatureObject * newOwner) :
	TangibleController(newOwner),
	m_secureTrade(0),
	m_secureTradeInitiator(0)
{
}

//-----------------------------------------------------------------------

CreatureController::~CreatureController()
{
	if (getHibernate())
		ObjectTracker::removeHibernatingAI();

	if (m_secureTrade)
		m_secureTrade->cancelTrade(*getCreature());
}

//-----------------------------------------------------------------------

void CreatureController::handleMessage(const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	CreatureObject * const owner = static_cast<CreatureObject*>(getOwner());
	DEBUG_FATAL(!owner, ("Owner is nullptr in CreatureController::handleMessage\n"));

	switch (message)
	{
	case CM_netUpdateTransform:
	{
		// if the auth server sent this to us, forward it to clients
		const MessageQueueDataTransform * const msg = NON_NULL(dynamic_cast<const MessageQueueDataTransform *>(data));
		owner->setLookAtYaw(msg->getLookAtYaw(), msg->getUseLookAtYaw());
		if (flags&GameControllerMessageFlags::SOURCE_REMOTE_SERVER)
		{
			// If we're not contained by what we are attached to, then transform
			// updates are not relevant to clients because our position is
			// dictated purely by our containing object.
			if (owner->getAttachedTo() == ContainerInterface::getContainedByObject(*owner))
			{
				UpdateTransformMessage utm(owner->getNetworkId(), msg->getSequenceNumber(), msg->getTransform(), static_cast<int8>(msg->getSpeed()), msg->getLookAtYaw(), msg->getUseLookAtYaw());
				owner->sendToClientsInUpdateRange(utm, flags & GameControllerMessageFlags::RELIABLE, false);
			}
		}
		TangibleController::handleMessage(message, value, data, flags);
	}
	break;
	case CM_netUpdateTransformWithParent:
	{
		// if the auth server sent this to us, forward it to clients
		const MessageQueueDataTransformWithParent * const msg = NON_NULL(dynamic_cast<const MessageQueueDataTransformWithParent *>(data));
		owner->setLookAtYaw(msg->getLookAtYaw(), msg->getUseLookAtYaw());
		if (flags&GameControllerMessageFlags::SOURCE_REMOTE_SERVER)
		{
			// If we're not contained by what we are attached to, then transform
			// updates are not relevant to clients because our position is
			// dictated purely by our containing object.
			if (owner->getAttachedTo() == ContainerInterface::getContainedByObject(*owner))
			{
				UpdateTransformWithParentMessage utm(owner->getNetworkId(), msg->getSequenceNumber(), msg->getParent(), msg->getTransform(), static_cast<int8>(msg->getSpeed()), msg->getLookAtYaw(), msg->getUseLookAtYaw());
				owner->sendToClientsInUpdateRange(utm, flags & GameControllerMessageFlags::RELIABLE, false);
			}
		}
		TangibleController::handleMessage(message, value, data, flags);
	}
	break;

	case CM_clientResourceHarvesterActivate:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));

		InstallationObject * const harvester = dynamic_cast<InstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getNetworkId()));

		UNREF(harvester);

		if (harvester && harvester->isOnAdminList(*owner))
			harvester->activate(owner->getNetworkId());
	}
	break;

	case CM_clientResourceHarvesterDeactivate:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));

		InstallationObject * const harvester = dynamic_cast<InstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getNetworkId()));

		if (harvester && harvester->isOnAdminList(*owner))
			harvester->deactivate();
	}
	break;

	case CM_clientResourceHarvesterListen:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));

		HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getNetworkId()));

		if (harvester && owner)
			harvester->addSynchronizedUiClient(*owner);
	}
	break;

	case CM_clientResourceHarvesterStopListening:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));

		HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getNetworkId()));

		if (harvester && owner)
			harvester->removeSynchronizedUiClient(owner->getNetworkId());
	}
	break;

	case CM_clientResourceHarvesterResourceSelect:
	{
		const MessageQueueNetworkIdPair * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkIdPair *>(data));

		HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getFirstNetworkId()));

		harvester->selectResource(msg->getSecondNetworkId(), owner->getNetworkId());
	}
	break;

	case CM_clientResourceHarvesterGetResourceData:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));

		typedef std::vector<InstallationResourceData> DataVector;
		DataVector dv;

		HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getNetworkId()));
		ServerObject * const player = owner;

		if (harvester && player)
		{
			harvester->getResourceData(dv);

			if (!dv.empty())
			{
				MessageQueueHarvesterResourceData * const newData = new MessageQueueHarvesterResourceData(msg->getNetworkId(), dv);
				appendMessage(static_cast<int>(CM_clientResourceHarvesterResourceData), 0.0f, newData, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
	}
	break;

	case CM_clientResourceHarvesterEmptyHopper:
	{
		const MessageQueueResourceEmptyHopper * const msg = NON_NULL(dynamic_cast<const MessageQueueResourceEmptyHopper *>(data));

		HarvesterInstallationObject * const harvester = dynamic_cast<HarvesterInstallationObject *>(ServerWorld::findObjectByNetworkId(msg->getHarvesterId()));

		if (harvester && owner && harvester->isOnHopperList(*owner))
			harvester->emptyHopper(msg->getPlayerId(), msg->getResourceId(), msg->getAmount(), msg->getDiscard(), msg->getSequenceId());
	}
	break;
	case CM_getTokenForObject:
	{
		WARNING_STRICT_FATAL(true, ("Received CM_getTokenForObject controller message. This message is depracated"));
	}
	break;

	case CM_getWaypointForObject:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));
		if (msg)
		{
			if (owner)
			{
				PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
				if (player)
				{
					const ServerObject * target = ServerWorld::findObjectByNetworkId(msg->getNetworkId());
					if (target)
					{
						const Object * topMost = ContainerInterface::getFirstParentInWorld(*target);
						if (topMost)
						{
							const Vector pos = topMost->getPosition_w();
							IGNORE_RETURN(player->createWaypoint(Location(pos, NetworkId::cms_invalid, Location::getCrcBySceneName(ServerWorld::getSceneId())), true));
						}
					}
				}
			}
		}
	}
	break;
	case CM_secureTrade:
	{
		if (owner->isAuthoritative())
		{
			handleSecureTradeMessage(dynamic_cast<const MessageQueueSecureTrade *>(data));
		}
		// @todo else
			//resend to auth
	}
	break;
	case CM_clientLookAtTarget:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));
		owner->setLookAtTarget(msg->getNetworkId());
	}
	break;
	case CM_clientIntendedTarget:
	{
		const MessageQueueNetworkId * const msg = NON_NULL(dynamic_cast<const MessageQueueNetworkId *>(data));
		owner->setIntendedTarget(msg->getNetworkId());
	}
	break;
	case CM_clientMoodChange:
		if (value < 0.0f || value > 255.0f)
			WARNING(true, ("Out of range mood: %f", value));
		else
			owner->setMood(static_cast<uint32>(value));

		break;

	case CM_setState:
	{
		if (!(flags&GameControllerMessageFlags::SOURCE_REMOTE_CLIENT))
		{
			MessageQueueSetState const *msg = safe_cast<MessageQueueSetState const *>(data);
			owner->setState(static_cast<States::Enumerator>(msg->getState()), msg->getValue());
		}
	}
	break;
	case CM_setAttribute:
	{
		const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > *>(data);
		if (owner)
		{
			if (msg)
			{
				const std::pair<Attributes::Enumerator, Attributes::Value> & attributePair = msg->getValue();
				owner->setAttribute(attributePair.first, attributePair.second);
			}
			DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setAttribute message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
		}
	}
	break;
	case CM_setMaxAttribute:
	{
		const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > *>(data);
		// set the object value and send a message to all it's proxies
		if (msg)
		{
			owner->setMaxAttribute(msg->getValue().first, msg->getValue().second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMaxAttribute message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setMentalState:
	{
		const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > *>(data);
		// set the object value and send a message to all it's proxies
		if (msg)
		{
			owner->setMentalState(msg->getValue().first, msg->getValue().second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMentalState message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setMentalStateToward:
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > > *>(data);
		if (msg)
		{
			owner->setMentalStateToward(msg->getValue().first, msg->getValue().second.first, msg->getValue().second.second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMentalStateToward message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setCover:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->setCover(msg->getValue());
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setCover message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setMentalStateTowardClampBehavior:
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > > *>(data);
		if (msg)
		{
			owner->setMentalStateTowardClampBehavior(
				msg->getValue().first,
				msg->getValue().second.first,
				msg->getValue().second.second.first,
				msg->getValue().second.second.second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMentalStateTowardClampBehavior message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setMaxMentalState:
	{
		const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > *>(data);
		if (msg)
		{
			owner->setMaxMentalState(msg->getValue().first, msg->getValue().second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMaxMentalState message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setMentalStateDecay:
	{
		const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> > *>(data);
		if (msg)
		{
			owner->setMentalStateDecay(msg->getValue().first, msg->getValue().second);
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setMentalStateDecay message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setPosture:
	{
		const MessageQueuePosture * const msg = safe_cast<const MessageQueuePosture *>(data);

		if (msg)
		{
			owner->setPosture(msg->getPosture(), msg->isClientImmediate());
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setPosture message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setLocomotion:
	{
		const MessageQueueGenericValueType<Locomotions::Enumerator> * const msg = safe_cast<const MessageQueueGenericValueType<Locomotions::Enumerator> *>(data);
		if (msg)
		{
			owner->setLocomotion(msg->getValue());
		}
		DEBUG_WARNING(!msg, ("CreatureController (%s:%s) received a CM_setLocomotion message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
	}
	break;
	case CM_setGroupInviter:
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > > *>(data);
		if (msg)
		{
			owner->setGroupInviter(msg->getValue().second.first, msg->getValue().first, msg->getValue().second.second);
		}
	}
	break;
	case CM_setPerformanceType:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->setPerformanceType(msg->getValue());
		}
	}
	break;
	case CM_setPerformanceStartTime:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->setPerformanceStartTime(msg->getValue());
		}
	}
	break;
	case CM_setPerformanceListenTarget:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setPerformanceListenTarget(msg->getValue());
		}
	}
	break;
	case CM_setPerformanceWatchTarget:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setPerformanceWatchTarget(msg->getValue());
		}
	}
	break;
	case CM_setModValue:
	{
		const
			MessageQueueGenericValueType<std::pair<std::string, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int> > *>(data);
		if (msg)
		{
			owner->setModValue(msg->getValue().first, msg->getValue().second);
		}
	}
	break;
	case CM_grantCommand:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->grantCommand(msg->getValue(), false);
		}
	}
	break;
	case CM_grantExperiencePoints:
	{
		const MessageQueueGenericValueType<std::pair<std::string, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int> > *>(data);
		if (msg)
		{
			owner->grantExperiencePoints(msg->getValue().first, msg->getValue().second);
		}
	}
	break;
	case CM_incrementKillMeter:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->incrementKillMeter(msg->getValue());
		}
	}
	break;
	case CM_grantSkill:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			const SkillObject * newSkill = SkillManager::getInstance().getSkill(msg->getValue());
			if (newSkill)
			{
				owner->grantSkill(*newSkill);
			}
		}
	}
	break;
	case CM_revokeCommand:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->revokeCommand(msg->getValue(), false);
		}
	}
	break;
	case CM_revokeSkill:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			const SkillObject * oldSkill = SkillManager::getInstance().getSkill(msg->getValue());
			if (oldSkill)
			{
				LOG("CustomerService", ("Skill: Removing skill %s from character %s via controller message.",
					msg->getValue().c_str(), owner->getNetworkId().getValueString().c_str()));
				owner->revokeSkill(*oldSkill);
			}
		}
	}
	break;
	case CM_alterAttribute:
	{
		const MessageQueueAlterAttribute * const msg = safe_cast<const MessageQueueAlterAttribute *>(data);
		if (msg)
		{
			owner->alterAttribute(msg->getAttrib(),
				msg->getDelta(),
				msg->getCheckIncapacitation(),
				msg->getSource()
			);
		}
	}
	break;
	case CM_addAttributeModifier:
	{
		const MessageQueueGenericValueType<AttribMod::AttribMod> * const msg = safe_cast<const MessageQueueGenericValueType<AttribMod::AttribMod> *>(data);
		if (msg)
		{
			owner->addAttributeModifier(msg->getValue());
		}
	}
	break;
	case CM_removeAttributeModifiers:
	{
		const MessageQueueGenericValueType<Attributes::Enumerator> * const msg = safe_cast<const MessageQueueGenericValueType<Attributes::Enumerator> *>(data);
		if (msg)
		{
			owner->removeAttributeModifiers(msg->getValue());
		}
	}
	break;
	case CM_removeAllAttributeModifiers:
	{
		owner->removeAllAttributeModifiers();
	}
	break;
	case CM_removeAllAttributeAndSkillmodModifiers:
	{
		owner->removeAllAttributeAndSkillmodMods();
	}
	break;
	case CM_setCurrentWeapon:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			WeaponObject * weapon = safe_cast<WeaponObject *>(NetworkIdManager::getObjectById(msg->getValue()));
			if (weapon)
				owner->setCurrentWeapon(*weapon);
		}
	}
	break;
	case CM_setGroup:
	{
		const MessageQueueGenericValueType<std::pair<bool, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<bool, NetworkId> > *>(data);
		if (msg)
		{
			GroupObject * group = safe_cast<GroupObject *>(NetworkIdManager::getObjectById(msg->getValue().second));

			if (group)
			{
				owner->setGroup(group, msg->getValue().first);
			}
			else if (!msg->getValue().second.isValid())
			{
				if (owner->getGroup())
					owner->getGroup()->onGroupMemberRemoved(owner->getNetworkId(), msg->getValue().first);
			}
		}
	}
	break;
	case CM_setIncapacitated:
	{
		const MessageQueueGenericValueType<std::pair<bool, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<bool, NetworkId> > *>(data);
		if (msg != nullptr)
			owner->setIncapacitated(msg->getValue().first, msg->getValue().second);
	}
	break;
	case CM_setSayMode:
	{
		const MessageQueueGenericValueType<unsigned long> * const msg = safe_cast<const MessageQueueGenericValueType<unsigned long> *>(data);
		if (msg)
		{
			owner->setSayMode(msg->getValue());
		}
	}
	break;
	case CM_setAnimationMood:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->setAnimationMood(msg->getValue());
		}
	}
	break;
	case CM_setScaleFactor:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setScaleFactor(msg->getValue());
		}
	}
	break;
	case CM_setShockWounds:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->setShockWounds(msg->getValue());
		}
	}
	break;
	case CM_setLookAtTarget:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setLookAtTarget(msg->getValue());
		}
	}
	break;
	case CM_setIntendedTarget:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setIntendedTarget(msg->getValue());
		}
	}
	break;
	case CM_setMovementStationary:
	{
		owner->setMovementStationary();
	}
	break;
	case CM_setMovementWalk:
	{
		owner->setMovementWalk();
	}
	break;
	case CM_setMovementRun:
	{
		owner->setMovementRun();
	}
	break;
	case CM_setSlopeModAngle:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setSlopeModAngle(msg->getValue());
		}
	}
	break;
	case CM_setSlopeModPercent:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setSlopeModPercent(msg->getValue());
		}
	}
	break;
	case CM_setWaterModPercent:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setWaterModPercent(msg->getValue());
		}
	}
	break;
	case CM_setMovementScale:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setMovementScale(msg->getValue());
		}
	}
	break;
	case CM_setMovementPercent:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setMovementPercent(msg->getValue());
		}
	}
	break;
	case CM_setTurnPercent:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setTurnPercent(msg->getValue());
		}
	}
	break;
	case CM_setAccelScale:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setAccelScale(msg->getValue());
		}
	}
	break;
	case CM_setAccelPercent:
	{
		const MessageQueueGenericValueType<float> * const msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if (msg)
		{
			owner->setAccelPercent(msg->getValue());
		}
	}
	break;
	case CM_sitOnObject:
	{
		const MessageQueueSitOnObject * const msg = safe_cast<const MessageQueueSitOnObject *>(data);
		if (msg)
		{
			owner->sitOnObject(msg->getChairCellId(), msg->getChairPosition_p());
		}
	}
	break;
	case CM_setMasterId:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setMasterId(msg->getValue());
		}
	}
	break;
	case CM_setHouse:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			owner->setHouse(CachedNetworkId(msg->getValue()));
		}
	}
	break;

	case CM_setDifficulty:
	case CM_setLevel:
	{
		MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
		if (msg)
			owner->setLevel(static_cast<int16>(msg->getValue()));
	}
	break;
	case CM_recalculateLevel:
	{
		MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
		if (msg)
			owner->recalculateLevel();
	}
	break;

	case CM_creatureSetBaseWalkSpeed:
		owner->setBaseWalkSpeed(value);
		break;

	case CM_creatureSetBaseRunSpeed:
		owner->setBaseRunSpeed(value);
		break;

	case CM_emergencyDismountForRider:
		owner->emergencyDismountForRider();
		break;

	case CM_detachRiderForMount:
	{
		typedef MessageQueueGenericValueType<NetworkId> Message;
		Message const * const msg = safe_cast<Message const *>(data);

		if (msg != 0)
		{
			owner->detachRider(msg->getValue());
		}
		break;
	}

	case CM_detachAllRidersForMount:
		owner->detachAllRiders();
		break;

	case CM_setAppearanceFromObjectTemplate:
	{
		MessageQueueGenericValueType<std::string> const *const msg = dynamic_cast<MessageQueueGenericValueType<std::string> const *>(data);
		if (owner && msg)
			owner->setAlternateAppearance(msg->getValue());
		else
			WARNING(true, ("CreatureController: received CM_setAppearanceFromObjectTemplate but owner or message was nullptr."));
	}
	break;

	case CM_makeDead:
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *>(data);
		if (msg)
		{
			owner->makeDead(msg->getValue().first, msg->getValue().second);
		}
	}
	break;

	case CM_pushCreature:
	{
		const MessageQueuePushCreature * const msg = safe_cast<const MessageQueuePushCreature *>(data);
		if (msg)
		{
			ServerObject * defender = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getDefender()));
			if (defender != nullptr)
			{
				if (defender->asCreatureObject() != nullptr)
				{
					defender->asCreatureObject()->pushedMe(msg->getAttacker(), msg->getAttackerPos(), msg->getDefenderPos(), msg->getDistance());
				}
				else
				{
					WARNING(true, ("Received MessageQueuePushCreature for non-creature defender %s", msg->getDefender().getValueString().c_str()));
				}
			}
		}
	}
	break;

	case CM_slowDownEffect:
	{
		const MessageQueueSlowDownEffect * const msg = safe_cast<const MessageQueueSlowDownEffect * const>(data);
		if (msg)
		{
			ServerObject * target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getTarget()));
			if (target != nullptr && target->asTangibleObject() != nullptr)
			{
				if (owner->isAuthoritative())
					owner->addSlowDownEffect(*(target->asTangibleObject()), msg->getConeLength(), msg->getConeAngle(), msg->getSlopeAngle(), msg->getExpireTime());
				else
					owner->addSlowDownEffectProxy(*(target->asTangibleObject()), msg->getConeLength(), msg->getConeAngle(), msg->getSlopeAngle(), msg->getExpireTime());
			}
			else
			{
				WARNING(true, ("Received slow down effect messge for creature %s "
					"with unknown target %s",
					owner->getNetworkId().getValueString().c_str(),
					msg->getTarget().getValueString().c_str()));
			}
		}
	}
	break;

	case CM_removeSlowDownEffect:
	{
		if (owner->isAuthoritative())
			owner->removeSlowDownEffect();
		else
		{
			WARNING(true, ("CM_removeSlowDownEffect message recieved by proxy object %s", owner->getNetworkId().getValueString().c_str()));
		}
	}
	break;

	case CM_removeSlowDownEffectProxy:
	{
		if (!owner->isAuthoritative())
			owner->removeSlowDownEffectProxy();
		else
		{
			WARNING(true, ("CM_removeSlowDownEffectProxy message recieved by authoritative object %s", owner->getNetworkId().getValueString().c_str()));
		}
	}
	break;

	case CM_groupLotteryWindowCloseResults:
	{
		typedef std::pair<NetworkId, std::vector<NetworkId> > Payload;

		MessageQueueGenericValueType<Payload> const * const msg = safe_cast<MessageQueueGenericValueType<Payload> const *>(data);
		if (msg != 0)
		{
			Payload const & incomingPayload = msg->getValue();

			typedef std::vector<NetworkId> Ids;

			NetworkId const & player = incomingPayload.first;
			Ids const & selectedItems = incomingPayload.second;

			GameScriptObject * const scriptObject = owner->getScriptObject();
			if (scriptObject != 0)
			{
				ScriptParams params;
				params.addParam(player);
				params.addParam(selectedItems);
				IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_LOOT_LOTTERY_SELECTED, params));
			}
		}
	}
	break;

	case CM_cyberneticsChangeRequestToNPC:
	{
		MessageQueueCyberneticsChangeRequest const * const msg = safe_cast<MessageQueueCyberneticsChangeRequest const *>(data);
		NOT_NULL(msg);
		if (msg != nullptr)
		{
			NetworkId const & playerId = msg->getTarget();
			GameScriptObject * const scriptObject = owner->getScriptObject();
			if (scriptObject)
			{
				ScriptParams params;
				params.addParam(playerId);
				params.addParam(static_cast<int>(msg->getChangeType()));
				params.addParam(msg->getCyberneticPiece());

				IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_CYBERNETIC_CHANGE_REQUEST, params));
			}
		}
	}
	break;

	case CM_setCurrentQuest:
	{
		MessageQueueGenericValueType<uint32> const * const msg = safe_cast<MessageQueueGenericValueType<uint32> const *>(data);
		if (msg != nullptr)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player != nullptr)
			{
				player->setCurrentQuest(msg->getValue());
			}
			else
			{
				DEBUG_WARNING(true, ("Non-player creature %s sent CM_setCurrentQuest message (%u)",
					owner->getNetworkId().getValueString().c_str(), msg->getValue()));
			}
		}
	}
	break;

	case CM_setRegenRate:
	{
		MessageQueueGenericValueType<std::pair<int, float> > const * const msg = safe_cast<MessageQueueGenericValueType<std::pair<int, float> > const *>(data);
		if (msg != nullptr)
		{
			owner->setRegenRate(static_cast<Attributes::Enumerator>(msg->getValue().first), msg->getValue().second);
		}
	}
	break;

	case CM_modifyCurrentGcwPoints:
	{
		const MessageQueueGenericValueType<std::pair<int, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, int> > *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyCurrentGcwPoints(msg->getValue().first, (msg->getValue().second != 0));
			}
		}
	}
	break;

	case CM_modifyCurrentGcwRating:
	{
		const MessageQueueGenericValueType<std::pair<int, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, int> > *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyCurrentGcwRating(msg->getValue().first, (msg->getValue().second != 0));
			}
		}
	}
	break;

	case CM_modifyCurrentPvpKills:
	{
		const MessageQueueGenericValueType<std::pair<int, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, int> > *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyCurrentPvpKills(msg->getValue().first, (msg->getValue().second != 0));
			}
		}
	}
	break;

	case CM_modifyLifetimeGcwPoints:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyLifetimeGcwPoints(msg->getValue());
			}
		}
	}
	break;

	case CM_modifyMaxGcwImperialRating:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyMaxGcwImperialRating(msg->getValue());
			}
		}
	}
	break;

	case CM_modifyMaxGcwRebelRating:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyMaxGcwRebelRating(msg->getValue());
			}
		}
	}
	break;

	case CM_modifyLifetimePvpKills:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyLifetimePvpKills(msg->getValue());
			}
		}
	}
	break;

	case CM_modifyNextGcwRatingCalcTime:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyNextGcwRatingCalcTime(msg->getValue());
			}
		}
	}
	break;

	case CM_customizeFinished:
	{
		typedef std::pair<NetworkId, std::string > Payload;

		MessageQueueGenericValueType<Payload> const * const msg = safe_cast<MessageQueueGenericValueType<Payload> const *>(data);
		if (msg != 0)
		{
			Payload const & incomingPayload = msg->getValue();

			NetworkId const & object = incomingPayload.first;
			std::string const & msgValue = incomingPayload.second;

			GameScriptObject * const scriptObject = owner->getScriptObject();
			if (scriptObject != 0)
			{
				ScriptParams params;
				params.addParam(object);
				params.addParam(msgValue.c_str());
				IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_ON_CUSTOMIZE_FINISHED, params));
			}
		}
	}
	break;

	case CM_clearSessionActivity:
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
		if (player)
		{
			player->clearSessionActivity();
		}
	}
	break;

	case CM_addSessionActivity:
	{
		const MessageQueueGenericValueType<unsigned long> * const msg = safe_cast<const MessageQueueGenericValueType<unsigned long> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->addSessionActivity(static_cast<uint32>(msg->getValue()));
			}
		}
	}
	break;

	case CM_modifyHologramType:
	{
		MessageQueueGenericValueType<int32> const *msg = safe_cast<MessageQueueGenericValueType<int32> const *>(data);
		if (msg)
			owner->setHologramType(msg->getValue());
	}
	break;

	case CM_modifyVisibleOnMapAndRadar:
	{
		MessageQueueGenericValueType<bool> const *msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);
		if (msg)
			owner->setVisibleOnMapAndRadar(msg->getValue());
	}
	break;

	case CM_setCoverVisibility:
	{
		const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
		if (msg)
		{
			owner->setCoverVisibility(msg->getValue());
		}
	}
	break;

	case CM_incubatorCancel:
	{
		IncubatorCommitMessage const * const msg = safe_cast<IncubatorCommitMessage const *>(data);
		if (msg)
		{
			ServerObject * incubatorSo = ServerWorld::findObjectByNetworkId(msg->getTerminalId());
			if (incubatorSo)
			{
				TangibleObject * incubatorTo = incubatorSo->asTangibleObject();

				if (incubatorTo)
					incubatorTo->handleIncubatorCancel(*owner);
			}
		}
	}
	break;

	case CM_incubatorCommit:
	{
		IncubatorCommitMessage const * const msg = safe_cast<IncubatorCommitMessage const *>(data);
		if (msg)
		{
			ServerObject * incubatorSo = ServerWorld::findObjectByNetworkId(msg->getTerminalId());
			if (incubatorSo)
			{
				TangibleObject * incubatorTo = incubatorSo->asTangibleObject();

				if (incubatorTo)
					incubatorTo->handleIncubatorCommit(*owner, *msg);
			}
		}
	}
	break;

	case CM_modifyCollectionSlotValue:
	{
		const MessageQueueGenericValueType<std::pair<std::string, int64> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int64> > *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->modifyCollectionSlotValue(msg->getValue().first, msg->getValue().second);
			}
		}
	}
	break;

	case CM_adjustLotCount:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->adjustLotCount(msg->getValue());
			}
		}
	}
	break;

	case CM_squelch:
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->squelch(msg->getValue().second.first, msg->getValue().first, msg->getValue().second.second);
			}
		}
	}
	break;

	case CM_unsquelch:
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
		if (player)
		{
			player->unsquelch();
		}
	}
	break;

	case CM_setPriviledgedTitle:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(owner);
			if (player)
			{
				player->setPriviledgedTitle(static_cast<int8>(msg->getValue()));
			}
		}
	}
	break;

	case CM_addBuff:
	{
		const MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > > *>(data);
		if (msg)
		{
			owner->addBuff(static_cast<uint32>(msg->getValue().first), msg->getValue().second.first, msg->getValue().second.second.first, msg->getValue().second.second.second.first, msg->getValue().second.second.second.second);
		}
	}
	break;

	case CM_removeBuff:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			owner->removeBuff(static_cast<uint32>(msg->getValue()));
		}
	}
	break;

	case CM_ratingFinished:
	{
		const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if (msg)
		{
			if (owner->getScriptObject())
			{
				ScriptParams params;
				params.addParam(msg->getValue());
				IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_RATING_FINISHED, params));
			}
		}
	}
	break;
	case CM_abandonPlayerQuest:
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if (msg)
		{
			if (owner->getScriptObject())
			{
				ScriptParams params;
				params.addParam(msg->getValue());
				IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_ABANDON_PLAYER_QUEST, params));
			}
		}
	}
	break;

	default:
		TangibleController::handleMessage(message, value, data, flags);
		break;
	}
}

//-----------------------------------------------------------------------

float CreatureController::realAlter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("CreatureController::realAlter");

	float alterResult = TangibleController::realAlter(time);

	CreatureObject * const owner = NON_NULL(getCreature());

	if (getHibernate())
		return alterResult;

	if (owner->isAuthoritative())
		owner->decayAttributes(time);

	if (owner->isInWorld())
	{
		bool isBurning = false;
		float lavaResistance = 0.0f;

		if (owner->isAuthoritative())
		{
			//owner->decayMentalStates(time);
			owner->reportMonitoredCreatures(time);

			//-- Handle swimming state.
			bool const currentSwimState = owner->getState(States::Swimming);
			bool newSwimState = false;
			float waterHeight = 0.0f;
			calculateWaterState(newSwimState, isBurning, lavaResistance, waterHeight);

			bool const isVehicle = GameObjectTypes::isTypeOf(owner->getGameObjectType(), static_cast<int>(SharedObjectTemplate::GOT_vehicle));
			if (!isVehicle) // make everything but vehicles walk on lava
			{
				const Vector & position = owner->getPosition_w();
				Footprint *const footprint = owner->getFootprint();
				if (footprint)
				{
					if (isBurning)
					{
						footprint->setSwimHeight(owner->getSwimHeight() * 0.2f);
						owner->move_p(Vector::unitY * (waterHeight - position.y));
					}
					else
					{
						footprint->setSwimHeight(owner->getSwimHeight());
					}
				}
			}

			if (currentSwimState && !newSwimState)
			{
				// We've just detected that the owner stopped swimming.
				onExitSwimming();
			}
			else if (!currentSwimState && newSwimState)
			{
				// We've just detected that the owner started swimming.
				onEnterSwimming();
			}

			setObjectStopWalkRun();
		}

		CollisionProperty *const collisionProperty = owner->getCollisionProperty();
		if (owner->getState(States::RidingMount))
		{
			// Tell the rider's CollisionProperty to update its extent.  Since
			// the rider is yanked out of the CollisionWorld, the
			// CollisionProperty doesn't get updated properly.  Failure to
			// update the rider's CollisionProperty extent causes line of sight
			// to fail (line of sight ends up using the value present in CollisionProperty
			// at the time of mounting).

			if (collisionProperty)
				collisionProperty->updateExtents();
		}

		const bool isPlayerControlled = (collisionProperty && collisionProperty->isPlayerControlled());

		// lava damage
		if (WaterTypeManager::getCausesDamage(TGWT_lava)
			&& isBurning
			&& (isPlayerControlled || owner->isBeast())
			)
		{
			const unsigned long serverTime = ServerClock::getInstance().getGameTimeSeconds();
			const unsigned long lastBurnTime = owner->getLastWaterDamageTime();
			const unsigned long waterDamageInterval = WaterTypeManager::getDamageInterval(TGWT_lava);

			if (serverTime - lastBurnTime > (waterDamageInterval * 3)) // skip the first lava damage frame
			{
				owner->setLastWaterDamageTime(serverTime - waterDamageInterval);
				//DEBUG_WARNING(true,("LAVA DAMAGE SLUSH TIME- serverTime=%ul, lastBurnTime=%ul, diff=%d",serverTime,lastBurnTime,serverTime-lastBurnTime));
			}
			else if (serverTime - lastBurnTime > waterDamageInterval && !owner->isDead())
			{
				owner->setLastWaterDamageTime(serverTime);
				const float percentWaterDamagePerInterval = WaterTypeManager::getDamagePerInterval(TGWT_lava) / 100.0f;
				const float lavaResistanceModifier = 1.0f - lavaResistance / 100.0f;

				const int damageValue =
					static_cast<int> (
						-(owner->getMaxAttribute(Attributes::Health)
							* percentWaterDamagePerInterval
							* lavaResistanceModifier
							)
						);

				StringId stringId = StringId("combat_effects", "environment_damage_lava");
				owner->showFlyText(stringId, 5.0f, 255, 0, 0);

				if (WaterTypeManager::getDamageKills(TGWT_lava)
					&& (owner->getAttribute(Attributes::Health) + damageValue) <= 0)
				{
					owner->makeDead(NetworkId::cms_invalid, NetworkId::cms_invalid);
					ScriptParams params;
					owner->getScriptObject()->trigAllScripts(Scripting::TRIG_ENVIRONMENTAL_DEATH, params);
				}
				else
				{
					CombatEngineData::DamageData damageData;
					struct AttribMod::AttribMod damage;
					damage.tag = 0;
					damage.attrib = Attributes::Health;
					damage.attack = 0;
					damage.sustain = 0;
					damage.decay = AttribMod::AMDS_pool;
					damage.flags = AttribMod::AMF_directDamage;
					damage.value = damageValue;
					damageData.damage.push_back(damage);
					owner->applyDamage(damageData);
				}
			}
		}
	}
	return alterResult;
}

// ----------------------------------------------------------------------

void CreatureController::conclude()
{
	PROFILER_AUTO_BLOCK_DEFINE("CreatureController::conclude");

	MessageQueue::Data* data;

	int        message;
	real       value;
	uint32     flags;
	NetworkId  networkId;

	int const messageCount = getNumberOfMessages();
	for (int i = 0; i < messageCount; ++i)
	{
		//-- Get the message.
		getMessage(i, &message, &value, &data, &flags); //@todo why does getMessage need a signed size?

		if (message == CM_combatAction)
		{
			//-- Check if it's a combat action message.  If so, the message's end postures for the
			//   combatants must be adjusted to reflect the server's idea of the post-alter end posture.
			//   This information was not known at the time the message was constructed.
			MessageQueueCombatAction * combatMessage = safe_cast<MessageQueueCombatAction *>(data);
			if (combatMessage != nullptr)
			{
				//-- Fix up end postures in messages.

				// @todo: change postures to locomotion
				// set the attacker's posture
				MessageQueueCombatAction::AttackerData & attackerData =
					const_cast<MessageQueueCombatAction::AttackerData &>(
						combatMessage->getAttacker());
				const CreatureObject * attacker = dynamic_cast<const CreatureObject *>(
					NetworkIdManager::getObjectById(attackerData.id));
				attackerData.endPosture = (attacker != nullptr) ? attacker->getPosture() : static_cast<Postures::Enumerator>(0);

				// set the defenders' posture
				const MessageQueueCombatAction::DefenderDataVector & defenderData =
					combatMessage->getDefenders();
				for (MessageQueueCombatAction::DefenderDataVector::const_iterator
					iter(defenderData.begin()); iter != defenderData.end(); ++iter)
				{
					MessageQueueCombatAction::DefenderData & defenderData =
						const_cast<MessageQueueCombatAction::DefenderData &>(*iter);
					const CreatureObject * defender = dynamic_cast<const CreatureObject *>(
						NetworkIdManager::getObjectById(defenderData.id));
					defenderData.endPosture = (defender != nullptr) ? defender->getPosture() : static_cast<Postures::Enumerator>(0);
				}
			}
		}
	}

	CreatureObject * const owner = static_cast<CreatureObject*>(getOwner());
	if (owner)
	{
		owner->checkNotifyRegions();
	}
	//-- Chain up to parent controller.
	TangibleController::conclude();
}

//--------------------------------------------------------------------

void CreatureController::setObjectStopWalkRun()
{
	float vel = getCurrentVelocity().magnitude();
	CreatureObject* owner = static_cast<CreatureObject *>(getOwner());
	float walkSpeed = owner->getWalkSpeed();
	float runSpeed = owner->getRunSpeed();
	if (vel == 0)
		owner->setMovementStationary();
	else if (runSpeed == 0 || (vel < (walkSpeed + runSpeed) * 0.5f))
		owner->setMovementWalk();
	else
		owner->setMovementRun();
}

//----------------------------------------------------------------------

void CreatureController::setOwner(Object *newOwner)
{
	NOT_NULL(safe_cast<CreatureObject*>(newOwner));
	Controller::setOwner(newOwner);
}

// ----------------------------------------------------------------------

void CreatureController::setSecureTrade(ServerSecureTrade * t)
{
	m_secureTrade = t;
}

//----------------------------------------------------------

ServerSecureTrade* CreatureController::getSecureTrade() const
{
	return m_secureTrade;
}

//----------------------------------------------------------

void CreatureController::handleSecureTradeMessage(const MessageQueueSecureTrade * data)
{
	if (!data)
		return;

	CreatureObject * owner = getCreature();
	NOT_NULL(owner);

	switch (data->getTradeMessageId())
	{
	case MessageQueueSecureTrade::TMI_RequestTrade:
	{
		if (data->getRecipient() == owner->getNetworkId())
		{
			DEBUG_REPORT_LOG(true, ("Creature %s tried to trade with himself.", getOwner()->getNetworkId().getValueString().c_str()));
			return;
		}
		if (m_secureTrade)
		{
			DEBUG_REPORT_LOG(true, ("Creature %s is already trading\n", getOwner()->getNetworkId().getValueString().c_str()));
			return;
		}

		CreatureObject * recipient = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(data->getRecipient()));
		if (!recipient)
		{
			DEBUG_REPORT_LOG(true, ("Could not find recipient for secure trade message\n"));
			MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedPlayerUnreachable, owner->getNetworkId(), data->getRecipient());
			appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			return;
		}

		if (owner->isIncapacitated() || owner->isDead())
		{
			DEBUG_REPORT_LOG(true, ("Owner creature %s is incapacitated or dead.\n", getOwner()->getNetworkId().getValueString().c_str()));
			MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedPlayerUnreachable, owner->getNetworkId(), data->getRecipient());
			appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			return;
		}
		else if (recipient->isIncapacitated() || recipient->isDead())
		{
			DEBUG_REPORT_LOG(true, ("Recipient creature %s is incapacitated or dead.\n", getOwner()->getNetworkId().getValueString().c_str()));
			MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedPlayerUnreachable, owner->getNetworkId(), data->getRecipient());
			appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			return;
		}

		if (!recipient->isAuthoritative())
		{
			if (owner->isInWorldCell())
			{
				// transfer player to authoritative server
				owner->transferAuthority(recipient->getAuthServerProcessId(), true, false, true);
				owner->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTrade, owner->getNetworkId(), data->getRecipient()));
			}
			else if (recipient->isInWorldCell())
			{
				recipient->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTradeReversed, owner->getNetworkId(), data->getRecipient()));
			}
			else
			{
				DEBUG_REPORT_LOG(true, ("Player %s tried to trade with player "
					"%s, but both were in interiors that are on different "
					"servers. Cell/server= (%s/%lu), (%s/%lu)",
					owner->getNetworkId().getValueString().c_str(),
					recipient->getNetworkId().getValueString().c_str(),
					ContainerInterface::getTopmostContainer(*owner)->getNetworkId().getValueString().c_str(),
					owner->getAuthServerProcessId(),
					ContainerInterface::getTopmostContainer(*recipient)->getNetworkId().getValueString().c_str(),
					recipient->getAuthServerProcessId()
					));
			}
		}
		else if (recipient->getClient() == nullptr)
		{
			//				GameServer::getInstance().sendToPlanetServer(
			//					GenericValueTypeMessage<std::pair<NetworkId, NetworkId> >(
			//						"RequestSameServer",
			//						std::make_pair(
			//							ContainerInterface::getTopmostContainer(*owner)->getNetworkId(),
			//							ContainerInterface::getTopmostContainer(*recipient)->getNetworkId())),
			//					true);

							// transfer player to authoritative server
							//owner->transferAuthority(recipient->getAuthServerProcessId(), true, false);
							//owner->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTrade, owner->getNetworkId(), data->getRecipient()));

							// try again next frame
			owner->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTrade, owner->getNetworkId(), data->getRecipient()));
		}
		else if (!ServerSecureTradeManager::requestTradeWith(*owner, *recipient))
		{
			MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedPlayerBusy, owner->getNetworkId(), recipient->getNetworkId());
			appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
	break;

	// we get this when the traders are on different servers, but the trade
	// initiator can't be moved to our server
	case MessageQueueSecureTrade::TMI_RequestTradeReversed:
	{
		CreatureObject * initiator = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(data->getInitiator()));
		if (!initiator)
		{
			DEBUG_REPORT_LOG(true, ("Could not find initiator for reversed secure trade message\n"));
			return;
		}
		if (!initiator->isAuthoritative())
		{
			if (owner->isInWorldCell())
			{
				// transfer player to authoritative server
				owner->transferAuthority(initiator->getAuthServerProcessId(), true, false, true);
				owner->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTradeReversed, initiator->getNetworkId(), owner->getNetworkId()));
			}
			else
			{
				WARNING(true, ("Got TMI_RequestTradeReversed, but we are not in the world! Trade request abandoned."));
				return;
			}
		}
		else
			initiator->sendControllerMessageToAuthServer(CM_secureTrade, new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTrade, initiator->getNetworkId(), owner->getNetworkId()));
	}
	break;

	case MessageQueueSecureTrade::TMI_TradeRequested:
	{
		WARNING_STRICT_FATAL(true, ("Server should not receive this message\n"));
		return;
	}
	break;

	case MessageQueueSecureTrade::TMI_AcceptTrade:
	{
		if (m_secureTrade || !m_secureTradeInitiator)
		{
			DEBUG_REPORT_LOG(true, ("Trade acceptance denied because of previous trade or failed startup steps\n"));
			return;
		}
		ServerSecureTradeManager::acceptTradeRequest(*m_secureTradeInitiator, *(static_cast<CreatureObject *>(getOwner())));
		m_secureTradeInitiator = 0;
	}
	break;
	case MessageQueueSecureTrade::TMI_DeniedTrade:
	{
		if (m_secureTrade || !m_secureTradeInitiator)
		{
			DEBUG_REPORT_LOG(true, ("Trade acceptance denied because of previous trade or failed startup steps\n"));
			return;
		}
		ServerSecureTradeManager::refuseTrade(*m_secureTradeInitiator, *(static_cast<CreatureObject *>(getOwner())));
		m_secureTradeInitiator = 0;
	}
	break;
	default:
	{
		DEBUG_REPORT_LOG(true, ("Unhandled secure trade message type\n"));
	}
	break;
	}
}

//----------------------------------------------------------

bool CreatureController::tradeRequested(CreatureObject & initiator)
{
	//Check for outstanding trade requests.
	if (m_secureTradeInitiator || m_secureTrade)
		return false;

	m_secureTradeInitiator = &initiator;

	MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_TradeRequested, initiator.getNetworkId(), getOwner()->getNetworkId());
	appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);

	return true;
}

//----------------------------------------------------------------------

void CreatureController::sendGenericResponse(int responseType, int requestType, bool success, uint8 sequenceId)
{
	// send result back to player
	MessageQueueGenericResponse * response = new MessageQueueGenericResponse(requestType, success, sequenceId);
	appendMessage(responseType, 0.0f, response, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

//----------------------------------------------------------------------

void CreatureController::steeringChanged()
{
	return;
}

//-----------------------------------------------------------------------

bool CreatureController::setSlopeModAngle(float angle)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	bool result = owner->setSlopeModAngle(angle);
	steeringChanged();
	return result;
}

//-----------------------------------------------------------------------

bool CreatureController::setSlopeModPercent(float percent)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	bool result = owner->setSlopeModPercent(percent);
	steeringChanged();
	return result;
}

//-----------------------------------------------------------------------

bool CreatureController::setWaterModPercent(float percent)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	bool result = owner->setWaterModPercent(percent);
	steeringChanged();
	return result;
}

//-----------------------------------------------------------------------

bool CreatureController::setMovementScale(float scale)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	return owner->setMovementScale(scale);
}

//-----------------------------------------------------------------------

bool CreatureController::setMovementPercent(float percent)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	return owner->setMovementPercent(percent);
}

// ----------------------------------------------------------------------

bool CreatureController::setAccelScale(float scale)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	return owner->setAccelScale(scale);
}

//-----------------------------------------------------------------------

bool CreatureController::setAccelPercent(float percent)
{
	CreatureObject * owner = static_cast<CreatureObject *>(getOwner());
	return owner->setAccelPercent(percent);
}

// ----------------------------------------------------------------------

void CreatureController::setAppearanceFromObjectTemplate(std::string const &serverObjectTemplateName)
{
	CreatureObject * owner = dynamic_cast<CreatureObject *>(getOwner());
	if (!owner)
	{
		WARNING(true, ("setAppearanceFromObjectTemplate(): owner is nullptr or not a CreatureObject."));
		return;
	}

	if (owner->isAuthoritative())
		owner->setAlternateAppearance(serverObjectTemplateName);
	else
		owner->sendControllerMessageToAuthServer(CM_setAppearanceFromObjectTemplate, new MessageQueueGenericValueType<std::string>(serverObjectTemplateName));
}

//-----------------------------------------------------------------------

CreatureObject * CreatureController::getCreature()
{
	return safe_cast<CreatureObject *>(getOwner());
}

//-----------------------------------------------------------------------

CreatureObject const * CreatureController::getCreature() const
{
	return safe_cast<CreatureObject const *>(getOwner());
}

// ----------------------------------------------------------------------

void CreatureController::endBaselines()
{
	updateHibernate();
}

// ----------------------------------------------------------------------

void CreatureController::updateHibernate()
{
	if (getOwner()->isAuthoritative())
	{
		setHibernate(shouldHibernate());
	}
}

// ----------------------------------------------------------------------

bool CreatureController::shouldHibernate() const
{
	if (!ConfigServerGame::getHibernateEnabled())
		return false;

	// ----------

	CreatureObject const * creature = getCreature();
	NOT_NULL(creature);

	// If the creature is static, it hibernates.

	if (creature->getIsStatic())
		return true;

	// If proxied creatures aren't allowed to hibernate and this creature is proxied,
	// it's not hibernating.

	if (!ConfigServerGame::getHibernateProxies() && (creature->getProxyCount() > 0))
		return false;

	// if any players are observing me, don't hibernate
	return creature->getObserversCount() <= 0;
}

// ----------------------------------------------------------------------

bool CreatureController::getHibernate() const
{
	CreatureObject const * owner = static_cast<CreatureObject const *>(getOwner());

	return owner->hasCondition(ServerTangibleObjectTemplate::C_hibernating);
}

// ----------------------------------------------------------------------

void CreatureController::setHibernate(bool newHibernate)
{
	bool oldHibernate = getHibernate();
	if (oldHibernate == newHibernate)
		return;

	CreatureObject * owner = static_cast<CreatureObject*>(getOwner());
	if (!owner->isAuthoritative())
		return;

	// ----------

	bool transitionOk = true;

	if (newHibernate)
	{
		ScriptParams params;
		int result = getServerOwner()->getScriptObject()->trigAllScripts(Scripting::TRIG_HIBERNATE_BEGIN, params);

		if (result == SCRIPT_OVERRIDE) transitionOk = false;
	}
	else
	{
		ScriptParams params;
		int result = getServerOwner()->getScriptObject()->trigAllScripts(Scripting::TRIG_HIBERNATE_END, params);

		if (result == SCRIPT_OVERRIDE) transitionOk = false;
	}

	// ----------

	if (transitionOk)
	{
		if (newHibernate)
		{
			//			DEBUG_REPORT_LOG(true, ("[aitest] Hibernating %s\n", owner->getNetworkId().getValueString().c_str()));
			owner->setCondition(ServerTangibleObjectTemplate::C_hibernating);
			owner->setDefaultAlterTime(AlterResult::cms_keepNoAlter);
		}
		else
		{
			//			DEBUG_REPORT_LOG(true, ("[aitest] Waking up %s\n", owner->getNetworkId().getValueString().c_str()));
			owner->clearCondition(ServerTangibleObjectTemplate::C_hibernating);
			if (owner->isPlayerControlled())
				owner->setDefaultAlterTime(0);
			else
				owner->setDefaultAlterTime(AlterResult::cms_alterQuickly);
		}
		owner->scheduleForAlter();
		// update the planet server for our hibernation change
		owner->updatePlanetServerInternal(true);
	}
}

//====================================================================

void CreatureController::calculateWaterState(bool& isSwimming, bool &isBurning, float& lavaResistance, float& waterHeight) const
{
	//-- Mounts: if this is a rider, take the swim state from the mount.
	CreatureObject const *const ownerCreature = safe_cast<CreatureObject const*>(getOwner());
	if (!ownerCreature)
		return;

	if (ownerCreature->getState(States::RidingMount))
	{
		CreatureObject const     *const mountCreature = ownerCreature->getMountedCreature();
		CreatureController const *const mountCreatureController = mountCreature ? safe_cast<CreatureController const*>(mountCreature->getController()) : nullptr;
		if (mountCreatureController)
		{
			// Note: we do the real computation for the mount here because the rider gets
			// an alter prior to the mount.  If we just checked the mount's swim state here,
			// we would be a frame behind on determining the player's swim state.
			mountCreatureController->calculateWaterState(isSwimming, isBurning, lavaResistance, waterHeight);
			return;
		}
	}

	//-- Determine if the owner is swimming.
	CollisionProperty const * const collisionProperty = ownerCreature->getCollisionProperty();
	if (!collisionProperty)
		return;

	// JU_TODO: disabled - possibly make burning a persistant state
#if 0
	//-- Don't recompute if we're idle --- it shouldn't have changed.
	bool const isIdle = collisionProperty && collisionProperty->isIdle();
	if (isIdle)
		return ownerCreature->getState(States::Swimming);
#endif
	// JU_TODO: end disabled

	TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
	bool const isOnSolidFloor = collisionProperty && collisionProperty->getFootprint() && collisionProperty->getFootprint()->isOnSolidFloor();

	isSwimming = false;

	if (collisionProperty && ownerCreature->isInWorldCell() && !isOnSolidFloor)
	{
		Vector const position = ownerCreature->getPosition_w();
		float terrainHeight;
		Footprint const * foot = collisionProperty->getFootprint();

		// a vehicle is immune to lava if either its template is identified as always having immunity
		// see sku.0/sys.shared/compiled/game/datatables/terrain/creature_water_values.tab OR
		// if it has the "vehicle.lava_resistance" ObjVar, which is added when the Lava Resistance Kit
		// is used on the specific vehicle in question to create a resistance vehicle out of one that
		// wouldn't otherwise normally be resistant (handled through scripts).
		// SWG Source Change - 3.1 (for GU 17 item addition) - Aconite
		lavaResistance = ownerCreature->getLavaResistance();
		bool const isImmuneToLava = (lavaResistance == 100.0f) ||
			ownerCreature->getObjVars().hasItem("vehicle.lava_resistance");

		TerrainGeneratorWaterType waterType = terrainObject->getWaterType(position);
		if (!foot || !foot->getTerrainHeight(terrainHeight))
		{
			if (!terrainObject->getHeight(position, terrainHeight))
			{
				if (!terrainObject->getHeightForceChunkCreation(position, terrainHeight))
				{
					return;
				}
			}
		}

		//-- see if the object is swimming and/or burning
		if (terrainObject->getWaterHeight(position, waterHeight))
		{
			if (position.y - waterHeight < 1.0f && waterHeight >= terrainHeight)
			{
				float const swimHeight = ownerCreature->getSwimHeight();
				isSwimming = (terrainHeight + swimHeight) < waterHeight;

				if (waterType == TGWT_lava)
				{
					isSwimming = false;
					if (!isImmuneToLava)
					{
						isBurning = true;
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Called when the owner CreatureObject has first been detected to start swimming.
 */

void CreatureController::onEnterSwimming()
{
	CreatureObject *const owner = safe_cast<CreatureObject*>(getOwner());
	if (!owner)
		return;

	//-- Set the state to swimming.
	owner->setState(States::Swimming, true);

	//-- Fire off the script trigger indicating that the owner is now swimming.
	ScriptParams params;
	IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(Scripting::TRIG_ENTER_SWIMMING, params));
}

// ----------------------------------------------------------------------
/**
 * Called when the owner CreatureObject has first been detected to stop swimming.
 */

void CreatureController::onExitSwimming()
{
	CreatureObject *const owner = safe_cast<CreatureObject*>(getOwner());
	if (!owner)
		return;

	//-- Clear the state to swimming.
	owner->setState(States::Swimming, false);

	//-- Fire off the script trigger indicating that the owner is no longer swimming.
	ScriptParams params;
	IGNORE_RETURN(owner->getScriptObject()->trigAllScripts(Scripting::TRIG_EXIT_SWIMMING, params));
}

//----------------------------------------------------------------------

CreatureController * CreatureController::asCreatureController()
{
	return this;
}

//----------------------------------------------------------------------

CreatureController const * CreatureController::asCreatureController() const
{
	return this;
}

//----------------------------------------------------------------------

PlayerCreatureController * CreatureController::asPlayerCreatureController()
{
	return nullptr;
}

//----------------------------------------------------------------------

PlayerCreatureController const * CreatureController::asPlayerCreatureController() const
{
	return nullptr;
}

//----------------------------------------------------------------------

AICreatureController * CreatureController::asAiCreatureController()
{
	return nullptr;
}

//----------------------------------------------------------------------

AICreatureController const * CreatureController::asAiCreatureController() const
{
	return nullptr;
}

//-----------------------------------------------------------------------

CreatureController * CreatureController::getCreatureController(NetworkId const & networkId)
{
	return getCreatureController(NetworkIdManager::getObjectById(networkId));
}

//-----------------------------------------------------------------------

CreatureController * CreatureController::getCreatureController(Object * object)
{
	Controller * controller = (object != nullptr) ? object->getController() : nullptr;

	return (controller != nullptr) ? controller->asCreatureController() : nullptr;
}

// ----------------------------------------------------------------------

CreatureController * CreatureController::asCreatureController(Controller * controller)
{
	return (controller != nullptr) ? controller->asCreatureController() : nullptr;
}

// ----------------------------------------------------------------------

CreatureController const * CreatureController::asCreatureController(Controller const * controller)
{
	return (controller != nullptr) ? controller->asCreatureController() : nullptr;
}

// ======================================================================
