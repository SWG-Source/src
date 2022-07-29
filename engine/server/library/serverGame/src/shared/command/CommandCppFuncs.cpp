// ======================================================================
//
// CommandCppFuncs.cpp
// Copyright 2002-2004, Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommandCppFuncs.h"

#include "UnicodeUtils.h"
#include "serverGame/BiographyManager.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/BuffManager.h"
#include "serverGame/CellObject.h"
#include "serverGame/CharacterMatchManager.h"
#include "serverGame/Chat.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/Client.h"
#include "serverGame/CommandStringId.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/FormManagerServer.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GroupStringId.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/ReportManager.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerAsteroidManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipController.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TaskGetAttributes.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverNetworkMessages/DownloadCharacterMessage.h"
#include "serverNetworkMessages/ExchangeListCreditsMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "serverNetworkMessages/TeleportToMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Footprint.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/CitizenRankDataTable.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/GroupPickupPoint.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedGame/PlayerFormationManager.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/TravelPoint.h"
#include "sharedUtility/Location.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"
#include "sharedGame/MatchMakingCharacterProfileId.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/SocialsManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedGame/TextIterator.h"
#include "sharedGame/Waypoint.h"
#include "sharedLog/Log.h"
#include "sharedMath/MultiShape.h"
#include "sharedNetworkMessages/CharacterSheetResponseMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatRequestLog.h"
#include "sharedNetworkMessages/ClientEffectMessages.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/EditAppearanceMessage.h"
#include "sharedNetworkMessages/EditStatsMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedTerrain/TerrainObject.h"

#include <cstdlib>

namespace CommandCppFuncsNamespace
{
	const std::string BEAST_SCRIPT = "ai.beast";

	void internalSetBoosterOnOff(NetworkId const & actor, bool onOff)
	{
		ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;

		if (actorCreature == nullptr)
			return;

		ShipObject * const shipObject = actorCreature->getPilotedShip();
		if (shipObject == nullptr)
			return;

		if (!shipObject->isSlotInstalled(ShipChassisSlotType::SCST_booster))
		{
			Chat::sendSystemMessage(*actorCreature, SharedStringIds::no_booster, Unicode::emptyString);
			return;
		}

		if (!shipObject->isComponentFunctional(ShipChassisSlotType::SCST_booster))
		{
			Chat::sendSystemMessage(*actorCreature, SharedStringIds::booster_disabled, Unicode::emptyString);
			return;
		}

		if (onOff && shipObject->getBoosterEnergyCurrent() < shipObject->getBoosterEnergyRechargeRate())
		{
			Chat::sendSystemMessage(*actorCreature, SharedStringIds::booster_low_energy, Unicode::emptyString);
			return;
		}

		if (onOff && !shipObject->isBoosterReady())
		{
			Chat::sendSystemMessage(*actorCreature, SharedStringIds::booster_not_ready, Unicode::emptyString);
			return;
		}

		shipObject->setComponentActive(ShipChassisSlotType::SCST_booster, onOff);
		shipObject->restartBoosterTimer();
	}

	ServerObject const *getFirstParentInWorldOrPlayer(ServerObject const *o)
	{
		while (o && !o->isInWorld() && !o->isPlayerControlled())
			o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o));
		return o;
	}

	ServerObject const *getFirstCreatureContainer(ServerObject const *o)
	{
		while (o && !o->isInWorld() && !o->asCreatureObject())
			o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o));
		return o;
	}

	ShipObject *getAttachedShip(CreatureObject *creature)
	{
		// The "attached" ship is the ship which must move along with the creature.
		// This means if the creature is piloting, the ship is is piloting,
		// or the containing ship if the creature is the ship's owner.
		ShipObject * const ship = ShipObject::getContainingShipObject(creature);
		if (ship
			&& (ship->getOwnerId() == creature->getNetworkId()
				|| creature->getPilotedShip() == ship))
			return ship;
		return 0;
	}

	CreatureObject * findAndResolveCreatureByNetworkId(NetworkId const & targetId)
	{
		CreatureObject * targetCreatureObject = nullptr;

		{
			// find the target.  the target could be either a creature or ship
			ServerObject * const serverObject = ServerWorld::findObjectByNetworkId(targetId);
			targetCreatureObject = (serverObject != nullptr) ? serverObject->asCreatureObject() : nullptr;

			if (targetCreatureObject == nullptr)
			{
				ShipObject * const shipObject = (serverObject != nullptr) ? serverObject->asShipObject() : nullptr;

				if (shipObject != nullptr)
				{
					targetCreatureObject = shipObject->getPilot();

					if (targetCreatureObject == nullptr)
					{
						// this means that it is a POB ship that doesn't have a pilot
						// in this case we find the owner
						std::vector<CreatureObject *> passengers;
						shipObject->findAllPassengers(passengers, true);

						std::vector<CreatureObject *>::const_iterator ii = passengers.begin();
						std::vector<CreatureObject *>::const_iterator iiEnd = passengers.end();

						for (; ii != iiEnd && targetCreatureObject == nullptr; ++ii)
						{
							if ((*ii)->getNetworkId() == shipObject->getOwnerId())
							{
								targetCreatureObject = *ii;
								break;
							}
						}
					}
				}
			}
		}
		return targetCreatureObject;
	}

	ShipObject const *getFirstPackedShipForCreature(CreatureObject const &creature)
	{
		ServerObject const * const datapad = creature.getDatapad();
		if (datapad)
		{
			Container const * const container = ContainerInterface::getContainer(*datapad);
			if (container)
			{
				for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
				{
					ServerObject const * const content = safe_cast<ServerObject const *>((*i).getObject());
					if (content
						&& content->getGameObjectType() == SharedObjectTemplate::GOT_data_ship_control_device
						&& !content->isBeingDestroyed())
					{
						Container const * const scdContainer = ContainerInterface::getContainer(*content);
						if (scdContainer)
						{
							for (ContainerConstIterator j = scdContainer->begin(); j != scdContainer->end(); ++j)
							{
								ServerObject const * const scdContent = safe_cast<ServerObject const *>((*j).getObject());
								if (scdContent && !scdContent->isBeingDestroyed() && scdContent->asShipObject())
									return scdContent->asShipObject();
							}
						}
					}
				}
			}
		}
		return 0;
	}

	// Move a list of items back into their slots. We assume the items were just in their slots and can be moved back with no problems.
	void reequipItems(ServerObject & destination, const std::vector<std::pair<ServerObject *, SlotId> > & oldItems)
	{
		Container::ContainerErrorCode errorCode = Container::CEC_Success;
		for (std::vector<std::pair<ServerObject *, SlotId> >::const_iterator i = oldItems.begin(); i != oldItems.end(); ++i)
		{
			IGNORE_RETURN(ContainerInterface::transferItemToSlottedContainer(destination, *((*i).first), (*i).second, nullptr, errorCode));
		}
	}

	void doSocial(ServerObject & actor, NetworkId const & targetId, uint32 socialType)
	{
		if (actor.getClient())
		{
			ServerObject const * const target = ServerWorld::findObjectByNetworkId(targetId);
			if (target && ObserveTracker::isObserving(*(actor.getClient()), *target))
			{
				actor.performSocial(targetId, socialType, true, true);
			}
		}
	}

	namespace GroupHelpers
	{
		bool creatureIsContainedInPOBShip(CreatureObject const * creatureObject);
		void removeFromGroupAndCreatePOBGroup(GroupObject * groupToRemoveFrom,
			GroupMemberParam const & leader,
			GroupObject::GroupMemberParamVector & membersInsidePOB);
		// The owner of the POB ship is not included in the inside POB member lists
		void separateGroupBasedOffofPOBShip(GroupObject * groupObj,
			NetworkId const & ownerOfThePOBShipId,
			NetworkId const & POBShipId,
			GroupObject::GroupMemberParamVector & membersInsidePOB,
			GroupObject::GroupMemberParamVector & membersOutsidePOB);

		TravelPoint const * getNearestTravelPoint(std::string const & planetName, Vector const & location, std::vector<int> const & cityBanList, uint32 faction, bool starPortAndShuttleportOnly);
	}

	void triggerSpaceEjectPlayerFromShip(CreatureObject * creatureObject);

	int const cs_spaceSpeechMultiple = 10;
}

// ======================================================================

bool CommandCppFuncsNamespace::GroupHelpers::creatureIsContainedInPOBShip(CreatureObject const * const creatureObject)
{
	NOT_NULL(creatureObject);

	ShipObject const * const shipObject = ShipObject::getContainingShipObject(creatureObject);
	return shipObject && shipObject->getPortalProperty();
}

// ----------------------------------------------------------------------

void CommandCppFuncsNamespace::GroupHelpers::removeFromGroupAndCreatePOBGroup(GroupObject * const groupToRemoveFrom,
	GroupMemberParam const & leader,
	GroupObject::GroupMemberParamVector & membersInsidePOB)
{
	if (groupToRemoveFrom != 0)
	{
		GroupObject::GroupMemberParamVector::const_iterator ii = membersInsidePOB.begin();
		GroupObject::GroupMemberParamVector::const_iterator iiEnd = membersInsidePOB.end();

		for (; ii != iiEnd; ++ii)
		{
			NetworkId const & memberId = (*ii).m_memberId;
			groupToRemoveFrom->removeGroupMember(memberId);

			CreatureObject * const memberCreatureObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(memberId));
			if (memberCreatureObject != 0)
			{
				if (memberCreatureObject->getGroup() == groupToRemoveFrom)
					memberCreatureObject->getGroup()->onGroupMemberRemoved(memberCreatureObject->getNetworkId(), false);
			}
		}
	}

	if (!membersInsidePOB.empty())
	{
		ServerUniverse::getInstance().createGroup(leader, membersInsidePOB);
	}
}

// ----------------------------------------------------------------------

// The owner of the POB ship is not included in the inside POB member lists
void CommandCppFuncsNamespace::GroupHelpers::separateGroupBasedOffofPOBShip(GroupObject * const groupObj,
	NetworkId const & ownerOfThePOBShipId,
	NetworkId const & POBShipId,
	GroupObject::GroupMemberParamVector & membersInsidePOB,
	GroupObject::GroupMemberParamVector & membersOutsidePOB)
{
	NOT_NULL(groupObj);

	GroupObject::GroupMemberVector const & groupMembers = groupObj->getGroupMembers();
	GroupObject::GroupMemberVector::const_iterator ii = groupMembers.begin();
	GroupObject::GroupMemberVector::const_iterator iiEnd = groupMembers.end();

	for (; ii != iiEnd; ++ii)
	{
		NetworkId const & memberId = ii->first;
		NetworkId const & POBShipMemberIsContainedInId = groupObj->getPOBShipMemberIsContainedIn(memberId);
		GroupMemberParam const & groupMemberParam = groupObj->createGroupMemberParamForMember(memberId);

		if (POBShipMemberIsContainedInId == POBShipId)
		{
			// this member is contained in the same POB
			if (memberId != ownerOfThePOBShipId)
			{
				// and is not the owner of the POB
				membersInsidePOB.push_back(groupMemberParam);
			}
		}
		else
		{
			membersOutsidePOB.push_back(groupMemberParam);
		}
	}
}

// ----------------------------------------------------------------------

void CommandCppFuncsNamespace::triggerSpaceEjectPlayerFromShip(CreatureObject * const creatureObject)
{
	if (creatureObject == 0)
	{
		WARNING(true, ("CommandCppFuncsNamespace::triggerSpaceEjectPlayerFromShip: nullptr CreatureObject."));
		return;
	}

	GameScriptObject * gameScriptObject = creatureObject->getScriptObject();

	if (gameScriptObject != 0)
	{
		ScriptParams scriptParams;
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_EJECT_PLAYER_FROM_SHIP, scriptParams));
	}
	else
	{
		WARNING(true, ("CommandCppFuncsNamespace::triggerSpaceEjectPlayerFromShip: nullptr ScriptObject."));
	}
}

// ----------------------------------------------------------------------

TravelPoint const * CommandCppFuncsNamespace::GroupHelpers::getNearestTravelPoint(std::string const & planetName, Vector const & location, std::vector<int> const & cityBanList, uint32 faction, bool starPortAndShuttleportOnly)
{
	TravelPoint const * nearestTravelPoint = nullptr;
	PlanetObject const * const planetObject = ServerUniverse::getInstance().getPlanetByName(planetName);
	if (planetObject)
	{
		int const numberOfTravelPoints = planetObject->getNumberOfTravelPoints();
		for (int i = 0; i < numberOfTravelPoints; ++i)
		{
			TravelPoint const * tp = planetObject->getTravelPoint(i);
			if (!tp)
				continue;

			if (starPortAndShuttleportOnly && ((tp->getType() & TravelPoint::TPT_PC_CampShuttleBeacon) || (tp->getType() & TravelPoint::TPT_NPC_StaticBaseBeacon)))
				continue;

			// need to check factional alignment in order to use the gcw static base travel point
			std::string const & tpName = tp->getName();
			if (tpName.find("gcwstaticbase") != std::string::npos)
			{
				if ((tpName.find("rebel") != std::string::npos) && !PvpData::isRebelFactionId(faction))
					continue;
				else if ((tpName.find("imperial") != std::string::npos) && !PvpData::isImperialFactionId(faction))
					continue;
			}

			// if the travel point is inside a player city, make sure player is not banned in that city
			if (!cityBanList.empty())
			{
				int const cityAtTp = CityInterface::getCityAtLocation(planetName, static_cast<int>(tp->getPosition_w().x), static_cast<int>(tp->getPosition_w().z), 0);
				if ((cityAtTp > 0) && (std::find(cityBanList.begin(), cityBanList.end(), cityAtTp) != cityBanList.end()))
					continue;
			}

			if (!nearestTravelPoint || (location.magnitudeBetweenSquared(tp->getPosition_w()) < location.magnitudeBetweenSquared(nearestTravelPoint->getPosition_w())))
				nearestTravelPoint = tp;
		}
	}

	return nearestTravelPoint;
}

// ======================================================================

using namespace CommandCppFuncsNamespace;

// ======================================================================

static const std::string OBJVAR_PLAYERS_CAN_ACCESS_CONTAINER("players_can_access_container");

// ======================================================================

static void sendProseMessage(const CreatureObject &actor, const ServerObject * const target, StringId const &stringId)
{
	Chat::sendSystemMessageSimple(actor, stringId, target);
}

// ======================================================================

static Client *getClientFromCharacterId(NetworkId const &charId)
{
	ServerObject *obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(charId));
	if (obj)
		return obj->getClient();
	return 0;
}

// ----------------------------------------------------------------------

static std::string nextStringParm(Unicode::String const &str, size_t &curpos)
{
	size_t endpos = 0;
	Unicode::String token;
	if (!Unicode::getFirstToken(str, curpos, endpos, token))
		return std::string();
	curpos = endpos;
	return Unicode::wideToNarrow(token);
}

// ----------------------------------------------------------------------

static int nextIntParm(Unicode::String const &str, size_t &curpos)
{
	std::string strParm = nextStringParm(str, curpos);
	if (strParm.empty())
		return -1;
	return atoi(strParm.c_str());
}

// ----------------------------------------------------------------------

static NetworkId nextOidParm(Unicode::String const &str, size_t &curpos)
{
	std::string const &s = nextStringParm(str, curpos);
	if (!s.empty())
	{
		if ((!isdigit(s.c_str()[0])) && ((s.c_str()[0]) != '-'))
			return NameManager::getInstance().getPlayerId(NameManager::normalizeName(s));
		return NetworkId(s);
	}
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

static float nextFloatParm(Unicode::String const &str, size_t &curpos)
{
	return static_cast<float>(strtod(nextStringParm(str, curpos).c_str(), nullptr));
}

// ----------------------------------------------------------------------

static bool nextBoolParm(Unicode::String const &str, size_t &curpos)
{
	return (nextIntParm(str, curpos) != 0);
}

// ----------------------------------------------------------------------

static Vector nextVectorParm(Unicode::String const &str, size_t &curpos)
{
	float x = nextFloatParm(str, curpos);
	float y = nextFloatParm(str, curpos);
	float z = nextFloatParm(str, curpos);

	return Vector(x, y, z);
}

// ======================================================================

static void commandFuncConsoleAll(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject *obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			Client *client = obj->getClient();
			if (client && client->isGod())
			{
				LOG("CustomerService", ("Avatar:%s executing command: %s", PlayerObject::getAccountDescription(client->getCharacterObjectId()).c_str(), Unicode::wideToNarrow(params).c_str()));
				ConsoleMgr::processString(Unicode::wideToNarrow(params), client, 0);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncConsoleCategory(NetworkId const &actor,
	std::string const &category, Unicode::String const &params)
{
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject *obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			Client *client = obj->getClient();
			if (client)
			{
				ConsoleMgr::processString(category + " " + Unicode::wideToNarrow(params), client, 0);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncConsoleCombat(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "combat", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleCraft(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "craft", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleDatabase(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "database", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleManufacture(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "manufacture", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleMoney(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "money", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleNpc(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "npc", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleObject(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "object", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleObjvar(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "objvar", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleResource(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "resource", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleScript(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "script", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleServer(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "server", params);
}

// ----------------------------------------------------------------------

static void commandFuncConsoleSkill(Command const &command, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	UNREF(target);
	commandFuncConsoleCategory(actor, "skill", params);
}

// ----------------------------------------------------------------------

static void commandFuncAdminSetGodMode(Command const &, NetworkId const &actor,
	NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	bool enable = nextBoolParm(params, pos);

	Client *client = getClientFromCharacterId(actor);
	if (client)
		IGNORE_RETURN(client->setGodMode(enable));
}

// ----------------------------------------------------------------------

static void commandFuncResetCooldowns(Command const &, NetworkId const &actor,
	NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj)
	{
		actorObj->getCommandQueue()->resetCooldowns();
	}
}

// ----------------------------------------------------------------------

static void commandFuncSpewCommandQueue(Command const &, NetworkId const &actor,
	NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj)
	{
		actorObj->getCommandQueue()->spew();
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminKick(Command const &, NetworkId const &actor,
	NetworkId const &target, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj)
	{
		NetworkId who(target);
		if (who == NetworkId::cms_invalid)
		{
			size_t curpos = 0;
			who = nextOidParm(params, curpos);
			if (who == NetworkId::cms_invalid)
				return;
		}

		ShipObject * const shipObject = dynamic_cast<ShipObject *>(NetworkIdManager::getObjectById(who));
		if (shipObject)
		{
			// If the target is a ship, kick all the people in the ship
			std::vector<CreatureObject *> passengers;
			shipObject->findAllPassengers(passengers, true);
			for (std::vector<CreatureObject *>::const_iterator i = passengers.begin(); i != passengers.end(); ++i)
			{
				KickPlayer const kickMessage((*i)->getNetworkId(), "Admin Kick");
				GameServer::getInstance().sendToConnectionServers(kickMessage);
			}
		}
		else
		{
			KickPlayer const kickMessage(who, "Admin Kick");
			GameServer::getInstance().sendToConnectionServers(kickMessage);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncLocateStructure(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
	if (!playerObj)
		return;

	Client * const clientObj = actorObj->getClient();
	if (!clientObj)
		return;

	bool const isGod = clientObj->isGod();
	NetworkId ownerId = actor;

	// god mode can specify a different player to search for
	if (isGod && !params.empty())
	{
		ownerId = NetworkId(Unicode::wideToNarrow(params));
		if (!ownerId.isValid())
			ownerId = actor;
	}

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!isGod && actorObj->getObjVars().hasItem("timeNextLocateStructureCommandAllowed") && (actorObj->getObjVars().getType("timeNextLocateStructureCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextLocateStructureCommandAllowed = 0;
		if (actorObj->getObjVars().getItem("timeNextLocateStructureCommandAllowed", timeNextLocateStructureCommandAllowed))
		{
			if (timeNextLocateStructureCommandAllowed > timeNow)
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("player_structure", "locate_structure_command_too_fast");
				prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(timeNextLocateStructureCommandAllowed - timeNow)));
				Chat::sendSystemMessage(*actorObj, prosePackage);

				return;
			}
		}
	}

	NetworkId ownerResidenceId;
	if (ownerId == actor)
	{
		ownerResidenceId = actorObj->getHouse();

		StringId::LocUnicodeString response;
		if (StringId("player_structure", "locate_structure_command_executing").localize(response))
			ConsoleMgr::broadcastString(Unicode::wideToNarrow(response), clientObj);
	}
	else
	{
		// should only be here for god mode
		ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("Searching galaxy for all unpacked structures and installations owned by %s.", ownerId.getValueString().c_str()),
			clientObj);
	}

	GenericValueTypeMessage<std::pair<std::pair<uint32, bool>, std::pair<std::pair<NetworkId, NetworkId>, NetworkId> > > locateStructureByOwnerIdReq("LSBOIReq", std::make_pair(std::make_pair(GameServer::getInstance().getProcessId(), isGod), std::make_pair(std::make_pair(ownerId, ownerResidenceId), actor)));
	GameServer::getInstance().sendToCentralServer(locateStructureByOwnerIdReq);

	if (!isGod)
		IGNORE_RETURN(actorObj->setObjVarItem("timeNextLocateStructureCommandAllowed", (timeNow + ConfigServerGame::getLocateStructureCommandIntervalSeconds())));
}

// ----------------------------------------------------------------------

static void commandFuncLocateVendor(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
	if (!playerObj)
		return;

	Client * const clientObj = actorObj->getClient();
	if (!clientObj)
		return;

	bool const isGod = clientObj->isGod();
	NetworkId ownerId = actor;

	// god mode can specify a different player to search for
	if (isGod && !params.empty())
	{
		ownerId = NetworkId(Unicode::wideToNarrow(params));
		if (!ownerId.isValid())
			ownerId = actor;
	}

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!isGod && actorObj->getObjVars().hasItem("timeNextLocateVendorCommandAllowed") && (actorObj->getObjVars().getType("timeNextLocateVendorCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextLocateVendorCommandAllowed = 0;
		if (actorObj->getObjVars().getItem("timeNextLocateVendorCommandAllowed", timeNextLocateVendorCommandAllowed))
		{
			if (timeNextLocateVendorCommandAllowed > timeNow)
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("player_vendor", "locate_vendor_command_too_fast");
				prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNextLocateVendorCommandAllowed - timeNow)));
				Chat::sendSystemMessage(*actorObj, prosePackage);

				return;
			}
		}
	}

	if (ownerId == actor)
	{
		StringId::LocUnicodeString response;
		if (StringId("player_vendor", "locate_vendor_command_executing").localize(response))
			ConsoleMgr::broadcastString(Unicode::wideToNarrow(response), clientObj);
	}
	else
	{
		// should only be here for god mode
		ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("Searching the commodities system for all initialized vendors owned by %s.", ownerId.getValueString().c_str()),
			clientObj);
	}

	GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, bool> > const msg("GetVendorInfoForPlayer", std::make_pair(std::make_pair(actor, ownerId), isGod));
	CommoditiesMarket::sendToCommoditiesServer(msg);

	if (!isGod)
		IGNORE_RETURN(actorObj->setObjVarItem("timeNextLocateVendorCommandAllowed", (timeNow + ConfigServerGame::getLocateVendorCommandIntervalSeconds())));
}

// ----------------------------------------------------------------------

static void commandFuncShowCtsHistory(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject const * targetObj = dynamic_cast<CreatureObject const*>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	Client * const clientObj = targetObj->getClient();
	if (!clientObj)
		return;

	NetworkId target = actor;

	// god mode can specify a different player to show CTS history for
	if (clientObj->isGod() && !params.empty())
	{
		target = NetworkId(Unicode::wideToNarrow(params));
		if (!target.isValid())
			target = actor;
	}

	if (target != actor)
	{
		targetObj = dynamic_cast<CreatureObject const*>(ServerWorld::findObjectByNetworkId(target));
		targetPlayerObj = (targetObj ? PlayerCreatureController::getPlayerObject(targetObj) : nullptr);
		if (!targetPlayerObj)
		{
			ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("%s is not a valid or nearby player character.", target.getValueString().c_str()), clientObj);
			return;
		}

		ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("Retrieving CTS history for %s.", target.getValueString().c_str()), clientObj);
	}
	else
	{
		ConsoleMgr::broadcastString("Retrieving CTS history for this character.", clientObj);
	}

	// we need to return the CTS history ordered by transfer time
	std::multimap<int, std::pair<std::string, std::string> > orderedCtsHistory;

	DynamicVariableList::NestedList const ctsTransactions(targetObj->getObjVars(), "ctsHistory");
	for (DynamicVariableList::NestedList::const_iterator i = ctsTransactions.begin(); i != ctsTransactions.end(); ++i)
	{
		Unicode::String ctsTransactionDetail;
		if (i.getValue(ctsTransactionDetail))
		{
			Unicode::UnicodeStringVector tokens;
			if (Unicode::tokenize(ctsTransactionDetail, tokens, nullptr, nullptr) && (tokens.size() >= 4))
			{
				Unicode::String characterName;
				for (size_t i = 3, j = tokens.size(); i < j; ++i)
				{
					if (!characterName.empty())
						characterName += Unicode::narrowToWide(" ");

					characterName += tokens[i];
				}

				IGNORE_RETURN(orderedCtsHistory.insert(std::make_pair(atoi(Unicode::wideToNarrow(tokens[0]).c_str()), std::make_pair(Unicode::wideToNarrow(characterName), Unicode::wideToNarrow(tokens[1])))));
			}
		}
	}

	if (orderedCtsHistory.empty())
	{
		ConsoleMgr::broadcastString("No CTS history found.", clientObj);
	}
	else
	{
		int count = 0;
		for (std::multimap<int, std::pair<std::string, std::string> >::const_iterator iter = orderedCtsHistory.begin(); iter != orderedCtsHistory.end(); ++iter)
		{
			++count;

			if (orderedCtsHistory.size() == 1)
				ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("Transferred from %s, %s on %s.", iter->second.first.c_str(), iter->second.second.c_str(), CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str()), clientObj);
			else
				ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("%d) Transferred from %s, %s on %s.", count, iter->second.first.c_str(), iter->second.second.c_str(), CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str()), clientObj);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminPlanetwarp(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	size_t pos = 0;
	std::string const planetName = nextStringParm(params, pos);
	Vector const newPosition_w = nextVectorParm(params, pos);
	NetworkId const newContainer = nextOidParm(params, pos);
	Vector const newPosition_p = nextVectorParm(params, pos);

	CreatureObject * const actorCreature = safe_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (actorCreature)
	{
		TangibleObject *targetTangible = actorCreature;
		if (target != NetworkId::cms_invalid)
			targetTangible = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(target));

		if (targetTangible)
		{
			if (!ServerWorld::isSpaceScene() && !strncmp(planetName.c_str(), "space_", 5) && targetTangible->asCreatureObject())
			{
				// going to space from the ground, so require a ship and set launch information
				ShipObject const * const ship = getFirstPackedShipForCreature(*targetTangible->asCreatureObject());
				if (!ship)
				{
					Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("No valid ship, cannot warp to space scene."), Unicode::emptyString);
					return;
				}

				DynamicVariableLocationData const loc(targetTangible->getPosition_w(), ServerWorld::getSceneId(), NetworkId::cms_invalid);
				targetTangible->setObjVarItem("space.launch.worldLoc", loc);
				targetTangible->setObjVarItem("space.launch.ship", ship->getNetworkId());
				targetTangible->setObjVarItem("space.launch.startIndex", static_cast<int>(0));
			}

			GameServer::getInstance().requestSceneWarp(CachedNetworkId(*targetTangible), planetName, newPosition_w, newContainer, newPosition_p);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminTeleport(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	size_t pos = 0;
	Vector position_w = nextVectorParm(params, pos);
	NetworkId targetCell = nextOidParm(params, pos);
	Vector position_p = nextVectorParm(params, pos);

	// make sure coordinates are sane
	float const maxSaneCoordinate = 100000;
	if ((position_w.x < -maxSaneCoordinate) ||
		(position_w.x > maxSaneCoordinate) ||
		(position_w.y < -maxSaneCoordinate) ||
		(position_w.y > maxSaneCoordinate) ||
		(position_w.z < -maxSaneCoordinate) ||
		(position_w.z > maxSaneCoordinate) ||
		(position_p.x < -maxSaneCoordinate) ||
		(position_p.x > maxSaneCoordinate) ||
		(position_p.y < -maxSaneCoordinate) ||
		(position_p.y > maxSaneCoordinate) ||
		(position_p.z < -maxSaneCoordinate) ||
		(position_p.z > maxSaneCoordinate))
	{
		const Object * const actorObj = NetworkIdManager::getObjectById(actor);
		if (actorObj && actorObj->asServerObject())
		{
			Client * const actorClient = actorObj->asServerObject()->getClient();
			if (actorClient)
			{
				char buffer[512];
				snprintf(buffer, sizeof(buffer) - 1, "specified coordinate (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f) is out of range",
					position_w.x, position_w.y, position_w.z,
					position_p.x, position_p.y, position_p.z);
				buffer[sizeof(buffer) - 1] = '\0';

				ConsoleMgr::broadcastString(buffer, actorClient);
			}
		}

		return;
	}

	ServerObject *teleportObj = 0;
	if (target != NetworkId::cms_invalid)
		teleportObj = ServerWorld::findObjectByNetworkId(target);
	else
		teleportObj = ServerWorld::findObjectByNetworkId(actor);

	if (teleportObj)
	{
		
		// prevents teleporting anything but creatures/players so GMs can't teleport buildings accidentally
		// note: /object commands can still be used to move non-player objects
		if (!GameObjectTypes::isTypeOf(teleportObj->getGameObjectType(), SharedObjectTemplate::GOT_creature)) {
			return;	
		}
		
		// if the person teleporting is the owner of the containing ship, move the ship instead
		ShipObject * const ship = getAttachedShip(teleportObj->asCreatureObject());
		if (ship)
			ship->teleportObject(position_w, NetworkId::cms_invalid, "", position_w, "");
		else
			teleportObj->teleportObject(position_w, targetCell, "", position_p, "");
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminTeleportTo(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj)
	{
		CachedNetworkId who(target);
		if (who == NetworkId::cms_invalid)
		{
			size_t curpos = 0;
			who = nextOidParm(params, curpos);
		}
		// if the object exists locally, just teleport to it, otherwise send to db server for scatter/gather
		Object * const destObj = who.getObject();
		if (destObj)
		{
			// if the person teleporting has a ship that must go with it, move the ship instead
			ShipObject * const ship = getAttachedShip(actorObj->asCreatureObject());
			if (ship)
				ship->teleportObject(destObj->findPosition_w(), NetworkId::cms_invalid, "", destObj->findPosition_w(), "");
			else
			{
				Object const * const firstParent = ContainerInterface::getFirstParentInWorld(*destObj);
				if (firstParent)
				{
					actorObj->teleportObject(
						firstParent->getPosition_w(),
						firstParent->getAttachedTo() ? firstParent->getAttachedTo()->getNetworkId() : NetworkId::cms_invalid,
						"",
						firstParent->getPosition_p(),
						"");
				}
			}
		}
		else
		{
			TeleportToMessage const teleportToMessage(actor, who, GameServer::getInstance().getProcessId());
			GameServer::getInstance().sendToCentralServer(teleportToMessage);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminListGuilds(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	Object *actorObj = NetworkIdManager::getObjectById(actor);
	if (actorObj && actorObj->asServerObject())
	{
		Client *client = actorObj->asServerObject()->getClient();
		if (client)
		{
			char buf[512];
			std::vector<int> guildIds;

			GuildInterface::getAllGuildIds(guildIds);
			for (std::vector<int>::const_iterator i = guildIds.begin(); i != guildIds.end(); ++i)
			{
				snprintf(buf, 511, "id:%-8d name:%s abbrev:%s leaderId:%s",
					*i,
					GuildInterface::getGuildName(*i).c_str(),
					GuildInterface::getGuildAbbrev(*i).c_str(),
					GuildInterface::getGuildLeaderId(*i).getValueString().c_str());
				buf[511] = '\0';
				ConsoleMgr::broadcastString(buf, client);
			}
			snprintf(buf, 511, "%d guilds total.", guildIds.size());
			ConsoleMgr::broadcastString(buf, client);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminEditBank(Command const &cmd, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	CreatureObject * actorCreature = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	CreatureObject * targetCreature = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(target));
	if (!targetCreature || !actorCreature)
	{
		DEBUG_REPORT_LOG(true, ("Received open command for nullptr player or target.\n"));
		return;
	}

	ServerObject * targetBank = targetCreature->getBankContainer();
	if (!targetBank)
	{
		DEBUG_REPORT_LOG(true, ("Couldn't get bank for player %s.\n", target.getValueString().c_str()));
		return;
	}

	if (targetBank->isAuthoritative())
	{
		ScriptParams params;
		params.addParam(actor);
		if (targetBank->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_OPEN_CONTAINER, params) == SCRIPT_CONTINUE)
		{
			Client *client = actorCreature->getClient();
			if (client)
				client->openContainer(*targetBank, 0, std::string());
		}
	}
	else
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
			"RequestSameServer",
			std::make_pair(
				ContainerInterface::getTopmostContainer(*actorCreature)->getNetworkId(),
				ContainerInterface::getTopmostContainer(*targetBank)->getNetworkId()));
		GameServer::getInstance().sendToPlanetServer(rssMessage);
		ContainerInterface::sendContainerMessageToClient(*actorCreature, Container::CEC_TryAgain, targetBank);

		//actorCreature->transferAuthority(targetBank->getAuthServerProcessId(), true, false);
		//actorCreature->commandQueueEnqueue(cmd, target, params, 0, false, Command::CP_Normal, false);
		//LOG("container", ("Received open editBank.  Player not on same server as the target's bank. Migrating player"));
	}
}

// ----------------------------------------------------------------------

static void commandFuncAdminEditStats(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject* const creatureActor = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (!creatureActor)
	{
		WARNING(true, ("commandFuncAdminEditStats: nullptr actor"));
		return;
	}

	CreatureObject* const creatureTarget = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(target));
	if (!creatureTarget)
	{
		WARNING(true, ("commandFuncAdminEditStats: nullptr target"));
		return;
	}

	EditStatsMessage const esm(target);
	Client * const client = creatureActor->getClient();
	if (client)
		client->send(esm, true);
}

// ----------------------------------------------------------------------

static void commandFuncAdminEditAppearance(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject* const creatureActor = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (!creatureActor)
	{
		WARNING(true, ("commandFuncAdminEditAppearance: nullptr actor"));
		return;
	}

	CreatureObject* const creatureTarget = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(target));
	if (!creatureTarget)
	{
		WARNING(true, ("commandFuncAdminEditAppearance: nullptr target"));
		return;
	}

	EditAppearanceMessage const eam(target);
	Client * const client = creatureActor->getClient();
	if (client)
		client->send(eam, true);
}

// ----------------------------------------------------------------------

static void commandFuncAdminGrantTitle(Command const &, NetworkId const &, NetworkId const &, Unicode::String const &)
{
}

// ----------------------------------------------------------------------

static void commandFuncAdminCredits(Command const &, NetworkId const &, NetworkId const &, Unicode::String const &)
{
}

// ----------------------------------------------------------------------

static void commandFuncAdminGetStationName(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	const Object * const actorObj = NetworkIdManager::getObjectById(actor);
	if (actorObj && actorObj->asServerObject())
	{
		Client * const actorClient = actorObj->asServerObject()->getClient();
		if (actorClient)
		{
			const CreatureObject* creatureTarget = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(target));
			if (!creatureTarget)
			{
				size_t pos = 0;
				NetworkId targetId(nextOidParm(params, pos));
				creatureTarget = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(targetId));
			}

			if (!creatureTarget)
			{
				ConsoleMgr::broadcastString("Could not resolve target passed to getStationName", actorClient);
				return;
			}

			const Client * const clientTarget = creatureTarget->getClient();
			if (clientTarget)
			{
				const std::string & stationName = clientTarget->getAccountName();
				char buf[512];
				snprintf(buf, 511, "Station name is: %s.", stationName.c_str());
				ConsoleMgr::broadcastString(buf, actorClient);
			}
			else
				DEBUG_WARNING(true, ("Could not get the target Client in commandFuncAdminGetStationName"));
		}
		else
			DEBUG_WARNING(true, ("Could not get the actor Client in commandFuncAdminGetStationName"));
	}
	else
		DEBUG_WARNING(true, ("Could not get the actor object in commandFuncAdminGetStationName"));
}

// ----------------------------------------------------------------------

static void commandFuncAuctionCreate(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	/*printf("!!!!AUCTION: process = %i\n", (int)GameServer::getInstance().getProcessId());
	printf("!!!!PID = %i\n", (int)getpid());

	Chat::getFriendsList("SWG.msivertson.Orami");

	return;*/

	size_t pos = 0;
	NetworkId itemId(nextOidParm(params, pos));
	NetworkId auctionContainerId(nextOidParm(params, pos));
	int minBid = nextIntParm(params, pos);
	int timer = nextIntParm(params, pos);
	Unicode::String userDescription = Unicode::narrowToWide(nextStringParm(params, pos));

	UNREF(timer);

	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	TangibleObject *itemTangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(itemId));
	ServerObject *auctionContainer = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(auctionContainerId));

	if (actorCreature && itemTangible && auctionContainer)
	{
		CommoditiesMarket::auctionCreatePermanent(
			"Test Name",
			*itemTangible,
			*auctionContainer,
			minBid,
			userDescription);
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionCreateImmediate(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	NetworkId itemId(nextOidParm(params, pos));
	NetworkId auctionContainerId(nextOidParm(params, pos));
	int price = nextIntParm(params, pos);
	int timer = nextIntParm(params, pos);
	Unicode::String userDescription = Unicode::narrowToWide(nextStringParm(params, pos));

	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));

	TangibleObject *itemTangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(itemId));
	ServerObject *auctionContainer = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(auctionContainerId));

	if (actorCreature && itemTangible && auctionContainer)
	{
		CommoditiesMarket::auctionCreateImmediate(
			*actorCreature,
			*itemTangible,
			Unicode::String(),
			*auctionContainer,
			price,
			timer,
			userDescription);
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionBid(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	NetworkId auctionId(nextOidParm(params, pos));
	int bidAmount = nextIntParm(params, pos);
	int maxProxyBid = nextIntParm(params, pos);
	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));

	if (actorCreature)
	{
		CommoditiesMarket::getAuctionDetails(*actorCreature, auctionId);

		CommoditiesMarket::auctionBid(
			*actorCreature,
			auctionId.getValue(),
			bidAmount,
			maxProxyBid);
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionCancel(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	NetworkId auctionId(nextOidParm(params, pos));

	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));

	if (actorCreature)
	{
		CommoditiesMarket::auctionCancel(
			*actorCreature,
			auctionId.getValue());
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionAccept(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	NetworkId auctionId(nextOidParm(params, pos));

	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));

	if (actorCreature)
	{
		CommoditiesMarket::auctionAccept(
			*actorCreature,
			auctionId.getValue());
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionQuery(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
}

// ----------------------------------------------------------------------

static void commandFuncAuctionRetrieve(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	NetworkId itemId(nextOidParm(params, pos));
	NetworkId auctionId(nextOidParm(params, pos));
	NetworkId auctionContainerId(nextOidParm(params, pos));

	CreatureObject *actorCreature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	ServerObject *auctionContainer = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(auctionContainerId));

	if (actorCreature && auctionContainer && actorCreature->isAuthoritative() && auctionContainer->isAuthoritative())
	{
		CommoditiesMarket::auctionRetrieve(
			*actorCreature,
			auctionId.getValue(),
			itemId,
			*auctionContainer);
	}
}

// ----------------------------------------------------------------------

static void commandFuncSocial(Command const &command, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	UNREF(command);
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			Unicode::UnicodeStringVector sv;
			Unicode::tokenize(params, sv);

			if (sv.empty())
			{
				WARNING(true, ("commandFuncSocial no params"));
				return;
			}

			const std::string & socialName = Unicode::wideToNarrow(sv[0]);
			const uint32 socialType = SocialsManager::getSocialTypeByName(socialName);

			if (!socialType)
				WARNING(true, ("commandFuncSocial Bad social type specified: [%s]", socialName.c_str()));
			else
			{
				bool animationOk = true;
				bool textOk = true;
				if (sv.size() > 1)
					animationOk = !sv[1].empty() && sv[1][0] == '1';
				if (sv.size() > 2)
					textOk = !sv[2].empty() && sv[2][0] == '1';

				obj->performSocial(target, socialType, animationOk, textOk);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncSocialInternal(Command const &command, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	UNREF(command);
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			Unicode::UnicodeStringVector sv;
			Unicode::tokenize(params, sv);

			if (sv.size() < 2)
			{
				WARNING(true, ("commandFuncSocial not enough params"));
				return;
			}

			const NetworkId targetId(Unicode::wideToNarrow(sv[0]));
			const uint32    socialType = atoi(Unicode::wideToNarrow(sv[1]).c_str());

			if (!socialType)
				WARNING(true, ("commandFuncSocialInternal Bad social type specified: '%d'", socialType));
			else
			{
				bool animationOk = true;
				bool textOk = true;
				if (sv.size() > 3)
				{
					animationOk = !sv[2].empty() && sv[2][0] == '1';
					textOk = !sv[3].empty() && sv[3][0] == '1';
				}
				obj->performSocial(targetId, socialType, animationOk, textOk);
			}
		}
	}
}

//----------------------------------------------------------------------

static void commandFuncSetMood(Command const &command, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	UNREF(command);
	if (actor != NetworkId::cms_invalid)
	{
		CreatureObject * const obj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			const std::string moodName = Unicode::wideToNarrow(params);

			if (moodName == "none")
			{
				obj->setMood(0);
			}
			else
			{
				const uint32 moodType = MoodManager::getMoodByCanonicalName(moodName);
				if (moodType)
					obj->setMood(moodType);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetMoodInternal(Command const &command, NetworkId const &actor,
	NetworkId const &, Unicode::String const &params)
{
	UNREF(command);
	if (actor != NetworkId::cms_invalid)
	{
		CreatureObject * const obj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			const uint32 moodType = atoi(Unicode::wideToNarrow(params).c_str());
			obj->setMood(moodType);
		}
	}
}

//----------------------------------------------------------------------

static void commandFuncRequestWaypointAtPosition(Command const &command, NetworkId const &actor,
	NetworkId const &, Unicode::String const &params)
{
	UNREF(command);
	if (actor != NetworkId::cms_invalid)
	{
		CreatureObject * const obj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			if (obj->isPlayerControlled())
			{
				PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(obj);
				if (playerObject)
				{
					size_t curpos = 0;

					uint8 color = static_cast<uint8>(Waypoint::Blue);
					std::string planet = nextStringParm(params, curpos);

					// the first parameter may be the name of the planet or an unsigned
					// int specifying the color of the waypoint, in which case the
					// second parameter is the name of the planet

					// obfuscation to prevent player from manually entering the
					// requestWaypointAtPosition command and specifying the waypoint color
					char buffer[256];
					snprintf(buffer, sizeof(buffer) - 1, "(^-,=+_)color_%s(,+-=_^)=", actor.getValueString().c_str());
					buffer[sizeof(buffer) - 1] = '\0';

					if (planet.find(buffer) == 0)
					{
						size_t const bufferLength = strlen(buffer);
						if (planet.size() > bufferLength)
						{
							std::string const colorStr = planet.substr(bufferLength);
							color = static_cast<uint8>(atoi(colorStr.c_str()));

							if ((color >= static_cast<uint8>(Waypoint::NumColors)) || (color == static_cast<uint8>(Waypoint::Invisible)))
								color = static_cast<uint8>(Waypoint::Blue);
						}

						planet = nextStringParm(params, curpos);
					}

					const float x = nextFloatParm(params, curpos);
					const float y = nextFloatParm(params, curpos);
					const float z = nextFloatParm(params, curpos);
					curpos = Unicode::skipWhitespace(params, curpos);

					const std::string& sceneId = ServerWorld::getSceneId();
					StringId s("planet_n", sceneId);

					const Unicode::String & name = ((curpos != Unicode::String::npos) ? (params.substr(curpos)) : (Unicode::narrowToWide("@" + s.getCanonicalRepresentation())));

					// get player object
					Waypoint waypoint(playerObject->createWaypoint(Location(Vector(x, y, z), NetworkId::cms_invalid, Location::getCrcBySceneName(planet)), false));
					if (waypoint.isValid())
					{
						waypoint.setName(name); // Waypoints are a bit like smart pointers, so this is changing the data on the PlayerObject, not just on a local variable
						waypoint.setColor(color);
					}
					else
					{
						Chat::sendSystemMessage(*obj, SharedStringIds::too_many_waypoints, Unicode::emptyString);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
* Parameters: <target> <chatType> <mood> <flags> <text...>
* All parameters except text are integers
*/

static void commandFuncSpatialChatInternal(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			size_t curpos = 0;

			const NetworkId targetId(nextStringParm(params, curpos));
			const int chatType = nextIntParm(params, curpos);
			const int mood = nextIntParm(params, curpos);
			int flags = nextIntParm(params, curpos);
			int language = nextIntParm(params, curpos);

			// Verify the language parameter

			if (!GameLanguageManager::isLanguageValid(language))
			{
				language = GameLanguageManager::getBasicLanguageId();
			}

			curpos = Unicode::skipWhitespace(params, curpos);

			if (curpos == Unicode::String::npos)
			{
				DEBUG_WARNING(true, ("empty string in spatial chat"));
				return;
			}

			const size_t nullpos = params.find(static_cast<unsigned short>(0), curpos);

			Unicode::String text;
			Unicode::String oob;

			if (nullpos != Unicode::String::npos)
			{
				text = params.substr(curpos, nullpos - curpos);
				oob = params.substr(nullpos + 1);
			}
			else
			{
				// Strip any color codes the user may have entered

				text = TextIterator(params.substr(curpos)).getPrintableText();
			}

			if (text.empty())
			{
				return;
			}

			const bool isPrivate = SpatialChatManager::isPrivate(chatType);

			if (isPrivate)
				flags |= MessageQueueSpatialChat::F_isPrivate;

			if (chatType != -1 && mood != -1 && flags != -1 && curpos != Unicode::String::npos)
			{
				uint16 volume = SpatialChatManager::getVolume(chatType);

				CreatureObject * const creatureActor = obj->asCreatureObject();

				//speak cs_spaceSpeechMultiple times when piloting a ship
				if (creatureActor && creatureActor->getPilotedShip())
					volume *= cs_spaceSpeechMultiple;

				// track amount of spatial chat for the character
				bool allowToSpeak = true;
				bool squelched = false;
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creatureActor);
				if (playerObject)
				{
					// update chat character count
					playerObject->updateChatSpamSpatialNumCharacters(actor, text.size());

					// see if player has exceeded chat limit
					if (ConfigServerGame::getChatSpamLimiterEnabledForFreeTrial() &&
						obj->getClient() &&
						((obj->getClient()->getSubscriptionFeatures() & ClientSubscriptionFeature::Base) == 0) &&
						((playerObject->getChatSpamSpatialNumCharacters() + playerObject->getChatSpamNonSpatialNumCharacters()) > ConfigServerGame::getChatSpamLimiterNumCharacters()))
					{
						allowToSpeak = false;
					}
					// see if the player is squelched
					else if (0 != playerObject->getSecondsUntilUnsquelched())
					{
						allowToSpeak = false;
						squelched = true;
					}
				}

				if (allowToSpeak)
				{
					// character allowed to talk
					obj->speakText(
						MessageQueueSpatialChat(
							CachedNetworkId(actor),
							CachedNetworkId(targetId),
							text,
							volume,
							static_cast<uint16>(chatType),
							static_cast<uint16>(mood),
							flags,
							language,
							oob));
				}
				else if (!squelched && (ConfigServerGame::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds() > 0) && obj->getClient())
				{
					// send message telling character he can no longer talk
					const int timeNow = static_cast<int>(::time(nullptr));
					const int chatSpamTimeEndInterval = playerObject->getChatSpamTimeEndInterval();
					if ((chatSpamTimeEndInterval > timeNow) && (timeNow >= playerObject->getChatSpamNextTimeToNotifyPlayerWhenLimited()))
					{
						GenericValueTypeMessage<int> csl("ChatSpamLimited", (chatSpamTimeEndInterval - timeNow));
						obj->getClient()->send(csl, true);

						playerObject->setChatSpamNextTimeToNotifyPlayerWhenLimited((timeNow + ConfigServerGame::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds()));
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
* Parameters: <target> <chatType name> <mood name> <text...>[nullptr terminator + oob]
* All parameters are strings
*/

static void commandFuncSpatialChat(Command const &, NetworkId const &actor, NetworkId const & targetId, Unicode::String const &params)
{
	if (actor != NetworkId::cms_invalid)
	{
		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		if (obj)
		{
			size_t curpos = 0;

			Unicode::String chatTypeName;
			Unicode::String moodTypeName;
			Unicode::String flagsString;

			if (!Unicode::getFirstToken(params, curpos, curpos, chatTypeName) || curpos == Unicode::String::npos)
			{
				WARNING(true, ("Not enough arguments to commandFuncSpatialChat (no chat type)"));
				return;
			}

			if (!Unicode::getFirstToken(params, ++curpos, curpos, moodTypeName) || curpos == Unicode::String::npos)
			{
				WARNING(true, ("Not enough arguments to commandFuncSpatialChat (no mood type)"));
				return;
			}

			if (!Unicode::getFirstToken(params, ++curpos, curpos, flagsString) || curpos == Unicode::String::npos)
			{
				WARNING(true, ("Not enough arguments to commandFuncSpatialChat (no flags)"));
				return;
			}

			curpos = Unicode::skipWhitespace(params, ++curpos);

			const std::string & narrow_chatTypeName = Unicode::wideToNarrow(chatTypeName);
			const std::string & narrow_moodTypeName = Unicode::wideToNarrow(moodTypeName);

			const uint32 chatType = SpatialChatManager::getChatTypeByName(narrow_chatTypeName);

			curpos = Unicode::skipWhitespace(params, curpos);

			if (curpos == Unicode::String::npos)
			{
				DEBUG_WARNING(true, ("empty string in spatial chat"));
				return;
			}

			size_t nullpos = params.find(static_cast<unsigned short>(0), curpos);

			Unicode::String text;
			Unicode::String oob;

			if (nullpos != Unicode::String::npos)
			{
				text = params.substr(curpos, nullpos - curpos);
				oob = params.substr(nullpos + 1);
			}
			else
			{
				text = params.substr(curpos);
			}

			if (curpos != Unicode::String::npos)
			{
				const uint32 moodType = MoodManager::getMoodByCanonicalName(narrow_moodTypeName);
				const bool isPrivate = SpatialChatManager::isPrivate(chatType);

				uint32 flags = atoi(Unicode::wideToNarrow(flagsString).c_str());
				if (isPrivate)
					flags |= MessageQueueSpatialChat::F_isPrivate;

				const uint16 volume = SpatialChatManager::getVolume(chatType);

				// track amount of spatial chat for the character
				bool allowToSpeak = true;
				bool squelched = false;
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(obj->asCreatureObject());
				if (playerObject)
				{
					// update chat character count
					playerObject->updateChatSpamSpatialNumCharacters(actor, text.size());

					// see if player has exceeded chat limit
					if (ConfigServerGame::getChatSpamLimiterEnabledForFreeTrial() &&
						obj->getClient() &&
						((obj->getClient()->getSubscriptionFeatures() & ClientSubscriptionFeature::Base) == 0) &&
						((playerObject->getChatSpamSpatialNumCharacters() + playerObject->getChatSpamNonSpatialNumCharacters()) > ConfigServerGame::getChatSpamLimiterNumCharacters()))
					{
						allowToSpeak = false;
					}
					// see if the player is squelched
					else if (0 != playerObject->getSecondsUntilUnsquelched())
					{
						allowToSpeak = false;
						squelched = true;
					}
				}

				if (allowToSpeak)
				{
					// character allowed to talk
					obj->speakText(
						MessageQueueSpatialChat(
							CachedNetworkId(actor),
							CachedNetworkId(targetId),
							text,
							volume,
							static_cast<uint16>(chatType),
							static_cast<uint16>(moodType),
							flags,
							0,
							oob));
				}
				else if (!squelched && (ConfigServerGame::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds() > 0) && obj->getClient())
				{
					// send message telling character he can no longer talk
					const int timeNow = static_cast<int>(::time(nullptr));
					const int chatSpamTimeEndInterval = playerObject->getChatSpamTimeEndInterval();
					if ((chatSpamTimeEndInterval > timeNow) && (timeNow >= playerObject->getChatSpamNextTimeToNotifyPlayerWhenLimited()))
					{
						GenericValueTypeMessage<int> csl("ChatSpamLimited", (chatSpamTimeEndInterval - timeNow));
						obj->getClient()->send(csl, true);

						playerObject->setChatSpamNextTimeToNotifyPlayerWhenLimited((timeNow + ConfigServerGame::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds()));
					}
				}
			}
			else
				WARNING(true, ("Invalid empty output string in commandFuncSpatialChat"));
		}
	}
}

//----------------------------------------------------------------------

/**
*
* string Params:
*       - string networkId of 'other' participant
*		- integer numeric value
*		- integer bitflags for transmission (0x01 == sendToActor, 0x02 == sendToTarget, 0x04 == sendToBystanders)
*       - string or stringID.  StringId is preceded with a '@'
*
static void commandFuncCombatSpam (Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	const CachedNetworkId actorId  (actor);
	const CachedNetworkId targetId (target);

	ServerObject * const obj = safe_cast<ServerObject *>(actorId.getObject ());
	if (!obj)
	{
		WARNING (true, ("nullptr actor in commandFuncCombatSpam"));
		return;
	}

	size_t curpos = 0;

	Unicode::String otherIdStr;
	Unicode::String intValueStr;
	Unicode::String bitflagsStr;
	Unicode::String stringId;
	Unicode::String combatSpamType;

	if (!Unicode::getFirstToken (params, curpos, curpos, otherIdStr) || curpos == Unicode::String::npos)
	{
		WARNING (true, ("Not enough arguments to commandFuncCombatSpam (missing networkId value)"));
		return;
	}

	if (!Unicode::getFirstToken (params, ++curpos, curpos, intValueStr) || curpos == Unicode::String::npos)
	{
		WARNING (true, ("Not enough arguments to commandFuncCombatSpam (missing int value)"));
		return;
	}

	if (!Unicode::getFirstToken (params, ++curpos, curpos, bitflagsStr) || curpos == Unicode::String::npos)
	{
		WARNING (true, ("Not enough arguments to commandFuncCombatSpam (missing bitflags)"));
		return;
	}

	if (!Unicode::getFirstToken (params, ++curpos, curpos, stringId) || curpos == Unicode::String::npos)
	{
		WARNING (true, ("Not enough arguments to commandFuncCombatSpam (missing stringId)"));
		return;
	}

	if (!Unicode::getFirstToken (params, ++curpos, curpos, combatSpamType))
	{
		WARNING (true, ("Not enough arguments to commandFuncCombatSpam (missing combatSpamType)"));
		return;
	}

	//DEBUG_REPORT_LOG(true, ("Combat spam type: %d\n", Unicode::toInt(combatSpamType)));

	const CachedNetworkId otherId (Unicode::wideToNarrow (otherIdStr));

	MessageQueueCombatSpam msg (actorId, (actor.getObject() ? actor.getObject()->getPosition_w() : Vector::zero), targetId, (target.getObject() ? target.getObject()->getPosition_w() : Vector::zero), otherId, 0, Unicode::String (), static_cast<unsigned char>(Unicode::toInt(combatSpamType)));

	curpos = Unicode::skipWhitespace(params, ++curpos);

	const int    intValue = atoi (Unicode::wideToNarrow (intValueStr).c_str ());
	const int    bitflags = atoi (Unicode::wideToNarrow (bitflagsStr).c_str ());

	if (bitflags == 0)
	{
		WARNING_STRICT_FATAL (true, ("Got a commandFuncCombatSpam with no recepients specified."));
	}

	msg.m_intValue = intValue;

	if (stringId [0] == '@')
	{
		stringId.erase (static_cast<size_t>(0), 1);
		msg.m_msgId = StringId (Unicode::wideToNarrow (stringId));
	}
	else
		msg.m_msg = stringId;

	enum Bitflags
	{
		B_sendToActor      = 0x01,
		B_sendToTarget     = 0x02,
		B_sendToBystanders = 0x04
	};

	const bool sendToSelf       = (bitflags & B_sendToActor)      != 0;
	const bool sendToTarget     = (bitflags & B_sendToTarget)     != 0;
	const bool sendToBystanders = (bitflags & B_sendToBystanders) != 0;

	obj->performCombatSpam (msg, sendToSelf, sendToTarget, sendToBystanders);
}
*/
//----------------------------------------------------------------------

/**
*
* string Params:
*		- string   target type (Player or ChatChannel)
*		- string   target id   (Player name or chat channel name, e.g. SWG.betaGalaxy.Justin or SWG.betaGalaxy.system)
*       - string   message
*/

static void commandFuncSystemMessage(Command const &, NetworkId const &, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;

	Unicode::String targetTypeStr;
	Unicode::String targetIdStr;

	if (!Unicode::getFirstToken(params, curpos, curpos, targetTypeStr) || curpos == Unicode::String::npos)
	{
		WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing int value)"));
		return;
	}

	if (!Unicode::getFirstToken(params, ++curpos, curpos, targetIdStr) || curpos == Unicode::String::npos)
	{
		WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing bitflags)"));
		return;
	}

	if (curpos == Unicode::String::npos)
	{
		WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing msg)"));
		return;
	}

	curpos = Unicode::skipWhitespace(params, ++curpos);

	Unicode::String output = params.substr(curpos);

	if (output.empty())
	{
		WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing msg)"));
		return;
	}

	std::string targetType = Unicode::wideToNarrow(targetTypeStr);
	std::string targetId = Unicode::wideToNarrow(targetIdStr);
	static const Unicode::String oob;

	if (targetType == "Player")
	{
		Chat::sendInstantMessage("SYSTEM", targetId, output, oob);
	}
	else if (targetType == "ChatChannel")
	{
		Chat::sendToRoom("SYSTEM", targetId, output, oob);
	}
}

//-----------------------------------------------------------------------

static void commandFuncSetWaypointActiveStatus(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const &params)
{
	CachedNetworkId actorId(actor);
	WARNING(!actorId.getObject(), ("commandSetWaypointActiveStatus: Could not get an object for actor id %s\n", actor.getValueString().c_str()));
	if (actorId.getObject())
	{
		CreatureObject * creature = dynamic_cast<CreatureObject *>(actorId.getObject());
		if (creature)
		{
			if (creature->isPlayerControlled())
			{
				PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
				if (playerObject)
				{
					Waypoint w = playerObject->getWaypoint(target);
					if (w.isValid())
					{
						Unicode::String status;
						size_t curpos = 0;
						if (!Unicode::getFirstToken(params, curpos, curpos, status))
						{
							WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing int value)"));
							return;
						}
						if (status == Unicode::narrowToWide("on"))
						{
							w.setActive(true);
						}
						else if (status == Unicode::narrowToWide("off"))
						{
							w.setActive(false);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncSetWaypointName(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const &params)
{
	CachedNetworkId actorId(actor);
	WARNING(!actorId.getObject(), ("commandFuncSetWaypointName: Could not get an object for actor id %s\n", actor.getValueString().c_str()));
	if (actorId.getObject())
	{
		CreatureObject * creature = dynamic_cast<CreatureObject *>(actorId.getObject());
		if (creature)
		{
			if (creature->isPlayerControlled())
			{
				PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
				if (playerObject)
				{
					Waypoint w = playerObject->getWaypoint(target);
					if (w.isValid())
					{
						Unicode::String name;
						size_t curpos = 0;
						if (!Unicode::getFirstToken(params, curpos, curpos, name))
						{
							WARNING(true, ("Not enough arguments to commandFuncSystemMessage (missing text value)"));
							return;
						}
						w.setName(params);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandSetPosture(Command const &command, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CachedNetworkId actorId(actor);
	if (actorId.getObject() != nullptr)
	{
		CreatureController * const controller = dynamic_cast<CreatureController *>(actorId.getObject()->getController());
		if (controller != nullptr)
		{
			CreatureObject * const creature = safe_cast<CreatureObject*>(actorId.getObject());
			NOT_NULL(creature);
			Postures::Enumerator const posture = creature->getPosture();

			// send the creature's posture to it
			MessageQueuePosture * message = new MessageQueuePosture(posture, true);
			controller->appendMessage(
				CM_setPosture,
				0.0f,
				message,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_ALL_CLIENT
			);

			// Ensure the chair sitting state is off if the player is not sitting.
			if (posture != Postures::Sitting)
			{
				creature->setState(States::SittingOnChair, false);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncJumpServer(Command const &command, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CachedNetworkId actorId(actor);
	if (actorId.getObject() != nullptr)
	{
		CreatureController * const controller = dynamic_cast<CreatureController *>(actorId.getObject()->getController());
		if (controller != nullptr)
		{
			controller->appendMessage(
				CM_jump,
				0.0f,
				nullptr,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_ALL_CLIENT
			);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAddBannedPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	curpos = Unicode::skipWhitespace(params, curpos);

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		CellObject *cell = dynamic_cast<CellObject *>(obj);
		if (cell)
		{
			BuildingObject *building = cell->getOwnerBuilding();
			if (building && building->isOwner(actor, client))
				cell->addBanned(Unicode::wideToNarrow(params.substr(curpos)));
		}
		else
		{
			BuildingObject *building = dynamic_cast<BuildingObject *>(obj);
			if (building && building->isOwner(actor, client))
				building->addBanned(Unicode::wideToNarrow(params.substr(curpos)));
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncRemoveBannedPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	curpos = Unicode::skipWhitespace(params, curpos);

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		CellObject *cell = dynamic_cast<CellObject *>(obj);
		if (cell)
		{
			BuildingObject *building = cell->getOwnerBuilding();
			if (building && building->isOwner(actor, client))
				cell->removeBanned(Unicode::wideToNarrow(params.substr(curpos)));
		}
		else
		{
			BuildingObject *building = dynamic_cast<BuildingObject *>(obj);
			if (building && building->isOwner(actor, client))
				building->removeBanned(Unicode::wideToNarrow(params.substr(curpos)));
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAddAllowedPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	curpos = Unicode::skipWhitespace(params, curpos);

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		CellObject *cell = dynamic_cast<CellObject *>(obj);
		if (cell)
		{
			BuildingObject *building = cell->getOwnerBuilding();
			if (building && building->isOwner(actor, client))
				cell->addAllowed(Unicode::wideToNarrow(params.substr(curpos)));
		}
		else
		{
			BuildingObject *building = dynamic_cast<BuildingObject *>(obj);
			if (building && building->isOwner(actor, client))
				building->addAllowed(Unicode::wideToNarrow(params.substr(curpos)));
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncRemoveAllowedPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	curpos = Unicode::skipWhitespace(params, curpos);

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		CellObject *cell = dynamic_cast<CellObject *>(obj);
		if (cell)
		{
			BuildingObject *building = cell->getOwnerBuilding();
			if (building && building->isOwner(actor, client))
				cell->removeAllowed(Unicode::wideToNarrow(params.substr(curpos)));
		}
		else
		{
			BuildingObject *building = dynamic_cast<BuildingObject *>(obj);
			if (building && building->isOwner(actor, client))
				building->removeAllowed(Unicode::wideToNarrow(params.substr(curpos)));
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetPublicState(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	int publicState = nextIntParm(params, curpos);

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		CellObject *cell = dynamic_cast<CellObject *>(obj);
		if (cell)
		{
			BuildingObject *building = cell->getOwnerBuilding();
			if (building && building->isOwner(actor, client))
				cell->setIsPublic(publicState ? true : false);
		}
		else
		{
			BuildingObject *building = dynamic_cast<BuildingObject *>(obj);
			if (building && building->isOwner(actor, client))
				building->setIsPublic(publicState ? true : false);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetOwner(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t curpos = 0;
	NetworkId objId(nextOidParm(params, curpos));
	NetworkId ownerId(nextOidParm(params, curpos));

	Object *obj = NetworkIdManager::getObjectById(objId);
	Client *client = getClientFromCharacterId(actor);
	if (obj)
	{
		ServerObject *tangibleObj = safe_cast<ServerObject *>(obj);
		if (tangibleObj && tangibleObj->isOwner(actor, client))
		{
			tangibleObj->setOwnerId(ownerId);
		}
	}
}

// ----------------------------------------------------------------------

static void resolveDuelParticipants(CreatureObject &actor, TangibleObject &target, TangibleObject *&actorFlagObj, CreatureObject *&targetMessageObj)
{
	ShipObject * const targetShip = target.asShipObject();
	if (targetShip)
	{
		// Dueling a ship
		// actorFlagObj is the containing ship of the actor if he owns it
		ShipObject * const actorShip = ShipObject::getContainingShipObject(&actor);
		if (actorShip
			&& actorShip->isPlayerShip()
			&& actor.getNetworkId() == actorShip->getOwnerId())
			actorFlagObj = actorShip;
		// targetMessageObj is the owner of the target ship
		if (targetShip->isPlayerShip())
		{
			Object * const o = NetworkIdManager::getObjectById(targetShip->getOwnerId());
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so)
					targetMessageObj = so->asCreatureObject();
			}
		}
	}
	else
	{
		actorFlagObj = &actor;
		targetMessageObj = target.asCreatureObject();
	}
}

// ----------------------------------------------------------------------

static void commandFuncDuel(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	if (target != NetworkId::cms_invalid && target != actor)
	{
		CreatureObject * const actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
		TangibleObject * const targetObj = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(target));
		if (actorObj && targetObj)
		{
			TangibleObject *actorFlagObj = 0;
			CreatureObject *targetMessageObj = 0;
			resolveDuelParticipants(*actorObj, *targetObj, actorFlagObj, targetMessageObj);

			if (actorFlagObj && targetMessageObj && actorObj != targetMessageObj)
			{
				if (PlayerObject::isIgnoring(actor, targetMessageObj->getNetworkId()))
					return;

				if (actorObj->hasAutoDeclineDuel())
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_CANNOT_DUEL_AUTO_DECLINE);
					return;
				}

				if (Pvp::canAttack(*actorObj, *targetObj))
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_ALREADY_CAN_ATTACK);
					return;
				}

				if (Pvp::canAttack(*targetObj, *actorObj))
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_NOT_AVAILABLE_TO_DUEL);
					return;
				}

				if (targetObj->hasAutoDeclineDuel())
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_NOT_AVAILABLE_TO_DUEL);
					sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_AUTO_DECLINED_DUEL);
					return;
				}

				if (targetObj->getObjVars().hasItem("hologram_performer"))
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_NOT_HOLOGRAM);
				}
				else if (!Pvp::isDuelingAllowed(*actorFlagObj, *targetObj))
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_NOT_HERE);
				}
				else if (Pvp::hasDuelEnemyFlag(*actorFlagObj, targetObj->getNetworkId()))
				{
					if (Pvp::hasDuelEnemyFlag(*targetObj, actorFlagObj->getNetworkId()))
						sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_ALREADY_DUELING);
					else
						sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_ALREADY_CHALLENGED);
				}
				else if (Pvp::hasDuelEnemyFlag(*targetObj, actorFlagObj->getNetworkId()))
				{
					// Duel Accepted and Starting.
					Pvp::setDuelEnemyFlag(*actorFlagObj, *targetObj);
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_ACCEPT_SELF);
					sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_ACCEPT_TARGET);

					// Call their Script Triggers
					ServerObject* serverActor = actorObj->asServerObject();
					if (serverActor)
					{
						ScriptParams params;
						params.addParam(actor);
						params.addParam(target);
						if (serverActor->getScriptObject())
						{
							IGNORE_RETURN(serverActor->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_DUEL_START, params));
						}
					}

					ServerObject* serverTarget = targetObj->asServerObject();
					if (serverTarget)
					{
						ScriptParams params;
						params.addParam(actor);
						params.addParam(target);
						if (serverTarget->getScriptObject())
						{
							IGNORE_RETURN(serverTarget->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_DUEL_START, params));
						}
					}
					/// End script triggers.
				}
				else
				{
					// Duel Request
					// Call their Script Triggers
					ServerObject* serverActor = actorObj->asServerObject();
					if (serverActor)
					{
						ScriptParams params;
						params.addParam(actor);
						params.addParam(target);
						if (serverActor->getScriptObject())
						{
							if (serverActor->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_DUEL_REQUEST, params) == SCRIPT_OVERRIDE)
								return;
						}
					}

					ServerObject* serverTarget = targetObj->asServerObject();
					if (serverTarget)
					{
						ScriptParams params;
						params.addParam(actor);
						params.addParam(target);
						if (serverTarget->getScriptObject())
						{
							if (serverTarget->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_DUEL_REQUEST, params) == SCRIPT_OVERRIDE)
								return;
						}
					}
					/// End script triggers.
					Pvp::setDuelEnemyFlag(*actorFlagObj, *targetObj);
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_CHALLENGE_SELF);
					sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_CHALLENGE_TARGET);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncEndDuel(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	if (target != NetworkId::cms_invalid)
	{
		CreatureObject * const actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
		TangibleObject * const targetObj = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(target));
		if (actorObj && targetObj)
		{
			TangibleObject *actorFlagObj = 0;
			CreatureObject *targetMessageObj = 0;
			resolveDuelParticipants(*actorObj, *targetObj, actorFlagObj, targetMessageObj);

			if (actorFlagObj && targetMessageObj)
			{
				if (Pvp::hasDuelEnemyFlag(*targetObj, actorFlagObj->getNetworkId()))
				{
					Pvp::removeDuelEnemyFlags(*targetObj, actorFlagObj->getNetworkId());
					if (Pvp::hasDuelEnemyFlag(*actorFlagObj, targetObj->getNetworkId()))
					{
						Pvp::removeDuelEnemyFlags(*actorFlagObj, targetObj->getNetworkId());
						sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_END_SELF);
						sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_END_TARGET);

						// notify script that the duel has ended because of the EndDuel command
						{
							ScriptParams params;
							params.addParam(targetObj->getNetworkId(), "target");
							ScriptDictionaryPtr dictionary;
							GameScriptObject::makeScriptDictionary(params, dictionary);
							if (dictionary.get() != nullptr)
							{
								dictionary->serialize();
								MessageToQueue::getInstance().sendMessageToJava(actorFlagObj->getNetworkId(),
									"endDuelCommandNotification", dictionary->getSerializedData(), 0, false);
							}
						}

						{
							ScriptParams params;
							params.addParam(actorFlagObj->getNetworkId(), "target");
							ScriptDictionaryPtr dictionary;
							GameScriptObject::makeScriptDictionary(params, dictionary);
							if (dictionary.get() != nullptr)
							{
								dictionary->serialize();
								MessageToQueue::getInstance().sendMessageToJava(targetObj->getNetworkId(),
									"endDuelCommandNotification", dictionary->getSerializedData(), 0, false);
							}
						}
					}
					else
					{
						sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_REJECT_SELF);
						sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_REJECT_TARGET);
					}
				}
				else if (Pvp::hasDuelEnemyFlag(*actorFlagObj, targetObj->getNetworkId()))
				{
					Pvp::removeDuelEnemyFlags(*actorFlagObj, targetObj->getNetworkId());
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_CANCEL_SELF);
					sendProseMessage(*targetMessageObj, actorObj, CommandStringId::SID_DUEL_CANCEL_TARGET);
				}
				else
				{
					sendProseMessage(*actorObj, targetMessageObj, CommandStringId::SID_DUEL_NOT_DUELING);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterActivate(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	if (targetObj && actorObj && targetObj->isOnAdminList(*actorObj))
		targetObj->activate(actor);
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterDeactivate(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	if (targetObj && actorObj &&targetObj->isOnAdminList(*actorObj))
		targetObj->deactivate();
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterHarvest(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	if (targetObj && actorObj && targetObj->isOnAdminList(*actorObj))
		targetObj->harvest();
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterSelectResource(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &parameter)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	NetworkId poolId(Unicode::wideToNarrow(parameter));
	if (targetObj && actorObj && targetObj->isOnAdminList(*actorObj))
		targetObj->selectResource(poolId, actor);
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterDiscardHopper(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	if (targetObj && actorObj &&targetObj->isOnAdminList(*actorObj))
		targetObj->discardAllHopperContents();
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterMakeCrate(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	size_t pos = 0;
	const NetworkId resourceId = nextOidParm(params, pos);
	const int amount = nextIntParm(params, pos);
	const bool discard = nextBoolParm(params, pos);
	const uint8 sequenceId = static_cast<uint8>(nextIntParm(params, pos));

	if (targetObj && actorObj && targetObj->isOnAdminList(*actorObj))
		targetObj->emptyHopper(actor, resourceId, amount, discard, sequenceId);
}

// ----------------------------------------------------------------------

static void commandFuncResourceSetName(Command const &, NetworkId const &, NetworkId const &target, Unicode::String const &params)
{
	ResourceTypeObject * const targetObj = ServerUniverse::getInstance().getResourceTypeById(target);
	if (targetObj)
		targetObj->setName(Unicode::wideToNarrow(params));
}

// ----------------------------------------------------------------------

static void commandFuncResourceContainerTransfer(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	CreatureObject * const player = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!player)
		return;

	ResourceContainerObject *sourceObj = dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(target));
	size_t pos = 0;
	NetworkId destId = nextOidParm(params, pos);
	ResourceContainerObject *destObj = dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(destId));
	int amount = nextIntParm(params, pos);

	if (!sourceObj || !destObj || amount <= 0)
		return;

	Container::ContainerErrorCode error = Container::CEC_Success;
	if (!player->canManipulateObject(*sourceObj, true, true, true, 10.0f, error))
	{
		ContainerInterface::sendContainerMessageToClient(*player, error, sourceObj);
		return;
	}
	if (!player->canManipulateObject(*destObj, true, true, true, 10.0f, error))
	{
		ContainerInterface::sendContainerMessageToClient(*player, error, destObj);
		return;
	}

	sourceObj->transferTo(*destObj, amount);
}

// ----------------------------------------------------------------------

static void commandFuncMakeSurvey(Command const &, NetworkId const &, NetworkId const &, Unicode::String const &)
{
	//TODO: Do something here someday
}

// ----------------------------------------------------------------------

static void commandFuncRequestSurvey(Command const &, NetworkId const &actor, NetworkId const &tool, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj && actorObj->getClient())
	{
		size_t curpos = 0;
		Unicode::String resourceName = Unicode::narrowToWide(nextStringParm(params, curpos));

		ScriptParams params;
		params.addParam(actor);
		params.addParam(resourceName);

		//now trigger the script, the actor should have exactly one on himself
		ServerObject *toolObj = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(tool));
		if (toolObj)
			toolObj->getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_SURVEY, params);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestCoreSample(Command const &, NetworkId const &actor, NetworkId const &tool, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (actorObj && actorObj->getClient())
	{
		size_t curpos = 0;
		Unicode::String resourceName = Unicode::narrowToWide(nextStringParm(params, curpos));

		ScriptParams params;
		params.addParam(actor);
		params.addParam(resourceName);

		//now trigger the script, the actor should have exactly one on himself
		ServerObject *toolObj = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(tool));
		if (toolObj)
			toolObj->getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_CORESAMPLE, params);
	}
}

// ----------------------------------------------------------------------

static void commandFuncResourceContainerSplit(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	ResourceContainerObject * const sourceObj = dynamic_cast<ResourceContainerObject*>(NetworkIdManager::getObjectById(target));
	if (!sourceObj)
		return;

	CreatureObject * const player = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!player)
		return;

	size_t pos = 0;
	const int amount = nextIntParm(params, pos);
	const CachedNetworkId destContainer(nextOidParm(params, pos));
	const int arrangementId = nextIntParm(params, pos);
	const Vector & newLocation = nextVectorParm(params, pos);

	Container::ContainerErrorCode error = Container::CEC_Success;
	if (!player->canManipulateObject(*sourceObj, true, true, true, 10.0f, error))
	{
		ContainerInterface::sendContainerMessageToClient(*player, error, sourceObj);
	}
	else if (!sourceObj->splitContainer(amount, destContainer, arrangementId, newLocation, safe_cast<ServerObject*>(NetworkIdManager::getObjectById(actor))))
	{
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_Full, safe_cast<ServerObject *>(destContainer.getObject()));
	}
}

// ----------------------------------------------------------------------

static void commandFuncFactoryCrateSplit(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	FactoryObject * const sourceObj = dynamic_cast<FactoryObject*>(NetworkIdManager::getObjectById(target));
	if (!sourceObj)
		return;

	CreatureObject * const player = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!player)
		return;

	size_t pos = 0;
	const int amount = nextIntParm(params, pos);
	const CachedNetworkId destContainerId(nextOidParm(params, pos));
	ServerObject * destContainer = safe_cast<ServerObject *>(destContainerId.getObject());
	if (destContainer == nullptr || ContainerInterface::getVolumeContainer(*destContainer) == nullptr)
	{
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_NotFound);
		return;
	}

	Container::ContainerErrorCode error = Container::CEC_Success;
	if (!player->canManipulateObject(*sourceObj, true, true, true, 10.0f, error))
	{
		ContainerInterface::sendContainerMessageToClient(*player, error, sourceObj);
	}
	else if (sourceObj->makeCopy(*destContainer, amount) == nullptr)
	{
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_Full,
			destContainer);
	}
}

// ----------------------------------------------------------------------

static void commandFuncPermissionListModify(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));

    if (actorObj && actorObj->getClient())
	{
	    // unlike the other options we can get passed, a city name will contain spaces (most likely)
	    // and because of the order of the params here, we have to do some special handling to grab
	    // the entire city name, like "city:Pits of Karkoon" without the trailing params of listName
	    if(Unicode::wideToNarrow(params).rfind("city:", 0) == 0)
        {
            Unicode::UnicodeStringVector tokens;
            Unicode::tokenize(params, tokens);
            Unicode::String temp;
            for (int i = 0; i < tokens.size(); i++)
            {
                if(i < tokens.size() - 2)
                {
                    temp += tokens[i];
                    if(i < tokens.size() -3)
                    {
                        temp += Unicode::narrowToWide(" ");
                    }
                }
            }
            size_t curpos = 0;
            const Unicode::String & playerName = temp;
            const Unicode::String & listName = tokens[tokens.size()-2];
            const Unicode::String & action = tokens[tokens.size()-1];
            ScriptParams scriptParams;
            scriptParams.addParam(actor);
            scriptParams.addParam(playerName);
            scriptParams.addParam(listName);
            scriptParams.addParam(action);
            actorObj->getScriptObject()->trigAllScripts(Scripting::TRIG_PERMISSION_LIST_MODIFY, scriptParams);
        }
	    else
        {
            size_t curpos = 0;
            const Unicode::String & playerName = Unicode::narrowToWide(nextStringParm(params, curpos));
            const Unicode::String & listName = Unicode::narrowToWide(nextStringParm(params, curpos));
            const Unicode::String & action = Unicode::narrowToWide(nextStringParm(params, curpos));
            ScriptParams scriptParams;
            scriptParams.addParam(actor);
            scriptParams.addParam(playerName);
            scriptParams.addParam(listName);
            scriptParams.addParam(action);
            actorObj->getScriptObject()->trigAllScripts(Scripting::TRIG_PERMISSION_LIST_MODIFY, scriptParams);
        }
	}
}

// ----------------------------------------------------------------------

static void commandFuncSynchronizedUiListen(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	ServerObject *targetObj = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(target));
	CreatureObject * const actorObject = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	if (targetObj && actorObject)
		targetObj->addSynchronizedUiClient(*actorObject);
}

// ----------------------------------------------------------------------

static void commandFuncSynchronizedUiStopListening(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	ServerObject *targetObj = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(target));
	if (targetObj)
		targetObj->removeSynchronizedUiClient(actor);
}

// ----------------------------------------------------------------------

static void commandFuncHarvesterGetResourceData(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	HarvesterInstallationObject *targetObj = dynamic_cast<HarvesterInstallationObject*>(NetworkIdManager::getObjectById(target));
	CreatureObject *actorObject = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	if (targetObj && actorObject && targetObj->isOnAdminList(*actorObject))
	{
		targetObj->sendResourceDataToClient(*actorObject);
	}
}

// ----------------------------------------------------------------------
/**
* @param params the destination container, and an optional arrangement
*/

//------------------------------------------------------------------------------------------
static void commandFuncTransferItem(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * const item = ServerWorld::findObjectByNetworkId(target);
	ServerObject * const playerSo = ServerWorld::findObjectByNetworkId(actor);

	if (!playerSo || !item)
	{
		DEBUG_REPORT_LOG(true, ("Received transfer item command for nullptr player or target.\n"));
		return;
	}

	if (item->isPlayerControlled())
		return;

	CreatureObject* const player = playerSo->asCreatureObject();
	if (!player)
		return;

	size_t curpos = 0;
	const NetworkId & destId = nextOidParm(params, curpos);
	const int arrangement = nextIntParm(params, curpos);
	const Vector & pos = nextVectorParm(params, curpos);
	Transform t;
	t.setPosition_p(pos);

	ServerObject * const destination = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(destId));

	//@todo this value (6) came from a data table that cannot be referenced.  It's the distance you can be from an object for pick up.  When we get command table range validation in, we can remove it.
	Container::ContainerErrorCode errorCode = Container::CEC_Success;

	// call canManipulateObject telling it to skip the no trade check;
	// we'll do the no trade check separately if canManipulateObject passes
	bool allowedByGodMode = false;
	bool canManipulateTarget = player->canManipulateObject(*item, true, true, true, 6, errorCode, true, &allowedByGodMode);
	if (canManipulateTarget && !allowedByGodMode && item->markedNoTrade())
	{
		// the item must be owned by this creature
		NetworkId ownerId;
		if (item->asIntangibleObject())
		{
			ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*item));
			ownerId = containedBy ? containedBy->getOwnerId() : NetworkId::cms_invalid;
		}
		else
		{
			ownerId = item->getOwnerId();
		}

		if (ownerId != player->getNetworkId())
		{
			canManipulateTarget = false;

			if (item->markedNoTradeShared(true))
			{
				// if the no trade item is a "shared" no trade item and is immediately
				// contained in a structure cell, and is being transferred to the
				// transferer's inventory (i.e. picked up), also allow the transfer
				// if the transferer is on the same account as the current owner of the item
				ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*item));
				if (containedBy && containedBy->asCellObject())
				{
					if (destination && (player->getInventory() == destination))
					{
						PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(player);
						if (playerObject && (playerObject->getStationId() == NameManager::getInstance().getPlayerStationId(ownerId)))
							canManipulateTarget = true;
					}
				}
			}
		}
	}

	if (!canManipulateTarget)
	{
		if (errorCode == Container::CEC_NoPermission)
		{
			Object * parentObject = ContainerInterface::getContainedByObject(*item);
			if (parentObject && parentObject->asServerObject() && parentObject->asServerObject()->asTangibleObject()
				&& parentObject->asServerObject()->asTangibleObject()->isLocked())
			{
				ContainerInterface::sendContainerMessageToClient(*player, errorCode, parentObject->asServerObject());
				return;
			}
		}
		ContainerInterface::sendContainerMessageToClient(*player, errorCode, item);
		return;
	}

	//-- don't transfer from/to factory crates
	{
		if (destination && destination->getGameObjectType() == SharedObjectTemplate::GOT_misc_factory_crate)
			return;

		const ServerObject * const containedBy = safe_cast<const ServerObject *>(ContainerInterface::getContainedByObject(*item));

		if (containedBy && containedBy->getGameObjectType() == SharedObjectTemplate::GOT_misc_factory_crate)
			return;
	}

	// If our destination is a locked container, make sure we can access it.
	bool lockedDestContainer = false;
	{
		if (destination && destination->asTangibleObject() && destination->asTangibleObject()->isLocked())
		{
			lockedDestContainer = true;
			TangibleObject * destTangible = destination->asTangibleObject();
			if (!destTangible->isUserOnAccessList(player->getNetworkId()) && !destTangible->isGuildOnAccessList(player->getGuildId())
				&& player->getClient() && !player->getClient()->isGod())
			{
				ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_NoPermission, destination);
				return;
			}
		}
	}

	{
		// No Drag and Drop while using Loot rules (especially random).
		const ServerObject *  source = getFirstCreatureContainer(item);

		if (source && source->asCreatureObject() && source->asCreatureObject()->isDead() && destination)
		{
			const ServerObject* dest = getFirstCreatureContainer(destination);
			if (dest && dest->asCreatureObject() && dest->asCreatureObject()->getGroup())
			{
				int lootRule = dest->asCreatureObject()->getGroup()->getLootRule();
				// Random/Lotto Loot Rule, no drag and drop allowed!
				if (lootRule == 3 || lootRule == 2)
				{
					return;
				}
			}
		}
	}

	if (!item->isAuthoritative())
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
			"RequestSameServer",
			std::make_pair(
				ContainerInterface::getTopmostContainer(*player)->getNetworkId(),
				ContainerInterface::getTopmostContainer(*item)->getNetworkId()));
		GameServer::getInstance().sendToPlanetServer(rssMessage);
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_TryAgain, item);
		return;
	}

	bool retval = true;
	if (destId == NetworkId::cms_invalid)
	{
		//to world
		retval = ContainerInterface::transferItemToWorld(*item, t, player, errorCode);
	}
	else
	{
		if (!destination)
		{
			DEBUG_REPORT_LOG(true, ("Cannot transfer item to unknown destination container\n"));
			return;
		}
		if (!destination->isAuthoritative())
		{
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
				"RequestSameServer",
				std::make_pair(
					ContainerInterface::getTopmostContainer(*player)->getNetworkId(),
					ContainerInterface::getTopmostContainer(*destination)->getNetworkId()));
			GameServer::getInstance().sendToPlanetServer(rssMessage);
			ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_TryAgain, destination);
			return;
		}

		//@todo quick check to make sure people aren't exploiting unlimited volume containers.  Really there shouldn't be any of these, but there have been bugs in the past...
		VolumeContainer * vol = ContainerInterface::getVolumeContainer(*destination);
		if (vol && vol->getTotalVolume() == static_cast<int>(VolumeContainer::VL_NoVolumeLimit))
		{
			WARNING(true, ("Player accessing infinite volume container %s", destination->getNetworkId().getValueString().c_str()));
			//send error message and return
		}

		//Don't allow people to put things in unowned containers. Locked containers are allowed since they enforce their own list.
		if (!destination->getObjVars().hasItem(OBJVAR_PLAYERS_CAN_ACCESS_CONTAINER) && !lockedDestContainer &&
			player->getClient() && !player->getClient()->isGod() &&
			destination->getOwnerId() == NetworkId::cms_invalid)
		{
			errorCode = Container::CEC_NoPermission;
			ContainerInterface::sendContainerMessageToClient(*player, errorCode, destination);
			return;
		}

		//Check to make sure you can manipulate the destination
		if (!player->canManipulateObject(*destination, false, true, true, 6, errorCode))
		{
			ContainerInterface::sendContainerMessageToClient(*player, errorCode, destination);
			return;
		}

		//special case.  If this is our inventory we are unequipping to, allow it to be somewhat overloaded.
		const Object * itemParent = ContainerInterface::getContainedByObject(*item);
		if (itemParent && itemParent == player && player->getInventory() == destination && item->getVolume() < 2)
		{
			VolumeContainer * volContainer = ContainerInterface::getVolumeContainer(*destination);
			if (!volContainer)
			{
				errorCode = Container::CEC_Unknown;
			}
			else
			{
				const int allowedOverload = volContainer->getTotalVolume() + 2;
				if (volContainer->getCurrentVolume() >= allowedOverload)
				{
					errorCode = Container::CEC_Full;
				}
				else
				{
					int oldCap = volContainer->debugDoNotUseSetCapacity(allowedOverload);
					retval = ContainerInterface::transferItemToVolumeContainer(*destination, *item, player, errorCode, true);
					IGNORE_RETURN(volContainer->debugDoNotUseSetCapacity(oldCap));
				}
			}
		}
		else
		{
			const Object * destParent = ContainerInterface::getContainedByObject(*destination);
			if (destParent && destParent == player->getAppearanceInventory())
			{
				// Trying to transfer to a container currently in our Appearance inventory. Not allowed.
				StringId const code("container_error_message", "container34_prose");

				ProsePackage pp;
				pp.stringId = code;
				pp.actor.id = player->getNetworkId();

				// Send to the player.
				Chat::sendSystemMessage(*player, pp);

				retval = false;
			}
			else if (destination == player->getAppearanceInventory() && ContainerInterface::getContainer(*item) != nullptr)
			{
				const Container * itemContainer = ContainerInterface::getContainer(*item);
				if (itemContainer && itemContainer->getNumberOfItems() == 0)
				{
					retval = ContainerInterface::transferItemToUnknownContainer(*destination, *item, arrangement, t, player, errorCode);
				}
				else
				{
					StringId const code("container_error_message", "container33_prose");

					ProsePackage pp;
					pp.stringId = code;
					pp.actor.id = player->getNetworkId();

					// Send to the player.
					Chat::sendSystemMessage(*player, pp);
				}
			}
			else
			{
				retval = ContainerInterface::transferItemToUnknownContainer(*destination, *item, arrangement, t, player, errorCode);

				// if we are equipping an object to a filled slot, move the equipped item(s) to our inventory
				if (!retval && errorCode == Container::CEC_SlotOccupied && destination->getNetworkId() == player->getNetworkId())
				{
					ServerObject * inventory = player->getInventory();
					const SlottedContainer * equipment = ContainerInterface::getSlottedContainer(*player);
					const SlottedContainmentProperty * itemContainmentProperty = ContainerInterface::getSlottedContainmentProperty(*item);

					if (inventory != nullptr && equipment != nullptr && itemContainmentProperty != nullptr)
					{
						std::vector<std::pair<ServerObject *, SlotId> > oldItems;
						retval = true;
						int slotCount = itemContainmentProperty->getNumberOfSlots(arrangement);
						for (int i = 0; i < slotCount && retval; ++i)
						{
							SlotId slot = itemContainmentProperty->getSlotId(arrangement, i);
							const Container::ContainedItem & currentWeaponId = equipment->getObjectInSlot(slot, errorCode);
							ServerObject * oldItem = safe_cast<ServerObject *>(currentWeaponId.getObject());
							if (oldItem != nullptr)
							{
								if (ContainerInterface::transferItemToVolumeContainer(*inventory, *oldItem, player, errorCode, true))
								{
									oldItems.push_back(std::make_pair(oldItem, slot));
								}
								else
									retval = false;
							}
						}
						if (retval)
						{
							if (!ContainerInterface::transferItemToUnknownContainer(*destination, *item, arrangement, t, player, errorCode))
							{
								// move the old items back into their slots
								reequipItems(*destination, oldItems);
								retval = false;
							}
						}
						else
						{
							// move the old items back into their slots
							reequipItems(*destination, oldItems);
							errorCode = Container::CEC_SlotOccupied;
						}
					}
				}
			}
		}
	}

	if (!retval && player)
	{
		if (errorCode == Container::CEC_SlotOccupied && destination == player->getAppearanceInventory())
		{
			// Failed to place an object in our appearance inventory. Let's flush out some more information.
			ServerObject * appearanceInv = player->getAppearanceInventory();
			const SlottedContainer * equipment = ContainerInterface::getSlottedContainer(*appearanceInv);
			const SlottedContainmentProperty * itemContainmentProperty = ContainerInterface::getSlottedContainmentProperty(*item);

			if (!itemContainmentProperty || !equipment || arrangement < 0)
			{
				// This should never happen, but if it does - just send our normal bland error message.
				ContainerInterface::sendContainerMessageToClient(*player, errorCode);
				return;
			}

			int slotCount = itemContainmentProperty->getNumberOfSlots(arrangement);
			std::vector<ServerObject *> objectsToSend;
			for (int i = 0; i < slotCount; ++i)
			{
				SlotId slot = itemContainmentProperty->getSlotId(arrangement, i);
				const Container::ContainedItem & currentWeaponId = equipment->getObjectInSlot(slot, errorCode);
				ServerObject * oldItem = safe_cast<ServerObject *>(currentWeaponId.getObject());
				if (oldItem != nullptr)
				{
					if (std::find(objectsToSend.begin(), objectsToSend.end(), oldItem) == objectsToSend.end())
						objectsToSend.push_back(oldItem);
				}
			}

			std::vector<ServerObject *>::iterator iter = objectsToSend.begin();
			for (; iter != objectsToSend.end(); ++iter)
			{
				// No idea how this could happen, but just incase.
				if ((*iter) == nullptr)
					continue;

				StringId const code("container_error_message", "container32_prose");

				// Setup the prose package with all the %TT and %TO information.
				ProsePackage pp;
				pp.stringId = code;

				pp.target.id = (*iter)->getNetworkId();
				pp.target.str = (*iter)->getAssignedObjectName();
				if (pp.target.str.empty())
					pp.target.stringId = (*iter)->getObjectNameStringId();

				pp.other.id = item->getNetworkId();
				pp.other.str = item->getAssignedObjectName();
				if (pp.other.str.empty())
					pp.other.stringId = item->getObjectNameStringId();

				pp.actor.id = player->getNetworkId();

				// Send to the player.
				Chat::sendSystemMessage(*player, pp);
			}

			return;
		}

		ContainerInterface::sendContainerMessageToClient(*player, errorCode);
	}
}
// ----------------------------------------------------------------------

static bool isGoingInWeaponSlot(const ServerObject& target, int arrangement)
{
	if (arrangement <= 0)
		return false;

	const SlottedContainmentProperty * slotted = ContainerInterface::getSlottedContainmentProperty(target);
	bool useWeapon = false;
	if (slotted)
	{
		//In this case, the player is transferring something from their equipment, which may be armor or a weapon
		static const SlotId right_hand = SlotIdManager::findSlotId(CrcLowerString("hold_r"));
		static const SlotId left_hand = SlotIdManager::findSlotId(CrcLowerString("hold_l"));

		const int numSlots = slotted->getNumberOfSlots(arrangement);
		for (int i = 0; i < numSlots; ++i)
		{
			SlotId currentSlot = slotted->getSlotId(arrangement, i);
			if (currentSlot == right_hand || currentSlot == left_hand)
			{
				useWeapon = true;
				break;
			}
		}
	}
	return useWeapon;
}

//------------------------------------------------------------------------------------------

static void commandFuncTransferWeapon(Command const & c, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * item = ServerWorld::findObjectByNetworkId(target);
	CreatureObject * actorObject = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));

	size_t curpos = 0;
	const NetworkId & destId = nextOidParm(params, curpos);
	const int arrangement = nextIntParm(params, curpos);
	UNREF(destId);

	if (item && actorObject && isGoingInWeaponSlot(*item, arrangement))
	{
		commandFuncTransferItem(c, actor, target, params);
	}
}

//------------------------------------------------------------------------------------------

static void commandFuncTransferArmor(Command const & c, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	CreatureObject * actorObject = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObject)
	{
		commandFuncTransferItem(c, actor, target, params);
	}
}

//------------------------------------------------------------------------------------------

void CommandCppFuncs::commandFuncTransferMisc(Command const & c, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * item = ServerWorld::findObjectByNetworkId(target);
	CreatureObject * actorObject = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));

	size_t curpos = 0;
	const NetworkId & destId = nextOidParm(params, curpos);
	const int arrangement = nextIntParm(params, curpos);

	//This command can only be used to transfer non-weapon objects not contained directly by the player unless it is not armor
	if (item && actorObject && !isGoingInWeaponSlot(*item, arrangement) &&
		(destId != actor || !GameObjectTypes::isTypeOf(item->getGameObjectType(), SharedObjectTemplate::GOT_armor)))
	{
		commandFuncTransferItem(c, actor, target, params);
	}
}

//------------------------------------------------------------------------------------------
/**
* @param params the slotname, if any, to be echo'd back to the client or passed to the script
*/

static void commandFuncOpenContainer(Command const & cmd, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * container = ServerWorld::findObjectByNetworkId(target);
	CreatureObject * player = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!container || !player)
	{
		DEBUG_REPORT_LOG(true, ("Received open command for nullptr player or target.\n"));
		return;
	}
	//@todo check permissions
	if (!ContainerInterface::getContainer(*container))
	{
		DEBUG_REPORT_LOG(true, ("Received open command on non container object.\n"));
		return;
	}

	//-- don't open factory crates or crafting tools
	const int got = container->getGameObjectType();
	if (got == SharedObjectTemplate::GOT_misc_factory_crate || got == SharedObjectTemplate::GOT_tool_crafting)
	{
		return;
	}
	
	//-- if we are requesting to open an inventory, datapad, or bank, make sure it belongs to us or we're a CSR
	//-- this provides better security for /editDatapad /editInventory and /editBank admin commands.
	const uint32 crc = container->getTemplateCrc();
	if((player->isPlayerControlled()) && (crc == 2007924155 || crc == -1783727815 || crc == -172438875))
	{
		Client * const clientObj = player->getClient();
		if(clientObj)
		{
			if(container->getOwnerId() != actor && !clientObj->isGod())
			{
				return;
			}
		}
	}

	//-- if they are opening a crafting station, what they really want is the hopper
	//-- but only if the object is not a volume container
	if (container->getGameObjectType() == SharedObjectTemplate::GOT_misc_crafting_station
		&& (nullptr == ContainerInterface::getVolumeContainer(*container)))
	{
		static const SlotId inputHopperId(SlotIdManager::findSlotId(CrcLowerString("ingredient_hopper")));

		ServerObject const * const station = container;
		ServerObject * hopper = nullptr;
		const SlottedContainer * stationContainer = ContainerInterface::getSlottedContainer(*station);
		if (stationContainer != nullptr)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object* tmpHopperObj = (stationContainer->getObjectInSlot(inputHopperId, tmp)).getObject();
			if (tmp == Container::CEC_Success && tmpHopperObj)
			{
				hopper = tmpHopperObj->asServerObject();
			}
		}

		if (hopper)
		{
			container = hopper;
		}
		else
		{
			// report the failure as it indicates a change to some other system
			WARNING(true, ("Received open command on a crafting station and failed to open the hopper.\n"));
		}
	}

	size_t curpos = 0;

	std::string slotName;
	int sequence = nextIntParm(params, curpos);
	if (curpos != std::string::npos)
	{
		++curpos;
		slotName = nextStringParm(params, curpos);
	}

	if (container->isAuthoritative())
	{
		Container::ContainerErrorCode code = Container::CEC_Success;

		bool const isPublicContainer = container->getGameObjectType() == SharedObjectTemplate::GOT_misc_container_public;

		if (isPublicContainer && player->getClient())
			ObserveTracker::onClientAboutToOpenPublicContainer(*player->getClient(), *container);

		bool const doPermissionCheckOnItem = !isPublicContainer;
		bool const doPermissionCheckOnParents = !isPublicContainer;

		if (player->canManipulateObject(*container, false, doPermissionCheckOnItem, doPermissionCheckOnParents, 6.0f, code))
		{
			// Additional check for Locked containers.
			if (container->asTangibleObject() && container->asTangibleObject()->isLocked())
			{
				TangibleObject * tangibleTarget = container->asTangibleObject();
				if (!tangibleTarget->isUserOnAccessList(player->getNetworkId()) && !tangibleTarget->isGuildOnAccessList(player->getGuildId()))
				{
					// Player is NOT on the admin list or guild list, they cannot open the container.
					ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_NoPermission, container);
					return;
				}
			}

			ScriptParams params;
			params.addParam(actor);
			if (container->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_OPEN_CONTAINER, params) == SCRIPT_CONTINUE)
			{
				Client *client = player->getClient();
				if (client)
					client->openContainer(*container, sequence, slotName);
			}
		}
		else
		{
			ContainerInterface::sendContainerMessageToClient(*player, code, container);
		}
	}
	else
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
			"RequestSameServer",
			std::make_pair(
				ContainerInterface::getTopmostContainer(*player)->getNetworkId(),
				ContainerInterface::getTopmostContainer(*container)->getNetworkId()));
		GameServer::getInstance().sendToPlanetServer(rssMessage);
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_TryAgain, container);
	}
}

// ----------------------------------------------------------------------

static void commandFuncCloseContainer(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	ServerObject * const container = ServerWorld::findObjectByNetworkId(target);
	ServerObject * const player = ServerWorld::findObjectByNetworkId(actor);
	if (!container || !player)
	{
		DEBUG_REPORT_LOG(true, ("Received close command for nullptr player or target.\n"));
		return;
	}

	//-- can't close ourself
	if (player == container)
		return;

	if (!ContainerInterface::getContainer(*container))
	{
		DEBUG_REPORT_LOG(true, ("Received close command on non container object.\n"));
		return;
	}

	if (container->isAuthoritative())
	{
		Client * const client = player->getClient();
		if (client)
			client->closeContainer(*container);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Multi-Server not supported for containers yet"));
		//@todo the player is trying to access a container not on this server.  Somehow get it here
		//Then do the check
	}
}

//------------------------------------------------------------------------------------------

static void commandFuncOpenLotteryContainer(Command const & /*command*/, NetworkId const & /*actor*/, NetworkId const & /*target*/, Unicode::String const & /*params*/)
{
	// moved from a command to a script hook.
	// This needs to be removed for the next
	// official non-hotfix patch
}

// ----------------------------------------------------------------------

static void commandFuncCloseLotteryContainer(Command const & cmd, NetworkId const & actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * const container = ServerWorld::findObjectByNetworkId(target);
	if (container == 0)
	{
		DEBUG_REPORT_LOG(true, ("commandFuncOpenLotteryContainer: container == 0.\n"));
		return;
	}

	CreatureObject * const player = safe_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (player == 0)
	{
		DEBUG_REPORT_LOG(true, ("commandFuncOpenLotteryContainer: player == 0.\n"));
		return;
	}

	if (ContainerInterface::getContainer(*container) == 0)
	{
		DEBUG_REPORT_LOG(true, ("commandFuncOpenLotteryContainer: ContainerInterface::getContainer == 0.\n"));
		return;
	}

	Controller * const controller = player->getController();

	if (controller != 0)
	{
		typedef MessageQueueGenericValueType<NetworkId> Message;
		Message * const message = new Message(target);

		controller->appendMessage(
			CM_groupCloseLotteryWindowOnClient,
			0.0f,
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

// ----------------------------------------------------------------------
/**
* @param params the destination container, and an optional arrangement
*/
static void commandFuncGiveItem(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	ServerObject * const item = ServerWorld::findObjectByNetworkId(target);
	ServerObject * const player = ServerWorld::findObjectByNetworkId(actor);
	if (!player || !item)
	{
		DEBUG_REPORT_LOG(true, ("Received give item command for nullptr player or target.\n"));
		return;
	}
	size_t curpos = 0;
	NetworkId destId = nextOidParm(params, curpos);

	//@todo this value (6) came from a data table that cannot be referenced.  It's the distance you can be from an object for pick up.  When we get command table range validation in, we can remove it.
	Container::ContainerErrorCode errorCode = Container::CEC_Success;
	if (!player->canManipulateObject(*item, true, true, true, 6, errorCode))
	{
		ContainerInterface::sendContainerMessageToClient(*player, errorCode, item);
		return;
	}

	if (!item->isAuthoritative())
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
			"RequestSameServer",
			std::make_pair(
				ContainerInterface::getTopmostContainer(*player)->getNetworkId(),
				ContainerInterface::getTopmostContainer(*item)->getNetworkId()));
		GameServer::getInstance().sendToPlanetServer(rssMessage);
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_TryAgain, item);
		return;
	}

	ServerObject* const destination = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(destId));

	if (!destination)
	{
		DEBUG_REPORT_LOG(true, ("Cannot transfer item to unknown destination container\n"));
		return;
	}

	if (!destination->isAuthoritative())
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
			"RequestSameServer",
			std::make_pair(
				ContainerInterface::getTopmostContainer(*player)->getNetworkId(),
				ContainerInterface::getTopmostContainer(*destination)->getNetworkId()));
		GameServer::getInstance().sendToPlanetServer(rssMessage);
		ContainerInterface::sendContainerMessageToClient(*player, Container::CEC_TryAgain, destination);
		return;
	}

	if (!player->canManipulateObject(*destination, false, false, false, 6, errorCode))
	{
		ContainerInterface::sendContainerMessageToClient(*player, errorCode, destination);
		return;
	}

	// check and see if this is a gem->socket operation, which is handled by our code
	TangibleObject * const socket = dynamic_cast<TangibleObject *>(destination);
	if (socket != nullptr)
	{
		TangibleObject * const gem = dynamic_cast<TangibleObject *>(item);
		if (gem != nullptr)
		{
			const int destGot = socket->getGameObjectType();
			const SharedTangibleObjectTemplate * const gemTemplate = safe_cast<const SharedTangibleObjectTemplate *>(gem->getSharedTemplate());
			const int count = gemTemplate->getSocketDestinationsCount();

			for (int i = 0; i < count; ++i)
			{
				const int gemSocketGot = gemTemplate->getSocketDestinations(i);
				if (destGot == gemSocketGot || GameObjectTypes::isTypeOf(destGot, gemSocketGot))
				{
					// make sure the gem and socket are both in the player's inventory
					if (!socket->isContainedBy(*player, true) ||
						!gem->isContainedBy(*player, true))
					{
						Chat::sendSystemMessage(*player, SharedStringIds::gem_not_in_inventory, Unicode::emptyString);
						return;
					}

					// if the socketed item is equipped, unequip it temporarily
					CreatureObject * owner = nullptr;
					Object * container = ContainerInterface::getContainedByObject(*socket);
					if (container != nullptr && container->asServerObject()->asCreatureObject() != nullptr)
					{
						owner = container->asServerObject()->asCreatureObject();
						// fake unequipping the item
						owner->onContainerLostItem(nullptr, *socket, nullptr);
					}

					std::vector<std::pair<std::string, int> > skillModBonuses;
					gem->getSkillModBonuses(skillModBonuses, false);
					if (socket->addSkillModSocketBonuses(skillModBonuses))
					{
						gem->permanentlyDestroy(DeleteReasons::Consumed);
						Chat::sendSystemMessage(*player, SharedStringIds::gem_inserted, Unicode::emptyString);
					}
					else
					{
						// tell the player they can't use the gem
						Chat::sendSystemMessage(*player, SharedStringIds::gem_not_inserted, Unicode::emptyString);
					}
					if (owner != nullptr)
					{
						// "re-equip" the item
						owner->onContainerGainItem(*socket, nullptr, nullptr);
					}
					return;
				}
			}
		}
	}

	// nope, pass it on to scripts
	ScriptParams scriptParameters;
	scriptParameters.addParam(target);
	scriptParameters.addParam(actor);

	if (destination->getScriptObject()->trigAllScripts(Scripting::TRIG_GIVE_ITEM, scriptParameters) == SCRIPT_OVERRIDE)
	{
		item->permanentlyDestroy(DeleteReasons::BadContainerTransfer);
	}
}

// ----------------------------------------------------------------------

static void commandFuncGroupInvite(Command const & command, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	if (actor == target)
		return;

	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	ShipObject const * const actorShipObject = ShipObject::getContainingShipObject(actorObj);

	GroupObject * const groupObj = actorObj->getGroup();
	if (groupObj)
	{
		if (groupObj->getGroupLeaderId() != actor)
		{
			sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_MUST_BE_LEADER);
			return;
		}
	}

	NetworkId parsedNetworkId = NetworkId::cms_invalid;
	if (target.isValid())
	{
		CreatureObject * const targetObj = findAndResolveCreatureByNetworkId(target);
		if (targetObj)
		{
			// pointer check is done for space
			if (actorObj == targetObj)
			{
				return;
			}

			parsedNetworkId = targetObj->getNetworkId();
		}
	}
	else if (!params.empty())
	{
		// check to see if a target name was specified, and see
		// if that resolves into a player character in the galaxy
		parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
	}

	if (!parsedNetworkId.isValid())
	{
		sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_INVITE_NO_TARGET_SELF);
		return;
	}

	// cannot invite while in combat
	if (actorObj->getState(States::Combat) && ConfigServerGame::getGroupInviteInviterCombatRestriction())
	{
		sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_INVITE_IN_COMBAT);
		return;
	}

	// send a messageTo to the target to invite to group
	char buffer[1024];
	snprintf(buffer, sizeof(buffer) - 1, "%s|%s|%s|%s", actor.getValueString().c_str(), (actorShipObject ? actorShipObject->getNetworkId().getValueString().c_str() : "0"), (groupObj ? groupObj->getNetworkId().getValueString().c_str() : "0"), Unicode::wideToNarrow(actorObj->getAssignedObjectName()).c_str());
	buffer[sizeof(buffer) - 1] = '\0';

	MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
		"C++InviteToGroupReq",
		buffer,
		0,
		false,
		actor,
		"C++InviteToGroupRspTargetNotFound");
}

// ----------------------------------------------------------------------

static void commandFuncGroupUninvite(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	if (actor == target)
		return;

	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	NetworkId parsedNetworkId = NetworkId::cms_invalid;
	if (target.isValid())
	{
		CreatureObject * const targetObj = findAndResolveCreatureByNetworkId(target);
		if (targetObj)
		{
			parsedNetworkId = targetObj->getNetworkId();
		}
	}
	else if (!params.empty())
	{
		// check to see if a target name was specified, and see
		// if that resolves into a player character in the galaxy
		parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
	}

	if (!parsedNetworkId.isValid())
	{
		sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_UNINVITE_NO_TARGET_SELF);
		return;
	}

	// send a messageTo to the target to uninvite
	char buffer[1024];
	snprintf(buffer, sizeof(buffer) - 1, "%s|%s", actor.getValueString().c_str(), Unicode::wideToNarrow(actorObj->getAssignedObjectName()).c_str());
	buffer[sizeof(buffer) - 1] = '\0';

	MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
		"C++UninviteFromGroupReq",
		buffer,
		0,
		false,
		actor,
		"C++UninviteFromGroupRspTargetNotFound");
}

// ----------------------------------------------------------------------

static void commandFuncGroupJoin(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	if ((actorObj->getGroup()) && (!GroupHelpers::creatureIsContainedInPOBShip(actorObj)))
		return;

	NetworkId const & inviterId = actorObj->getGroupInviterId();
	if (!inviterId.isValid())
	{
		sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_MUST_BE_INVITED);
		return;
	}

	// cannot join while in combat
	if (actorObj->getState(States::Combat) && ConfigServerGame::getGroupJoinInviteeCombatRestriction())
	{
		sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_JOIN_IN_COMBAT);
		return;
	}

	// send a messageTo to the inviter requesting the inviter's
	// group/group member information so we can join the group
	// or create a new group with the inviter as the group leader
	MessageToQueue::getInstance().sendMessageToC(inviterId,
		"C++GroupJoinInviterInfoReq",
		actorObj->getNetworkId().getValueString(),
		0,
		false,
		actor,
		"C++GroupJoinInviterInfoReqInviterNotFound");

	actorObj->setGroupInviter(NetworkId::cms_invalid, std::string(), NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

static void commandFuncGroupDecline(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		NetworkId inviterId = actorObj->getGroupInviterId();
		if (inviterId.isValid())
		{
			// notify invitee
			ProsePackage pp;
			pp.stringId = GroupStringId::SID_GROUP_DECLINE_SELF;
			pp.target.str = Unicode::narrowToWide(actorObj->getGroupInviterName());
			pp.actor.id = actorObj->getNetworkId();
			Chat::sendSystemMessage(*actorObj, pp);

			// notify inviter
			StringId responseSid = GroupStringId::SID_GROUP_DECLINE_LEADER;
			std::string const inviteeName(Unicode::wideToNarrow(actorObj->getAssignedObjectName()));
			std::string response = responseSid.getCanonicalRepresentation();
			response += "|";

			if (!inviteeName.empty())
			{
				response += "nameString|";
				response += inviteeName;
			}
			else
			{
				StringId inviteeNameSid = actorObj->getObjectNameStringId();
				response += "nameStringId|";
				response += inviteeNameSid.getCanonicalRepresentation();
			}

			MessageToQueue::getInstance().sendMessageToC(inviterId,
				"C++GroupOperationGenericRsp",
				response,
				0,
				false);
		}

		actorObj->setGroupInviter(NetworkId::cms_invalid, std::string(), NetworkId::cms_invalid);
	}
}

// ----------------------------------------------------------------------

static void commandFuncGroupDisband(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		GroupObject * const groupObj = actorObj->getGroup();
		if (groupObj)
		{
			if (groupObj->getGroupLeaderId() != actor)
			{
				// we are not the leader.  we are tring to leave the group
				NetworkId const & POBShipId = groupObj->getPOBShipMemberIsContainedIn(actor);

				if ((POBShipId.isValid()) && (groupObj->getPOBShipOwnedByMember(actor) == POBShipId))
				{
					// we own this POB so we can remove everyone inside it from the group
					GroupMemberParam const & leader = groupObj->createGroupMemberParamForMember(actor);

					GroupObject::GroupMemberParamVector membersInsidePOB;
					GroupObject::GroupMemberParamVector membersOutsidePOB;

					GroupHelpers::separateGroupBasedOffofPOBShip(groupObj,
						actor,
						POBShipId,
						membersInsidePOB,
						membersOutsidePOB);

					// if there are no members outside, then the group only consisted of
					// the people in the POB.  If that's the case, then don't destroy the
					// old group and create a new one
					if (!membersOutsidePOB.empty())
					{
						groupObj->removeGroupMember(actor);

						if (actorObj->getGroup() == groupObj)
							actorObj->getGroup()->onGroupMemberRemoved(actorObj->getNetworkId(), false);

						// remove everyone inside the target POB from the group
						GroupHelpers::removeFromGroupAndCreatePOBGroup(groupObj, leader, membersInsidePOB);
					}
				}
				else
				{
					// we don't own the POB or we are not in a POB ship so just remove us
					groupObj->removeGroupMember(actor);

					if (actorObj->getGroup() == groupObj)
						actorObj->getGroup()->onGroupMemberRemoved(actorObj->getNetworkId(), false);
				}
			}
			else
			{
				// we are the group leader, no target is specified, so we disband the group
				if (!target.isValid())
				{
					// find everyone in a POB and remove them from the group
					// and create new POB based groups for them

					// this is done in 2 steps because the second half will actually remove
					// members from the group.  we don't want to iterate over the members as
					// they are being removed

					GroupObject::GroupMemberParamVector membersOwningPOBs;
					std::vector<NetworkId> membersNotInPOBs;

					{
						GroupObject::GroupMemberVector const & groupMembers = groupObj->getGroupMembers();
						GroupObject::GroupMemberVector::const_iterator ii = groupMembers.begin();
						GroupObject::GroupMemberVector::const_iterator iiEnd = groupMembers.end();

						for (; ii != iiEnd; ++ii)
						{
							// get the member
							NetworkId const & memberId = ii->first;
							NetworkId const & POBShipId = groupObj->getPOBShipOwnedByMember(memberId);

							if ((POBShipId.isValid()) && (groupObj->getPOBShipMemberIsContainedIn(memberId).isValid()))
							{
								GroupMemberParam const & memberWithPOBShip = groupObj->createGroupMemberParamForMember(memberId);
								membersOwningPOBs.push_back(memberWithPOBShip);
							}
							else
							{
								membersNotInPOBs.push_back(memberId);
							}
						}
					}

					// we build and cache this data for after the GroupObject has been destroyed
					std::map<NetworkId /* memberId of POB owner is key */, GroupObject::GroupMemberParamVector> membersInsidePOB;
					std::map<NetworkId /* memberId of POB owner is key */, GroupObject::GroupMemberParamVector> membersOutsidePOB;

					{
						// create the new POB ship group data collections for those that need it
						GroupObject::GroupMemberParamVector::const_iterator ii = membersOwningPOBs.begin();
						GroupObject::GroupMemberParamVector::const_iterator iiEnd = membersOwningPOBs.end();

						for (; ii != iiEnd; ++ii)
						{
							NetworkId const & memberId = (*ii).m_memberId;
							NetworkId const & POBShipId = (*ii).m_memberShipId;

							GroupHelpers::separateGroupBasedOffofPOBShip(groupObj,
								memberId,
								POBShipId,
								membersInsidePOB[memberId],
								membersOutsidePOB[memberId]);
						}
					}

					if (membersOwningPOBs.empty())
					{
						groupObj->disbandGroup();

						if (actorObj->getGroup() == groupObj)
							actorObj->getGroup()->onGroupMemberRemoved(actorObj->getNetworkId(), true);
					}
					else
					{
						// manually remove everyone from the actors group.  this helps us avoid
						// calling disband and then creteGroup with the actor being set up as
						// the leader.  Calling disband and create with each group would cause
						// the actions to be executed out of order on multiserver due to creates
						// message being processed before the controller's remove and disband
						// messages were processed
						{
							std::set<NetworkId> membersNotInActorsGroup;

							{
								// gather all of the members that are not inside the current
								// group leader's ship

								GroupObject::GroupMemberParamVector::const_iterator ii = membersOwningPOBs.begin();
								GroupObject::GroupMemberParamVector::const_iterator iiEnd = membersOwningPOBs.end();

								for (; ii != iiEnd; ++ii)
								{
									GroupMemberParam const & owner = *ii;

									if (owner.m_memberId != actor)
									{
										GroupObject::GroupMemberParamVector const & membersOutsideActorPOB = membersOutsidePOB[owner.m_memberId];

										GroupObject::GroupMemberParamVector::const_iterator jj = membersOutsideActorPOB.begin();
										GroupObject::GroupMemberParamVector::const_iterator jjEnd = membersOutsideActorPOB.end();

										for (; jj != jjEnd; ++jj)
										{
											membersNotInActorsGroup.insert((*jj).m_memberId);
										}
									}
								}

								std::vector<NetworkId>::const_iterator jj = membersNotInPOBs.begin();
								std::vector<NetworkId>::const_iterator jjEnd = membersNotInPOBs.end();

								// we need to make sure that we also add in other non POB ships to be removed

								for (; jj != jjEnd; ++jj)
								{
									if (*jj != actor)
									{
										membersNotInActorsGroup.insert(*jj);
									}
								}
							}

							// remove everyone that is is not in the original leader's POB from the
							// current group
							std::set<NetworkId>::const_iterator ii = membersNotInActorsGroup.begin();
							std::set<NetworkId>::const_iterator iiEnd = membersNotInActorsGroup.end();

							for (; ii != iiEnd; ++ii)
							{
								NetworkId const & memberId = *ii;
								groupObj->removeGroupMember(memberId);

								CreatureObject * const memberCreatureObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(memberId));
								if (memberCreatureObject != 0)
								{
									if (memberCreatureObject->getGroup() == groupObj)
										memberCreatureObject->getGroup()->onGroupMemberRemoved(memberCreatureObject->getNetworkId(), false);
								}
							}
						}

						// now create the POB groups that we just built the data sets for
						{
							// create the new POB ship groups for those that need it
							GroupObject::GroupMemberParamVector::const_iterator ii = membersOwningPOBs.begin();
							GroupObject::GroupMemberParamVector::const_iterator iiEnd = membersOwningPOBs.end();

							for (; ii != iiEnd; ++ii)
							{
								GroupMemberParam const & leader = *ii;

								// create the new POB groups.  notice nullptr is passed in for the
								// groupToRemoveFrom because the original group has already had
								// all of the members removed

								if (leader.m_memberId != actor)
								{
									GroupHelpers::removeFromGroupAndCreatePOBGroup(0, leader, membersInsidePOB[leader.m_memberId]);
								}
							}
						}
					}
				}

				// we are the group leader. a target is specified, so we remove them
				else if (groupObj->isGroupMember(target))
				{
					// send message to target to leave the group
					MessageToQueue::getInstance().sendMessageToC(target,
						"C++LeaveGroupReq",
						groupObj->getNetworkId().getValueString(),
						0,
						false);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncKickFromShip(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const &params)
{
	CreatureObject const * const actorObject = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObject != 0)
	{
		CreatureObject * const targetObject = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(target));
		if ((targetObject != 0) && (actorObject != targetObject))
		{
			ShipObject const * const actorShipObject = ShipObject::getContainingShipObject(actorObject);

			if ((actorShipObject != 0) && (actorShipObject->getOwnerId() == actorObject->getNetworkId()))
			{
				// check that both are in the same ship
				if (ShipObject::getContainingShipObject(targetObject) == actorShipObject)
				{
					triggerSpaceEjectPlayerFromShip(targetObject);
				}
			}
			else
			{
				sendProseMessage(*actorObject, 0, GroupStringId::SID_GROUP_NO_KICK_FROM_SHIP_OWNER);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncGroupChat(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		GroupObject *groupObj = actorObj->getGroup();
		if (groupObj)
		{
			// don't allow if the actor is squelched
			PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
			if (!playerObj || (playerObj->getSecondsUntilUnsquelched() != 0))
			{
				return;
			}

			groupObj->sendGroupChat(actor, params);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncAuctionChat(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		std::string firstName;
		size_t pos = 0;
		Unicode::getFirstToken(Unicode::wideToNarrow(actorObj->getObjectName()), 0, pos, firstName);

		const std::string channelName = "SWG." + GameServer::getInstance().getClusterName() + "." + ServerWorld::getSceneId() + ".named.Auction";
		Chat::sendToRoom(firstName, channelName, params, Unicode::String());
	}
}

// ----------------------------------------------------------------------

static void commandFuncPlanetChat(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		std::string firstName;
		size_t pos = 0;
		Unicode::getFirstToken(Unicode::wideToNarrow(actorObj->getObjectName()), 0, pos, firstName);

		const std::string channelName = "SWG." + GameServer::getInstance().getClusterName() + "." + ServerWorld::getSceneId() + ".named.Planet";
		Chat::sendToRoom(firstName, channelName, params, Unicode::String());
	}
}

// ----------------------------------------------------------------------

static void commandFuncGroupMakeLeader(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	if (actor == target)
		return;

	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	if (!target.isValid())
		return;

	GroupObject * const groupObj = actorObj->getGroup();
	if (groupObj)
	{
		if (groupObj->getGroupLeaderId() != actor)
		{
			sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_MUST_BE_LEADER);
			return;
		}

		groupObj->makeLeader(target);
	}
}

// ----------------------------------------------------------------------

static void commandFuncGroupMakeMasterLooter(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	GroupObject * const groupObj = actorObj->getGroup();
	if (groupObj != 0)
	{
		if (groupObj->getGroupLeaderId() != actor)
		{
			sendProseMessage(*actorObj, 0, GroupStringId::SID_GROUP_MUST_BE_LEADER);
			return;
		}

		groupObj->makeLootMaster(target.isValid() ? target : actor);
	}
}

// ----------------------------------------------------------------------

static void commandFuncCreateGroupPickup(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
	if (!playerObj)
		return;

	Client * const clientObj = actorObj->getClient();
	if (!clientObj)
		return;

	GroupObject * const groupObj = actorObj->getGroup();
	if (!groupObj || ((groupObj->getGroupLeaderId() != actor) && (playerObj->getSkillTemplate().find("officer_") != 0)))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "create_group_pickup_not_group_leader"));
		return;
	}

	Vector const currentWorldLocation = actorObj->findPosition_w();
	std::string const currentScene = ServerWorld::getSceneId();
	if (!GroupPickupPoint::isGroupPickupAllowedAtLocation(currentScene, static_cast<int>(currentWorldLocation.x), static_cast<int>(currentWorldLocation.z)))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "create_group_pickup_unsupported_location"));
		return;
	}

	if (groupObj->getSecondsLeftOnGroupPickup())
	{
		sendProseMessage(*actorObj, 0, StringId("group", "create_group_pickup_existing"));
		return;
	}

	// see if there are any connected player group members within 20m to assist/approve creating the group pickup point
	std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
	GroupObject::GroupMemberVector const & groupMembers = groupObj->getGroupMembers();
	bool anyConnectedGroupMemberInRange = false;

	GroupObject::GroupMemberVector::const_iterator iterGroupMember;
	for (iterGroupMember = groupMembers.begin(); iterGroupMember != groupMembers.end(); ++iterGroupMember)
	{
		if ((iterGroupMember->first != actor) && groupObj->isMemberPC(iterGroupMember->first) && (connectedCharacterLfgData.count(iterGroupMember->first) > 0))
		{
			CreatureObject const * memberObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(iterGroupMember->first));
			if (memberObj)
			{
				Vector const memberWorldLocation = memberObj->findPosition_w();
				if (static_cast<int>(currentWorldLocation.magnitudeBetweenSquared(memberWorldLocation)) <= ConfigServerGame::getGroupPickupPointApprovalRangeSquared())
				{
					anyConnectedGroupMemberInRange = true;
					break;
				}
			}
		}
	}

	// if there are no connected group members within 20m, bail
	if (!anyConnectedGroupMemberInRange)
	{
		ProsePackage prosePackage;
		prosePackage.stringId = StringId("group", "create_group_pickup_no_assistant");
		prosePackage.digitInteger = static_cast<int>(sqrt(static_cast<float>(ConfigServerGame::getGroupPickupPointApprovalRangeSquared())));

		Chat::sendSystemMessage(*actorObj, prosePackage);
		return;
	}

	// tell other group members that the group leader has created the group pickup point;
	// we need to contact all other group members, even if they are not currently connected,
	// so that we can create the group pickup point waypoint for them when they reconnect
	std::string const actorName = Unicode::wideToNarrow(actorObj->getAssignedObjectName());
	for (iterGroupMember = groupMembers.begin(); iterGroupMember != groupMembers.end(); ++iterGroupMember)
	{
		if ((iterGroupMember->first != actor) && groupObj->isMemberPC(iterGroupMember->first))
		{
			char buffer[1024];
			snprintf(buffer, sizeof(buffer) - 1, "%s|%d|%d|%d|%s", currentScene.c_str(), static_cast<int>(currentWorldLocation.x), static_cast<int>(currentWorldLocation.y), static_cast<int>(currentWorldLocation.z), actorName.c_str());
			buffer[sizeof(buffer) - 1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(iterGroupMember->first,
				"C++GroupPickupPointCreated",
				buffer,
				0,
				false);
		}
	}

	// create the group pickup point
	time_t const timeNow = ::time(nullptr);
	groupObj->setGroupPickupTimer(timeNow, timeNow + static_cast<time_t>(ConfigServerGame::getGroupPickupPointTimeLimitSeconds()));
	groupObj->setGroupPickupLocation(currentScene, currentWorldLocation);

	// tell group leader that the group pickup point has been created
	StringId::LocUnicodeString response;
	if (StringId("group", "create_group_pickup_success_leader").localize(response))
	{
		ConsoleMgr::broadcastString(FormattedString<2048>().sprintf(Unicode::wideToNarrow(response).c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigServerGame::getGroupPickupPointTimeLimitSeconds())).c_str()),
			clientObj);
	}

	// create/update the group leader's group pickup point waypoint
	Location const location(currentWorldLocation, NetworkId::cms_invalid, Location::getCrcBySceneName(currentScene));
	playerObj->createOrUpdateReusableWaypoint(location, "groupPickupWp", Unicode::narrowToWide("Group Pickup Point"), Waypoint::White);
}

// ----------------------------------------------------------------------

static void commandFuncUseGroupPickup(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject * const actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (!actorObj)
		return;

	PlayerObject const * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
	if (!playerObj)
		return;

	Client const * const clientObj = actorObj->getClient();
	if (!clientObj)
		return;

	GameScriptObject * const gameScriptObject = actorObj->getScriptObject();
	if (!gameScriptObject)
		return;

	bool starPortAndShuttleportOnly = false;
	static Unicode::String const noCampParam = Unicode::narrowToWide("nocamp");
	if (Unicode::caseInsensitiveCompare(params, noCampParam))
		starPortAndShuttleportOnly = true;

	GroupObject * const groupObj = actorObj->getGroup();
	if (!groupObj || (groupObj->getSecondsLeftOnGroupPickup() == 0))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_none_active"));
		return;
	}

	Vector const currentWorldLocation = actorObj->findPosition_w();
	std::string const currentScene = ServerWorld::getSceneId();
	if (!GroupPickupPoint::isGroupPickupAllowedAtLocation(currentScene, static_cast<int>(currentWorldLocation.x), static_cast<int>(currentWorldLocation.z)))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_unsupported_location"));
		return;
	}

	if (actorObj->getState(States::Combat))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_in_combat"));
		return;
	}

	if (actorObj->isIncapacitated() || actorObj->isDead())
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_dead_or_incap"));
		return;
	}

	if (actorObj->getMountedCreature())
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_on_mount"));
		return;
	}

	// trigger script to make sure script is ok with the player traveling to the group pickup point
	ScriptParams sp1;
	if (gameScriptObject->trigAllScripts(Scripting::TRIG_ABOUT_TO_TRAVEL_TO_GROUP_PICKUP_POINT, sp1) != SCRIPT_CONTINUE)
		return;

	// if group pickup point is in mustafar or any of the kashyyyk zones,
	// check to make sure player has the required feature bits to go there
	std::pair<std::string, Vector> const & groupPickupLocation = groupObj->getGroupPickupLocation();
	uint32 const gameFeatureBits = clientObj->getGameFeatures();

	if (groupPickupLocation.first.find("kashyyyk") == 0)
	{
		if (!((gameFeatureBits & ClientGameFeature::Episode3PreorderDownload) || (gameFeatureBits & ClientGameFeature::Episode3ExpansionRetail)))
		{
			sendProseMessage(*actorObj, 0, StringId("travel", "kashyyyk_unauthorized"));
			return;
		}
	}
	else if (groupPickupLocation.first == "mustafar")
	{
		if (!((gameFeatureBits & ClientGameFeature::TrialsOfObiwanPreorder) || (gameFeatureBits & ClientGameFeature::TrialsOfObiwanRetail)))
		{
			sendProseMessage(*actorObj, 0, StringId("travel", "mustafar_unauthorized"));
			return;
		}
	}

	// determine cost, and make sure player has enough credits
	int totalCost;
	if (!GroupPickupPoint::getGroupPickupTravelCost(currentScene, groupPickupLocation.first, totalCost) || (totalCost <= 0))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_internal_error_cannot_determine_cost"));
		return;
	}

	int const bankBalance = actorObj->getBankBalance();
	int const cashBalance = actorObj->getCashBalance();

	if ((bankBalance + cashBalance) < totalCost)
	{
		ProsePackage prosePackage;
		prosePackage.stringId = StringId("group", "use_group_pickup_not_enough_credits");
		prosePackage.digitInteger = totalCost;

		Chat::sendSystemMessage(*actorObj, prosePackage);
		return;
	}

	// find the travel point (starport, shuttleport, camp with shuttle beacon)
	// nearest to the group pickup point

	// get the list of cities the player is currently banned from
	std::vector<int> cityBanList;
	DynamicVariableList const & objvars = actorObj->getObjVars();
	if (objvars.hasItem("city.banlist") && (objvars.getType("city.banlist") == DynamicVariable::INT_ARRAY))
		IGNORE_RETURN(objvars.getItem("city.banlist", cityBanList));

	std::string sceneIdOfNearestTravelPoint = groupPickupLocation.first;
	TravelPoint const * nearestTravelPoint = GroupHelpers::getNearestTravelPoint(groupPickupLocation.first, groupPickupLocation.second, cityBanList, actorObj->getPvpFaction(), starPortAndShuttleportOnly);

	// if no nearest travel point found, and the pickup point is inside one
	// of the kashyyyk zones that do not have a starport, use kashyyyk_main
	// as the pickup point planet and find the nearest travel point there,
	// and that should return at least the kachirho starport
	// *****OR*****
	// if nearest travel point is found inside any kashyyyk zone other than kashyyyk_main
	// and the player is not already in that respective zone, don't allow them to travel
	// there as entrance to those zones requires going through kashyyyk_main first, so
	// find the nearest travel point in kashyyyk_main and use that as the travel point
	bool findNearestTravelPointOnKashyyykMain = false;
	if (!nearestTravelPoint && (groupPickupLocation.first.find("kashyyyk") == 0) && (groupPickupLocation.first != "kashyyyk_main"))
	{
		// if already on the kashyyyk zone where the group pickup point is, player is
		// already closest to the group pickup point than any travel point on kashyyyk_main
		if (currentScene == groupPickupLocation.first)
		{
			sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_closest_to_group_pickup_point"));
			return;
		}

		findNearestTravelPointOnKashyyykMain = true;
	}
	else if (nearestTravelPoint && (groupPickupLocation.first.find("kashyyyk") == 0) && (groupPickupLocation.first != "kashyyyk_main") && (currentScene != groupPickupLocation.first))
	{
		findNearestTravelPointOnKashyyykMain = true;
	}

	if (findNearestTravelPointOnKashyyykMain)
	{
		sceneIdOfNearestTravelPoint = "kashyyyk_main";

		if (groupPickupLocation.first == "kashyyyk_hunting")
		{
			// we want to find the travel point on kashyyyk_main nearest to the entrance to kashyyyk_hunting which is at (190.0f, 20.0f, -430.0f)
			nearestTravelPoint = GroupHelpers::getNearestTravelPoint(sceneIdOfNearestTravelPoint, Vector(190.0f, 20.0f, -430.0f), cityBanList, actorObj->getPvpFaction(), starPortAndShuttleportOnly);
		}
		else if (groupPickupLocation.first == "kashyyyk_dead_forest")
		{
			// we want to find the travel point on kashyyyk_main nearest to the entrance to kashyyyk_dead_forest which is at (-745.0f, 18.0f, 256.0f)
			nearestTravelPoint = GroupHelpers::getNearestTravelPoint(sceneIdOfNearestTravelPoint, Vector(-745.0f, 18.0f, 256.0f), cityBanList, actorObj->getPvpFaction(), starPortAndShuttleportOnly);
		}
		else
		{
			// (-672.0f, 19.0f, -157.0f) is the kachirho starport
			nearestTravelPoint = GroupHelpers::getNearestTravelPoint(sceneIdOfNearestTravelPoint, Vector(-672.0f, 19.0f, -157.0f), cityBanList, actorObj->getPvpFaction(), starPortAndShuttleportOnly);
		}
	}

	if (!nearestTravelPoint)
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_internal_error_cannot_find_travelpoint"));
		return;
	}

	// if nearest travel point and current planet is the same, and the nearest travel point to the
	// group pickup point is farther away than the current location to the group pickup point, then
	// fail, as there is no need to travel as the player is currently closer to the group pickup point
	if ((sceneIdOfNearestTravelPoint == currentScene) && (sceneIdOfNearestTravelPoint == groupPickupLocation.first) && (groupPickupLocation.second.magnitudeBetweenSquared(nearestTravelPoint->getPosition_w()) > groupPickupLocation.second.magnitudeBetweenSquared(currentWorldLocation)))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_closest_to_group_pickup_point"));
		return;
	}

	// if the nearest travel point is a shuttleport in a player city, the city gets a share of the travel cost
	int cityShareOfCost = 0;
	int systemShareOfCost = totalCost;
	int const cityIdAtNearestTravelPoint = CityInterface::getCityAtLocation(sceneIdOfNearestTravelPoint, static_cast<int>(nearestTravelPoint->getPosition_w().x), static_cast<int>(nearestTravelPoint->getPosition_w().z), 0);
	NetworkId cityHallAtNearestTravelPoint;
	std::string cityNameAtNearestTravelPoint;
	if (cityIdAtNearestTravelPoint > 0)
	{
		CityInfo const & ci = CityInterface::getCityInfo(cityIdAtNearestTravelPoint);
		cityHallAtNearestTravelPoint = ci.getCityHallId();
		if (cityHallAtNearestTravelPoint.isValid())
		{
			cityNameAtNearestTravelPoint = ci.getCityName();
			cityShareOfCost = (totalCost * ConfigServerGame::getGroupPickupTravelPlayerCityPercent()) / 100;
			systemShareOfCost = totalCost - cityShareOfCost;
		}
	}

	// if player doesn't have enough money in bank, transfer from cash into back to make up the difference
	if (bankBalance < totalCost)
	{
		if (!actorObj->depositCashToBank(totalCost - bankBalance))
		{
			sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_internal_error_cannot_move_cash_to_bank"));
			return;
		}
	}

	// transfer the total travel cost from the player
	if (!actorObj->transferBankCreditsTo("GroupPickupPointTravel", totalCost))
	{
		sendProseMessage(*actorObj, 0, StringId("group", "use_group_pickup_internal_error_cannot_transfer_payment_from_bank"));
		return;
	}

	// tell player his credits have been deducted
	ProsePackage prosePackage;
	prosePackage.stringId = StringId("group", "use_group_pickup_credits_deducted");
	prosePackage.digitInteger = totalCost;

	Chat::sendSystemMessage(*actorObj, prosePackage);

	// if the city gets a share of the travel cost, send message to the city hall to transfer the city's share
	if (cityHallAtNearestTravelPoint.isValid() && (cityShareOfCost > 0))
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer) - 1, "%d", cityShareOfCost);
		buffer[sizeof(buffer) - 1] = '\0';

		MessageToQueue::getInstance().sendMessageToC(cityHallAtNearestTravelPoint,
			"C++CityShareGroupPickupPointTravelCost",
			buffer,
			0,
			false);
	}

	// log the transaction
	if (cityHallAtNearestTravelPoint.isValid())
		LOG("CustomerService", ("GroupPickupPointTravel:%s in group %s traveled from %s to %s for a cost of %d (%d to system, %d to %s)", playerObj->getAccountDescription().c_str(), groupObj->getNetworkId().getValueString().c_str(), currentScene.c_str(), groupPickupLocation.first.c_str(), totalCost, systemShareOfCost, cityShareOfCost, cityNameAtNearestTravelPoint.c_str()));
	else
		LOG("CustomerService", ("GroupPickupPointTravel:%s in group %s traveled from %s to %s for a cost of %d", playerObj->getAccountDescription().c_str(), groupObj->getNetworkId().getValueString().c_str(), currentScene.c_str(), groupPickupLocation.first.c_str(), totalCost));

	// trigger script to teleport player to the destination travel location;
	// script has various "cleanup/setup" it needs to be when the player travels
	ScriptParams sp2;
	sp2.addParam(sceneIdOfNearestTravelPoint.c_str());
	sp2.addParam(nearestTravelPoint->getName().c_str());

	IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_TRAVEL_TO_GROUP_PICKUP_POINT, sp2));
}

// ----------------------------------------------------------------------

static void commandFuncGroupPickRandomGroupMember(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	GroupObject const * const group = targetObj->getGroup();
	if (!group)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently in a group."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextGroupPickRandomGroupMemberCommandAllowed") && (targetObj->getObjVars().getType("timeNextGroupPickRandomGroupMemberCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextGroupPickRandomGroupMemberCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextGroupPickRandomGroupMemberCommandAllowed", timeNextGroupPickRandomGroupMemberCommandAllowed))
		{
			if (timeNextGroupPickRandomGroupMemberCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextGroupPickRandomGroupMemberCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	// handle options
	static uint32 const pointatSocialType = SocialsManager::getSocialTypeByName("pointat");
	uint32 socialType = 0;

	if (!params.empty())
	{
		Unicode::UnicodeStringVector tokens;
		if (Unicode::tokenize(Unicode::toLower(params), tokens, nullptr, nullptr) && !tokens.empty())
		{
			static Unicode::String const paramsPoint1 = Unicode::narrowToWide("-point");
			static Unicode::String const paramsPoint2 = Unicode::narrowToWide("point");
			static Unicode::String const paramsEmote1 = Unicode::narrowToWide("-emote=");
			static Unicode::String const paramsEmote2 = Unicode::narrowToWide("emote=");
			std::string emote;

			for (Unicode::UnicodeStringVector::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
			{
				if ((*iter == paramsPoint1) || (*iter == paramsPoint2))
				{
					socialType = pointatSocialType;
				}
				else if (iter->find(paramsEmote1) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(7);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else if (iter->find(paramsEmote2) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(6);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	GroupObject::GroupMemberVector const & results = group->getGroupMembers();
	if (results.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No group member could be found."), Unicode::emptyString);
	}
	else
	{
		GroupObject::GroupMemberVector pcMembers;
		for (GroupObject::GroupMemberVector::const_iterator iter = results.begin(); iter != results.end(); ++iter)
		{
			if (group->isMemberPC(iter->first))
			{
				pcMembers.push_back(*iter);
			}
		}

		if (pcMembers.empty())
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No group member could be found."), Unicode::emptyString);
		}
		else
		{
			int const random = ::rand() % pcMembers.size();
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("Group member \"%s\" was randomly chosen.", pcMembers[random].second.c_str())), Unicode::emptyString);

			// do emote at the selected player, if requested
			if (socialType > 0)
			{
				// if I'm the one selected, use the /pointat social to point at myself
				// since other socials may not make sense when I am the target
				doSocial(*targetObj, pcMembers[random].first, ((pcMembers[random].first == targetObj->getNetworkId()) ? pointatSocialType : socialType));
			}
		}
	}

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextGroupPickRandomGroupMemberCommandAllowed", (timeNow + 30)));
}

// ----------------------------------------------------------------------

static void commandFuncGroupTextChatRoomRejoin(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	GroupObject const * const group = targetObj->getGroup();
	if (!group)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently in a group."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextGroupTextChatRoomRejoinCommandAllowed") && (targetObj->getObjVars().getType("timeNextGroupTextChatRoomRejoinCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextGroupTextChatRoomRejoinCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextGroupTextChatRoomRejoinCommandAllowed", timeNextGroupTextChatRoomRejoinCommandAllowed))
		{
			if (timeNextGroupTextChatRoomRejoinCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextGroupTextChatRoomRejoinCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Attempting to leave and rejoin the group text chat room."), Unicode::emptyString);
	group->reenterGroupChatRoom(*targetObj);

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextGroupTextChatRoomRejoinCommandAllowed", (timeNow + 60)));
}

// ----------------------------------------------------------------------

static void commandFuncGuildChat(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		// don't allow if the actor is squelched
		PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
		if (!playerObj || (playerObj->getSecondsUntilUnsquelched() != 0))
		{
			return;
		}

		GuildInterface::sendGuildChat(*actorObj, params);
	}
}

// ----------------------------------------------------------------------

static void commandFuncGuildTextChatRoomRejoin(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	if (targetObj->getGuildId() == 0)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently in a guild."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextGuildTextChatRoomRejoinCommandAllowed") && (targetObj->getObjVars().getType("timeNextGuildTextChatRoomRejoinCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextGuildTextChatRoomRejoinCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextGuildTextChatRoomRejoinCommandAllowed", timeNextGuildTextChatRoomRejoinCommandAllowed))
		{
			if (timeNextGuildTextChatRoomRejoinCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextGuildTextChatRoomRejoinCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Attempting to leave and rejoin the guild text chat room."), Unicode::emptyString);
	GuildInterface::reenterGuildChatRoom(*targetObj);

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextGuildTextChatRoomRejoinCommandAllowed", (timeNow + 60)));
}

// ----------------------------------------------------------------------

static void commandFuncGuildPickRandomGuildMember(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	if (targetObj->getGuildId() == 0)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently in a guild."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextGuildPickRandomGuildMemberCommandAllowed") && (targetObj->getObjVars().getType("timeNextGuildPickRandomGuildMemberCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextGuildPickRandomGuildMemberCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextGuildPickRandomGuildMemberCommandAllowed", timeNextGuildPickRandomGuildMemberCommandAllowed))
		{
			if (timeNextGuildPickRandomGuildMemberCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextGuildPickRandomGuildMemberCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	// handle options
	static uint32 const pointatSocialType = SocialsManager::getSocialTypeByName("pointat");
	uint32 socialType = 0;
	bool onlineOption = false;

	if (!params.empty())
	{
		Unicode::UnicodeStringVector tokens;
		if (Unicode::tokenize(Unicode::toLower(params), tokens, nullptr, nullptr) && !tokens.empty())
		{
			static Unicode::String const paramsOnline1 = Unicode::narrowToWide("-online");
			static Unicode::String const paramsOnline2 = Unicode::narrowToWide("online");
			static Unicode::String const paramsPoint1 = Unicode::narrowToWide("-point");
			static Unicode::String const paramsPoint2 = Unicode::narrowToWide("point");
			static Unicode::String const paramsEmote1 = Unicode::narrowToWide("-emote=");
			static Unicode::String const paramsEmote2 = Unicode::narrowToWide("emote=");
			std::string emote;

			for (Unicode::UnicodeStringVector::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
			{
				if ((*iter == paramsOnline1) || (*iter == paramsOnline2))
				{
					onlineOption = true;
				}
				else if ((*iter == paramsPoint1) || (*iter == paramsPoint2))
				{
					socialType = pointatSocialType;
				}
				else if (iter->find(paramsEmote1) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(7);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else if (iter->find(paramsEmote2) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(6);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	std::vector<NetworkId> results;
	GuildInterface::getGuildMemberIdsWithPermissions(targetObj->getGuildId(), GuildInterface::Member, results);
	if (results.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No guild member could be found."), Unicode::emptyString);
	}
	else
	{
		std::vector<NetworkId> resultsOnline;
		if (onlineOption)
		{
			std::map<NetworkId, LfgCharacterData> const & connectedPlayers = ServerUniverse::getConnectedCharacterLfgData();
			for (std::vector<NetworkId>::const_iterator iter = results.begin(); iter != results.end(); ++iter)
			{
				if ((*iter == targetObj->getNetworkId()) || (connectedPlayers.count(*iter) > 0))
					resultsOnline.push_back(*iter);
			}

			if (resultsOnline.empty())
				resultsOnline.push_back(targetObj->getNetworkId());
		}

		NetworkId const randomMember = (onlineOption ? resultsOnline[::rand() % resultsOnline.size()] : results[::rand() % results.size()]);
		GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(targetObj->getGuildId(), randomMember);
		if (!gmi)
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No guild member could be found."), Unicode::emptyString);
		}
		else
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("Guild member \"%s\" was randomly chosen out of %d guild members in guild %s [%s].", gmi->m_name.c_str(), results.size(), GuildInterface::getGuildName(targetObj->getGuildId()).c_str(), GuildInterface::getGuildAbbrev(targetObj->getGuildId()).c_str())), Unicode::emptyString);

			// reminder of the -online option
			if (!onlineOption && (randomMember != targetObj->getNetworkId()) && (ServerUniverse::getConnectedCharacterLfgData().count(randomMember) <= 0))
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Use \"/guildPickRandomGuildMember -online\" to only select online guild members."), Unicode::emptyString);
			}

			// do emote at the selected player, if requested
			if (socialType > 0)
			{
				// if I'm the one selected, use the /pointat social to point at myself
				// since other socials may not make sense when I am the target
				doSocial(*targetObj, randomMember, ((randomMember == targetObj->getNetworkId()) ? pointatSocialType : socialType));
			}
		}
	}

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextGuildPickRandomGuildMemberCommandAllowed", (timeNow + 30)));
}

// ----------------------------------------------------------------------

static void commandFuncCityChat(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actor));
	if (actorObj)
	{
		// don't allow if the actor is squelched
		PlayerObject * const playerObj = PlayerCreatureController::getPlayerObject(actorObj);
		if (!playerObj || (playerObj->getSecondsUntilUnsquelched() != 0))
		{
			return;
		}

		CityInterface::sendCityChat(*actorObj, params);
	}
}

// ----------------------------------------------------------------------

static void commandFuncCityTextChatRoomRejoin(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(targetObj->getNetworkId());
	if (cityIds.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently a citizen of a player city."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextCityTextChatRoomRejoinCommandAllowed") && (targetObj->getObjVars().getType("timeNextCityTextChatRoomRejoinCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextCityTextChatRoomRejoinCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextCityTextChatRoomRejoinCommandAllowed", timeNextCityTextChatRoomRejoinCommandAllowed))
		{
			if (timeNextCityTextChatRoomRejoinCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextCityTextChatRoomRejoinCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Attempting to leave and rejoin the city text chat room."), Unicode::emptyString);
	CityInterface::reenterCityChatRoom(*targetObj);

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextCityTextChatRoomRejoinCommandAllowed", (timeNow + 60)));
}

// ----------------------------------------------------------------------

static void commandFuncCityPickRandomCitizen(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(targetObj->getNetworkId());
	if (cityIds.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You are not currently a citizen of any city."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextCityPickRandomCitizenCommandAllowed") && (targetObj->getObjVars().getType("timeNextCityPickRandomCitizenCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextCityPickRandomCitizenCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextCityPickRandomCitizenCommandAllowed", timeNextCityPickRandomCitizenCommandAllowed))
		{
			if (timeNextCityPickRandomCitizenCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextCityPickRandomCitizenCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	// handle options
	static uint32 const pointatSocialType = SocialsManager::getSocialTypeByName("pointat");
	uint32 socialType = 0;
	bool onlineOption = false;

	if (!params.empty())
	{
		Unicode::UnicodeStringVector tokens;
		if (Unicode::tokenize(Unicode::toLower(params), tokens, nullptr, nullptr) && !tokens.empty())
		{
			static Unicode::String const paramsOnline1 = Unicode::narrowToWide("-online");
			static Unicode::String const paramsOnline2 = Unicode::narrowToWide("online");
			static Unicode::String const paramsPoint1 = Unicode::narrowToWide("-point");
			static Unicode::String const paramsPoint2 = Unicode::narrowToWide("point");
			static Unicode::String const paramsEmote1 = Unicode::narrowToWide("-emote=");
			static Unicode::String const paramsEmote2 = Unicode::narrowToWide("emote=");
			std::string emote;

			for (Unicode::UnicodeStringVector::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
			{
				if ((*iter == paramsOnline1) || (*iter == paramsOnline2))
				{
					onlineOption = true;
				}
				else if ((*iter == paramsPoint1) || (*iter == paramsPoint2))
				{
					socialType = pointatSocialType;
				}
				else if (iter->find(paramsEmote1) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(7);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else if (iter->find(paramsEmote2) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(6);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	std::vector<NetworkId> results;
	CityInterface::getCitizenIds(cityIds[0], results);
	if (results.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No citizen could be found."), Unicode::emptyString);
	}
	else
	{
		std::vector<NetworkId> resultsOnline;
		if (onlineOption)
		{
			std::map<NetworkId, LfgCharacterData> const & connectedPlayers = ServerUniverse::getConnectedCharacterLfgData();
			for (std::vector<NetworkId>::const_iterator iter = results.begin(); iter != results.end(); ++iter)
			{
				if ((*iter == targetObj->getNetworkId()) || (connectedPlayers.count(*iter) > 0))
					resultsOnline.push_back(*iter);
			}

			if (resultsOnline.empty())
				resultsOnline.push_back(targetObj->getNetworkId());
		}

		NetworkId const randomMember = (onlineOption ? resultsOnline[::rand() % resultsOnline.size()] : results[::rand() % results.size()]);
		CitizenInfo const * const ci = CityInterface::getCitizenInfo(cityIds[0], randomMember);
		if (!ci)
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No citizen could be found."), Unicode::emptyString);
		}
		else
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("Citizen \"%s\" was randomly chosen out of %d citizens in city %s.", ci->m_citizenName.c_str(), results.size(), CityInterface::getCityInfo(cityIds[0]).getCityName().c_str())), Unicode::emptyString);

			// reminder of the -online option
			if (!onlineOption && (randomMember != targetObj->getNetworkId()) && (ServerUniverse::getConnectedCharacterLfgData().count(randomMember) <= 0))
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Use \"/cityPickRandomCitizen -online\" to only select online citizens."), Unicode::emptyString);
			}

			// do emote at the selected player, if requested
			if (socialType > 0)
			{
				// if I'm the one selected, use the /pointat social to point at myself
				// since other socials may not make sense when I am the target
				doSocial(*targetObj, randomMember, ((randomMember == targetObj->getNetworkId()) ? pointatSocialType : socialType));
			}
		}
	}

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextCityPickRandomCitizenCommandAllowed", (timeNow + 30)));
}

// ----------------------------------------------------------------------

static void commandFuncShowDanceVisuals(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	bool enable = nextBoolParm(params, pos);
	CreatureObject *creature = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	if (creature)
	{
		IGNORE_RETURN(creature->showDanceVisuals(enable));
	}
}

// ----------------------------------------------------------------------

static void commandFuncShowMusicianVisuals(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	bool enable = nextBoolParm(params, pos);
	CreatureObject *creature = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(actor));
	if (creature)
	{
		IGNORE_RETURN(creature->showMusicianVisuals(enable));
	}
}

// ----------------------------------------------------------------------

static void commandFuncPlaceStructure(const Command& /*command*/, const NetworkId& actor, const NetworkId& /*target*/, const Unicode::String& parameters)
{
	Object* const object = NetworkIdManager::getObjectById(actor);
	if (!object)
	{
		DEBUG_WARNING(true, ("commandFuncPlaceStructure: PB nullptr actor\n"));
		return;
	}

	ServerObject* const serverObject = dynamic_cast<ServerObject*> (object);
	if (!serverObject)
	{
		DEBUG_WARNING(true, ("commandFuncPlaceStructure: object is not a server object\n"));
		return;
	}

	size_t pos = 0;
	const NetworkId deedNetworkId = nextOidParm(parameters, pos);
	const float x = nextFloatParm(parameters, pos);
	const float z = nextFloatParm(parameters, pos);
	const Vector position(x, 0.f, z);
	const int rotation = nextIntParm(parameters, pos);

	ScriptParams scriptParameters;
	scriptParameters.addParam(actor);
	scriptParameters.addParam(deedNetworkId);
	scriptParameters.addParam(position);
	scriptParameters.addParam(rotation);

	if (serverObject->getScriptObject()->trigAllScripts(Scripting::TRIG_PLACE_STRUCTURE, scriptParameters) != SCRIPT_CONTINUE)
		DEBUG_REPORT_LOG(true, ("commandFuncPlaceStructure: did not return SCRIPT_CONTINUE\n"));
}

// ----------------------------------------------------------------------
/**
 * Change the posture of the player to sitting.
 *
 * Broadcast the request to sit on a chair.
 */

static void commandFuncSitServer(const Command& /*command*/, const NetworkId& actorId, const NetworkId& /* targetId */, const Unicode::String& parameters)
{
	// The command handler only runs on the authoratative server for the actor,
	// so it should be fine to directly grab the CreatureController from the actor.

	//-- Get the actor object.
	Object *const actorObject = NetworkIdManager::getObjectById(actorId);
	if (!actorObject)
	{
		DEBUG_WARNING(true, ("commandFuncPostureSit: failed to lookup Object from actorId network id [%s], aborting sit request.\n", actorId.getValueString().c_str()));
		return;
	}

	CreatureObject *const creatureActorObject = CreatureObject::asCreatureObject(actorObject);
	if (!creatureActorObject)
	{
		DEBUG_WARNING(true, ("commandFuncPostureSit:  Object [%s] is not a creature, aborting sit request.\n", actorId.getValueString().c_str()));
		return;
	}

	//-- Do not let a player sit on the ground in water.  (Sitting on a chair in water is okay, but that implies a designer stuck a chair in water).
	bool const  sittingOnChair = !parameters.empty();
	NetworkId   chairCellId;
	Vector      chairPosition_p;

	if (sittingOnChair)
	{
		//-- Pull the chair cell id and position out of the parameters string.
		std::string const narrowParameters = Unicode::wideToNarrow(parameters);

		std::string::size_type const lastCommaIndex = narrowParameters.find_last_of(',');

		std::string const            chairCellIdString(narrowParameters, lastCommaIndex + 1);
		std::string const            chairPosition_pString(narrowParameters, 0, lastCommaIndex);

		//-- Get chair position.  Abort command if cannot scan vector position from string.
		int const scanCount = sscanf(chairPosition_pString.c_str(), "%g,%g,%g", &chairPosition_p.x, &chairPosition_p.y, &chairPosition_p.z);
		if (scanCount != 3)
		{
			DEBUG_WARNING(true, ("commandFuncPostureSit: sitting on chair but chair parameter [%s] was malformed: couldn't be converted to parent x,y,z.", Unicode::wideToNarrow(parameters).c_str()));
			return;
		}

		chairCellId = NetworkId(chairCellIdString);
	}

	if (!sittingOnChair)
	{
		// Trying to sit on the ground.
		const TerrainObject* const     terrainObject = TerrainObject::getConstInstance();
		const CollisionProperty* const collisionProperty = actorObject->getCollisionProperty();
		const bool                     isOnSolidFloor = collisionProperty && collisionProperty->getFootprint() && collisionProperty->getFootprint()->isOnSolidFloor();

		Vector normal = Vector::unitY;
		if (terrainObject && actorObject->isInWorldCell() && !isOnSolidFloor)
		{
			const Vector position = actorObject->getPosition_w();

			float terrainHeight;
			if (terrainObject->getHeight(position, terrainHeight, normal))
			{
				//-- see if the object is swimming
				float waterHeight;
				if (terrainObject->getWaterHeight(position, waterHeight))
				{
					if (waterHeight > terrainHeight)
					{
						// Client is in the water, abort the command.
						Chat::sendSystemMessage(*creatureActorObject, SharedStringIds::no_sitting_in_water, Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	//-- Get the controller.
	Controller *const controller = actorObject->getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("commandFuncPostureSit: object [%s] has no controller, aborting sit request.\n", actorId.getValueString().c_str()));
		return;
	}

	//-- Send the SitOnObject controller message to all clients.
	if (!sittingOnChair)
	{
		//-- Just tell the client to change to the sitting posture immediately.
		const_cast<CreatureObject*>(creatureActorObject)->setPosture(Postures::Sitting, true);
	}
	else
	{
		//-- Send a SitOnObject message with the chair coordinates.
		const_cast<CreatureObject*>(creatureActorObject)->sitOnObject(chairCellId, chairPosition_p);
	}
}

// ----------------------------------------------------------------------

static void commandFuncGetAttributes(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const & params)
{
	size_t curpos = 0;
	int const clientRevision = nextIntParm(params, curpos);
	TaskGetAttributes * const task = new TaskGetAttributes(actor, target, clientRevision);
	NonCriticalTaskQueue::getInstance().addTask(task);
}

// ----------------------------------------------------------------------

static void commandFuncGetAttributesBatch(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	size_t curpos = 0;
	NetworkId obj = nextOidParm(params, curpos);
	while (obj != NetworkId::cms_invalid)
	{
		int const clientRevision = nextIntParm(params, curpos);
		TaskGetAttributes * const task = new TaskGetAttributes(actor, obj, clientRevision);
		NonCriticalTaskQueue::getInstance().addTask(task);
		obj = nextOidParm(params, curpos);
	}
}

// ----------------------------------------------------------------------

static std::string underscoreToSpace(const std::string& source)
{
	std::string result = source;
	std::replace(result.begin(), result.end(), '_', ' ');

	return result;
}

// ----------------------------------------------------------------------

static void commandFuncPurchaseTicket(const Command& /*command*/, const NetworkId& actor, const NetworkId& /*target*/, const Unicode::String& parameters)
{
	ServerObject* const serverObject = ServerObject::getServerObject(actor);
	if (!serverObject)
	{
		DEBUG_WARNING(true, ("commandFuncPurchaseTicket: object is not a server object"));
		return;
	}

	size_t pos = 0;
	const Unicode::String planetName1 = Unicode::narrowToWide(nextStringParm(parameters, pos));
	const Unicode::String travelPoint1 = Unicode::narrowToWide(underscoreToSpace(nextStringParm(parameters, pos)));
	const Unicode::String planetName2 = Unicode::narrowToWide(nextStringParm(parameters, pos));
	const Unicode::String travelPoint2 = Unicode::narrowToWide(underscoreToSpace(nextStringParm(parameters, pos)));
	const bool roundTrip = nextBoolParm(parameters, pos);
	const bool instantTravel = nextBoolParm(parameters, pos);

	ScriptParams scriptParameters;
	scriptParameters.addParam(actor);
	scriptParameters.addParam(planetName1);
	scriptParameters.addParam(travelPoint1);
	scriptParameters.addParam(planetName2);
	scriptParameters.addParam(travelPoint2);
	scriptParameters.addParam(roundTrip);

	Scripting::TrigId id = instantTravel ? Scripting::TRIG_PURCHASE_TICKET_INSTANT_TRAVEL : Scripting::TRIG_PURCHASE_TICKET;

	if (serverObject->getScriptObject()->trigAllScripts(id, scriptParameters) != SCRIPT_CONTINUE)
		DEBUG_REPORT_LOG(true, ("commandFuncPurchaseTicket: did not return SCRIPT_CONTINUE\n"));
}

//----------------------------------------------------------------------

static void commandFuncRequestResourceWeights(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestResourceWeights: PB nullptr actor"));
		return;
	}

	uint32 schematicCrc;
	sscanf(Unicode::wideToNarrow(params).c_str(), "%lu", &schematicCrc);
	DraftSchematicObject::requestResourceWeights(*creature, schematicCrc);
}

//----------------------------------------------------------------------

static void commandFuncRequestResourceWeightsBatch(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestResourceWeightsBatch: PB nullptr actor"));
		return;
	}

	int schematicCrc;

	size_t curpos = 0;
	schematicCrc = nextIntParm(params, curpos);
	while (schematicCrc != -1)
	{
		DraftSchematicObject::requestResourceWeights(*creature, static_cast<uint32>(schematicCrc));
		schematicCrc = nextIntParm(params, curpos);
	}
}

//----------------------------------------------------------------------

static void commandFuncRequestDraftSlots(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestDraftSlots: PB nullptr actor"));
		return;
	}

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncRequestDraftSlots: no player object for actor ""%s", actor.getValueString().c_str()));
		return;
	}

	uint32 serverCrc, sharedCrc;
	sscanf(Unicode::wideToNarrow(params).c_str(), "%lu %lu", &serverCrc, &sharedCrc);

	MessageQueueDraftSlotsQueryResponse * const message = new MessageQueueDraftSlotsQueryResponse(std::make_pair(serverCrc, sharedCrc));
	if (!player->requestDraftSlots(serverCrc, nullptr, message))
	{
		WARNING(true, ("commandFuncRequestDraftSlots failed to request draft slots for %u", serverCrc));
		delete message;
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestDraftSlotsBatch(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestDraftSlotsBatch: PB nullptr actor"));
		return;
	}

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncRequestDraftSlotsBatch: no player object for actor ""%s", actor.getValueString().c_str()));
		return;
	}

	uint32 uServerCrc = 0;
	uint32 uSharedCrc = 0;
	size_t curpos = 0;

	std::string serverCrcString = nextStringParm(params, curpos);
	if (serverCrcString.empty())
		return;
	sscanf(serverCrcString.c_str(), "%lu", &uServerCrc);
	std::string sharedCrcString = nextStringParm(params, curpos);
	if (sharedCrcString.empty())
		return;
	sscanf(sharedCrcString.c_str(), "%lu", &uSharedCrc);

	bool done = false;
	while (uServerCrc != 0 && uSharedCrc != 0 && !done)
	{
		MessageQueueDraftSlotsQueryResponse * const message = new MessageQueueDraftSlotsQueryResponse(std::make_pair(uServerCrc, uSharedCrc));
		if (!player->requestDraftSlots(uServerCrc, nullptr, message))
		{
			WARNING(true, ("commandFuncRequestDraftSlotsBatch failed to request draft slots for %lu", uServerCrc));
			delete message;
		}
		serverCrcString = nextStringParm(params, curpos);
		if (serverCrcString.empty())
			done = true;
		sscanf(serverCrcString.c_str(), "%lu", &uServerCrc);
		sharedCrcString = nextStringParm(params, curpos);
		if (sharedCrcString.empty())
			done = true;
		sscanf(sharedCrcString.c_str(), "%lu", &uSharedCrc);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestManfSchematicSlots(const Command&, const NetworkId& actor, const NetworkId& target, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestDraftSlots: PB nullptr actor"));
		return;
	}

	const ManufactureSchematicObject * schematic = dynamic_cast<
		const ManufactureSchematicObject *>(NetworkIdManager::getObjectById(target));
	if (schematic != nullptr)
	{
		schematic->requestSlots(*creature);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestCraftingSession(const Command&, const NetworkId& actor, const NetworkId& target, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRequestCraftingSession: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncRequestCraftingSession: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	if (!player->requestCraftingSession(target))
	{
		// get the station we are attempting to use.
		TangibleObject const * const craftingObject = TangibleObject::getTangibleObject(target);

		// 1 means this request was made on a crafting station versus a crafting tool.
		uint8 const sequenceId = static_cast<uint8>(craftingObject && craftingObject->isCraftingStation() ? 1 : 0);

		MessageQueueGenericIntResponse * response = new MessageQueueGenericIntResponse(
			CM_requestCraftingSession, false, sequenceId);
		creature->getController()->appendMessage(CM_craftingResult, 0.0f, response,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestCraftingSessionFail(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject* const creatureActor = CreatureObject::getCreatureObject(actor);
	if (creatureActor)
	{
		creatureActor->getController()->appendMessage(
			CM_craftingResult,
			0.0f,
			new MessageQueueGenericIntResponse(CM_requestCraftingSession, false, 0),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

// ----------------------------------------------------------------------

static void commandFuncSelectDraftSchematic(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncSelectDraftSchematic: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncSelectDraftSchematic: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	int schematicIndex = atoi(Unicode::wideToNarrow(params).c_str());

	player->selectDraftSchematic(schematicIndex);
}

// ----------------------------------------------------------------------

static void commandFuncNextCraftingStage(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncNextCraftingStage: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncNextCraftingStage: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	uint8 sequenceId = static_cast<uint8>(atoi(Unicode::wideToNarrow(params).c_str()));
	int result = player->goToNextCraftingStage();

	MessageQueueGenericIntResponse * response = new MessageQueueGenericIntResponse(
		CM_nextCraftingStage, result, sequenceId);
	creature->getController()->appendMessage(CM_nextCraftingStageResult, 0.0f, response,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

static void commandFuncCreatePrototype(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncCreatePrototype: PB nullptr actor"));
		return;
	}

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncCreatePrototype: no player object for actor %s", actor.getValueString().c_str()));
		return;
	}

	size_t pos = 0;
	const uint8 sequenceId = static_cast<uint8>(nextIntParm(params, pos));
	bool realPrototype = nextBoolParm(params, pos);

	const bool result = player->createPrototype(realPrototype);

	MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(CM_createPrototype, result, sequenceId);
	creature->getController()->appendMessage(CM_craftingResult, 0.0f, response,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);

	player->stopCrafting(result);
}

// ----------------------------------------------------------------------

static void commandFuncCreateManfSchematic(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncCreateManfSchematic: PB nullptr actor"));
		return;
	}

	PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncCreateManfSchematic: no player object for actor %s", actor.getValueString().c_str()));
		return;
	}

	const uint8 sequenceId = static_cast<uint8>(atoi(Unicode::wideToNarrow(params).c_str()));

	const bool result = player->createManufacturingSchematic();

	MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(CM_createManfSchematic, result, sequenceId);
	creature->getController()->appendMessage(CM_craftingResult, 0.0f, response,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);

	player->stopCrafting(result);
}

// ----------------------------------------------------------------------

static void commandFuncCancelCraftingSession(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncCancelCraftingSession: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncCancelCraftingSession: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	player->stopCrafting(true);
}

// ----------------------------------------------------------------------

static void commandFuncStopCraftingSession(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncStopCraftingSession: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncStopCraftingSession: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	player->stopCrafting(true);
}

// ----------------------------------------------------------------------

static void commandFuncRestartCraftingSession(const Command&, const NetworkId& actor, const NetworkId&, const Unicode::String& params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRestartCraftingSession: PB nullptr actor"));
		return;
	}

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
	{
		WARNING(true, ("commandFuncRestartCraftingSession: no player object for actor "
			"%s", actor.getValueString().c_str()));
		return;
	}

	uint8 sequenceId = static_cast<uint8>(atoi(Unicode::wideToNarrow(params).c_str()));

	const bool success = player->restartCrafting();

	MessageQueueGenericIntResponse * response = new MessageQueueGenericIntResponse(
		CM_restartCraftingSession, success, sequenceId);
	creature->getController()->appendMessage(CM_craftingResult, 0.0f, response,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

static void commandFuncSetMatchMakingPersonalId(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncSetMatchMakingPersonalId() <networkId> %s <params> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), Unicode::wideToNarrow(params).c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		MatchMakingId matchMakingId;
		matchMakingId.unPackIntString(Unicode::wideToNarrow(params));

		playerObject->setMatchMakingPersonalProfileId(matchMakingId);

		//DEBUG_REPORT_LOG(true, ("commandFuncSetMatchMakingPersonalId() <networkId> %s %s\n", actor.getValueString().c_str(), matchMakingId.isBitSet(MatchMakingId::B_anonymous) ? "anonymous" : "searchable"));
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetMatchMakingCharacterId(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncSetMatchMakingCharacterId() <networkId> %s <params> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), Unicode::wideToNarrow(params).c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		MatchMakingId matchMakingId;
		matchMakingId.unPackIntString(Unicode::wideToNarrow(params));

		playerObject->setMatchMakingCharacterProfileId(matchMakingId);

		//DEBUG_REPORT_LOG(true, ("commandFuncSetMatchMakingCharacterId() <networkId> %s %s\n", actor.getValueString().c_str(), matchMakingId.isBitSet(MatchMakingId::B_anonymous) ? "anonymous" : "searchable"));
	}
}

// ----------------------------------------------------------------------

static void commandFuncAddFriend(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		size_t pos = 0;
		std::string name(nextStringParm(params, pos));

		if (!name.empty())
		{
			std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

			//DEBUG_REPORT_LOG(true, ("commandFuncAddFriend() <networkId> %s <player> %s <name> %s\n", actor.getValueString().c_str(), player.c_str(), name.c_str()));

			Chat::addFriend(player, name);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncRemoveFriend(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		size_t pos = 0;
		std::string name(nextStringParm(params, pos));

		if (!name.empty())
		{
			std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

			//DEBUG_REPORT_LOG(true, ("commandFuncRemoveFriend() <networkId> %s <player> %s <name> %s\n", actor.getValueString().c_str(), player.c_str(), name.c_str()));

			Chat::removeFriend(player, name);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncGetFriendList(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

		//DEBUG_REPORT_LOG(true, ("commandFuncGetFriendList() <networkId> %s <player> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), player.c_str()));

		Chat::getFriendsList(player);
	}
}

// ----------------------------------------------------------------------

static void commandFuncAddIgnore(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		size_t pos = 0;
		std::string name(nextStringParm(params, pos));

		if (!name.empty())
		{
			std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

			//DEBUG_REPORT_LOG(true, ("commandFuncAddIgnore() <networkId> %s <player> %s <name> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), player.c_str(), name.c_str()));

			Chat::addIgnore(player, name);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncRemoveIgnore(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		size_t pos = 0;
		std::string name(nextStringParm(params, pos));

		if (!name.empty())
		{
			std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

			//DEBUG_REPORT_LOG(true, ("commandFuncRemoveIgnore() <networkId> %s <player> %s <name> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), player.c_str(), name.c_str()));

			Chat::removeIgnore(player, name);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncGetIgnoreList(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	ServerObject * const serverObject = ServerObject::getServerObject(actor);

	if (serverObject != nullptr)
	{
		std::string player(Chat::constructChatAvatarId(*serverObject).getFullName());

		//DEBUG_REPORT_LOG(true, ("commandFuncGetIgnoreList() <networkId> %s <player> %s\n", actor.getValueString().empty() ? "noId" : actor.getValueString().c_str(), player.c_str()));

		Chat::getIgnoreList(player);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestBiography(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &)
{
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(actor);

	if (creatureObject != nullptr)
	{
		BiographyManager::requestBiography(target, creatureObject);
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetBiography(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const &params)
{
	if (target != NetworkId::cms_invalid)
	{
		// get the actor
		CreatureObject * const creatureActor = CreatureObject::getCreatureObject(actor);
		if (!creatureActor)
		{
			WARNING(true, ("commandFuncSetBiography: bad actor"));
			return;
		}

		Client * const clientActor = creatureActor->getClient();
		if (!clientActor)
		{
			WARNING(true, ("no Client in commandFuncSetBiography"));
			return;
		}

		if (!clientActor->isGod())
		{
			LOG("CustomerService", ("CheatChannel:%s attempted to set biography data on %s, but is not a God", PlayerObject::getAccountDescription(actor).c_str(), PlayerObject::getAccountDescription(target).c_str()));
		}
		else
		{
			BiographyManager::setBiography(target, params);
		}
	}
	else
	{
		BiographyManager::setBiography(actor, params);
	}
}

// ----------------------------------------------------------------------

static void commandFuncRequestCharacterSheetInfo(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	const CreatureObject* const creatureActor = CreatureObject::getCreatureObject(actor);
	if (creatureActor == nullptr)
	{
		WARNING(true, ("commandFuncRequestCharacterSheetInfo: nullptr actor"));
		return;
	}

	//TODO get the born and played times (once they're in the DB)
	int born = 0;
	int played = 0;

	//get the bind location
	Vector bindLoc;
	std::string bindPlanet;
	NetworkId bindId;
	if (creatureActor->getObjVars().hasItem("bind.facility"))
	{
		creatureActor->getObjVars().getItem("bind.facility", bindId);
	}
	if (bindId != NetworkId::cms_invalid)
	{
		const ServerObject* const bindObject = ServerObject::getServerObject(bindId);
		if (bindObject != nullptr)
		{
			bindLoc = bindObject->getPosition_w();
			bindPlanet = bindObject->getSceneId();
		}
	}

	//get the bankId
	Vector bankLoc(0, 0, 0);
	std::string bankPlanet;
	if (creatureActor->getObjVars().hasItem("banking_bankid"))
	{
		creatureActor->getObjVars().getItem("banking_bankid", bankPlanet);
	}

	// get the city(ies) this character is a citizen of
	NetworkId cityHallOfMayorCity;
	std::string citizensOf;
	std::vector<int> const & cityId = CityInterface::getCitizenOfCityId(creatureActor->getNetworkId());
	if (!cityId.empty())
	{
		std::string cityName;
		for (std::vector<int>::const_iterator iterCity = cityId.begin(); iterCity != cityId.end(); ++iterCity)
		{
			CityInfo const & cityInfo = CityInterface::getCityInfo(*iterCity);
			cityName = cityInfo.getCityName();
			if (cityName.empty())
				continue;

			if (!citizensOf.empty())
				citizensOf += ",";

			if (creatureActor->getNetworkId() == cityInfo.getLeaderId())
			{
				citizensOf += "*";
				cityHallOfMayorCity = cityInfo.getCityHallId();
			}

			citizensOf += cityName;
		}
	}

	// get the residence location
	Vector resLoc;
	std::string resPlanet;
	NetworkId houseNetworkId = creatureActor->getHouse();

	// mayor has city hall as his residence
	if (!houseNetworkId.isValid() && cityHallOfMayorCity.isValid())
		houseNetworkId = cityHallOfMayorCity;

	const ServerObject* const resObject = ServerObject::getServerObject(houseNetworkId);
	if (resObject != nullptr)
	{
		resLoc = resObject->getPosition_w();
		resPlanet = ServerWorld::getSceneId();
	}
	else if (houseNetworkId.isValid() && MessageToQueue::isInstalled())
	{
		// the residence may not be on this game server, try to find it
		MessageToQueue::getInstance().sendMessageToC(houseNetworkId,
			"C++CharacterSheetInfoResidenceLocationReq",
			creatureActor->getNetworkId().getValueString(),
			0,
			false);
	}

	//get the spouse's name
	Unicode::String spouseName = Unicode::emptyString;
	if (creatureActor->getObjVars().hasItem("marriage.spouseName"))
	{
		creatureActor->getObjVars().getItem("marriage.spouseName", spouseName);
	}

	//get the number of used lots
	int lots = creatureActor->getMaxNumberOfLots();

	PlayerObject const * const player = PlayerCreatureController::getPlayerObject(creatureActor);
	if (player)
	{
		int lotsUsed = player->getAccountNumLots();
		lots -= lotsUsed;
	}

	//build, send the message back to the client
	CharacterSheetResponseMessage const chrm(born, played, bindLoc, bindPlanet, bankLoc, bankPlanet, resLoc, resPlanet, citizensOf, spouseName, lots);
	Client * const client = creatureActor->getClient();
	if (client)
		client->send(chrm, true);
}

// ----------------------------------------------------------------------

static void commandFuncRequestCharacterMatch(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncRequestCharacterMatch() - networkId: %s params: %s\n", actor.getValueString().c_str(), Unicode::wideToNarrow(params).c_str()));

	// Build the new id

	MatchMakingCharacterPreferenceId matchMakingCharacterPreferenceId;

	matchMakingCharacterPreferenceId.unPackString(Unicode::wideToNarrow(params));

	CharacterMatchManager::requestMatch(actor, matchMakingCharacterPreferenceId);
}

// ----------------------------------------------------------------------

static void commandFuncExtractObject(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	CreatureObject* const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncExtractObject: PB nullptr actor"));
		return;
	}

	if (creature->getInventory() == nullptr)
	{
		WARNING(true, ("commandFuncExtractObject: actor %s has no inventory",
			actor.getValueString().c_str()));
		return;
	}

	FactoryObject* const factory = dynamic_cast<FactoryObject *>(NetworkIdManager::getObjectById(target));
	if (factory == nullptr)
	{
		WARNING(true, ("commandFuncExtractObject: PB nullptr target"));
		return;
	}

	Container::ContainerErrorCode error;
	if (creature->canManipulateObject(*factory, false, true, true, 10.0f, error))
		factory->removeObject(*creature->getInventory());
	else
		ContainerInterface::sendContainerMessageToClient(*creature, error, factory);
}

// ----------------------------------------------------------------------

static void commandFuncRevokeSkill(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	CreatureObject * const creature = CreatureObject::getCreatureObject(actor);
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRevokeSkill: PB nullptr actor"));
		return;
	}

	size_t pos = 0;
	std::string skillName = nextStringParm(params, pos);
	const SkillObject * skill = SkillManager::getInstance().getSkill(skillName);
	if (skill == nullptr)
	{
		WARNING(true, ("commandFuncRevokeSkill: can't revoke bad skill"));
	}
	else
	{
		LOG("CustomerService", ("Skill: God (via cmdfunc) has requested the revocation of skill %s from character %s.",
			skillName.c_str(), creature->getNetworkId().getValueString().c_str()));
		creature->revokeSkill(*skill);
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetCurrentSkillTitle(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(actor);
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);

	if (playerObject != nullptr)
	{
		size_t pos = 0;
		std::string const &title = nextStringParm(params, pos);

		if (title.empty())
		{
			playerObject->setTitle(title);
		}
		else
		{
			bool validTitle = false;

			// citizenship title is always valid
			if (title == "citizenship")
			{
				playerObject->setTitle(title);
				validTitle = true;
			}

			// see if it's a collection slot title and the character actually has the collection slot
			if (!validTitle)
			{
				CollectionsDataTable::CollectionInfoSlot const * const slot = CollectionsDataTable::isASlotTitle(title);

				if (slot && playerObject->hasCompletedCollectionSlot(*slot))
				{
					playerObject->setTitle(title);
					validTitle = true;
				}
			}

			// see if it's a collection title and the character actually has the collection
			if (!validTitle)
			{
				CollectionsDataTable::CollectionInfoCollection const * const collection = CollectionsDataTable::isACollectionTitle(title);

				if (collection && playerObject->hasCompletedCollection(collection->name))
				{
					playerObject->setTitle(title);
					validTitle = true;
				}
			}

			// see if it's a collection page title and the character actually has the collection page
			if (!validTitle)
			{
				CollectionsDataTable::CollectionInfoPage const * const page = CollectionsDataTable::isAPageTitle(title);

				if (page && playerObject->hasCompletedCollectionPage(page->name))
				{
					playerObject->setTitle(title);
					validTitle = true;
				}
			}

			// see if it's a guild member rank title and the character actually has the guild member rank
			if (!validTitle)
			{
				GuildRankDataTable::GuildRank const * const guildRankInfo = GuildRankDataTable::isARankTitle(title);

				if (guildRankInfo && playerObject->getGuildRank().testBit(guildRankInfo->slotId))
				{
					playerObject->setTitle(title);
					validTitle = true;
				}
			}

			// see if it's a citizen rank title and the character actually has the citizen rank
			if (!validTitle)
			{
				CitizenRankDataTable::CitizenRank const * const citizenRankInfo = CitizenRankDataTable::isARankTitle(title);

				if (citizenRankInfo && playerObject->getCitizenRank().testBit(citizenRankInfo->slotId))
				{
					playerObject->setTitle(title);
					validTitle = true;
				}
			}

			// see if it's a guild GCW Region Defender title
			if (!validTitle)
			{
				if (title == "guild_gcw_region_defender")
				{
					std::pair<std::string, std::pair<bool, bool> > const & guildGcwDefenderRegionInfo = playerObject->getGuildGcwDefenderRegionInfo();
					if (!guildGcwDefenderRegionInfo.first.empty() && guildGcwDefenderRegionInfo.second.second)
					{
						playerObject->setTitle(title);
						validTitle = true;
					}
				}
			}

			// see if it's a city GCW Region Defender title
			if (!validTitle)
			{
				if (title == "city_gcw_region_defender")
				{
					std::pair<std::string, std::pair<bool, bool> > const & cityGcwDefenderRegionInfo = playerObject->getCityGcwDefenderRegionInfo();
					if (!cityGcwDefenderRegionInfo.first.empty() && cityGcwDefenderRegionInfo.second.second)
					{
						playerObject->setTitle(title);
						validTitle = true;
					}
				}
			}

			// Run through all the skills for this player and make sure
			// this is a valid skill title for them
			if (!validTitle)
			{
				CreatureObject::SkillList::const_iterator iterSkillList = creatureObject->getSkillList().begin();
				for (; iterSkillList != creatureObject->getSkillList().end(); ++iterSkillList)
				{
					SkillObject const *skillObject = (*iterSkillList);

					if ((skillObject != nullptr)
						&& skillObject->isTitle()
						&& (skillObject->getSkillName() == title))
					{
						playerObject->setTitle(title);
						validTitle = true;
						break;
					}
				}
			}

			if (!validTitle)
			{
				playerObject->setTitle(std::string());
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncRepair(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	CreatureObject* const creature = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (creature == nullptr)
	{
		WARNING(true, ("commandFuncRepair: PB nullptr actor"));
		return;
	}

	TangibleObject* const object = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(target));
	if (object == nullptr)
	{
		WARNING(true, ("commandFuncRepair: PB nullptr target"));
		return;
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleSearchableByCtsSourceGalaxy(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleSearchableByCtsSourceGalaxy() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleSearchableByCtsSourceGalaxy();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleDisplayLocationInSearchResults(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleDisplayLocationInSearchResults() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleDisplayLocationInSearchResults();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleAnonymous(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleAnonymous() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleAnonymous();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleHelper(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleHelper() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleHelper();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleRolePlay(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleRolePlay() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleRolePlay();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleOutOfCharacter(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleOutOfCharacter();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleLookingForWork(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleLookingForWork();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleLookingForGroup(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleLookingForGroup() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleLookingForGroup();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleAwayFromKeyBoard(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	//DEBUG_REPORT_LOG(true, ("commandFuncToggleAwayFromKeyBoard() - networkId: %s\n", actor.getValueString().c_str()));

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleAwayFromKeyBoard();
	}
}

// ----------------------------------------------------------------------

static void commandFuncToggleDisplayingFactionRank(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(CreatureObject::getCreatureObject(actor));

	if (playerObject != nullptr)
	{
		playerObject->toggleDisplayingFactionRank();
	}
}

// ----------------------------------------------------------------------

static void commandFuncReport(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject const * const reportingCreatureObject = CreatureObject::getCreatureObject(actor);

	if (reportingCreatureObject != nullptr)
	{
		if (ReportManager::isThrottled(actor))
		{
			// Player is trying to report too quickly after a previous report

			ProsePackage prosePackage;
			prosePackage.stringId = StringId("system_msg", "report_wait");
			prosePackage.digitInteger = ReportManager::getThrottleTimeRemaining(actor);

			Chat::sendSystemMessage(*reportingCreatureObject, prosePackage);
		}
		else
		{
			Unicode::String harassingPlayerName;
			//size_t curpos = 0;
			//size_t endpos = 0;

			// If this player is on this galaxy, we can get the network id

			//Vivox phase 1 temporary solution: the client is appending
			//its connection information to the report command, this should
			//come from VChat really
			Unicode::String const delimiter = Unicode::narrowToWide("|");

			Unicode::UnicodeStringVector tokens;
			if (Unicode::tokenize(params, tokens, &delimiter) && tokens.size() > 1)
			{
				harassingPlayerName = tokens[0];
				Unicode::String rest;
				uint32 numTokens = tokens.size();
				for (uint32 i = 1; i < numTokens; ++i)
				{
					if (i != 1)
					{
						rest.append(delimiter);
					}
					rest.append(tokens[i]);
				}

				NetworkId const harassingPlayerNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(harassingPlayerName)));

				ReportManager::addReport(reportingCreatureObject->getAssignedObjectFirstName(), actor, harassingPlayerName, harassingPlayerNetworkId, std::string("Report"), rest);
			}
			else
			{
				// No harassing player specified

				Chat::sendSystemMessage(*reportingCreatureObject, StringId("system_msg", "report_no_name"), Unicode::emptyString);
			}

			//if (Unicode::getFirstToken(name, curpos, endpos, harassingPlayerName))
			//{
			//	NetworkId const harassingPlayerNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(harassingPlayerName)));

			//	ReportManager::addReport(reportingCreatureObject->getAssignedObjectFirstName(), actor, harassingPlayerName, harassingPlayerNetworkId, std::string("Report"));
			//}
			//else
			//{
			//	// No harassing player specified

			//	Chat::sendSystemMessage(*reportingCreatureObject, StringId("system_msg", "report_no_name"), Unicode::emptyString);
			//}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("A non-player is trying to report a harassment. oid: %s", actor.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

static void commandFuncNpcConversationStart(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	ServerObject * const actorObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	CreatureObject * const player = actorObject != nullptr ? actorObject->asCreatureObject() : nullptr;
	if (player == nullptr)
	{
		DEBUG_WARNING(true, ("commandFuncNpcConversationStart: couldn't find actor"));
		return;
	}

	ServerObject * const npcObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
	TangibleObject * const npc = npcObject != nullptr ? npcObject->asTangibleObject() : nullptr;
	if (npc == nullptr)
	{
		DEBUG_WARNING(true, ("commandFuncNpcConversationStart: Couldn't find npc to converse with"));
		return;
	}

	if (!ServerWorld::isSpaceScene())
	{
		Container::ContainerErrorCode error = Container::CEC_Success;
		if (!player->canManipulateObject(*npc, false, false, false, 30.0f, error))
		{
			DEBUG_WARNING(true, ("commandFuncNpcConversationStart (ground): failed canManipulateObject check"));
			return;
		}
	}

	std::string const & realParams = Unicode::wideToNarrow(params);
	if (realParams.size() < 2)
	{
		DEBUG_WARNING(true, ("commandFuncNpcConversationStart: bad params %s", realParams.c_str()));
		return;
	}

	const char * const conversationName = &realParams[2];
	NpcConversationData::ConversationStarter const starter = static_cast<NpcConversationData::ConversationStarter>(atoi(realParams.c_str()));

	player->startNpcConversation(*npc, conversationName, starter, 0);
}

//----------------------------------------------------------------------

static void commandFuncNpcConversationStop(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	ServerObject * const actorObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	TangibleObject * const player = actorObject != nullptr ? actorObject->asTangibleObject() : nullptr;
	if (player == nullptr)
	{
		DEBUG_WARNING(true, ("commandFuncNpcConversationStop: couldn't find actor"));
		return;
	}
	player->endNpcConversation();
}

//----------------------------------------------------------------------

static void commandFuncNpcConversationSelect(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	Object * const actorObject = NetworkIdManager::getObjectById(actor);
	CreatureObject * const player = dynamic_cast<CreatureObject * const>(actorObject);
	if (player == nullptr)
	{
		DEBUG_WARNING(true, ("commandFuncNpcConversationStop: couldn't find actor"));
		return;
	}

	std::string realParams(Unicode::wideToNarrow(params));
	int selection = atoi(realParams.c_str());
	player->respondToNpc(selection);
}

//----------------------------------------------------------------------

static void commandFuncServerDestroyObject(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	CreatureObject * const player = dynamic_cast<CreatureObject * const> (NetworkIdManager::getObjectById(actor));
	if (player == nullptr)
	{
		DEBUG_WARNING(true, ("commandFuncServerDestroyObject: couldn't find actor"));
		return;
	}

	ServerObject * const targetObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
	if (!targetObject)
	{
		PlayerObject * p = PlayerCreatureController::getPlayerObject(player);
		if (p)
		{
			p->destroyWaypoint(target);
			return;
		}
		else
		{
			DEBUG_WARNING(true, ("commandFuncServerDestroyObject: couldn't find target"));
			return;
		}
	}

	ServerObject const * const topMost = getFirstParentInWorldOrPlayer(targetObject);
	if (!topMost || topMost->getNetworkId() != actor)
	{
		DEBUG_WARNING(true, ("player tried to delete something not in their inventory"));
		return;
	}

	ProsePackage pp;
	const bool sendResponse = !params.empty();

	int const got = targetObject->getGameObjectType();
	if (sendResponse)
	{
		ProsePackageManagerServer::createSimpleProsePackage(*targetObject, SharedStringIds::rsp_object_deleted_prose, pp);
		pp.other.stringId = GameObjectTypes::getStringId(got);
	}

	// Cannot destroy an empty ship pcd while contained by a ship you own
	if (got == SharedObjectTemplate::GOT_data_ship_control_device)
	{
		ShipObject const * const containingShip = ShipObject::getContainingShipObject(player);
		if (containingShip && containingShip->getOwnerId() == actor && containingShip->getControlDevice() == targetObject)
		{
			if (sendResponse)
				Chat::sendSystemMessage(*player, SharedStringIds::no_destroy_in_flight, Unicode::emptyString);
			return;
		}
	}

	// Cannot destroy a tcg card that is in the middle of the redemption process
	if (!VeteranRewardManager::checkForTcgRedemptionInProgress(*targetObject))
		return;

	// Cannot destroy a reward item that is in the middle of the trade in process
	if (!VeteranRewardManager::checkForTradeInInProgress(*targetObject))
		return;

	Object const * const currentContainer = ContainerInterface::getContainedByObject(*targetObject);
	if (currentContainer && currentContainer->asServerObject() && (currentContainer->asServerObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron || currentContainer->asServerObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe))
	{
		// Give the player a warning here?
		return;
	}

	if (targetObject->permanentlyDestroy(DeleteReasons::Player))
	{
		LOG("CustomerService", ("Deletion:%s is deleting object %s", PlayerObject::getAccountDescription(actor).c_str(), ServerObject::getLogDescription(targetObject).c_str()));
		if (sendResponse)
			Chat::sendSystemMessage(*player, pp);
	}
	else
	{
		// don't throw a warning if the only reason destruction failed is because the no destroy script blocked the request
		if (targetObject->getScriptObject()->hasScript("item.special.nodestroy")) {
			return;
		}
		WARNING(true, ("commandFuncServerDestroyObject: Error encountered while deleting object %s", target.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

static void commandFuncSetSpokenLanguage(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	Object * const object = NetworkIdManager::getObjectById(actor);
	CreatureObject * const creatureObject = dynamic_cast<CreatureObject * const>(object);

	if (creatureObject != nullptr)
	{
		PlayerObject *playerObject = PlayerCreatureController::getPlayerObject(creatureObject);

		if (playerObject != nullptr)
		{
			size_t pos = 0;
			int const languageId = nextIntParm(params, pos);
			std::string skillModName;
			GameLanguageManager::getLanguageSpeakSkillModName(languageId, skillModName);

			// Make sure the player is setting a valid game language

			std::map<std::string, std::pair<int, int> >::const_iterator iterModMap = creatureObject->getModMap().find(skillModName);

			if (iterModMap != creatureObject->getModMap().end())
			{
				int const baseValue = iterModMap->second.first;
				int const modValue = iterModMap->second.second;

				if ((baseValue + modValue) > 0)
				{
					playerObject->setSpokenLanguage(languageId);
				}
			}
			else
			{
				DEBUG_WARNING(true, ("Invalid game language specified: %d", languageId));
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncUnstick(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	Object * const object = NetworkIdManager::getObjectById(actor);
	CreatureObject * const creatureObject = dynamic_cast<CreatureObject * const>(object);
	if (creatureObject != nullptr)
	{
		if (!ShipObject::getContainingShipObject(creatureObject)) // no unsticking in ships
		{
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
			if (playerObject)
			{
				Chat::sendSystemMessageSimple(*creatureObject, SharedStringIds::unstick_in_progress, nullptr);
				if (!playerObject->getIsUnsticking())
				{
					Vector position = creatureObject->getPosition_p();
					//Send positional information with this unstick along with the message.  If you change
					//the format of this positional information, you must also change it in the handler in ServerObject.cpp
					char buf[255];
					snprintf(buf, sizeof(buf), "%f:%f", position.x, position.z);
					buf[sizeof(buf) - 1] = '\0';

					int wait = 30;
					if (creatureObject->getClient() && creatureObject->getClient()->isGodValidated())
					{
						if (creatureObject->getObjVars().getItem("debugUnstickWait", wait))
						{
							if (wait <= 0)
								wait = 1;
						}
						else
						{
							wait = 30;
						}
					}

					MessageToQueue::getInstance().sendMessageToC(actor, "C++Unstick", buf, wait, false);
					playerObject->setIsUnsticking(true);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncGetAccountInfo(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			bool foundAsPilot = false;
			const CreatureObject * player = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(target));
			if (!player)
			{
				ShipObject const * const ship = dynamic_cast<ShipObject const *>(NetworkIdManager::getObjectById(target));
				if (ship)
				{
					foundAsPilot = true;
					player = ship->getPilot();
				}
			}
			std::string result;
			if (player)
			{
				const Client * client = player->getClient();
				if (client)
				{
					if (foundAsPilot)
						result = "Pilot Character Name   : ";
					else
						result = "Character Name         : ";

					result += Unicode::wideToNarrow(player->getAssignedObjectName());
					result += "\nAccount Name           : ";
					result += client->getAccountName();
					result += "\nStation Id             : ";
					char buf[32];
					IGNORE_RETURN(snprintf(buf, sizeof(buf) - 1, "%d", client->getStationId()));
					buf[sizeof(buf) - 1] = '\0';
					result += buf;
					result += "\nGame Features          : ";
					result += ClientGameFeature::getDescription(client->getGameFeatures());
					result += "\nSubscription Features  : ";
					uint32 const subscriptionFeatures = client->getSubscriptionFeatures();
					result += ClientSubscriptionFeature::getDescription(subscriptionFeatures);

					result += "\nAccount SWG Feature Ids: ";
					std::string accountFeatureIdsString;
					Client::AccountFeatureIdList const & accountFeatureIds = client->getAccountFeatureIds();
					for (Client::AccountFeatureIdList::const_iterator iterAccountFeatureIds = accountFeatureIds.begin(); iterAccountFeatureIds != accountFeatureIds.end(); ++iterAccountFeatureIds)
					{
						if (!accountFeatureIdsString.empty())
							accountFeatureIdsString += ", ";

						accountFeatureIdsString += FormattedString<1024>().sprintf("%lu:%d", iterAccountFeatureIds->first, iterAccountFeatureIds->second);

						if (VeteranRewardManager::isFeatureIdBlockedBySubscriptionBit(*player, iterAccountFeatureIds->first))
							accountFeatureIdsString += "*";
					}

					if (!accountFeatureIdsString.empty())
						result += accountFeatureIdsString;

					result += "\nIP Address             : ";
					result += client->getIpAddress();
					result += "\nGod Mode               : ";
					if (client->isGod())
						result += "true";
					else
						result += "false";
				}
				else
				{
					result = "Object " + target.getValueString() + " is not controlled by a player.";
				}
			}
			else
			{
				result = "Object " + target.getValueString() + " is not a creature or a ship.";
			}
			ConGenericMessage const msg(result, 0);
			gmClient->send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncApplyPowerup(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const NetworkId & powerupId = target;
	const NetworkId targetOfApplyId(Unicode::wideToNarrow(params));
	ServerObject * const powerup = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(powerupId));
	if (powerup)
	{
		const ServerObject * const targetOfApply = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(targetOfApplyId));

		if (targetOfApply)
		{
			const int got_targetOfApply = targetOfApply->getGameObjectType();
			const int got_powerup = powerup->getGameObjectType();

			if (GameObjectTypes::doesPowerupApply(got_powerup, got_targetOfApply))
			{
				GameScriptObject * const gso = powerup->getScriptObject();
				if (gso)
				{
					ScriptParams scriptParams;
					scriptParams.addParam(actor);
					scriptParams.addParam(targetOfApplyId);
					gso->trigAllScripts(Scripting::TRIG_APPLY_POWERUP, scriptParams);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncLag(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
#if 0
	ServerObject * a = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	ServerObject * t = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
	if (!t)
	{
		t = a;
	}
	if (t != 0)
	{
		if (t->getClient())
		{
			if (a && a->getClient())
			{
				GameNetworkMessage const request("LagRequest");
				t->getClient()->send(request, true);
				int connectionServerLag = t->getClient()->getConnectionServerLag();
				int gameServerLag = t->getClient()->getGameServerLag();
				char lagMsg[1024];
				snprintf(lagMsg, sizeof(lagMsg), "Lag update request sent to client.\nCurrent Lag: ConnectionServer=%d, GameServer=%d\n", connectionServerLag, gameServerLag);
				std::string msg(lagMsg);
				ConGenericMessage const result(msg, 0);
				a->getClient()->send(result, true);
			}
		}
	}
#endif
}

// ----------------------------------------------------------------------

static void commandExecuteKnowledgeBaseMessage(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	UNREF(actor);
	UNREF(params);
}

// ----------------------------------------------------------------------

static void commandFuncEmptyMailTarget(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	size_t pos = 0;
	std::string networkId(nextStringParm(params, pos));

	NetworkId targetNetworkId(networkId);

	DEBUG_REPORT_LOG(true, ("commandFuncEmptyMailTarget() - Player(%s) is emptying the mailbox of player(%s).\n", actor.getValueString().c_str(), targetNetworkId.getValueString().c_str()));

	Chat::emptyMail(actor, targetNetworkId);
}

// ----------------------------------------------------------------------

static void commandFuncSetPlayerAppearance(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	//-- Collect parameters.
	std::string  token;
	std::string  objectTemplateName;

	bool    useTarget = false;
	size_t  pos = 0;

	do
	{
		token = Unicode::toLower(nextStringParm(params, pos));
		if (token == "-target")
			useTarget = true;
		else if (!token.empty())
			objectTemplateName = token;
	} while (!token.empty());

	//-- Log attempt.
	NetworkId subjectNetworkId = useTarget ? target : actor;

	LOG("setPlayerAppearance",
		("commandFuncSetPlayerAppearance(): command called by actor id=[%s]: attempting to change appearance for player id=[%s], using appearance info from object template=[%s], subject=[%s]",
			actor.getValueString().c_str(),
			subjectNetworkId.getValueString().c_str(),
			objectTemplateName.c_str(),
			useTarget ? "target" : "self"
			));

	//-- Validate parameters.
	if (subjectNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("commandFuncSetPlayerAppearance(): subjectNetworkId is invalid, useTarget=[%s]", useTarget ? "true" : "false"));
		return;
	}

	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(subjectNetworkId));
	if (!creatureObject)
	{
		WARNING(true, ("commandFuncSetPlayerAppearance(): subject network id=[%s] is not a CreatureObject-derived class.", subjectNetworkId.getValueString().c_str()));
		return;
	}

	if (objectTemplateName.empty())
	{
		// Use the Creature's natural object template name.
		objectTemplateName = creatureObject->getObjectTemplateName();
	}
	else
	{
		// Ensure non-default object template name refers to an existing file.
		if (!TreeFile::exists(objectTemplateName.c_str()))
		{
			WARNING(true,
				("commandFuncSetPlayerAppearance(): could not change id=[%s] to non-default object template name=[%s]: object template does not exist",
					subjectNetworkId.getValueString().c_str(),
					objectTemplateName.c_str()
					));
			return;
		}
	}

	//-- Get the controller, instruct it to change the appearance.
	CreatureController *const controller = dynamic_cast<CreatureController*>(creatureObject->getController());
	if (!controller)
	{
		WARNING(true, ("commandFuncSetPlayerAppearance(): subject network id=[%s] does not have a CreatureController.", subjectNetworkId.getValueString().c_str()));
		return;
	}

	//-- Set the new appearance.
	controller->setAppearanceFromObjectTemplate(objectTemplateName);
}

static void commandFuncRevertPlayerAppearance(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	//-- Collect parameters.
	std::string  token;
	std::string  objectTemplateName;

	bool    useTarget = false;
	size_t  pos = 0;

	do
	{
		token = Unicode::toLower(nextStringParm(params, pos));
		if (token == "-target")
			useTarget = true;
		else if (!token.empty())
			objectTemplateName = token;
	} while (!token.empty());

	//-- Log attempt.
	NetworkId subjectNetworkId = useTarget ? target : actor;

	LOG("setPlayerAppearance",
		("commandFuncRevertPlayerAppearance(): command called by actor id=[%s]: attempting to change appearance for player id=[%s], using appearance info from object template=[%s], subject=[%s]",
			actor.getValueString().c_str(),
			subjectNetworkId.getValueString().c_str(),
			objectTemplateName.c_str(),
			useTarget ? "target" : "self"
			));

	//-- Validate parameters.
	if (subjectNetworkId == NetworkId::cms_invalid)
	{
		WARNING(true, ("commandFuncRevertPlayerAppearance(): subjectNetworkId is invalid, useTarget=[%s]", useTarget ? "true" : "false"));
		return;
	}

	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(subjectNetworkId));
	if (!creatureObject)
	{
		WARNING(true, ("commandFuncRevertPlayerAppearance(): subject network id=[%s] is not a CreatureObject-derived class.", subjectNetworkId.getValueString().c_str()));
		return;
	}

	//-- Get the controller, instruct it to change the appearance.
	CreatureController *const controller = dynamic_cast<CreatureController*>(creatureObject->getController());
	if (!controller)
	{
		WARNING(true, ("commandFuncRevertPlayerAppearance(): subject network id=[%s] does not have a CreatureController.", subjectNetworkId.getValueString().c_str()));
		return;
	}

	//-- Set the new appearance.
	std::string const none("none");
	controller->setAppearanceFromObjectTemplate(none);
}

//-----------------------------------------------------------------------

static void commandFuncReconnectToTransferServer(Command const &, NetworkId const &, NetworkId const &, Unicode::String const &)
{
	GameNetworkMessage const msg("ReconnectToTransferServer");
	GameServer::getInstance().sendToCentralServer(msg);
}

//-----------------------------------------------------------------------

static void commandFuncFindFriend(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	ServerObject * a = safe_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (a && a->getClient())
	{
		size_t pos = 0;
		const std::string playerName = nextStringParm(params, pos);
		CreatureObject * creatureObject = a->asCreatureObject();
		if (creatureObject)
		{
			if (!playerName.empty())
			{
				// break the name down into its subcomponents
				ChatAvatarId const cav(NameManager::normalizeName(playerName));

				// can only find friend on the same cluster
				bool const sameGame = (cav.gameCode.empty() || !_stricmp(cav.gameCode.c_str(), Chat::getGameCode().c_str()));
				bool const sameCluster = (sameGame && (cav.cluster.empty() || !_stricmp(cav.cluster.c_str(), GameServer::getInstance().getClusterName().c_str())));

				if (!sameGame || !sameCluster)
				{
					StringId stringId = StringId("ui_cmnty", "friend_location_failed_remote_name");
					Chat::sendSystemMessage(*creatureObject, stringId, Unicode::emptyString);
				}
				else
				{
					PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
					if (playerObject)
					{
						playerObject->findFriend(cav.name);
					}
				}
			}
			else
			{
				// playerName is empty, send an error to requesting client
				StringId stringId = StringId("ui_cmnty", "friend_location_failed_usage");
				Chat::sendSystemMessage(*creatureObject, stringId, Unicode::emptyString);
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncFormCommand(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	UNREF(target);

	Unicode::UnicodeStringVector tokens;
	IGNORE_RETURN(Unicode::tokenize(params, tokens));

	if (tokens.size() == 0)
		return;

	std::string const commandStr = Unicode::wideToNarrow(tokens[0]);
	FormManager::Command const command = static_cast<FormManager::Command>(atoi(commandStr.c_str()));

	switch (command)
	{
	case FormManager::CREATE_OBJECT:
	{
		if (tokens.size() < 5)
			return;

		std::string const templateName = Unicode::wideToNarrow(tokens[1]);

		std::string const & xStr = Unicode::wideToNarrow(tokens[2]);
		std::string const & yStr = Unicode::wideToNarrow(tokens[3]);
		std::string const & zStr = Unicode::wideToNarrow(tokens[4]);
		std::string const & cellStr = Unicode::wideToNarrow(tokens[5]);
		NetworkId const cellId(cellStr);

		float const x = static_cast<float>(atof(xStr.c_str()));
		float const y = static_cast<float>(atof(yStr.c_str()));
		float const z = static_cast<float>(atof(zStr.c_str()));

		//we must have an odd number of tokens left (so that the datamap has a full key->value mapping)
		if (tokens.size() % 2 != 0)
			return;

		//TODO centralize pack/unpack
		FormManager::UnpackedFormData dataMap;
		std::vector<std::string> values;
		for (int i = 6; i < static_cast<int>(tokens.size()) - 1; i += 2)
		{
			std::string const & key = Unicode::wideToNarrow(tokens[static_cast<size_t>(i)]);
			std::string const & value = Unicode::wideToNarrow(tokens[static_cast<size_t>(i + 1)]);
			values.clear();
			values.push_back(value);
			dataMap[key] = values;
		}

		FormManagerServer::handleCreateObjectData(actor, templateName, Vector(x, y, z), cellId, dataMap);
		break;
	}

	case FormManager::EDIT_OBJECT:
	{
		if (tokens.size() < 1)
			return;

		//we must have an even number of tokens left (so that the datamap has a full key->value mapping)
		if (tokens.size() % 2 != 1)
			return;

		//TODO centralize pack/unpack
		FormManager::UnpackedFormData dataMap;
		std::vector<std::string> values;
		for (int i = 1; i < static_cast<int>(tokens.size()) - 1; i += 2)
		{
			std::string const & key = Unicode::wideToNarrow(tokens[static_cast<size_t>(i)]);
			std::string const & value = Unicode::wideToNarrow(tokens[static_cast<size_t>(i + 1)]);
			values.clear();
			values.push_back(value);
			dataMap[key] = values;
		}

		FormManagerServer::handleEditObjectData(actor, target, dataMap);
		break;
	}

	case FormManager::REQUEST_EDIT_OBJECT:
	{
		FormManagerServer::requestEditObjectDataForClient(actor, target);
		break;
	}

	default:
		DEBUG_FATAL(true, ("Unknown command in commandFuncFormCommand"));
		break;
	}
}

// ----------------------------------------------------------------------

static void commandFuncInstallShipComponents(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	//params for installShipComponent are "<shipId> <slotnumber> <objId>"

	Object const * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject const * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject const * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
	{
		return;
	}

	//they must be near a space terminal (if not a god)
	Client const * const client = actorCreature->getClient();
	if (!client)
		return;
	if (!client->isGod())
	{
		Object * const targetObj = NetworkIdManager::getObjectById(target);
		ServerObject * const targetServerObj = targetObj ? targetObj->asServerObject() : nullptr;
		if (!targetServerObj || ((targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space) &&
			(targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space_npe)))
			return;
	}
	else
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("Bypassing terminal check due to GOD mode."), Unicode::emptyString);
	}

	size_t pos = 0;
	NetworkId const & shipId = nextOidParm(params, pos);
	Object * const shipObj = NetworkIdManager::getObjectById(shipId);
	ServerObject * const shipServerObj = shipObj ? shipObj->asServerObject() : nullptr;
	ShipObject * const ship = shipServerObj ? shipServerObj->asShipObject() : nullptr;
	if (!ship)
		return;

	ShipChassisSlotType::Type const slotType = static_cast<ShipChassisSlotType::Type const>(nextIntParm(params, pos));
	NetworkId const & componentId = nextOidParm(params, pos);
	Object * const componentObj = NetworkIdManager::getObjectById(componentId);
	ServerObject * const componentServerObj = componentObj ? componentObj->asServerObject() : nullptr;
	TangibleObject * const component = componentServerObj ? componentServerObj->asTangibleObject() : nullptr;
	if (!component)
	{
		return;
	}

	//make sure the player can manipulate the object
	Container::ContainerErrorCode error = Container::CEC_Success;
	if (!actorCreature->canManipulateObject(*component, true, true, true, 16.0f, error))
	{
		StringId cantManipulateId("player_utility", "cannot_manipulate");
		Chat::sendSystemMessage(*actorCreature, cantManipulateId, Unicode::emptyString);
		return;
	}

	//make sure the object is not in a non-player modifiable slot
	ContainedByProperty const * const containedByProperty = component->getContainedByProperty();
	if (containedByProperty)
	{
		Object const * const containedByObject = containedByProperty->getContainedBy();
		if (containedByObject)
		{
			SlottedContainer const * const slottedContainer = ContainerInterface::getSlottedContainer(*containedByObject);
			if (slottedContainer)
			{
				SlotId const & slotId = slottedContainer->findFirstSlotIdForObject(component->getNetworkId());
				if (slotId != SlotId::invalid)
				{
					if (!ContainerInterface::canPlayerManipulateSlot(slotId))
					{
						StringId cantManipulateId("player_utility", "cannot_manipulate_slot");
						Chat::sendSystemMessage(*actorCreature, cantManipulateId, Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	IGNORE_RETURN(ship->installComponent(actor, slotType, *component));
}

// ----------------------------------------------------------------------

static void commandFuncUninstallShipComponents(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	//params for uninstallShipComponent are "<shipId> <slotnumber>"

	Object * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
		return;
	ServerObject * const actorInv = actorCreature->getInventory();
	if (!actorInv)
		return;

	//they must be near a space terminal (if not a god)
	Client const * const client = actorCreature->getClient();
	if (!client)
		return;
	if (!client->isGod())
	{
		Object * const targetObj = NetworkIdManager::getObjectById(target);
		ServerObject * const targetServerObj = targetObj ? targetObj->asServerObject() : nullptr;
		if (!targetServerObj ||
			((targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space)
				&& (targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space_npe)))
			return;
	}
	else
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("Bypassing terminal check due to GOD mode."), Unicode::emptyString);
	}

	ServerObject const * const actorInventoryObject = actorCreature->getInventory();
	if (!actorInventoryObject)
	{
		Chat::sendSystemMessage(*actorCreature, SharedStringIds::shipcomponentinstall_noinventory, Unicode::emptyString);
		return;
	}
	VolumeContainer const * const inventoryContainer = ContainerInterface::getVolumeContainer(*actorInventoryObject);
	if (!inventoryContainer)
	{
		Chat::sendSystemMessage(*actorCreature, SharedStringIds::shipcomponentinstall_noinventorycontainer, Unicode::emptyString);
		return;
	}
	if (inventoryContainer->getCurrentVolume() >= inventoryContainer->getTotalVolume())
	{
		Chat::sendSystemMessage(*actorCreature, SharedStringIds::shipcomponentinstall_inventoryfull, Unicode::emptyString);
		return;
	}

	size_t pos = 0;
	NetworkId const & shipId = nextOidParm(params, pos);
	Object * const shipObj = NetworkIdManager::getObjectById(shipId);
	ServerObject * const shipServerObj = shipObj ? shipObj->asServerObject() : nullptr;
	ShipObject * const ship = shipServerObj ? shipServerObj->asShipObject() : nullptr;
	if (!ship)
		return;

	ShipChassisSlotType::Type const slotType = static_cast<ShipChassisSlotType::Type const>(nextIntParm(params, pos));

	IGNORE_RETURN(ship->uninstallComponent(actor, slotType, *actorInv));
}

// ----------------------------------------------------------------------

static void commandFuncInsertItemIntoShipComponentSlot(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	//params for insertItemIntoShipComponentSlot are "<shipId> <slotnumber> <objid>"

	Object * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
		return;

	//they must be near a space terminal (if not a god)
	Client const * const client = actorCreature->getClient();
	if (!client)
		return;
	if (!client->isGod())
	{
		Object * const targetObj = NetworkIdManager::getObjectById(target);
		ServerObject * const targetServerObj = targetObj ? targetObj->asServerObject() : nullptr;
		if (!targetServerObj || ((targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space) &&
			(targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space_npe)))
			return;
	}
	else
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("Bypassing terminal check due to GOD mode."), Unicode::emptyString);
	}

	size_t pos = 0;
	NetworkId const & shipId = nextOidParm(params, pos);
	Object * const shipObj = NetworkIdManager::getObjectById(shipId);
	ServerObject * const shipServerObj = shipObj ? shipObj->asServerObject() : nullptr;
	ShipObject * const ship = shipServerObj ? shipServerObj->asShipObject() : nullptr;
	if (!ship)
		return;

	ShipChassisSlotType::Type const slotType = static_cast<ShipChassisSlotType::Type const>(nextIntParm(params, pos));

	if (!ship->isSlotInstalled(slotType))
		return;

	NetworkId const componentId = nextOidParm(params, pos);

	ScriptParams trigParams;
	trigParams.addParam(slotType);
	trigParams.addParam(componentId);
	trigParams.addParam(actor);

	//now trigger the ship
	IGNORE_RETURN(ship->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_ITEM_DROPPED_ON_SLOT, trigParams));
}

// ----------------------------------------------------------------------

static void commandFuncAssociateDroidControlDeviceWithShip(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	//params for associateDroidControlDeviceWithShip are "<shipId> <droidControlDeviceId>"

	Object const * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject const * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject const * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
		return;

	//they must be near a space terminal (if not a god)
	Client const * const client = actorCreature->getClient();
	if (!client)
		return;
	if (!client->isGod())
	{
		Object * const targetObj = NetworkIdManager::getObjectById(target);
		ServerObject * const targetServerObj = targetObj ? targetObj->asServerObject() : nullptr;
		if (!targetServerObj || ((targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space) &&
			(targetServerObj->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space_npe)))
			return;
	}
	else
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("Bypassing terminal check due to GOD mode."), Unicode::emptyString);
	}

	size_t pos = 0;
	NetworkId const & shipId = nextOidParm(params, pos);
	Object * const shipObj = NetworkIdManager::getObjectById(shipId);
	ServerObject * const shipServerObj = shipObj ? shipObj->asServerObject() : nullptr;
	ShipObject * const ship = shipServerObj ? shipServerObj->asShipObject() : nullptr;
	if (!ship)
		return;

	NetworkId const droidControlDeviceId = nextOidParm(params, pos);

	if (!droidControlDeviceId.isValid())
	{
		// removing from ship
		ship->setInstalledDroidControlDevice(NetworkId::cms_invalid);
	}
	else
	{
		// putting in ship
		ScriptParams trigParams;
		trigParams.addParam(actorCreature->getNetworkId());
		trigParams.addParam(droidControlDeviceId);

		//now trigger the ship
		IGNORE_RETURN(ship->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_TRY_TO_EQUIP_DROID_CONTROL_DEVICE_IN_SHIP, trigParams));
	}
}

// ----------------------------------------------------------------------

static void commandFuncServerAsteroidDataListen(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const &)
{
	Object const * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject const * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject const * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
		return;

	ServerAsteroidManager::listenforServerAsteroidDebugData(actor);
}

// ----------------------------------------------------------------------

static void commandFuncServerAsteroidDataStopListening(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const &)
{
	Object const * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject const * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	CreatureObject const * const actorCreature = actorServerObj ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
		return;

	ServerAsteroidManager::endListenforServerAsteroidDebugData(actor);
}

//----------------------------------------------------------------------

static void commandFuncBoosterOn(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const &)
{
	internalSetBoosterOnOff(actor, true);
}

//----------------------------------------------------------------------

static void commandFuncBoosterOff(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const &)
{
	internalSetBoosterOnOff(actor, false);
}

//----------------------------------------------------------------------

static void commandFuncSetFormation(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const & params)
{
	Object * const o = NetworkIdManager::getObjectById(actor);
	ServerObject * const so = o ? o->asServerObject() : nullptr;
	CreatureObject * const actorCreature = so ? so->asCreatureObject() : nullptr;
	if (actorCreature)
	{
		GroupObject * const group = actorCreature->getGroup();
		if (group)
		{
			if (group->getGroupLeaderId() == actor)
			{
				std::string paramsNarrow = Unicode::wideToNarrow(params);
				if (paramsNarrow == "none")
				{
					group->setFormationNameCrc(Crc::crcNull);
				}
				else
				{
					if (PlayerFormationManager::isValidFormationName(paramsNarrow))
						group->setFormationNameCrc(Crc::normalizeAndCalculate(paramsNarrow.c_str()));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

static void commandFuncSetFormationSlot(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	Object * const actorObject = NetworkIdManager::getObjectById(actor);
	ServerObject * const actorServerObject = (actorObject != 0) ? actorObject->asServerObject() : 0;
	CreatureObject * const actorCreatureObject = (actorServerObject != 0) ? actorServerObject->asCreatureObject() : 0;
	if ((actorCreatureObject != 0) && (actor != NetworkId::cms_invalid))
	{
		GroupObject * const group = actorCreatureObject->getGroup();
		if (group)
		{
			if (group->getGroupLeaderId() == actor)
			{
				size_t pos = 0;
				int const formationSlot = nextIntParm(params, pos);
				if (formationSlot > -1)
				{
					group->setShipFormationSlotForMember(target, formationSlot);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

static void commandFuncUnDock(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	Object * const object = NetworkIdManager::getObjectById(actor);

	if (object != nullptr)
	{
		ShipObject * const shipObject = ShipObject::getContainingShipObject(object->asServerObject());

		if (shipObject != nullptr)
		{
			ShipController * const shipController = dynamic_cast<ShipController *>(shipObject->getController());

			if (shipController != nullptr)
			{
				shipController->unDock();
			}
			else
			{
				WARNING(true, ("commandFuncUnDock() Undock requested on an object(%s) without a ShipController.", object->getDebugInformation().c_str()));
			}
		}
		else
		{
			WARNING(true, ("commandFuncUnDock() Undock requested on an object(%s) contained by a nullptr ShipObject.", object->getDebugInformation().c_str()));
		}
	}
	else
	{
		WARNING(true, ("commandFuncUnDock() Undock requested on a nullptr object(%s).", actor.getValueString().c_str()));
	}
}

//----------------------------------------------------------------------

static void commandFuncLaunchIntoSpace(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
	{
		WARNING(true, ("No actorCreature in commandFuncLaunchIntoSpace"));
		return;
	}

	if (!target.isValid())
	{
		StringId invalidTargetId("player_utility", "invalid_target");
		Chat::sendSystemMessage(*actorCreature, invalidTargetId, Unicode::emptyString);
		return;
	}

	//target must be a space terminal
	Object const * const o = NetworkIdManager::getObjectById(target);
	ServerObject const * const so = o ? o->asServerObject() : nullptr;
	if (!so)
	{
		StringId invalidTargetId("player_utility", "target_not_server_object");
		Chat::sendSystemMessage(*actorCreature, invalidTargetId, Unicode::emptyString);
		return;
	}
	if (so->getGameObjectType() != SharedObjectTemplate::GOT_terminal_space)
	{
		ProsePackage p;
		p.stringId = StringId("player_utility", "not_space_terminal");
		p.target.id = target;
		Unicode::String oob;
		OutOfBandPackager::pack(p, -1, oob);
		Chat::sendSystemMessage(*actorCreature, Unicode::emptyString, oob);;
		return;
	}

	Unicode::UnicodeStringVector tokens;
	Unicode::tokenize(params, tokens);

	if (tokens.empty())
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("(unlocalized) No parameters."), Unicode::emptyString);
		return;
	}

	NetworkId const shipPCDId(Unicode::wideToNarrow(tokens[0]).c_str());
	if (!shipPCDId.isValid())
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("(unlocalized) Invalid ship."), Unicode::emptyString);
		return;
	}
	Object * const terminalO = NetworkIdManager::getObjectById(target);
	ServerObject * const terminalSO = terminalO ? terminalO->asServerObject() : nullptr;
	if (terminalSO)
	{
		std::vector<NetworkId> networkIds;

		unsigned int tokenIndex = 1;

		if (tokens.size() > tokenIndex)
		{
			int const numberOfNetworkIds = atoi(Unicode::wideToNarrow(tokens[tokenIndex]).c_str());
			for (int i = 0; i < numberOfNetworkIds; ++i)
			{
				++tokenIndex;
				if (tokens.size() > tokenIndex)
				{
					NetworkId const Id(Unicode::wideToNarrow(tokens[tokenIndex]));
					if (Id.isValid())
					{
						networkIds.push_back(Id);
					}
				}
			}
		}

		++tokenIndex;

		//adding an empty string is fine (it means they aren't traveling to another ground starport)
		std::string strPointPlanetResult;
		if (tokens.size() > tokenIndex)
		{
			strPointPlanetResult = Unicode::wideToNarrow(tokens[tokenIndex]);
		}

		++tokenIndex;

		//adding an empty string is fine (it means they aren't traveling to another ground starport)
		std::string strPointLocationResult;
		if (tokens.size() > tokenIndex)
		{
			strPointLocationResult = Unicode::wideToNarrow(tokens[tokenIndex]);
			std::replace(strPointLocationResult.begin(), strPointLocationResult.end(), '_', ' ');
		}

		ScriptParams params;
		params.addParam(actor);
		params.addParam(shipPCDId);
		params.addParam(networkIds);
		//adding an empty string is fine (it means they aren't traveling to another ground starport)
		params.addParam(strPointPlanetResult.c_str());
		//adding an empty string is fine (it means they aren't traveling to another ground starport)
		params.addParam(strPointLocationResult.c_str());

		IGNORE_RETURN(terminalSO->getScriptObject()->trigAllScripts(Scripting::TRIG_ABOUT_TO_LAUNCH_TO_SPACE, params));
	}
	else
	{
		Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide("(unlocalized) Invalid terminal"), Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

static void commandFuncAcceptQuest(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
	{
		WARNING(true, ("No actorCreature in commandFuncAcceptQuest"));
		return;
	}
	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(actorCreature);
	if (!playerObject)
	{
		WARNING(true, ("No playerObject in commandFuncAcceptQuest"));
		return;
	}

	uint32 const questCrc = atoi(Unicode::wideToNarrow(params).c_str());
	if (playerObject->getPendingRequestQuestCrc() == questCrc)
	{
		NetworkId const & questGiver = playerObject->getPendingRequestQuestGiver();
		playerObject->questActivateQuest(questCrc, questGiver);
		playerObject->setPendingRequestQuestInformation(0, NetworkId::cms_invalid);
	}
}

//----------------------------------------------------------------------

static void commandFuncReceiveReward(Command const &, NetworkId const & actor, NetworkId const & target, Unicode::String const & params)
{
	ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
	{
		WARNING(true, ("No actorCreature in commandFuncReceiveReward"));
		return;
	}
	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(actorCreature);
	if (!playerObject)
	{
		WARNING(true, ("No playerObject in commandFuncReceiveReward"));
		return;
	}

	Unicode::UnicodeStringVector sv;
	Unicode::tokenize(params, sv);

	if (sv.empty())
		return;
	uint32 const questCrc = atoi(Unicode::wideToNarrow(sv[0]).c_str());
	std::string selectedReward;
	if (sv.size() > 1)
		selectedReward = Unicode::wideToNarrow(sv[1]);

	if ((playerObject->getPendingRequestQuestCrc() == questCrc) || playerObject->questPlayerCanClaimRewardFor(questCrc))
	{
		playerObject->questGrantQuestReward(questCrc, selectedReward);
		playerObject->setPendingRequestQuestInformation(0, NetworkId::cms_invalid);
	}
}

//----------------------------------------------------------------------

static void commandFuncAbandonQuest(Command const &, NetworkId const & actor, NetworkId const &, Unicode::String const & params)
{
	size_t pos = 0;

	//not currently used, but can distinguish between different quest types
	int const questSystemType = nextIntParm(params, pos);
	UNREF(questSystemType);
	std::string const & questName = nextStringParm(params, pos);

	if (QuestManager::isQuestAbandonable(questName))
	{
		ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
		CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;
		if (actorCreature)
		{
			PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(actorCreature);
			if (playerObject)
			{
				Quest const * const q = QuestManager::getQuest(questName);
				if (q && playerObject->questHasActiveQuest(q->getId()) && !playerObject->questHasCompletedQuest(q->getId()))
				{
					LOG("Quest", ("Abandon:%s abandoned quest [%s] with status [0x%08X].\n",
						PlayerObject::getAccountDescription(actorServerObj->getClient()->getCharacterObjectId()).c_str(),
						questName.c_str(), playerObject->questGetQuestStatus(q->getId())));

					playerObject->questClearQuest(q->getId());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncExchangeListCredits(Command const &, NetworkId const &actor, NetworkId const &target, Unicode::String const &params)
{
	ServerObject * const actorServerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(actor));
	CreatureObject * const actorCreature = actorServerObj != nullptr ? actorServerObj->asCreatureObject() : nullptr;
	if (!actorCreature)
	{
		WARNING(true, ("No actorCreature in commandFuncReceiveReward"));
		return;
	}
	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(actorCreature);
	if (!playerObject)
	{
		WARNING(true, ("No playerObject in commandFuncReceiveReward"));
		return;
	}

	Unicode::UnicodeStringVector sv;
	Unicode::tokenize(params, sv);

	if (sv.empty())
		return;
	uint32 const credits = (uint32)atoi(Unicode::wideToNarrow(sv[0]).c_str());
	if (credits <= 0)
		return;

	char message[512];
	sprintf(message, "Listing %ld credits on station exchange.", credits);
	Chat::sendSystemMessage(*actorCreature, Unicode::narrowToWide(message), Unicode::emptyString);

	ExchangeListCreditsMessage const msg(actor, credits, GameServer::getInstance().getProcessId());
	GameServer::getInstance().sendToCentralServer(msg);
}

//-----------------------------------------------------------------------

static void commandFuncSquelch(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
            if(!gmClient->isGod())
            {
                return;
            }
			char buffer[2048];
			if (!target.isValid())
			{
				ConGenericMessage const msg("Error! No target specified for /squelch command.", 0);
				gmClient->send(msg, true);
			}
			else
			{
				ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
				if (!so)
				{
					snprintf(buffer, sizeof(buffer) - 1, "Error! Cannot find object for %s", target.getValueString().c_str());
					buffer[sizeof(buffer) - 1] = '\0';
					ConGenericMessage const msg(buffer, 0);
					gmClient->send(msg, true);
				}
				else
				{
					CreatureObject * c = so->asCreatureObject();
					if (!c)
					{
						snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a creature object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
						buffer[sizeof(buffer) - 1] = '\0';
						ConGenericMessage const msg(buffer, 0);
						gmClient->send(msg, true);
					}
					else
					{
						PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
						if (!p)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a player object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else if (p->getSecondsUntilUnsquelched() < 0)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is already squelched", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else
						{
							snprintf(buffer, sizeof(buffer) - 1, "Squelching %s (%s)", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);

							p->squelch(gm->getNetworkId(), Unicode::wideToNarrow(gm->getAssignedObjectName()), -1);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncUnsquelch(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
            if(!gmClient->isGod())
            {
                return;
            }
			char buffer[2048];
			if (!target.isValid())
			{
				ConGenericMessage const msg("Error! No target specified for /unsquelch command.", 0);
				gmClient->send(msg, true);
			}
			else
			{
				ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
				if (!so)
				{
					snprintf(buffer, sizeof(buffer) - 1, "Error! Cannot find object for %s", target.getValueString().c_str());
					buffer[sizeof(buffer) - 1] = '\0';
					ConGenericMessage const msg(buffer, 0);
					gmClient->send(msg, true);
				}
				else
				{
					CreatureObject * c = so->asCreatureObject();
					if (!c)
					{
						snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a creature object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
						buffer[sizeof(buffer) - 1] = '\0';
						ConGenericMessage const msg(buffer, 0);
						gmClient->send(msg, true);
					}
					else
					{
						PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
						if (!p)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a player object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else if (p->getSecondsUntilUnsquelched() == 0)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not currently squelched", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else
						{
							snprintf(buffer, sizeof(buffer) - 1, "Unsquelching %s (%s)", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);

							p->unsquelch();
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncGrantWarden(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
		    if(!gmClient->isGod())
            {
		        return;
            }
			char buffer[2048];
			if (!target.isValid())
			{
				ConGenericMessage const msg("Error! No target specified for /grantWarden command.", 0);
				gmClient->send(msg, true);
			}
			else
			{
				ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
				if (!so)
				{
					snprintf(buffer, sizeof(buffer) - 1, "Error! Cannot find object for %s", target.getValueString().c_str());
					buffer[sizeof(buffer) - 1] = '\0';
					ConGenericMessage const msg(buffer, 0);
					gmClient->send(msg, true);
				}
				else
				{
					CreatureObject * c = so->asCreatureObject();
					if (!c)
					{
						snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a creature object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
						buffer[sizeof(buffer) - 1] = '\0';
						ConGenericMessage const msg(buffer, 0);
						gmClient->send(msg, true);
					}
					else
					{
						PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
						if (!p)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a player object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else if (p->isWarden())
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is already a warden", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else
						{
							snprintf(buffer, sizeof(buffer) - 1, "Granting warden to %s (%s)", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);

							p->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::Warden));

							LOG("CustomerService", ("Warden:%s grantWarden to %s", PlayerObject::getAccountDescription(gm).c_str(), p->getAccountDescription().c_str()));
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncRevokeWarden(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
            if(!gmClient->isGod())
            {
                return;
            }
			char buffer[2048];
			if (!target.isValid())
			{
				ConGenericMessage const msg("Error! No target specified for /revokeWarden command.", 0);
				gmClient->send(msg, true);
			}
			else
			{
				ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
				if (!so)
				{
					snprintf(buffer, sizeof(buffer) - 1, "Error! Cannot find object for %s", target.getValueString().c_str());
					buffer[sizeof(buffer) - 1] = '\0';
					ConGenericMessage const msg(buffer, 0);
					gmClient->send(msg, true);
				}
				else
				{
					CreatureObject * c = so->asCreatureObject();
					if (!c)
					{
						snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a creature object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
						buffer[sizeof(buffer) - 1] = '\0';
						ConGenericMessage const msg(buffer, 0);
						gmClient->send(msg, true);
					}
					else
					{
						PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
						if (!p)
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a player object", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else if (!p->isWarden())
						{
							snprintf(buffer, sizeof(buffer) - 1, "Error! %s (%s) is not a warden", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);
						}
						else
						{
							snprintf(buffer, sizeof(buffer) - 1, "Revoking warden from %s (%s)", target.getValueString().c_str(), Unicode::wideToNarrow(so->getAssignedObjectName()).c_str());
							buffer[sizeof(buffer) - 1] = '\0';
							ConGenericMessage const msg(buffer, 0);
							gmClient->send(msg, true);

							p->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer));

							LOG("CustomerService", ("Warden:%s revokeWarden from %s", PlayerObject::getAccountDescription(gm).c_str(), p->getAccountDescription().c_str()));
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncSpammer(Command const &, NetworkId const &actor, NetworkId const & targetForCommand, Unicode::String const &params)
{
	NetworkId target = targetForCommand;
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check warden permission first or if god
			const PlayerObject * gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
			if (!gmPlayerObject || (!gmPlayerObject->isWarden()) && !gmClient->isGod())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "not_authorized"), Unicode::emptyString);
				return;
			}

			// check to see if warden functionality is enabled but don't restrict if god
			if (!ConfigServerGame::getEnableWarden() && !gmClient->isGod())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "warden_functionality_disabled"), Unicode::emptyString);
				return;
			}

			// check for valid target
			NetworkId parsedNetworkId;
			if (!target.isValid())
			{
				// check to see if a target name was specified, and see
				// if that resolves into a player character in the galaxy
				if (params.empty())
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "no_target"), Unicode::emptyString);
					return;
				}
				else
				{
					parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
					if (!parsedNetworkId.isValid())
					{
						Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
						return;
					}
					else
					{
						target = parsedNetworkId;
					}
				}
			}

			ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
			if (!so)
			{
				if (parsedNetworkId.isValid())
				{
					// send a messageTo to the target to do apply the /spammer operation
					char buffer[1024];
					snprintf(buffer, sizeof(buffer) - 1, "%s %s", actor.getValueString().c_str(), Unicode::wideToNarrow(gm->getAssignedObjectName()).c_str());
					buffer[sizeof(buffer) - 1] = '\0';

					MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
						"C++SpammerReq",
						buffer,
						0,
						false,
						actor,
						"C++SpammerRspTargetNotFound");
				}
				else
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				}

				return;
			}

			CreatureObject * c = so->asCreatureObject();
			if (!c)
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				return;
			}

			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (!p)
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				return;
			}

			// on a session-authenticated cluster, cannot /spammer a secured login character
			if (so->getClient() && so->getClient()->isGodValidated())
			{
				if (!ConfigServerGame::getAdminGodToAll() && !ConfigServerGame::getEnableWardenCanSquelchSecuredLogin())
				{
					ProsePackage prosePackage;
					prosePackage.stringId = StringId("warden", "cannot_spammer_target");
					prosePackage.target.str = so->getAssignedObjectName();
					Chat::sendSystemMessage(*gm, prosePackage);
					return;
				}
			}

			// can't spammer someone in god mode or another warden
			if(so->getClient()->isGod() || p->isWarden())
            {
                ProsePackage prosePackage;
                prosePackage.stringId = StringId("warden", "cannot_spammer_target");
                prosePackage.target.str = so->getAssignedObjectName();
                Chat::sendSystemMessage(*gm, prosePackage);
                return;
            }

			// cannot /spammer self
			if (target == actor)
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "cannot_spammer_target");
				prosePackage.target.str = so->getAssignedObjectName();
				Chat::sendSystemMessage(*gm, prosePackage);
				return;
			}

			// cannot /spammer a target that is already squelched
			if (p->getSecondsUntilUnsquelched() != 0)
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "cannot_spammer_target");
				prosePackage.target.str = so->getAssignedObjectName();
				Chat::sendSystemMessage(*gm, prosePackage);
				return;
			}

			// do it
			p->squelch(actor, Unicode::wideToNarrow(gm->getAssignedObjectName()), ConfigServerGame::getWardenSquelchDurationSeconds());
			ReportManager::addReport(gm->getAssignedObjectFirstName(), actor, so->getAssignedObjectFirstName(), target, std::string("Warden"));

			// log it
			LOG("CustomerService", ("Warden:%s has spammer %s", PlayerObject::getAccountDescription(gm).c_str(), p->getAccountDescription().c_str()));

			// display confirmation message
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("warden", "confirm_spammer");
			prosePackage.target.str = so->getAssignedObjectName();
			Chat::sendSystemMessage(*gm, prosePackage);
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncUnspammer(Command const &, NetworkId const &actor, NetworkId const & targetForCommand, Unicode::String const &params)
{
	NetworkId target = targetForCommand;
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check warden permission first or if god
			const PlayerObject * gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
			if (!gmPlayerObject || (!gmPlayerObject->isWarden()) && !gmClient->isGod())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "not_authorized"), Unicode::emptyString);
				return;
			}

			// check for valid target
			NetworkId parsedNetworkId;
			if (!target.isValid())
			{
				// check to see if a target name was specified, and see
				// if that resolves into a player character in the galaxy
				if (params.empty())
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "no_target"), Unicode::emptyString);
					return;
				}
				else
				{
					parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
					if (!parsedNetworkId.isValid())
					{
						Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
						return;
					}
					else
					{
						target = parsedNetworkId;
					}
				}
			}

			ServerObject * so = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(target));
			if (!so)
			{
				if (parsedNetworkId.isValid())
				{
					// send a messageTo to the target to do apply the /unspammer operation
					MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
						"C++UnspammerReq",
						actor.getValueString(),
						0,
						false,
						actor,
						"C++UnspammerRspTargetNotFound");
				}
				else
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				}

				return;
			}

			CreatureObject * c = so->asCreatureObject();
			if (!c)
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				return;
			}

			PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
			if (!p)
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "invalid_target"), Unicode::emptyString);
				return;
			}

			// cannot /unspammer a target that is not currently /spammer(ed)
			if (p->getSecondsUntilUnsquelched() <= 0)
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "cannot_unspammer_target");
				prosePackage.target.str = so->getAssignedObjectName();
				Chat::sendSystemMessage(*gm, prosePackage);
				return;
			}

			// can only /unspammer target that I /spammer(ed) unless I'm god
			if (p->getSquelchedById() != actor && !gmClient->isGod())
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("warden", "cannot_unspammer_target");
				prosePackage.target.str = so->getAssignedObjectName();
				Chat::sendSystemMessage(*gm, prosePackage);
				return;
			}

			// do it
			p->unsquelch();

			// log it
			LOG("CustomerService", ("Warden:%s has unspammer %s", PlayerObject::getAccountDescription(gm).c_str(), p->getAccountDescription().c_str()));

			// display confirmation message
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("warden", "confirm_unspammer");
			prosePackage.target.str = so->getAssignedObjectName();
			Chat::sendSystemMessage(*gm, prosePackage);
		}
	}
}

//-----------------------------------------------------------------------

/**
 * @deprecated This ungodly idea to empower wardens to grant warden permissions to other players is horrible
 * and I don't believe this functionality should remotely exist at all absent intentional implementation
 * by a specific server. GMs can use /grantWarden if they want a Warden or the grant can be scripted.
 *
 * Aconite - SWG Source - 2021
 */
static void commandFuncDeputizeWarden(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
    /*
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check warden permission first
			const PlayerObject * gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
			if (!gmPlayerObject || !gmPlayerObject->isWarden())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "not_authorized"), Unicode::emptyString);
				return;
			}

			// check to see if warden functionality is enabled
			if (!ConfigServerGame::getEnableWarden())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "warden_functionality_disabled"), Unicode::emptyString);
				return;
			}

			// check to see if wardens are allowed to deputize other wardens
			if (!ConfigServerGame::getEnableWardenToDeputizeWarden())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "not_authorized"), Unicode::emptyString);
				return;
			}

			// check for valid target
			NetworkId parsedNetworkId = target;
			if (!parsedNetworkId.isValid())
			{
				// check to see if a target name was specified, and see
				// if that resolves into a player character in the galaxy
				if (params.empty())
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "cannot_deputize"), Unicode::emptyString);
					return;
				}
				else
				{
					parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
					if (!parsedNetworkId.isValid())
					{
						Chat::sendSystemMessage(*gm, StringId("warden", "cannot_deputize"), Unicode::emptyString);
						return;
					}
				}
			}

			// cannot deputize self
			if (parsedNetworkId == gm->getNetworkId())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "cannot_deputize"), Unicode::emptyString);
				return;
			}

			// send a messageTo to the target to complete the deputize process
			char buffer[1024];
			snprintf(buffer, sizeof(buffer) - 1, "%s|%lu", gm->getNetworkId().getValueString().c_str(), gmPlayerObject->getStationId());
			buffer[sizeof(buffer) - 1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
				"C++DeputizeWardenReq",
				buffer,
				0,
				false,
				actor,
				"C++DeputizeWardenRspCannotDeputize");
		}
	}
    */
}

//-----------------------------------------------------------------------

/**
 * @deprecated
 * @see commandFuncDeputizeWarden
 */
static void commandFuncUndeputizeWarden(Command const &, NetworkId const &actor, NetworkId const & target, Unicode::String const &params)
{
    /*
	const CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (gm)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check warden permission first
			const PlayerObject * gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
			if (!gmPlayerObject || !gmPlayerObject->isWarden())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "not_authorized"), Unicode::emptyString);
				return;
			}

			// check for valid target
			NetworkId parsedNetworkId = target;
			if (!parsedNetworkId.isValid())
			{
				// check to see if a target name was specified, and see
				// if that resolves into a player character in the galaxy
				if (params.empty())
				{
					Chat::sendSystemMessage(*gm, StringId("warden", "cannot_undeputize"), Unicode::emptyString);
					return;
				}
				else
				{
					parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
					if (!parsedNetworkId.isValid())
					{
						Chat::sendSystemMessage(*gm, StringId("warden", "cannot_undeputize"), Unicode::emptyString);
						return;
					}
				}
			}

			// cannot undeputize self
			if (parsedNetworkId == gm->getNetworkId())
			{
				Chat::sendSystemMessage(*gm, StringId("warden", "cannot_undeputize"), Unicode::emptyString);
				return;
			}

			// send a messageTo to the target to complete the undeputize process
			char buffer[1024];
			snprintf(buffer, sizeof(buffer) - 1, "%s|%lu", gm->getNetworkId().getValueString().c_str(), gmPlayerObject->getStationId());
			buffer[sizeof(buffer) - 1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(parsedNetworkId,
				"C++UndeputizeWardenReq",
				buffer,
				0,
				false,
				actor,
				"C++UndeputizeWardenRspCannotUndeputize");
		}
	}
    */
}

//-----------------------------------------------------------------------

static void commandFuncRemoveBuff(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject * player = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	if (player)
	{
		size_t pos = 0;
		int32 const buffCrc = nextIntParm(params, pos);
		if (buffCrc && BuffManager::getIsBuffPlayerRemovable(buffCrc))
		{
			player->removeBuff(buffCrc);
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncOccupyUnlockedSlot(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	PlayerObject const * const gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
	if (gm && gmPlayerObject)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check to see if /occupyUnlockedSlot is enabled
			if (!ConfigServerGame::getEnableOccupyUnlockedSlotCommand())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "occupy_unlocked_slot_command_disabled"), Unicode::emptyString);
				return;
			}

			// check to see if the account currently has an unoccupied unlocked slot
			if (!gmClient->getHasUnoccupiedJediSlot())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "occupy_unlocked_slot_command_account_has_no_unoccupied_unlocked_slot"), Unicode::emptyString);
				return;
			}

			// already unlocked slot character
			if (gmClient->getIsJediSlotCharacter())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "occupy_unlocked_slot_command_already_unlocked_character"), Unicode::emptyString);
				return;
			}

			// check to see if there is a cooldown is in effect for the command
			int const timeNow = static_cast<int>(::time(nullptr));
			if (!gmClient->isGod() && gm->getObjVars().hasItem("timeNextOccupyVacateUnlockedSlotCommandAllowed") && (gm->getObjVars().getType("timeNextOccupyVacateUnlockedSlotCommandAllowed") == DynamicVariable::INT))
			{
				int timeNextOccupyVacateUnlockedSlotCommandAllowed = 0;
				if (gm->getObjVars().getItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", timeNextOccupyVacateUnlockedSlotCommandAllowed))
				{
					if (timeNextOccupyVacateUnlockedSlotCommandAllowed > timeNow)
					{
						ProsePackage prosePackage;
						prosePackage.stringId = StringId("unlocked_slot", "occupy_unlocked_slot_command_too_fast");
						prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(timeNextOccupyVacateUnlockedSlotCommandAllowed - timeNow)));
						Chat::sendSystemMessage(*gm, prosePackage);

						return;
					}
				}
			}

			// CS log the request
			LOG("CustomerService", ("JediUnlockedSlot:%s requesting OccupyUnlockedSlot", PlayerObject::getAccountDescription(gm).c_str()));

			// send off request to LoginServer to make this character occupy the unlocked slot
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, uint32> > const occupyUnlockedSlotReq("OccupyUnlockedSlotReq", std::make_pair(std::make_pair(static_cast<uint32>(gmPlayerObject->getStationId()), actor), GameServer::getInstance().getProcessId()));
			GameServer::getInstance().sendToCentralServer(occupyUnlockedSlotReq);

			// set cooldown
			if (!gmClient->isGod())
			{
				IGNORE_RETURN(gm->setObjVarItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", (timeNow + ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			}

			// display "wait for reply" message
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("unlocked_slot", "occupy_unlocked_slot_wait_for_reply");
			prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			Chat::sendSystemMessage(*gm, prosePackage);
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncVacateUnlockedSlot(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	PlayerObject const * const gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
	if (gm && gmPlayerObject)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check to see if /vacateUnlockedSlot is enabled
			if (!ConfigServerGame::getEnableVacateUnlockedSlotCommand())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "vacate_unlocked_slot_command_disabled"), Unicode::emptyString);
				return;
			}

			// check to see if this character is an unlocked slot character
			if (!gmClient->getIsJediSlotCharacter())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "vacate_unlocked_slot_command_not_unlocked_character"), Unicode::emptyString);
				return;
			}

			// check to see if there is a cooldown is in effect for the command
			int const timeNow = static_cast<int>(::time(nullptr));
			if (!gmClient->isGod() && gm->getObjVars().hasItem("timeNextOccupyVacateUnlockedSlotCommandAllowed") && (gm->getObjVars().getType("timeNextOccupyVacateUnlockedSlotCommandAllowed") == DynamicVariable::INT))
			{
				int timeNextOccupyVacateUnlockedSlotCommandAllowed = 0;
				if (gm->getObjVars().getItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", timeNextOccupyVacateUnlockedSlotCommandAllowed))
				{
					if (timeNextOccupyVacateUnlockedSlotCommandAllowed > timeNow)
					{
						ProsePackage prosePackage;
						prosePackage.stringId = StringId("unlocked_slot", "vacate_unlocked_slot_command_too_fast");
						prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(timeNextOccupyVacateUnlockedSlotCommandAllowed - timeNow)));
						Chat::sendSystemMessage(*gm, prosePackage);

						return;
					}
				}
			}

			// CS log the request
			LOG("CustomerService", ("JediUnlockedSlot:%s requesting VacateUnlockedSlot", PlayerObject::getAccountDescription(gm).c_str()));

			// send off request to LoginServer to make this character vacate the unlocked slot
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, uint32> > const vacateUnlockedSlotReq("VacateUnlockedSlotReq", std::make_pair(std::make_pair(static_cast<uint32>(gmPlayerObject->getStationId()), actor), GameServer::getInstance().getProcessId()));
			GameServer::getInstance().sendToCentralServer(vacateUnlockedSlotReq);

			// set cooldown
			if (!gmClient->isGod())
			{
				IGNORE_RETURN(gm->setObjVarItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", (timeNow + ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			}

			// display "wait for reply" message
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("unlocked_slot", "vacate_unlocked_slot_wait_for_reply");
			prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			Chat::sendSystemMessage(*gm, prosePackage);
		}
	}
}

//-----------------------------------------------------------------------

static void commandFuncSwapUnlockedSlot(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject * gm = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(actor));
	PlayerObject const * const gmPlayerObject = PlayerCreatureController::getPlayerObject(gm);
	if (gm && gmPlayerObject)
	{
		const Client * gmClient = gm->getClient();
		if (gmClient)
		{
			// check to see if /swapUnlockedSlot is enabled
			if (!ConfigServerGame::getEnableSwapUnlockedSlotCommand())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "swap_unlocked_slot_command_disabled"), Unicode::emptyString);
				return;
			}

			// check to see if this character is an unlocked slot character
			if (!gmClient->getIsJediSlotCharacter())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "swap_unlocked_slot_command_not_unlocked_character"), Unicode::emptyString);
				return;
			}

			// check to see if a target character name is specified
			if (params.empty())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "swap_unlocked_slot_command_no_params"), Unicode::emptyString);
				return;
			}

			// check to see if a valid target character name is specified
			NetworkId const parsedNetworkId = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(params)));
			if (!parsedNetworkId.isValid())
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "swap_unlocked_slot_command_invalid_target"), Unicode::emptyString);
				return;
			}

			// cannot swap with yourself
			if (parsedNetworkId == actor)
			{
				Chat::sendSystemMessage(*gm, StringId("unlocked_slot", "swap_unlocked_slot_command_target_is_already_unlocked_character"), Unicode::emptyString);
				return;
			}

			// check to see if there is a cooldown is in effect for the command
			int const timeNow = static_cast<int>(::time(nullptr));
			if (!gmClient->isGod() && gm->getObjVars().hasItem("timeNextOccupyVacateUnlockedSlotCommandAllowed") && (gm->getObjVars().getType("timeNextOccupyVacateUnlockedSlotCommandAllowed") == DynamicVariable::INT))
			{
				int timeNextOccupyVacateUnlockedSlotCommandAllowed = 0;
				if (gm->getObjVars().getItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", timeNextOccupyVacateUnlockedSlotCommandAllowed))
				{
					if (timeNextOccupyVacateUnlockedSlotCommandAllowed > timeNow)
					{
						ProsePackage prosePackage;
						prosePackage.stringId = StringId("unlocked_slot", "swap_unlocked_slot_command_too_fast");
						prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(timeNextOccupyVacateUnlockedSlotCommandAllowed - timeNow)));
						Chat::sendSystemMessage(*gm, prosePackage);

						return;
					}
				}
			}

			// CS log the request
			LOG("CustomerService", ("JediUnlockedSlot:%s requesting SwapUnlockedSlot with %s (%s)", PlayerObject::getAccountDescription(gm).c_str(), parsedNetworkId.getValueString().c_str(), NameManager::getInstance().getPlayerFullName(parsedNetworkId).c_str()));

			// send off request to LoginServer to make this character normal and the target characer unlocked
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, std::pair<uint32, NetworkId> > > const swapUnlockedSlotReq("SwapUnlockedSlotReq", std::make_pair(std::make_pair(static_cast<uint32>(gmPlayerObject->getStationId()), actor), std::make_pair(GameServer::getInstance().getProcessId(), parsedNetworkId)));
			GameServer::getInstance().sendToCentralServer(swapUnlockedSlotReq);

			// set cooldown
			if (!gmClient->isGod())
			{
				IGNORE_RETURN(gm->setObjVarItem("timeNextOccupyVacateUnlockedSlotCommandAllowed", (timeNow + ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			}

			// display "wait for reply" message
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("unlocked_slot", "swap_unlocked_slot_wait_for_reply");
			prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds())));
			Chat::sendSystemMessage(*gm, prosePackage);
		}
	}
}

// ----------------------------------------------------------------------

static void commandFuncPickupAllRoomItemsIntoInventory(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	// don't allow this in god mode because it could be used accidentally and pick up a bunch of stuff,
	// and god mode allows stuff to be picked up, which wouldn't normally be allowed to be picked up
	if (!targetObj->getClient())
		return;

	if (targetObj->getClient()->isGod())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You cannot do this while in GOD mode."), Unicode::emptyString);
		return;
	}

	// don't do this if the character is not "active", meaning he's probably doing it with a macro
	if (targetPlayerObj->getSessionLastActiveTime() == 0)
		return;

	// don't allow multiple /pickupAllRoomItemsIntoInventory and/or /dropAllInventoryItemsIntoRoom
	int const timeNow = static_cast<int>(::time(nullptr));
	int pickupDropAllItemsOperationTimeout = 0;
	if (targetObj->getObjVars().getItem("pickupDropAllItemsOperation.timeout", pickupDropAllItemsOperationTimeout) && (pickupDropAllItemsOperationTimeout > timeNow))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are already performing the /pickupAllRoomItemsIntoInventory or /dropAllInventoryItemsIntoRoom command.  Please wait for the current operation to complete, or if the current operation does not complete in %s, you can try another operation.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(pickupDropAllItemsOperationTimeout - timeNow)).c_str())), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*targetObj));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (!containingPOB->isOnAdminList(*targetObj))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const targetInventoryObj = targetObj->getInventory();
	if (!targetInventoryObj)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	VolumeContainer const * const targetInventoryContainer = ContainerInterface::getVolumeContainer(*targetInventoryObj);
	if (!targetInventoryContainer)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// see if the player specified to overload the inventory
	Unicode::String const lowercaseParams = Unicode::toLower(params);
	static Unicode::String const paramsOverload1 = Unicode::narrowToWide("-overload");
	static Unicode::String const paramsOverload2 = Unicode::narrowToWide("overload");
	bool const allowInventoryOverload = ((lowercaseParams == paramsOverload1) || (lowercaseParams == paramsOverload2));

	int const inventoryBaseLimit = targetInventoryContainer->getTotalVolume();
	int const inventoryOverLimit = std::max(inventoryBaseLimit, (inventoryBaseLimit + ConfigServerGame::getMoveValidationMaxInventoryOverload() - 11));
	int const inventoryLimit = (allowInventoryOverload ? inventoryOverLimit : inventoryBaseLimit);

	if (targetInventoryContainer->getCurrentVolume() >= inventoryLimit)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Your inventory is full."), Unicode::emptyString);

		// if they didn't run the command with the -overload
		// option, remind them about the -overload option
		if (!allowInventoryOverload && (targetInventoryContainer->getCurrentVolume() < inventoryOverLimit))
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("If you wish to pick up more items than available inventory space, you can specify the \"overload\" option with the command by typing \"/pickupAllRoomItemsIntoInventory -overload\" which will pick up additional items and overload your inventory, but not so overloaded as to prevent you from moving."), Unicode::emptyString);

		return;
	}

	Container const * const cellContainer = ContainerInterface::getContainer(*containingCell);
	if (!cellContainer)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// the player counts as 1 item in the room
	if (cellContainer->getNumberOfItems() <= 1)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("This room is empty."), Unicode::emptyString);
		return;
	}

	// start messageTo loop to pick up the items in the room, spread
	// out over a period of time so it doesn't lag out the server
	int pickupDropAllItemsOperationId = ::rand();
	while (pickupDropAllItemsOperationId == 0)
		pickupDropAllItemsOperationId = ::rand();

	targetPlayerObj->setPickupDropAllItemsOperationId(pickupDropAllItemsOperationId);
	targetPlayerObj->setPickupDropAllItemsCellId(containingCell->getNetworkId());
	targetPlayerObj->setPickupAllItemsAllowInventoryOverload(allowInventoryOverload);
	targetPlayerObj->setPickupDropAllItemsNumItemsSoFar(0);
	targetPlayerObj->getPickupDropAllItemsProcessedItemsSoFar().clear();

	IGNORE_RETURN(targetObj->setObjVarItem("pickupDropAllItemsOperation.timeout", timeNow + (5 * 60)));
	IGNORE_RETURN(targetObj->setObjVarItem("pickupDropAllItemsOperation.operationId", pickupDropAllItemsOperationId));

	Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please remain in the room while the operation is being performed.  You will be notified when the operation is complete."), Unicode::emptyString);

	MessageToQueue::getInstance().sendMessageToC(targetObj->getNetworkId(),
		"C++PickupAllRoomItemsIntoInventory",
		"",
		0,
		false);
}

// ----------------------------------------------------------------------

static void commandFuncDropAllInventoryItemsIntoRoom(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	// don't allow this in god mode because it could be used accidentally and drop a bunch of stuff,
	// and god mode allows stuff to be dropped, which wouldn't normally be allowed to be dropped
	if (!targetObj->getClient())
		return;

	if (targetObj->getClient()->isGod())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You cannot do this while in GOD mode."), Unicode::emptyString);
		return;
	}

	// don't do this if the character is not "active", meaning he's probably doing it with a macro
	if (targetPlayerObj->getSessionLastActiveTime() == 0)
		return;

	// don't allow multiple /pickupAllRoomItemsIntoInventory and/or /dropAllInventoryItemsIntoRoom
	int const timeNow = static_cast<int>(::time(nullptr));
	int pickupDropAllItemsOperationTimeout = 0;
	if (targetObj->getObjVars().getItem("pickupDropAllItemsOperation.timeout", pickupDropAllItemsOperationTimeout) && (pickupDropAllItemsOperationTimeout > timeNow))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are already performing the /pickupAllRoomItemsIntoInventory or /dropAllInventoryItemsIntoRoom command.  Please wait for the current operation to complete, or if the current operation does not complete in %s, you can try another operation.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(pickupDropAllItemsOperationTimeout - timeNow)).c_str())), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*targetObj));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// check for lot overlimit drop restriction
	if ((targetPlayerObj->getAccountNumLots() > targetObj->getMaxNumberOfLots()) && targetObj->getObjVars().hasItem("lotOverlimit.structure_id"))
	{
		NetworkId lotOverlimitStructure;
		if (targetObj->getObjVars().getItem("lotOverlimit.structure_id", lotOverlimitStructure) && lotOverlimitStructure.isValid())
		{
			if (GameObjectTypes::isTypeOf(containingPOB->getGameObjectType(), SharedObjectTemplate::GOT_building) && (containingPOB->getNetworkId() != lotOverlimitStructure))
			{
				std::string lotOverlimitStructureName;
				std::string lotOverlimitStructureLocation;

				IGNORE_RETURN(targetObj->getObjVars().getItem("lotOverlimit.structure_name", lotOverlimitStructureName));
				IGNORE_RETURN(targetObj->getObjVars().getItem("lotOverlimit.structure_location", lotOverlimitStructureLocation));

				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are currently over the lot limit, and cannot drop any item into (or move around items within) any factory or structure, other than the %s structure located at %s that caused you to exceed the lot limit.", lotOverlimitStructureName.c_str(), lotOverlimitStructureLocation.c_str())), Unicode::emptyString);
				return;
			}
		}
	}

	if (!containingPOB->isOnAdminList(*targetObj))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const targetInventoryObj = targetObj->getInventory();
	if (!targetInventoryObj)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	VolumeContainer const * const targetInventoryContainer = ContainerInterface::getVolumeContainer(*targetInventoryObj);
	if (!targetInventoryContainer)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	int pobBaseItemLimit = 0;
	int pobAdditionalItems = 0;
	const int pobItemLimit = containingPOB->getPobTotalItemLimit(pobBaseItemLimit, pobAdditionalItems);

	if (containingPOB->getPobItemCount() >= pobItemLimit)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("The structure or POB ship is full."), Unicode::emptyString);
		return;
	}

	if (targetInventoryContainer->getCurrentVolume() <= 0)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Your inventory  is empty."), Unicode::emptyString);
		return;
	}

	// start messageTo loop to drop the inventory items into the room,
	// spread out over a period of time so it doesn't lag out the server
	int pickupDropAllItemsOperationId = ::rand();
	while (pickupDropAllItemsOperationId == 0)
		pickupDropAllItemsOperationId = ::rand();

	targetPlayerObj->setPickupDropAllItemsOperationId(pickupDropAllItemsOperationId);
	targetPlayerObj->setPickupDropAllItemsCellId(containingCell->getNetworkId());
	targetPlayerObj->setPickupDropAllItemsNumItemsSoFar(0);
	targetPlayerObj->getPickupDropAllItemsProcessedItemsSoFar().clear();

	IGNORE_RETURN(targetObj->setObjVarItem("pickupDropAllItemsOperation.timeout", timeNow + (5 * 60)));
	IGNORE_RETURN(targetObj->setObjVarItem("pickupDropAllItemsOperation.operationId", pickupDropAllItemsOperationId));

	Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please remain in the room while the operation is being performed.  You will be notified when the operation is complete."), Unicode::emptyString);

	MessageToQueue::getInstance().sendMessageToC(targetObj->getNetworkId(),
		"C++DropAllInventoryItemsIntoRoom",
		"",
		0,
		false);
}

// ----------------------------------------------------------------------

static void commandFuncSaveDecorationLayout(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject const * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*targetObj));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	if (!containingPOB->isOnAdminList(*targetObj))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// poke script to pop up some SUI windows to let the user pick which save slot to use
	MessageToQueue::getInstance().sendMessageToJava(targetObj->getNetworkId(), "handleSaveDecorationLayout", std::vector<int8>(), 0, false);
}

// ----------------------------------------------------------------------

static void commandFuncRestoreDecorationLayout(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const &)
{
	CreatureObject const * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*targetObj));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	ServerObject const * const containingPOBso = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingCell));
	if (!containingPOBso)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	if (!containingPOBso->isInWorldCell())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// don't allow another restore on the pob while one is still in progress
	int const timeNow = static_cast<int>(::time(nullptr));
	int restoreDecorationOperationTimeout = 0;
	if (targetObj->getObjVars().getItem("restoreDecorationOperation.timeout", restoreDecorationOperationTimeout) && (restoreDecorationOperationTimeout > timeNow))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are already performing a restore decoration layout operation.  Please wait for the current operation to complete, or if the current operation does not complete in %s, you can try another operation.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(restoreDecorationOperationTimeout - timeNow)).c_str())), Unicode::emptyString);
		return;
	}

	if (containingPOBso->asBuildingObject() && !containingPOBso->asBuildingObject()->getContentsLoaded())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Please wait until the building has loaded its contents and try again."), Unicode::emptyString);
		return;
	}

	TangibleObject const * const containingPOB = containingPOBso->asTangibleObject();
	if (!containingPOB)
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("An internal error has been encountered."), Unicode::emptyString);
		return;
	}

	// check for lot overlimit drop restriction
	if ((targetPlayerObj->getAccountNumLots() > targetObj->getMaxNumberOfLots()) && targetObj->getObjVars().hasItem("lotOverlimit.structure_id"))
	{
		NetworkId lotOverlimitStructure;
		if (targetObj->getObjVars().getItem("lotOverlimit.structure_id", lotOverlimitStructure) && lotOverlimitStructure.isValid())
		{
			if (GameObjectTypes::isTypeOf(containingPOB->getGameObjectType(), SharedObjectTemplate::GOT_building) && (containingPOB->getNetworkId() != lotOverlimitStructure))
			{
				std::string lotOverlimitStructureName;
				std::string lotOverlimitStructureLocation;

				IGNORE_RETURN(targetObj->getObjVars().getItem("lotOverlimit.structure_name", lotOverlimitStructureName));
				IGNORE_RETURN(targetObj->getObjVars().getItem("lotOverlimit.structure_location", lotOverlimitStructureLocation));

				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You are currently over the lot limit, and cannot drop any item into (or move around items within) any factory or structure, other than the %s structure located at %s that caused you to exceed the lot limit.", lotOverlimitStructureName.c_str(), lotOverlimitStructureLocation.c_str())), Unicode::emptyString);
				return;
			}
		}
	}

	if (!containingPOB->isOnAdminList(*targetObj))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be the owner or an admin of the structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// see if there's any saved decoration layout to restore
	if (!targetPlayerObj->getObjVars().hasItem("savedDecoration1.saveTime") && !targetPlayerObj->getObjVars().hasItem("savedDecoration2.saveTime") && !targetPlayerObj->getObjVars().hasItem("savedDecoration3.saveTime"))
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You do not have any saved decoration layout."), Unicode::emptyString);
		return;
	}

	// poke script to pop up some SUI windows to let the user pick which save slot to use
	MessageToQueue::getInstance().sendMessageToJava(targetObj->getNetworkId(), "handleRestoreDecorationLayout", std::vector<int8>(), 0, false);
}

// ----------------------------------------------------------------------

static void commandFuncAreaPickRandomPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextAreaPickRandomPlayerCommandAllowed") && (targetObj->getObjVars().getType("timeNextAreaPickRandomPlayerCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextAreaPickRandomPlayerCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextAreaPickRandomPlayerCommandAllowed", timeNextAreaPickRandomPlayerCommandAllowed))
		{
			if (timeNextAreaPickRandomPlayerCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextAreaPickRandomPlayerCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	// handle options
	static uint32 const pointatSocialType = SocialsManager::getSocialTypeByName("pointat");
	uint32 socialType = 0;
	int range = 16; // default is 16m

	if (!params.empty())
	{
		Unicode::UnicodeStringVector tokens;
		if (Unicode::tokenize(Unicode::toLower(params), tokens, nullptr, nullptr) && !tokens.empty())
		{
			static Unicode::String const paramsPoint1 = Unicode::narrowToWide("-point");
			static Unicode::String const paramsPoint2 = Unicode::narrowToWide("point");
			static Unicode::String const paramsEmote1 = Unicode::narrowToWide("-emote=");
			static Unicode::String const paramsEmote2 = Unicode::narrowToWide("emote=");
			std::string emote;

			for (Unicode::UnicodeStringVector::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
			{
				if ((*iter == paramsPoint1) || (*iter == paramsPoint2))
				{
					socialType = pointatSocialType;
				}
				else if (iter->find(paramsEmote1) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(7);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else if (iter->find(paramsEmote2) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(6);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else
				{
					range = ::atoi(Unicode::wideToNarrow(*iter).c_str());
					if ((range < 1) || (range > 256))
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("Range must be 1-256."), Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	std::vector<ServerObject *> playersInRange;
	ServerWorld::findPlayerCreaturesInRange(targetObj->getPosition_w(), static_cast<float>(range), playersInRange);
	if (playersInRange.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("No eligible players found within %dm.", range)), Unicode::emptyString);
	}
	else
	{
		std::vector<ServerObject const *> validPlayers;
		CreatureObject const * co;
		for (std::vector<ServerObject *>::const_iterator iter = playersInRange.begin(); iter != playersInRange.end(); ++iter)
		{
			co = (*iter)->asCreatureObject();
			if (!co || !PlayerCreatureController::getPlayerObject(co))
				continue;

			// always include self, even if I am invisible
			if (co->getNetworkId() == targetObj->getNetworkId())
			{
				validPlayers.push_back(*iter);
				continue;
			}

			// don't include invisible players
			if (co->isHidden() || !co->isVisible() || !co->getCoverVisibility())
				continue;

			validPlayers.push_back(*iter);
		}

		if (validPlayers.empty())
		{
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("No eligible players found within %dm.", range)), Unicode::emptyString);
		}
		else
		{
			int const random = ::rand() % validPlayers.size();
			Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("Player \"%s\" within %dm was randomly chosen.", Unicode::wideToNarrow(validPlayers[random]->getAssignedObjectName()).c_str(), range)), Unicode::emptyString);

			// do emote at the selected player, if requested
			if (socialType > 0)
			{
				// if I'm the one selected, use the /pointat social to point at myself
				// since other socials may not make sense when I am the target
				doSocial(*targetObj, validPlayers[random]->getNetworkId(), ((validPlayers[random]->getNetworkId() == targetObj->getNetworkId()) ? pointatSocialType : socialType));
			}
		}
	}

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextAreaPickRandomPlayerCommandAllowed", (timeNow + 30)));
}

// ----------------------------------------------------------------------

static void commandFuncRoomPickRandomPlayer(Command const &, NetworkId const &actor, NetworkId const &, Unicode::String const & params)
{
	CreatureObject * const targetObj = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(actor));
	if (!targetObj)
		return;

	PlayerObject const * const targetPlayerObj = PlayerCreatureController::getPlayerObject(targetObj);
	if (!targetPlayerObj)
		return;

	ServerObject const * const containingCell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*targetObj));
	if (!containingCell || !containingCell->asCellObject())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("You must be standing inside a room of a structure or POB ship to do this."), Unicode::emptyString);
		return;
	}

	// prevent spamming of the command
	Client const * const client = targetObj->getClient();
	if (!client)
		return;

	int const timeNow = static_cast<int>(::time(nullptr));
	if (!client->isGod() && targetObj->getObjVars().hasItem("timeNextRoomPickRandomPlayerCommandAllowed") && (targetObj->getObjVars().getType("timeNextRoomPickRandomPlayerCommandAllowed") == DynamicVariable::INT))
	{
		int timeNextRoomPickRandomPlayerCommandAllowed = 0;
		if (targetObj->getObjVars().getItem("timeNextRoomPickRandomPlayerCommandAllowed", timeNextRoomPickRandomPlayerCommandAllowed))
		{
			if (timeNextRoomPickRandomPlayerCommandAllowed > timeNow)
			{
				Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("You must wait %ds before you can run this command again.", (timeNextRoomPickRandomPlayerCommandAllowed - timeNow))), Unicode::emptyString);
				return;
			}
		}
	}

	// handle options
	static uint32 const pointatSocialType = SocialsManager::getSocialTypeByName("pointat");
	uint32 socialType = 0;

	if (!params.empty())
	{
		Unicode::UnicodeStringVector tokens;
		if (Unicode::tokenize(Unicode::toLower(params), tokens, nullptr, nullptr) && !tokens.empty())
		{
			static Unicode::String const paramsPoint1 = Unicode::narrowToWide("-point");
			static Unicode::String const paramsPoint2 = Unicode::narrowToWide("point");
			static Unicode::String const paramsEmote1 = Unicode::narrowToWide("-emote=");
			static Unicode::String const paramsEmote2 = Unicode::narrowToWide("emote=");
			std::string emote;

			for (Unicode::UnicodeStringVector::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
			{
				if ((*iter == paramsPoint1) || (*iter == paramsPoint2))
				{
					socialType = pointatSocialType;
				}
				else if (iter->find(paramsEmote1) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(7);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
				else if (iter->find(paramsEmote2) == 0)
				{
					emote = Unicode::wideToNarrow(*iter).substr(6);
					socialType = SocialsManager::getSocialTypeByName(emote);
					if (socialType == 0)
					{
						Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("\"%s\" is not a valid emote.", emote.c_str())), Unicode::emptyString);
						return;
					}
				}
			}
		}
	}

	std::vector<ServerObject const *> validPlayers;
	Container const * const cellContainer = ContainerInterface::getContainer(*containingCell);
	if (cellContainer)
	{
		Object const * object;
		ServerObject const * serverObject;
		CreatureObject const * creatureObject;
		for (ContainerConstIterator iter = cellContainer->begin(); iter != cellContainer->end(); ++iter)
		{
			object = (*iter).getObject();
			if (!object)
				continue;

			serverObject = object->asServerObject();
			if (!serverObject)
				continue;

			creatureObject = serverObject->asCreatureObject();
			if (!creatureObject || !PlayerCreatureController::getPlayerObject(creatureObject))
				continue;

			// always include self, even if I am invisible
			if (creatureObject->getNetworkId() == targetObj->getNetworkId())
			{
				validPlayers.push_back(serverObject);
				continue;
			}

			// don't include invisible players
			if (creatureObject->isHidden() || !creatureObject->isVisible() || !creatureObject->getCoverVisibility())
				continue;

			validPlayers.push_back(serverObject);
		}
	}

	if (validPlayers.empty())
	{
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide("No eligible players found in this room."), Unicode::emptyString);
	}
	else
	{
		int const random = ::rand() % validPlayers.size();
		Chat::sendSystemMessage(*targetObj, Unicode::narrowToWide(FormattedString<1024>().sprintf("Player \"%s\" was randomly chosen out of %d players in this room.", Unicode::wideToNarrow(validPlayers[random]->getAssignedObjectName()).c_str(), validPlayers.size())), Unicode::emptyString);

		// do emote at the selected player, if requested
		if (socialType > 0)
		{
			// if I'm the one selected, use the /pointat social to point at myself
			// since other socials may not make sense when I am the target
			doSocial(*targetObj, validPlayers[random]->getNetworkId(), ((validPlayers[random]->getNetworkId() == targetObj->getNetworkId()) ? pointatSocialType : socialType));
		}
	}

	// set cooldown
	if (!client->isGod())
		IGNORE_RETURN(targetObj->setObjVarItem("timeNextRoomPickRandomPlayerCommandAllowed", (timeNow + 30)));
}

// ======================================================================

void CommandCppFuncs::install()
{
	// console style commands
	CommandTable::addCppFunction("console_all", commandFuncConsoleAll);
	CommandTable::addCppFunction("console_combat", commandFuncConsoleCombat);
	CommandTable::addCppFunction("console_craft", commandFuncConsoleCraft);
	CommandTable::addCppFunction("console_database", commandFuncConsoleDatabase);
	CommandTable::addCppFunction("console_manufacture", commandFuncConsoleManufacture);
	CommandTable::addCppFunction("console_money", commandFuncConsoleMoney);
	CommandTable::addCppFunction("console_npc", commandFuncConsoleNpc);
	CommandTable::addCppFunction("console_object", commandFuncConsoleObject);
	CommandTable::addCppFunction("console_objvar", commandFuncConsoleObjvar);
	CommandTable::addCppFunction("console_resource", commandFuncConsoleResource);
	CommandTable::addCppFunction("console_script", commandFuncConsoleScript);
	CommandTable::addCppFunction("console_server", commandFuncConsoleServer);
	CommandTable::addCppFunction("console_skill", commandFuncConsoleSkill);

	// communication commands
	CommandTable::addCppFunction("social", commandFuncSocial);
	CommandTable::addCppFunction("socialInternal", commandFuncSocialInternal);
	CommandTable::addCppFunction("spatialChat", commandFuncSpatialChat);
	CommandTable::addCppFunction("spatialChatInternal", commandFuncSpatialChatInternal);
	CommandTable::addCppFunction("setMood", commandFuncSetMood);
	CommandTable::addCppFunction("setMoodInternal", commandFuncSetMoodInternal);
	CommandTable::addCppFunction("systemMessage", commandFuncSystemMessage);
	//	CommandTable::addCppFunction("combatSpam",           commandFuncCombatSpam);

	// admin commands
	CommandTable::addCppFunction("admin_setGodMode", commandFuncAdminSetGodMode);
	CommandTable::addCppFunction("admin_kick", commandFuncAdminKick);
	CommandTable::addCppFunction("admin_planetwarp", commandFuncAdminPlanetwarp);
	CommandTable::addCppFunction("admin_teleport", commandFuncAdminTeleport);
	CommandTable::addCppFunction("admin_teleportTo", commandFuncAdminTeleportTo);
	CommandTable::addCppFunction("admin_listGuilds", commandFuncAdminListGuilds);
	CommandTable::addCppFunction("editBank", commandFuncAdminEditBank);
	CommandTable::addCppFunction("editStats", commandFuncAdminEditStats);
	CommandTable::addCppFunction("editAppearance", commandFuncAdminEditAppearance);
	CommandTable::addCppFunction("grantTitle", commandFuncAdminGrantTitle);
	CommandTable::addCppFunction("credits", commandFuncAdminCredits);
	CommandTable::addCppFunction("getStationName", commandFuncAdminGetStationName);
	CommandTable::addCppFunction("emptyMailTarget", commandFuncEmptyMailTarget);
	CommandTable::addCppFunction("setPlayerAppearance", commandFuncSetPlayerAppearance);
	CommandTable::addCppFunction("revertPlayerAppearance", commandFuncRevertPlayerAppearance);

	// auction commands
	CommandTable::addCppFunction("auction_create", commandFuncAuctionCreate);
	CommandTable::addCppFunction("auction_create_immediate", commandFuncAuctionCreateImmediate);
	CommandTable::addCppFunction("auction_bid", commandFuncAuctionBid);
	CommandTable::addCppFunction("auction_cancel", commandFuncAuctionCancel);
	CommandTable::addCppFunction("auction_accept", commandFuncAuctionAccept);
	CommandTable::addCppFunction("auction_query", commandFuncAuctionQuery);
	CommandTable::addCppFunction("auction_retrieve", commandFuncAuctionRetrieve);

	// cell/building permissions commands
	CommandTable::addCppFunction("addBannedPlayer", commandFuncAddBannedPlayer);
	CommandTable::addCppFunction("removeBannedPlayer", commandFuncRemoveBannedPlayer);
	CommandTable::addCppFunction("addAllowedPlayer", commandFuncAddAllowedPlayer);
	CommandTable::addCppFunction("removeAllowedPlayer", commandFuncRemoveAllowedPlayer);
	CommandTable::addCppFunction("setPublicState", commandFuncSetPublicState);
	CommandTable::addCppFunction("setOwner", commandFuncSetOwner);

	CommandTable::addCppFunction("setPosture", commandSetPosture);
	CommandTable::addCppFunction("duel", commandFuncDuel);
	CommandTable::addCppFunction("endDuel", commandFuncEndDuel);

	// ui
	CommandTable::addCppFunction("setWaypointActiveStatus", commandFuncSetWaypointActiveStatus);
	CommandTable::addCppFunction("setWaypointName", commandFuncSetWaypointName);

	// harvester
	CommandTable::addCppFunction("harvesterActivate", commandFuncHarvesterActivate);
	CommandTable::addCppFunction("harvesterDeactivate", commandFuncHarvesterDeactivate);
	CommandTable::addCppFunction("harvesterHarvest", commandFuncHarvesterHarvest);
	CommandTable::addCppFunction("harvesterSelectResource", commandFuncHarvesterSelectResource);
	CommandTable::addCppFunction("harvesterDiscardHopper", commandFuncHarvesterDiscardHopper);
	CommandTable::addCppFunction("harvesterMakeCrate", commandFuncHarvesterMakeCrate);
	CommandTable::addCppFunction("harvesterGetResourceData", commandFuncHarvesterGetResourceData);

	// syncronized ui
	CommandTable::addCppFunction("synchronizedUiListen", commandFuncSynchronizedUiListen);
	CommandTable::addCppFunction("synchronizedUiStopListening", commandFuncSynchronizedUiStopListening);

	// resource
	CommandTable::addCppFunction("resourceSetName", commandFuncResourceSetName);

	// resource container
	CommandTable::addCppFunction("resourceContainerTransfer", commandFuncResourceContainerTransfer);
	CommandTable::addCppFunction("resourceContainerSplit", commandFuncResourceContainerSplit);

	// survey
	CommandTable::addCppFunction("makeSurvey", commandFuncMakeSurvey);
	CommandTable::addCppFunction("requestSurvey", commandFuncRequestSurvey);
	CommandTable::addCppFunction("requestCoreSample", commandFuncRequestCoreSample);

	CommandTable::addCppFunction("transferItemWeapon", commandFuncTransferWeapon);
	CommandTable::addCppFunction("transferItemArmor", commandFuncTransferArmor);
	CommandTable::addCppFunction("transferItemMisc", commandFuncTransferMisc);
	CommandTable::addCppFunction("openContainer", commandFuncOpenContainer);
	CommandTable::addCppFunction("closeContainer", commandFuncCloseContainer);
	CommandTable::addCppFunction("openLotteryContainer", commandFuncOpenLotteryContainer);
	CommandTable::addCppFunction("closeLotteryContainer", commandFuncCloseLotteryContainer);
	CommandTable::addCppFunction("giveItem", commandFuncGiveItem);

	// grouping
	CommandTable::addCppFunction("groupInvite", commandFuncGroupInvite);
	CommandTable::addCppFunction("groupUninvite", commandFuncGroupUninvite);
	CommandTable::addCppFunction("groupJoin", commandFuncGroupJoin);
	CommandTable::addCppFunction("groupDecline", commandFuncGroupDecline);
	CommandTable::addCppFunction("groupDisband", commandFuncGroupDisband);
	CommandTable::addCppFunction("kickFromShip", commandFuncKickFromShip);
	CommandTable::addCppFunction("groupChat", commandFuncGroupChat);
	CommandTable::addCppFunction("groupMakeLeader", commandFuncGroupMakeLeader);
	CommandTable::addCppFunction("groupMakeMasterLooter", commandFuncGroupMakeMasterLooter);
	CommandTable::addCppFunction("createGroupPickup", commandFuncCreateGroupPickup);
	CommandTable::addCppFunction("useGroupPickup", commandFuncUseGroupPickup);
	CommandTable::addCppFunction("groupTextChatRoomRejoin", commandFuncGroupTextChatRoomRejoin);
	CommandTable::addCppFunction("groupPickRandomGroupMember", commandFuncGroupPickRandomGroupMember);

	// guilds
	CommandTable::addCppFunction("guildChat", commandFuncGuildChat);
	CommandTable::addCppFunction("guildTextChatRoomRejoin", commandFuncGuildTextChatRoomRejoin);
	CommandTable::addCppFunction("guildPickRandomGuildMember", commandFuncGuildPickRandomGuildMember);

	// city
	CommandTable::addCppFunction("cityChat", commandFuncCityChat);
	CommandTable::addCppFunction("cityTextChatRoomRejoin", commandFuncCityTextChatRoomRejoin);
	CommandTable::addCppFunction("cityPickRandomCitizen", commandFuncCityPickRandomCitizen);

	//chat
	CommandTable::addCppFunction("auctionChat", commandFuncAuctionChat);
	CommandTable::addCppFunction("planetChat", commandFuncPlanetChat);

	// crafting
	CommandTable::addCppFunction("requestResourceWeights", commandFuncRequestResourceWeights);
	CommandTable::addCppFunction("requestResourceWeightsBatch", commandFuncRequestResourceWeightsBatch);
	CommandTable::addCppFunction("requestDraftSlots", commandFuncRequestDraftSlots);
	CommandTable::addCppFunction("requestDraftSlotsBatch", commandFuncRequestDraftSlotsBatch);
	CommandTable::addCppFunction("requestManfSchematicSlots", commandFuncRequestManfSchematicSlots);
	CommandTable::addCppFunction("requestCraftingSession", commandFuncRequestCraftingSession);
	CommandTable::addCppFunction("requestCraftingSessionFail", commandFuncRequestCraftingSessionFail);
	CommandTable::addCppFunction("selectDraftSchematic", commandFuncSelectDraftSchematic);
	CommandTable::addCppFunction("nextCraftingStage", commandFuncNextCraftingStage);
	CommandTable::addCppFunction("createPrototype", commandFuncCreatePrototype);
	CommandTable::addCppFunction("createManfSchematic", commandFuncCreateManfSchematic);
	CommandTable::addCppFunction("cancelCraftingSession", commandFuncCancelCraftingSession);
	CommandTable::addCppFunction("stopCraftingSession", commandFuncStopCraftingSession);
	CommandTable::addCppFunction("restartCraftingSession", commandFuncRestartCraftingSession);
	CommandTable::addCppFunction("extractObject", commandFuncExtractObject);
	CommandTable::addCppFunction("repair", commandFuncRepair);
	CommandTable::addCppFunction("factoryCrateSplit", commandFuncFactoryCrateSplit);

	// misc
	CommandTable::addCppFunction("showDanceVisuals", commandFuncShowDanceVisuals);
	CommandTable::addCppFunction("showMusicianVisuals", commandFuncShowMusicianVisuals);
	CommandTable::addCppFunction("placeStructure", commandFuncPlaceStructure);
	CommandTable::addCppFunction("getAttributes", commandFuncGetAttributes);
	CommandTable::addCppFunction("getAttributesBatch", commandFuncGetAttributesBatch);
	CommandTable::addCppFunction("sitServer", commandFuncSitServer);
	CommandTable::addCppFunction("jumpServer", commandFuncJumpServer);
	CommandTable::addCppFunction("purchaseTicket", commandFuncPurchaseTicket);
	CommandTable::addCppFunction("addFriend", commandFuncAddFriend);
	CommandTable::addCppFunction("removeFriend", commandFuncRemoveFriend);
	CommandTable::addCppFunction("getFriendList", commandFuncGetFriendList);
	CommandTable::addCppFunction("addIgnore", commandFuncAddIgnore);
	CommandTable::addCppFunction("removeIgnore", commandFuncRemoveIgnore);
	CommandTable::addCppFunction("getIgnoreList", commandFuncGetIgnoreList);
	CommandTable::addCppFunction("requestBiography", commandFuncRequestBiography);
	CommandTable::addCppFunction("setBiography", commandFuncSetBiography);
	CommandTable::addCppFunction("requestWaypointAtPosition", commandFuncRequestWaypointAtPosition);
	CommandTable::addCppFunction("serverDestroyObject", commandFuncServerDestroyObject);
	CommandTable::addCppFunction("setSpokenLanguage", commandFuncSetSpokenLanguage);
	CommandTable::addCppFunction("applyPowerup", commandFuncApplyPowerup);
	CommandTable::addCppFunction("report", commandFuncReport);
	CommandTable::addCppFunction("resetCooldowns", commandFuncResetCooldowns);
	CommandTable::addCppFunction("spewCommandQueue", commandFuncSpewCommandQueue);
	CommandTable::addCppFunction("locateStructure", commandFuncLocateStructure);
	CommandTable::addCppFunction("locateVendor", commandFuncLocateVendor);
	CommandTable::addCppFunction("showCtsHistory", commandFuncShowCtsHistory);
	CommandTable::addCppFunction("pickupAllRoomItemsIntoInventory", commandFuncPickupAllRoomItemsIntoInventory);
	CommandTable::addCppFunction("dropAllInventoryItemsIntoRoom", commandFuncDropAllInventoryItemsIntoRoom);
	CommandTable::addCppFunction("saveDecorationLayout", commandFuncSaveDecorationLayout);
	CommandTable::addCppFunction("restoreDecorationLayout", commandFuncRestoreDecorationLayout);
	CommandTable::addCppFunction("areaPickRandomPlayer", commandFuncAreaPickRandomPlayer);
	CommandTable::addCppFunction("roomPickRandomPlayer", commandFuncRoomPickRandomPlayer);

	// match making
	CommandTable::addCppFunction("setMatchMakingPersonalId", commandFuncSetMatchMakingPersonalId);
	CommandTable::addCppFunction("setMatchMakingCharacterId", commandFuncSetMatchMakingCharacterId);
	CommandTable::addCppFunction("requestCharacterMatch", commandFuncRequestCharacterMatch);
	CommandTable::addCppFunction("toggleSearchableByCtsSourceGalaxy", commandFuncToggleSearchableByCtsSourceGalaxy);
	CommandTable::addCppFunction("toggleDisplayLocationInSearchResults", commandFuncToggleDisplayLocationInSearchResults);
	CommandTable::addCppFunction("toggleAnonymous", commandFuncToggleAnonymous);
	CommandTable::addCppFunction("toggleHelper", commandFuncToggleHelper);
	CommandTable::addCppFunction("toggleRolePlay", commandFuncToggleRolePlay);
	CommandTable::addCppFunction("toggleLookingForGroup", commandFuncToggleLookingForGroup);
	CommandTable::addCppFunction("toggleAwayFromKeyBoard", commandFuncToggleAwayFromKeyBoard);
	CommandTable::addCppFunction("toggleDisplayingFactionRank", commandFuncToggleDisplayingFactionRank);
	CommandTable::addCppFunction("toggleOutOfCharacter", commandFuncToggleOutOfCharacter);
	CommandTable::addCppFunction("toggleLookingForWork", commandFuncToggleLookingForWork);

	//skill
	CommandTable::addCppFunction("revokeSkill", commandFuncRevokeSkill);
	CommandTable::addCppFunction("setCurrentSkillTitle", commandFuncSetCurrentSkillTitle);

	//misc ui
	CommandTable::addCppFunction("permissionListModify", commandFuncPermissionListModify);
	CommandTable::addCppFunction("requestCharacterSheetInfo", commandFuncRequestCharacterSheetInfo);
	CommandTable::addCppFunction("removeBuff", commandFuncRemoveBuff);

	// npc conversation
	CommandTable::addCppFunction("npcConversationStart", commandFuncNpcConversationStart);
	CommandTable::addCppFunction("npcConversationStop", commandFuncNpcConversationStop);
	CommandTable::addCppFunction("npcConversationSelect", commandFuncNpcConversationSelect);
	CommandTable::addCppFunction("handleUnstick", commandFuncUnstick);
	CommandTable::addCppFunction("getAccountInfo", commandFuncGetAccountInfo);
	CommandTable::addCppFunction("lag", commandFuncLag);

	//knowledgebase (i.e. holocron)
	CommandTable::addCppFunction("executeKnowledgeBaseMessage", commandExecuteKnowledgeBaseMessage);
	CommandTable::addCppFunction("reconnectToTransferServer", commandFuncReconnectToTransferServer);
	CommandTable::addCppFunction("findFriend", commandFuncFindFriend);

	// godclient-specific functions
	CommandTable::addCppFunction(FormManager::getFormCommandName(), commandFuncFormCommand);

	// space
	CommandTable::addCppFunction("installShipComponent", commandFuncInstallShipComponents);
	CommandTable::addCppFunction("uninstallShipComponent", commandFuncUninstallShipComponents);
	CommandTable::addCppFunction("insertItemIntoShipComponentSlot", commandFuncInsertItemIntoShipComponentSlot);
	CommandTable::addCppFunction("associateDroidControlDeviceWithShip", commandFuncAssociateDroidControlDeviceWithShip);
	CommandTable::addCppFunction("unassociateDroidControlDeviceWithShip", commandFuncAssociateDroidControlDeviceWithShip);

	CommandTable::addCppFunction("serverAsteroidDataListen", commandFuncServerAsteroidDataListen);
	CommandTable::addCppFunction("serverAsteroidDataStopListening", commandFuncServerAsteroidDataStopListening);

	CommandTable::addCppFunction("boosterOn", commandFuncBoosterOn);
	CommandTable::addCppFunction("boosterOff", commandFuncBoosterOff);

	CommandTable::addCppFunction("setFormation", commandFuncSetFormation);
	CommandTable::addCppFunction("setFormationSlot", commandFuncSetFormationSlot);
	CommandTable::addCppFunction("undock", commandFuncUnDock);
	CommandTable::addCppFunction("launchIntoSpace", commandFuncLaunchIntoSpace);
	CommandTable::addCppFunction("acceptQuest", commandFuncAcceptQuest);
	CommandTable::addCppFunction("completeQuest", commandFuncReceiveReward);
	CommandTable::addCppFunction("abandonQuest", commandFuncAbandonQuest);

	// exchange
	CommandTable::addCppFunction("exchangeListCredits", commandFuncExchangeListCredits);

	// squelch
	CommandTable::addCppFunction("squelch", commandFuncSquelch);
	CommandTable::addCppFunction("unsquelch", commandFuncUnsquelch);

	// warden
	CommandTable::addCppFunction("grantWarden", commandFuncGrantWarden);
	CommandTable::addCppFunction("revokeWarden", commandFuncRevokeWarden);
	CommandTable::addCppFunction("spammer", commandFuncSpammer);
	CommandTable::addCppFunction("unspammer", commandFuncUnspammer);
	CommandTable::addCppFunction("deputizeWarden", commandFuncDeputizeWarden);
	CommandTable::addCppFunction("undeputizeWarden", commandFuncUndeputizeWarden);

	// unlocked slot management commands
	CommandTable::addCppFunction("occupyUnlockedSlot", commandFuncOccupyUnlockedSlot);
	CommandTable::addCppFunction("vacateUnlockedSlot", commandFuncVacateUnlockedSlot);
	CommandTable::addCppFunction("swapUnlockedSlot", commandFuncSwapUnlockedSlot);
}

// ----------------------------------------------------------------------

void CommandCppFuncs::remove()
{
}

// ======================================================================
