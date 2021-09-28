// ======================================================================
//
// TangibleObject.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleObject.h"

#include "SwgGameServer/CombatEngine.h"
#include "SwgGameServer/ConfigCombatEngine.h"
#include "serverGame/AiCombatPulseQueue.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/Chat.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/Client.h"
#include "serverGame/CombatTracker.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/NetworkTriggerVolume.h"
#include "serverGame/NpcConversation.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/RegionPvp.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerCollisionProperty.h"
#include "serverGame/ServerLotManagerNotification.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerSynchronizedUi.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipComponentDataManager.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TangibleController.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "serverNetworkMessages/MessageQueueCommandQueueForceExecuteCommandFwd.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverPathfinding/ServerPathfindingNotification.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/Command.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedLog/Log.h"
#include "sharedMath/PackedArgb.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/IncubatorCommitMessage.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueCraftIngredients.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueOpponentInfo.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#ifdef _DEBUG
#include "serverGame/ConsoleManager.h"
#endif

#include <cstdio>
#include <algorithm>
#include <limits>

#pragma warning(disable:4355)


namespace TangibleObjectNamespace
{
	int const s_minCombatDuration = 4;
	bool isEmpty(const ServerObject& obj)
	{
		const Container* container = obj.getContainerProperty();
		if (container)
		{
			return (container->getNumberOfItems() == 0);
		}
		return true;
	}

	std::string const s_attributeBonusNames[] =
	{
		"health",
		"health_regen",
		"action",
		"action_regen",
		"mind",
		"mind_regen"
	};

	const char * const s_accessListScriptVar = "tangible_object.accessList";
	const char * const s_guildAccessListScriptVar = "tangible_object.guildAccessList";

	void getCategorizedSkillModBonuses(TangibleObject const & to, std::string const & objvarName, std::map<std::string, int> & skillModBonuses);
	int getCategorizedSkillModBonus(TangibleObject const & to, std::string const & objvarName, std::string const & skillMod);
}

//----------------------------------------------------------------------

void TangibleObjectNamespace::getCategorizedSkillModBonuses(TangibleObject const & to, std::string const & objvarName,  std::map<std::string, int> & skillModBonuses)
{
	DynamicVariableList::NestedList children(to.getObjVars(), objvarName);
	for (DynamicVariableList::NestedList::const_iterator iterChildren = children.begin(); iterChildren != children.end(); ++iterChildren)
	{
		DynamicVariable::DynamicVariableType const type = iterChildren.getType();
		if (type == DynamicVariable::INT)
		{
			int bonus = 0;
			iterChildren.getValue(bonus);

			std::string const childName = iterChildren.getName();
			std::map<std::string, int>::iterator iterFind = skillModBonuses.find(childName);
			if (iterFind != skillModBonuses.end())
				iterFind->second += bonus;
			else
				IGNORE_RETURN(skillModBonuses.insert(std::make_pair(childName, bonus)));
		}
		else if (type == DynamicVariable::LIST)
		{
			std::string const childFullName = objvarName + '.' + iterChildren.getName();
			getCategorizedSkillModBonuses(to, childFullName, skillModBonuses);
		}
	}
}

//----------------------------------------------------------------------

int TangibleObjectNamespace::getCategorizedSkillModBonus(TangibleObject const & to, std::string const & objvarName, std::string const & skillMod)
{
	int totalBonus = 0;
	DynamicVariableList::NestedList children(to.getObjVars(), objvarName);
	for (DynamicVariableList::NestedList::const_iterator iterChildren = children.begin(); iterChildren != children.end(); ++iterChildren)
	{
		DynamicVariable::DynamicVariableType const type = iterChildren.getType();
		if (type == DynamicVariable::INT)
		{
			if (iterChildren.getName() == skillMod)
			{
				int bonus = 0;
				iterChildren.getValue(bonus);

				totalBonus += bonus;
			}
		}
		else if (type == DynamicVariable::LIST)
		{
			std::string const childFullName = objvarName + '.' + iterChildren.getName();
			totalBonus += getCategorizedSkillModBonus(to, childFullName, skillMod);
		}
	}

	return totalBonus;
}

//----------------------------------------------------------------------

using namespace TangibleObjectNamespace;

//=======================================================================
// class TangibleObject::CraftingToolSyncUi

class TangibleObject::CraftingToolSyncUi : public ServerSynchronizedUi
{
public:
	CraftingToolSyncUi(TangibleObject & owner) :
		ServerSynchronizedUi(owner),
		m_prototype(),
		m_manfSchematic(),
		m_manfSchematicObserver()
	{
		addToUiPackage(m_prototype);
		addToUiPackage(m_manfSchematic);
	}

	virtual ~CraftingToolSyncUi()
	{
		if (!isOwnerChangingAuthority())
		{
			if (m_manfSchematic.get() != CachedNetworkId::cms_cachedInvalid)
			{
				safe_cast<TangibleObject *>(getOwner())->clearCraftingManufactureSchematic();
			}
			if (m_prototype.get() != CachedNetworkId::cms_cachedInvalid)
			{
				safe_cast<TangibleObject *>(getOwner())->clearCraftingPrototype();
			}
		}
	}

	const CachedNetworkId & getPrototype(void) const
	{
		return m_prototype.get();
	}

	void setPrototype(const CachedNetworkId & prototype)
	{
		m_prototype = prototype;
	}

	const CachedNetworkId & getManfSchematic(void) const
	{
		return m_manfSchematic.get();
	}

	void setManfSchematic(const CachedNetworkId & manfSchematic, const CachedNetworkId & observer, bool deleteSchematic)
	{
		m_manfSchematic = manfSchematic;
		m_manfSchematicObserver = observer;
	}

private:

	CraftingToolSyncUi(const CraftingToolSyncUi & src);
	CraftingToolSyncUi & operator =(const CraftingToolSyncUi & src);

private:

	Archive::AutoDeltaVariable<CachedNetworkId> m_prototype;
	Archive::AutoDeltaVariable<CachedNetworkId> m_manfSchematic;

	CachedNetworkId m_manfSchematicObserver;
};

//=======================================================================

static const ConstCharCrcLowerString COMMAND_GROUP_COMBAT("combat");

// armor objvars - also defined in armor.scriptlib
static const std::string OBJVAR_ARMOR_BASE =                          "armor";
static const std::string OBJVAR_ARMOR_ENCUMBRANCE = "armorencumbrance";
static const std::string OBJVAR_ENCUMBRANCE_SPLIT = "encumbranceSplit";
static const std::string OBJVAR_ARMOR_LEVEL       = "armorLevel";

// objvar to flag the object as a shield
static const std::string OBJVAR_ARMOR_SHIELD =                        "armor.isShield";

// armor datatable strings
static const std::string DATATABLE_ARMOR               = "datatables/crafting/armor.iff";
static const std::string DATATABLE_FINAL_ROW           = "final";
static const std::string DATATABLE_TYPE_COL            = "type";
static const std::string DATATABLE_MIN_ENCUMBRANCE_COL = "min_encumbrance";
static const std::string DATATABLE_MAX_ENCUMBRANCE_COL = "max_encumbrance";

// bio-link objvars
static const std::string OBJVAR_BIO_LINK      = "biolink";
static const std::string OBJVAR_BIO_LINK_ID   = "biolink.id";
// This id is used for a bio-linked item that is linked to no one.
// It is safe as long as no player has id 1 (which should be a good assumption).
// Note this value is reflected in library.utils.scriptlib.
static const NetworkId PENDING_BIO_LINK_NETWORK_ID(static_cast<NetworkId::NetworkIdType>(1));

// template for crafting components container
const static ConstCharCrcLowerString CRAFTING_COMPONENTS_CONTAINER =  "object/tangible/crafting/crafting_components_container.iff";

// slots for a crafting tool
const static ConstCharCrcLowerString INGREDIENT_HOPPER_SLOT_NAME =    "ingredient_hopper";
const static ConstCharCrcLowerString MANF_OUTPUT_SLOT_NAME =          "anythingnomod2";
const static ConstCharCrcLowerString TEMP_PROTOTYPE_SLOT_NAME =       "anythingnomod1";
const static ConstCharCrcLowerString TEMP_SCHEMATIC_SLOT_NAME =       "test_manf_schematic";

// objvars for crafting
const static std::string OBJVAR_CRAFTING_TOOL =                       "crafting.tool";
const static std::string OBJVAR_CRAFTING_STATION =                    "crafting.station";
const static std::string OBJVAR_CRAFTING_CRAFTER =                    "crafting.crafter";
const static std::string OBJVAR_CRAFTING_COMPONENTS =                 "crafting.components";
const static std::string OBJVAR_CRAFTING_SCHEMATIC =                  "crafting.source_schematic";
const static std::string OBJVAR_CRAFTING_TYPE =                       "crafting.type";
const static std::string OBJVAR_CRAFTING_REPAIR_TYPE =                "crafting.repair_type";
const static std::string OBJVAR_CRAFTING_PROTOTYPE_TIME =             "crafting.prototypeTime";
const static std::string OBJVAR_CRAFTING_PROTOTYPE_START_TIME =       "crafting.prototypeStartTime";
const static std::string OBJVAR_CRAFTING_PROTOTYPE_OBJECT =           "crafting.prototypeObject";
const static std::string OBJVAR_CRAFTING_PROTOTYPE_CRAFTER =          "crafting.prototypeCrafter";
const static std::string OBJVAR_CREATOR_ID =                          "crafting.creator.id";
const static std::string OBJVAR_CREATOR_XP =                          "crafting.creator.xp";
const static std::string OBJVAR_CREATOR_XP_TYPE =                     "crafting.creator.xpType";
const static std::string OBJVAR_REPAIR_TOOL =                         "crafting.repair";
const static std::string OBJVAR_CRAFTING_STATIONMOD  =                "crafting.stationMod";
const static std::string OBJVAR_SKILLMOD_BONUS =                      "skillmod.bonus";
const static std::string OBJVAR_CATEGORIZED_SKILLMOD_BONUS =          "categorizedSkillmod.bonus";
const static std::string OBJVAR_SKILLMOD_SOCKETS =                    "skillmod.sockets";
const static std::string OBJVAR_ATTRIBUTE_BONUS =                     "attribute.bonus";

const std::string OBJVAR_COMPONENT_DATA("crafting_components");

// battlefield objvars
const static std::string OBJVAR_BATTLEFIELD_PARTICIPANTS =            "battleDudes";

// no trade
static const std::string NOMOVE_SCRIPT  = "item.special.nomove";

static const std::string OBJVAR_DECLINE_DUEL  = "decline_duel";

const SharedObjectTemplate * TangibleObject::m_defaultSharedTemplate = nullptr;


//-----------------------------------------------------------------------
TangibleObject::TangibleObject(const ServerTangibleObjectTemplate* newTemplate) :
	ServerObject(newTemplate),
	m_combatData(nullptr),
	m_pvpType(),
	m_pvpMercenaryType(PvpType_Neutral),
	m_pvpFutureType(-1),
	m_pvpFaction(),
	m_pvpMercenaryFaction(PvpData::getNeutralFactionId()),
	m_pvpRegionCrc(),
	m_pvpEnemies(),
	m_damageTaken(0),
	m_maxHitPoints(newTemplate->getMaxHitPoints()),
	m_ownerId(NetworkId(static_cast<NetworkId::NetworkIdType>(0))),
	m_customAppearance(),
	m_appearanceData(),
	m_locationTargets(),
	m_components(),
	m_visible(true),
	m_hideFromClient(false),
	m_count(newTemplate->getCount()),
	m_condition(newTemplate->getCondition()),
	m_creatorId(NetworkId::cms_invalid),
	m_sourceDraftSchematic(0),
	m_hateList(),
	m_hateOverTime(),
	m_inCombat(false),
	m_combatStartTime(0),
	m_attackableOverride(false),
	m_passiveReveal(),
	m_passiveRevealPlayerCharacter(),
	m_mapColorOverride(0),
	m_accessList(),
	m_guildAccessList(),
	m_effectsMap(),
	m_npcConversation(nullptr),
	m_conversations()
{
	WARNING_STRICT_FATAL(!getSharedTemplate(), ("Tried to create a TANGIBLE %s object without a shared template!\n", newTemplate->DataResource::getName()));

	m_hateList.addServerNpAutoDeltaVariables(m_serverPackage_np);
	m_hateList.setOwner(this);

	m_maxHitPoints.setSourceObject(this);
	m_ownerId.setSourceObject(this);
	m_customAppearance.setSourceObject(this);
	m_appearanceData.setSourceObject(this);
	m_pvpType.setSourceObject(this);
	m_pvpMercenaryType.setSourceObject(this);
	m_pvpFaction.setSourceObject(this);
	m_pvpMercenaryFaction.setSourceObject(this);
	m_pvpRegionCrc.setSourceObject(this);
	m_pvpEnemies.setSourceObject(this);
	m_visible.setSourceObject(this);
	m_hideFromClient.setSourceObject(this);
	m_condition.setSourceObject(this);
	m_inCombat.setSourceObject(this);
	m_attackableOverride.setSourceObject(this);
	m_passiveRevealPlayerCharacter.setSourceObject(this);

	// set to not disabled, and set invulnerable and wantSawAttack from template
	int condition = m_condition.get();
	condition &= ~(  ServerTangibleObjectTemplate::C_disabled
	               | ServerTangibleObjectTemplate::C_invulnerable
	               | ServerTangibleObjectTemplate::C_wantSawAttackTrigger);
	if (newTemplate->getInvulnerable())
		condition |= ServerTangibleObjectTemplate::C_invulnerable;
	if (newTemplate->getWantSawAttackTriggers())
		condition |= ServerTangibleObjectTemplate::C_wantSawAttackTrigger;
	m_condition = condition;

	// attach trigger volumes to this object
	size_t i;
	size_t count = newTemplate->getTriggerVolumesCount();
	for (i = 0; i < count; ++i)
	{
		const TriggerVolumeData & trigger = newTemplate->getTriggerVolumes(static_cast<int>(i));
		createTriggerVolume(trigger.getRadius(), trigger.getName(), true); //@todo : add "promiscuous param to template entry
	}

	//-- initialize CustomizationDataProperty if any customization variables are declared
	//   for the SharedTangibleObjectTemplate.
	const SharedTangibleObjectTemplate *const sharedObjectTemplate = safe_cast<const SharedTangibleObjectTemplate*>(getSharedTemplate());
	if (sharedObjectTemplate)
	{
		//-- create CustomizationDataProperty if appropriate
		sharedObjectTemplate->createCustomizationDataPropertyAsNeeded(*this);

		//-- if a CustomizationDataProperty was created, attach a modification listener.
		// check if the property exists
		CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));

		if (cdProperty)
		{
			// retrieve the CustomizationData instance associated with the property
			CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
			NOT_NULL(customizationData);

			// attach modification callback
			customizationData->registerModificationListener(customizationDataModificationCallback, this);

			// release local reference
			customizationData->release();
		}

		if(ConfigServerGame::getCreateAppearances())
		{
			const std::string &appearanceString = sharedObjectTemplate->getAppearanceFilename();
			if (appearanceString.size() != 0 && strstr (appearanceString.c_str (), ".apt") != 0)
			{
				Appearance * newAppearance = AppearanceTemplateList::createAppearance(appearanceString.c_str());

				if(newAppearance != nullptr)
				{
					setAppearance(newAppearance);
				} else {
					DEBUG_WARNING(true, ("Missing appearance for TangibleObject::TangibleObject %s", appearanceString.c_str()));
				}
			}
		}
	}

	// If the object is a path node, attach the pathfinding notification

	if (isWaypoint())
	{
		addNotification( ServerPathfindingNotification::getInstance() );
	}

	// attach the collision property

	ServerCollisionProperty* collision = new ServerCollisionProperty(*this,getSharedTemplate());

	addProperty(*collision);

	addMembersToPackages();

	ObjectTracker::addTangible();

	if(getGameObjectType() == SharedObjectTemplate::GOT_loadbeacon)
	{
		ServerWorld::addLoadBeacon(this);
	}

	//-- add the lot manager notification
	{
		SharedTangibleObjectTemplate const * const sharedTangibleObjectTemplate = dynamic_cast<SharedTangibleObjectTemplate const *> (getSharedTemplate ());
		if (sharedTangibleObjectTemplate)
		{
			StructureFootprint const * const structureFootprint = sharedTangibleObjectTemplate->getStructureFootprint ();
			if (structureFootprint)
				addNotification (ServerLotManagerNotification::getInstance ());
		}
		else
			DEBUG_WARNING (true, ("TangibleObject::TangibleObject - [%s] does not have a shared tangible object template", getObjectTemplate ()->getName ()));
	}
}

//-----------------------------------------------------------------------

TangibleObject::~TangibleObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	if (getSynchronizedUi() != nullptr)
	{
		CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
			getSynchronizedUi());
		if (sync != nullptr)
		{
			if (isAuthoritative() && isCraftingTool())
			{
				clearCraftingPrototype();
				clearCraftingManufactureSchematic();
			}
			else
			{
				if (!isAuthoritative())
				{
					WARNING(true, ("Tangible object %s is not authoritative "
						"but has a SynchronizedUi",
						getNetworkId().getValueString().c_str()));
				}
				if (!isCraftingTool())
				{
					WARNING(true, ("Tangible object %s is not a crafting tool "
						"but has a CraftingToolSyncUi",
						getNetworkId().getValueString().c_str()));
				}
				sync->setPrototype(CachedNetworkId::cms_cachedInvalid);
				sync->setManfSchematic(CachedNetworkId::cms_cachedInvalid,
					CachedNetworkId::cms_cachedInvalid, true);
			}
		}
	}

	if(getGameObjectType() == SharedObjectTemplate::GOT_loadbeacon)
	{
		ServerWorld::removeLoadBeacon(this);
	}

	CombatTracker::removeDefender(this);

	if (m_combatData != nullptr)
	{
		if (isAuthoritative())
		{
			if (!getCombatData()->defenseData.damage.empty())
			{
				WARNING(true, ("\tobject still has a non-empty damage list"));

				// look up the call stack information
				const int callStackOffset = 4;
				const int maxStackDepth   = 64;
				const int callStackSize = callStackOffset + maxStackDepth;
				uint32 callStack[callStackOffset + maxStackDepth];

				// look up the caller's file and line
				if (callStack[callStackOffset])
				{
					char lib[4 * 1024] = { '\0' };
					char file[4 * 1024] = { '\0' };
					int line = 0;
					REPORT_LOG(true, ("\tCall stack:"));
					for (int i = callStackOffset; i < callStackSize; ++i)
					{
						if (callStack[i])
						{
								REPORT_LOG(true, ("\t%s(%d) : caller %d\n", file, line, i-callStackOffset));
						}
					}
				}
			}
		}

		delete m_combatData;
		m_combatData = nullptr;

		if (!isPlayerControlled())
			ObjectTracker::removeCombatAI();
	}

	if (m_npcConversation != nullptr)
	{
		delete m_npcConversation;
		m_npcConversation = nullptr;
	}

	ObjectTracker::removeTangible();
}	// TangibleObject::~TangibleObject

//-----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void TangibleObject::initializeFirstTimeObject()
{
	DEBUG_FATAL(!isAuthoritative(), ("Trying to initialize non auth object %s", getNetworkId().getValueString().c_str()));

	// set up armor from the template
	const ServerTangibleObjectTemplate * newTemplate = safe_cast<
		const ServerTangibleObjectTemplate *>(getObjectTemplate());
	if (newTemplate != nullptr)
	{
		setInvulnerable(newTemplate->getInvulnerable());

		if (isAuthoritative())
		{
			initializeVisibility();

			const ServerArmorTemplate * armorTemplate = newTemplate->getArmor();
			if (armorTemplate != nullptr)
			{
				int const rating = static_cast<int>(armorTemplate->getRating());
				if (   rating < static_cast<int>(ServerArmorTemplate::AR_armorNone)
				    || rating > static_cast<int>(ServerArmorTemplate::AR_armorHeavy))
				{
					WARNING(true, ("TangibleObject::initializeFirstTimeObject trying "
						"to add armor data from template %s with an invalid rating "
						"of %d", armorTemplate->getName(), rating));
				}
				else
				{
					// armor integrity replaces hitpoints
					m_maxHitPoints = armorTemplate->getIntegrity();
				}
			}
		}
	}

	// attach the combat skeleton script to this object
	ServerTangibleObjectTemplate::CombatSkeleton skeleton = static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(getCombatSkeleton());
	const ConfigCombatEngine::SkeletonAttackMod & skeletonMod =
		ConfigCombatEngine::getSkeletonAttackMod(skeleton);
	if (!skeletonMod.script.empty() && skeletonMod.script[0] != '\0')
	{
		NOT_NULL(getScriptObject());
		getScriptObject()->attachScript(skeletonMod.script, true);
	}

	handleContainerSetup();

	ServerObject::initializeFirstTimeObject();

	setDefaultAlterTime(AlterResult::cms_keepNoAlter);

	calcPvpableState();
	if (!isNonPvpObject())
		MessageToQueue::sendRecurringMessageToC(getNetworkId(), "C++CheckPvpRegion", std::string(), static_cast<int>(getPvpRegionCheckTime()));
}

//-----------------------------------------------------------------------

/**
 * Initializes a proxy object.
 */
void TangibleObject::endBaselines()
{
	ServerObject::endBaselines();
	handleContainerSetup();

	// set the hate list owner to ensure the player object is set correctly
	m_hateList.setOwner(this);

	if (isAuthoritative())
	{
		// fix the load_with values if they don't match
		if ( getLoadWith() != ContainerInterface::getLoadWithContainerId(*this))
		{
			setLoadWith(ContainerInterface::getLoadWithContainerId(*this));
		}

		// test to flag the object as disabled
		if (getDamageTaken() >= getMaxHitPoints())
			setDisabled(true);

		// pvp needs to be notified after an object is both initialized and made authoritative.
		Pvp::handleAuthorityAcquire(*this);

		// this code is for legacy objects already in the database
		// Don't consider virtual overrides for invulnerability in this case
		if (getCacheVersion() > 0 && !TangibleObject::isInvulnerable())
			setInvulnerable(true);

		if (getObjVars().hasItem(OBJVAR_CRAFTING_SCHEMATIC))
			setCondition(C_crafted);
		else
			clearCondition(C_crafted);

		removeObjVarItem("debuggingLogCommandEnqueue");

		// destroy duped TCG Set5 AT-AT Head ITV
		if ((getStaticItemName() == "item_tcg_loot_reward_series5_atat_head_itv") && !getObjVars().hasItem("rewardGrantInfo"))
		{
			MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
				"C++DestroyDupedTcg5AtAtHeadItv",
				"",
				5,
				false);
		}
	}

	calcPvpableState();

	// check for existence of objvar to enable/disable m_logCommandEnqueue
	CommandQueue * const queue = getCommandQueue();
	if (queue != nullptr)
	{
		queue->setLogCommandEnqueue(getObjVars().hasItem("debuggingLogCommandEnqueue"));
	}

	// if this is an incubator, and it contains imported resource from another
	// galaxy, register the imported resource type on this game server, if it
	// hasn't already been registered on this game server; this must be done
	// on each game server as it receives the incubator
	static const uint32 crcIncubator = CrcLowerString::calculateCrc("object/tangible/crafting/station/incubator_station.iff");
	if (getTemplateCrc() == crcIncubator)
	{
		DynamicVariableList const & objvars = getObjVars();
		if (objvars.hasItem("power.resourceData") && (DynamicVariable::STRING == objvars.getType("power.resourceData")))
		{
			std::string resourceData;
			IGNORE_RETURN(objvars.getItem("power.resourceData", resourceData));
			IGNORE_RETURN(ResourceTypeObject::addImportedResourceType(resourceData));
		}
	}
}	// TangibleObject::endBaselines

//-----------------------------------------------------------------------

void TangibleObject::onLoadedFromDatabase()
{
	ServerObject::onLoadedFromDatabase();

	// if we are a crafting tool that is counting down a prototype,
	// reactivate the counter
	if (isCraftingTool() && isAuthoritative())
	{
		float prototypeTime = 0;
		getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_TIME, prototypeTime);
		if (prototypeTime > 0)
		{
			int timeLeft = getCraftingToolPrototypeTime();
			if (timeLeft < 1)
				timeLeft = 1;

			NetworkId ownerId;
			NetworkId prototypeId;
			bool result = getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_OBJECT,
				prototypeId) &&	getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_CRAFTER,
				ownerId);
			if (result)
			{
				ScriptParams params;
				params.addParam(ownerId, "crafter");
				params.addParam(prototypeId, "prototype");
				ScriptDictionaryPtr dictionary;
				getScriptObject()->makeScriptDictionary(params, dictionary);
				if (dictionary.get() != nullptr)
				{
					dictionary->serialize();
					MessageToQueue::getInstance().sendMessageToJava(getNetworkId(),
						"prototypeDone", dictionary->getSerializedData(),
						timeLeft, false);
					MessageToQueue::getInstance().sendMessageToC(getNetworkId(),
						"C++updateCounter", "", 1, false);
				}
			}
		}
	}

	setDefaultAlterTime(AlterResult::cms_keepNoAlter);

	calcPvpableState();
	if (!isNonPvpObject())
		MessageToQueue::sendRecurringMessageToC(getNetworkId(), "C++CheckPvpRegion", std::string(), static_cast<int>(getPvpRegionCheckTime()));

	if(isLocked())
	{
		readInUserAccessListObjVars();
		readInGuildAccessListObjVars();
	}

	// Invalid Socket check.
	bool checkForInvalidSockets = (GameObjectTypes::isTypeOf(getGameObjectType(), SharedObjectTemplate::GOT_clothing) || 
		                           GameObjectTypes::isTypeOf(getGameObjectType(), SharedObjectTemplate::GOT_armor) ) && isCrafted();
	if(checkForInvalidSockets)
	{
		std::vector< std::pair< std::string, int > > skillMods;
		getSkillModBonuses(skillMods, false);

		if(skillMods.size() > 3) // 3 is the max legit amount for any 1 socket item
		{
			// Check all our stats, make sure they aren't over the limit for 1 socket, cross reference names with the reverse engineering datatable.
			DataTable * modTable = DataTableManager::getTable("datatables/crafting/reverse_engineering_mods.iff", true);
			if(!modTable)
				return;
			int invalidSkillCounter = 0;
			// Check each skill mod against the datatable. If we don't have that skill mod in the table, then it could be a legitimate item from the older system.
			for(unsigned int i = 0; i < skillMods.size(); ++i)
			{
				std::string skillModName = skillMods[i].first;
				int row = modTable->searchColumnString(0, skillModName);

				if(row < 0) // wasn't in our table, could be legit.
					continue;

				++invalidSkillCounter;
			}

			if(invalidSkillCounter <= 3) // Valid item.
				return;

			// Player Name
			std::string playerName = NameManager::getInstance().getPlayerFullName(getOwnerId());
			Unicode::String objName = getObjectName();
			std::string narrowObjName;
			if(!objName.empty())
				narrowObjName = Unicode::wideToNarrow(objName);
			// Possible invalid item.
			LOG("CustomerService", ("extraSocketExploit:Reason: Invalid # of Skill mods. Item Name: [%s] OID:[%s] Shared Template Name: [%s]", 
				narrowObjName.empty() ? "" : narrowObjName.c_str(), getNetworkId().getValueString().c_str(), getSharedTemplateName() ? getSharedTemplateName() : ""));
			LOG("CustomerService", ("extraSocketExploit:Owner OID: [%s] Owner Station ID: [%u] Owner Name: [%s]", 
				getOwnerId().getValueString().c_str(), NameManager::getInstance().getPlayerStationId(getOwnerId()), playerName.empty() ? "" : playerName.c_str()));
			LOG("CustomerService", ("extraSocketExploit:Skill Mods are:"));

			for(unsigned int i = 0; i < skillMods.size(); ++i)
			{
				int row = modTable->searchColumnString(0, skillMods[i].first);
				int currentValue = skillMods[i].second;
				int maxValue = 0;

				if(row < 0)
				{
					LOG("CustomerService", ("extraSocketExploit:Skill Mod: [%s] Value: [%d]", skillMods[i].first.c_str(), skillMods[i].second));
					continue;
				}

				int ratioValue = modTable->getIntValue(1, row);

				if(ratioValue != 0)
					maxValue = 35 / ratioValue;

				if(currentValue > maxValue)
					LOG("CustomerService", ("extraSocketExploit:(Invalid)Skill Mod: [%s] Value: [%d] Max Legal Value [%d]", skillMods[i].first.c_str(), skillMods[i].second, maxValue));
				else if(currentValue > 35)
					LOG("CustomerService", ("extraSocketExploit:(Invalid)Skill Mod: [%s] Value: [%d] Max Legal Value [35]", skillMods[i].first.c_str(), skillMods[i].second));
				else
					LOG("CustomerService", ("extraSocketExploit:Skill Mod: [%s] Value: [%d]", skillMods[i].first.c_str(), skillMods[i].second));
			}
		}
		else
		{
			// Check all our stats, make sure they aren't over the limit for 1 socket.
			DataTable * modTable = DataTableManager::getTable("datatables/crafting/reverse_engineering_mods.iff", true);
			if(!modTable)
				return;

			for(unsigned int i = 0; i < skillMods.size(); ++i)
			{
				std::string skillModName = skillMods[i].first;
				int currentValue = skillMods[i].second;
				int maxValue = 0;
				bool logItem = false;

				if(currentValue > 35)
					logItem = true;
				else
				{
					int row = modTable->searchColumnString(0, skillModName);

					if(row < 0)
						return;

					int ratioValue = modTable->getIntValue(1, row);

					if(ratioValue != 0)
						maxValue = 35 / ratioValue;

					if(currentValue > maxValue)
						logItem = true;
				}

				if(logItem)
				{
					// Player Name
					std::string playerName = NameManager::getInstance().getPlayerFullName(getOwnerId());
					Unicode::String objName = getObjectName();
					std::string narrowObjName;
					if(!objName.empty())
						narrowObjName = Unicode::wideToNarrow(objName);
					// Possible invalid item.
					LOG("CustomerService", ("extraSocketExploit:Reason: Invalid Skill mod value. Item Name: [%s] OID:[%s] Shared Template Name: [%s]", 
						narrowObjName.empty() ? "" : narrowObjName.c_str(), getNetworkId().getValueString().c_str(), getSharedTemplateName() ? getSharedTemplateName() : ""));
					LOG("CustomerService", ("extraSocketExploit:Owner OID: [%s] Owner Station ID: [%u] Owner Name: [%s]", 
						getOwnerId().getValueString().c_str(), NameManager::getInstance().getPlayerStationId(getOwnerId()), playerName.empty() ? "" : playerName.c_str()));
					LOG("CustomerService", ("extraSocketExploit:Skill Mods are:"));

					for(unsigned int j = 0; j < skillMods.size(); ++j)
					{

						int row = modTable->searchColumnString(0, skillMods[j].first);
						currentValue = skillMods[j].second;

						if(row < 0)
						{
							LOG("CustomerService", ("extraSocketExploit:Skill Mod: [%s] Value: [%d]", skillMods[j].first.c_str(), skillMods[j].second));
							continue;
						}

						int ratioValue = modTable->getIntValue(1, row);

						if(ratioValue != 0)
							maxValue = 35 / ratioValue;

						if(currentValue > maxValue)
							LOG("CustomerService", ("extraSocketExploit:(Invalid)Skill Mod: [%s] Value: [%d] Max Legal Value [%d]", skillMods[j].first.c_str(), skillMods[j].second, maxValue));
						else if(currentValue > 35)
							LOG("CustomerService", ("extraSocketExploit:(Invalid)Skill Mod: [%s] Value: [%d] Max Legal Value [35]", skillMods[j].first.c_str(), skillMods[j].second));
						else
							LOG("CustomerService", ("extraSocketExploit:Skill Mod: [%s] Value: [%d]", skillMods[j].first.c_str(), skillMods[j].second));
					}

					return;
				}
			}
		}
	}

	// start the decay process going
}

//-----------------------------------------------------------------------

void TangibleObject::virtualOnSetAuthority()
{
	ServerObject::virtualOnSetAuthority();
}

//-----------------------------------------------------------------------

TangibleObject * TangibleObject::getTangibleObject(NetworkId const & networkId)
{
	return asTangibleObject(NetworkIdManager::getObjectById(networkId));
}

//-----------------------------------------------------------------------

TangibleObject * TangibleObject::asTangibleObject(Object * object)
{
	ServerObject * serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	return (serverObject != nullptr) ? serverObject->asTangibleObject() : nullptr;
}

//-----------------------------------------------------------------------

TangibleObject const * TangibleObject::asTangibleObject(Object const * object)
{
	ServerObject const * serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	return (serverObject != nullptr) ? serverObject->asTangibleObject() : nullptr;
}

//-----------------------------------------------------------------------

TangibleObject * TangibleObject::asTangibleObject()
{
	return this;
}

// ----------------------------------------------------------------------

TangibleObject const * TangibleObject::asTangibleObject() const
{
	return this;
}

// ----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * TangibleObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/tangible/base/shared_tangible_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "TangibleObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// TangibleObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void TangibleObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// TangibleObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Returns the name of the shared template that will be sent to the client when
 * this object is created. If this object has a custom appearance, that template
 * name will be returned, otherwise the default name will be returned.
 *
 *
 */
const char * TangibleObject::getClientSharedTemplateName(void) const
{
	if (!m_customAppearance.get().empty())
		return m_customAppearance.get().c_str();
	return ServerObject::getClientSharedTemplateName();
}	// TangibleObject::getClientSharedTemplateName

//-----------------------------------------------------------------------

void TangibleObject::addLocationTarget(const LocationData & location)
{
	if(isAuthoritative())
	{
		std::vector<LocationData>::const_iterator i;
		for(i = m_locationTargets.begin(); i != m_locationTargets.end(); ++i)
		{
			if((*i).name == location.name)
			{
				m_locationTargets.set(static_cast<size_t>(std::distance(m_locationTargets.begin(), i)), location);
				return;
			}
		}
		m_locationTargets.push_back(location);
		scheduleForAlter();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addLocationTarget, new MessageQueueGenericValueType<LocationData>(location));
	}
}

//-----------------------------------------------------------------------

void TangibleObject::removeLocationTarget(const Unicode::String & locationName)
{
	if(isAuthoritative())
	{
		std::vector<LocationData>::const_iterator i;
		for(i = m_locationTargets.begin(); i != m_locationTargets.end(); ++i)
		{
			if((*i).name == locationName)
			{
				m_locationTargets.erase(static_cast<size_t>(std::distance(m_locationTargets.begin(), i)));
				return;
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeLocationTarget, new MessageQueueGenericValueType<Unicode::String>(locationName));
	}
}

//-----------------------------------------------------------------------

void TangibleObject::onArrivedAtLocation(const LocationData & location)
{
	// only invokes a script trigger, which is automagically forwarded to
	// the authoritative server
	if(getScriptObject())
	{
		ScriptParams p;
		p.addParam(location.name);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_ARRIVE_AT_LOCATION, p));
	}
}
// ----------------------------------------------------------------------

/**
 * This function is called after the TangibleObject has received a
 * new custom appearance string.
 *
 * @arg value  the value of the new custom appearance string
 */
void TangibleObject::customAppearanceModified(const std::string& value)
{
	// we don't value to use the value arg since we already have the
	// data stored in m_customAppearance, but just in case the interface
	// changes, we'll use it here.

	if (value.empty())
		return;
}	// TangibleObject::customAppearanceModified

// ----------------------------------------------------------------------

/**
 * This function is called after the TangibleObject has received a
 * new appearance data customization string.
 *
 * @arg value  the value of the new appearance customization string
 */

// ----------------------------------------------------------------------

void TangibleObject::appearanceDataModified(const std::string& value)
{
	// we don't value to use the value arg since we already have the
	// data stored in m_appearanceData, but just in case the interface
	// changes, we'll use it here.  Technically we don't need to keep
	// the m_appearanceData string around on the client.  We can build
	// it at will from the appearance.

	//-- skip no-content customization data
	if (value.empty())
		return;

	// check if the property exists
	CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));

	if (!cdProperty)
	{
		// every debug warning I put here goes into never-never land in addr2line.  real handy feature...
		//	DEBUG_WARNING(true, ("non-zero-length appearance string sent to object that doesn't declare any customization variables in its template."));
		// DEBUG_WARNING(true, ("received appear string for non-customizable object."));
		return;
	}

	// retrieve the CustomizationData instance associated with the property
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	NOT_NULL(customizationData);

	//-- initialize CustomizationData variable values from the string
	customizationData->loadLocalDataFromString(value);

	//-- release local reference
	customizationData->release();
}
//-----------------------------------------------------------------------

/**
 * Gets all the equipped items corresponding to a combat skeleton "bone".
 *
 * @param combatBone		the bone we want equipment for
 * @param items				list to be filled in with the items
 */
void TangibleObject::getEquippedItems(uint32 combatBone, std::vector<TangibleObject *> &items) const
{
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
	if (container == nullptr)
		return;

	std::vector<Container::ContainedItem> itemIds;
	container->getObjectsForCombatBone(combatBone, itemIds);

	std::vector<Container::ContainedItem>::const_iterator iter;
	for (iter = itemIds.begin(); iter != itemIds.end(); ++iter)
	{
		Object * object = (*iter).getObject();
		if (object != nullptr)
		{
			TangibleObject * item = dynamic_cast<TangibleObject *>(object);
			if (item != nullptr)
				items.push_back(item);
		}
	}
}	// TangibleObject::getEquippedItems

//-----------------------------------------------------------------------

/**
 * Returns a random equipped item corresponding to a combat skeleton "bone".
 *
 * @param combatBone		the bone we want the item for
 *
 * @return the item
 */
TangibleObject * TangibleObject::getRandomEquippedItem(uint32 combatBone) const
{
	std::vector<TangibleObject *> items;
	getEquippedItems(combatBone, items);

	if (items.empty())
	{
		return nullptr;
	}

	int index = Random::random(0, items.size() - 1);
	return items[index];
}	// TangibleObject::getRandomEquippedItem

//-----------------------------------------------------------------------

/**
 * Updates the object for damage taken in battle.
 *
 * @param time		current game time
 *
 * @return base class result
 */
float TangibleObject::alter(real time)
{
	PROFILER_AUTO_BLOCK_DEFINE("TangibleObject::alter");

	if (isAuthoritative())
	{
		// Determine the combat state of the object
		{
			CombatEngine::alter(*this);

			m_hateList.alter();

			if (!m_hateList.isEmpty())
			{
				if (!isPlayerControlled())
				{
					// AI

					// Schedule the AI for an update
					{
						AiCombatPulseQueue::schedule(this, 0, static_cast<unsigned long>(time * 1000.0f));
					}
				}
				else
				{
					// Player

					// Make sure the hate list is purged of invalid hate targets. If a player is no
					// longer hated by a hate target, then remove that target from the player's list.
					// A player must be in their hate target's hate list and vice versa.
					{
						typedef std::vector<NetworkId> PurgeList;
						static PurgeList purgeList;
						purgeList.clear();

						HateList::UnSortedList const & unSortedHateList = getUnSortedHateList();
						HateList::UnSortedList::const_iterator iterUnSortedHateList = unSortedHateList.begin();

						for (; iterUnSortedHateList != unSortedHateList.end(); ++iterUnSortedHateList)
						{
							CachedNetworkId const & id = iterUnSortedHateList->first;

							if (!isHatedBy(id.getObject()))
							{
								purgeList.push_back(id);
							}
						}

						PurgeList::const_iterator iterPurgeList = purgeList.begin();

						for (; iterPurgeList != purgeList.end(); ++iterPurgeList)
						{
							removeHateTarget(*iterPurgeList);
						}
					}
				}
			}

			if (m_hateList.isEmpty())
			{
				if (   !isPlayerControlled()
					|| (getCombatDuration() > s_minCombatDuration))
				{
					setInCombat(false);
				}
			}
		}
	}

	// check for location target arrival
	if(isAuthoritative() && !m_locationTargets.empty())
	{
		Vector const effectiveWorldLocation = findPosition_w();
		for (std::vector<LocationData>::const_iterator i = m_locationTargets.begin(); i != m_locationTargets.end(); ++i)
		{
			if ((*i).scene == ServerWorld::getSceneId())
			{
				Sphere const &s = (*i).location;
				// 3d targets are denoted by y coordinates > 50k, with 100k added to them
				// If it's a 3d target, add 100k to our y coordinate
				// If it's a 2d target, set our y coordinate to the target's
				// 2d comparison, put locations on same plane
				Vector v = effectiveWorldLocation;
				if (s.getCenter().y > 50000.0f)
					v.y += 100000.0f;
				else
					v.y = s.getCenter().y;

				if (s.contains(v))
				{
					// copy the location data so we can remove it from the vector prior to triggerring
					LocationData triggeredLocation(*i);
					removeLocationTarget((*i).name);
					// trigger any scripts
					onArrivedAtLocation(triggeredLocation);
					// we'll catch any additional location triggers going off during the next alter
					break;
				}
			}
		}
	}

	//-- handle modified customization data
	if (isCustomizationDataModified())
	{
		// associated CustomizationData instance is modified.  Pack CustomizationData
		// into m_appearanceData string, which will get sent to others.

		//-- get property
		CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (!cdProperty)
		{
			DEBUG_WARNING(true, ("customization data modified but no customization data!"));
		}
		else
		{
			// retrieve the CustomizationData instance associated with the property
			CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
			NOT_NULL(customizationData);

			//-- initialize CustomizationData variable values from the string
			m_appearanceData.set(customizationData->writeLocalDataToString());

			//-- release reference
			customizationData->release();

		}

		// reset flag
		setCustomizationDataModified(false);
	}

	return ServerObject::alter(time);
}	// TangibleObject::alter

//-----------------------------------------------------------------------

float TangibleObject::getNextAlterTime(float baseAlterTime) const
{
	if (baseAlterTime == AlterResult::cms_kill || baseAlterTime == AlterResult::cms_alterNextFrame)
		return baseAlterTime;

	float result = baseAlterTime;

	if (getSynchronizedUi() && dynamic_cast<const CraftingToolSyncUi *>(getSynchronizedUi()))
		return AlterResult::cms_alterNextFrame;

	if (isAuthoritative())
	{
		if (isInCombat())
			return AlterResult::cms_alterNextFrame;

		if (!m_locationTargets.empty())
			AlterResult::incorporateAlterResult(result, ConfigServerGame::getLocationTargetCheckIntervalSec());
	}

	return result;
}

//-----------------------------------------------------------------------

void TangibleObject::conclude()
{
	PROFILER_AUTO_BLOCK_DEFINE("TangibleObject::conclude");

	// if we are a crafting tool, conclude our prototype and manf schematic
	if (getSynchronizedUi() != nullptr)
	{
		const CraftingToolSyncUi * sync = dynamic_cast<const CraftingToolSyncUi *>(
			getSynchronizedUi());
		if (sync != nullptr)
		{
			Object * object = sync->getPrototype().getObject();
			if (object != nullptr)
				object->conclude();
			object = sync->getManfSchematic().getObject();
			if (object != nullptr)
				object->conclude();
		}
	}

	ServerObject::conclude();
}

//-----------------------------------------------------------------------

float TangibleObject::getPvpRegionCheckTime()
{
	return ConfigServerGame::getTangiblePvpRegionCheckTime();
}

//-----------------------------------------------------------------------

void TangibleObject::setPvpType(Pvp::PvpType pvpType)
{
	FATAL(!isAuthoritative(), ("Inappropriate use of TangibleObject::setPvpType - this should only be called from the Pvp interface."));
	Pvp::PvpType oldType = static_cast<Pvp::PvpType>(m_pvpType.get());
	if (oldType != pvpType)
	{
		m_pvpType.set(static_cast<int>(pvpType));
		m_pvpFutureType.set(-1);
		if (getScriptObject())
		{
			ScriptParams params;
			params.addParam(oldType);
			params.addParam(pvpType);
			IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_PVP_TYPE_CHANGED, params));
		}
	}
}

//-----------------------------------------------------------------------

void TangibleObject::setPvpFutureType(Pvp::PvpType pvpFutureType)
{
	FATAL(!isAuthoritative(), ("Inappropriate use of TangibleObject::setPvpFutureType - this should only be called from the Pvp interface."));
	Pvp::PvpType oldFutureType = static_cast<Pvp::PvpType>(m_pvpFutureType.get());
	if (oldFutureType != pvpFutureType)
	{
		m_pvpFutureType.set(static_cast<int>(pvpFutureType));
	}
}

//-----------------------------------------------------------------------

void TangibleObject::setPvpFaction(Pvp::FactionId factionId)
{
	FATAL(!isAuthoritative(), ("Inappropriate use of TangibleObject::setPvpFaction - this should only be called from the Pvp interface."));
	Pvp::FactionId oldFaction = static_cast<Pvp::FactionId>(m_pvpFaction.get());
	if (oldFaction != factionId)
	{
		m_pvpFaction.set(static_cast<uint32>(factionId));
		if (getScriptObject())
		{
			ScriptParams params;
			params.addParam(static_cast<int>(oldFaction));
			params.addParam(static_cast<int>(factionId));
			IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_PVP_FACTION_CHANGED, params));
		}

		// if player character is changing faction, clear
		// out current accumulated CGW points and rating;
		// this *MUST* be done after m_pvpFaction has
		// been set to the new faction because 
		// modifyCurrentGcwRating() uses the faction information
		CreatureObject * creature = asCreatureObject();
		if (creature)
		{
			PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
			if (player)
			{
				// decrementing the value by a really large
				// number will cause it to reset to 0
				static int const min = std::numeric_limits<int>::min();

				player->modifyCurrentGcwPoints(min, false);
				player->modifyCurrentGcwRating(min, true);

				// if player is no longer factional, reset current pvp kills
				if (!PvpData::isImperialFactionId(factionId) && !PvpData::isRebelFactionId(factionId))
					player->modifyCurrentPvpKills(min, false);

				// if new faction is not neutral, remove any mercenary status
				if (!PvpData::isNeutralFactionId(factionId))
					Pvp::setNeutralMercenaryFaction(*creature, PvpData::getNeutralFactionId(), PvpType_Neutral);
			}
		}
	}
}

//-----------------------------------------------------------------------

void TangibleObject::setPvpMercenaryFaction(Pvp::FactionId factionId, Pvp::PvpType pvpType)
{
	FATAL(!isAuthoritative(), ("Inappropriate use of TangibleObject::setPvpMercenaryFaction - this should only be called from the Pvp interface."));

	// this only applies to neutral
	if (!PvpData::isNeutralFactionId(m_pvpFaction.get()) && !PvpData::isNeutralFactionId(factionId))
		return;

	// this only applies to player character
	PlayerObject * const po = PlayerCreatureController::getPlayerObject(asCreatureObject());
	if (!po || !po->isAuthoritative())
		return;

	// see if mercenary functionality is allowed
	if (!PvpData::isNeutralFactionId(factionId))
	{
		if (PvpData::isImperialFactionId(factionId))
		{
			if (pvpType == PvpType_Covert)
			{
				if (!ConfigServerGame::getEnableCovertImperialMercenary())
					return;

				po->modifyCollectionSlotValue("covert_imperial_mercenary", 1);
				po->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
				po->modifyCollectionSlotValue("overt_rebel_mercenary", -1);

				std::string const & currentTitle = po->getTitle();
				if (po->hasCompletedCollectionSlot("covert_imperial_mercenary"))
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle("covert_imperial_mercenary");
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle("covert_imperial_sympathizer");
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle("covert_imperial_loyalist");
				}
				else
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle(std::string());
				}
			}
			else if (pvpType == PvpType_Declared)
			{
				if (!ConfigServerGame::getEnableOvertImperialMercenary())
					return;

				po->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("overt_imperial_mercenary", 1);
				po->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
				po->modifyCollectionSlotValue("overt_rebel_mercenary", -1);

				std::string const & currentTitle = po->getTitle();
				if (po->hasCompletedCollectionSlot("overt_imperial_mercenary"))
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle("overt_imperial_mercenary");
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle("overt_imperial_sympathizer");
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle("overt_imperial_loyalist");
				}
				else
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle(std::string());
				}
			}
			else
			{
				return;
			}
		}
		else if (PvpData::isRebelFactionId(factionId))
		{
			if (pvpType == PvpType_Covert)
			{
				if (!ConfigServerGame::getEnableCovertRebelMercenary())
					return;

				po->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("covert_rebel_mercenary", 1);
				po->modifyCollectionSlotValue("overt_rebel_mercenary", -1);

				std::string const & currentTitle = po->getTitle();
				if (po->hasCompletedCollectionSlot("covert_rebel_mercenary"))
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle("covert_rebel_mercenary");
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle("covert_rebel_sympathizer");
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle("covert_rebel_insurgent");
				}
				else
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle(std::string());
				}
			}
			else if (pvpType == PvpType_Declared)
			{
				if (!ConfigServerGame::getEnableOvertRebelMercenary())
					return;

				po->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
				po->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
				po->modifyCollectionSlotValue("overt_rebel_mercenary", 1);

				std::string const & currentTitle = po->getTitle();
				if (po->hasCompletedCollectionSlot("overt_rebel_mercenary"))
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle("overt_rebel_mercenary");
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle("overt_rebel_sympathizer");
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle("overt_rebel_insurgent");
				}
				else
				{
					if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer"))
						po->setTitle(std::string());
					else if ((currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
						po->setTitle(std::string());
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		po->modifyCollectionSlotValue("covert_imperial_mercenary", -1);
		po->modifyCollectionSlotValue("overt_imperial_mercenary", -1);
		po->modifyCollectionSlotValue("covert_rebel_mercenary", -1);
		po->modifyCollectionSlotValue("overt_rebel_mercenary", -1);

		std::string const & currentTitle = po->getTitle();
		if ((currentTitle == "covert_imperial_mercenary") || (currentTitle == "overt_imperial_mercenary") || (currentTitle == "covert_rebel_mercenary") || (currentTitle == "overt_rebel_mercenary") ||
		    (currentTitle == "covert_imperial_sympathizer") || (currentTitle == "overt_imperial_sympathizer") || (currentTitle == "covert_rebel_sympathizer") || (currentTitle == "overt_rebel_sympathizer") ||
		    (currentTitle == "covert_imperial_loyalist") || (currentTitle == "overt_imperial_loyalist") || (currentTitle == "covert_rebel_insurgent") || (currentTitle == "overt_rebel_insurgent"))
		{
			po->setTitle(std::string());
		}
	}

	// flag when the player stopped being a factional helper for the particular faction
	if (m_pvpMercenaryFaction.get() != factionId)
	{
		if (PvpData::isImperialFactionId(m_pvpMercenaryFaction.get()))
		{
			IGNORE_RETURN(setObjVarItem("factionalHelper.timeStopHelpingImperial", static_cast<int>(::time(nullptr))));
			removeObjVarItem("factionalHelper.timeStopHelpingRebel");
		}
		else if (PvpData::isRebelFactionId(m_pvpMercenaryFaction.get()))
		{
			IGNORE_RETURN(setObjVarItem("factionalHelper.timeStopHelpingRebel", static_cast<int>(::time(nullptr))));
			removeObjVarItem("factionalHelper.timeStopHelpingImperial");
		}
	}

	m_pvpMercenaryFaction.set(static_cast<uint32>(factionId));
	m_pvpMercenaryType.set(((PvpData::isNeutralFactionId(factionId)) ? PvpType_Neutral : static_cast<int>(pvpType)));
}

//-----------------------------------------------------------------------

Region const *TangibleObject::getPvpRegion() const
{
	uint32 crc = m_pvpRegionCrc.get();
	if (crc)
		return Region::findRegionByNameCrc(crc);
	return 0;
}

//-----------------------------------------------------------------------

/**
 * Sets the pvp region this object is in.
 *
 * @param pvpRegion		the pvp region
 */
void TangibleObject::setPvpRegion(Region const * pvpRegion)
{
	if (isAuthoritative())
	{
		uint32 crc = pvpRegion ? pvpRegion->getNameCrc() : 0;
		if (m_pvpRegionCrc.get() != crc)
			m_pvpRegionCrc = crc;
	}
	else
	{
		FATAL(true, ("TangibleObject::setPvpRegion : send message to auth object!!!"));
	}
}

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* TangibleObject::createDefaultController(void)
{
	Controller* controller = new TangibleController(this);

	setController(controller);
	return controller;
}	// TangibleObject::createDefaultController

//-----------------------------------------------------------------------

void TangibleObject::forwardServerObjectSpecificBaselines() const
{
	ServerObject::forwardServerObjectSpecificBaselines();
}

//-----------------------------------------------------------------------

TangibleObject const &TangibleObject::getPvpViewer(Client const &client) const
{
	TangibleObject const * const clientObj = safe_cast<TangibleObject const *>(client.getCharacterObject());
	FATAL(!clientObj, ("Client trying to get pvp viewer with no character object?"));
	return *clientObj;
}

//-----------------------------------------------------------------------

void TangibleObject::sendObjectSpecificBaselinesToClient(Client const &client) const
{
	ServerObject::sendObjectSpecificBaselinesToClient(client);
	if (!isNonPvpObject())
	{
		uint32 flags, factionId;
		Pvp::getClientVisibleStatus(client, *this, flags, factionId);
		UpdatePvpStatusMessage const statusMessage(getNetworkId(), flags, factionId);
		client.send(statusMessage, true);
		PvpUpdateObserver::updatePvpStatusCache(&client, *this, flags, factionId);
	}
}

//-----------------------------------------------------------------------

/**
 * Flags the object as being cached on the client or not.
 */
void TangibleObject::setCacheVersion(const int cacheVersion)
{
	ServerObject::setCacheVersion(cacheVersion);

	// if we are cached, we need to be made invulnerable
	// @todo: if we are not cached, do we need to change our invulnerability?
	if (cacheVersion > 0)
		setInvulnerable(true);
}	// TangibleObject::setCacheVersion

//-----------------------------------------------------------------------

/**
 * Allows an object to do pre-deletion cleanup.
 */
void TangibleObject::onRemovingFromWorld()
{
	ServerObject::onRemovingFromWorld();

	clearHateList();
}

// ----------------------------------------------------------------------

void TangibleObject::unload()
{
	ServerObject::unload();
}

// ----------------------------------------------------------------------

void TangibleObject::onPermanentlyDestroyed()
{
	ServerObject::onPermanentlyDestroyed();

	if (isAuthoritative())
	{
		clearHateList();

		if (isCraftingTool())
		{
			ServerObject * owner = ServerWorld::findObjectByNetworkId(getOwnerId());
			if (owner != nullptr && owner->asCreatureObject() != nullptr)
			{
				PlayerObject * player = PlayerCreatureController::getPlayerObject(
					owner->asCreatureObject());
				if (player != nullptr && player->isCrafting() && player->getCraftingTool() == getNetworkId())
					player->stopCrafting(false);
			}
		}
	}

	if (isWaypoint())
	{
		ServerPathfindingNotification::destroyWaypoint(this);
	}
}

//-----------------------------------------------------------------------

/**
 * Creates a synchronized ui class in order to get data to the client during a
 * crafting session.
 *
 * @return the newly created class
 */
ServerSynchronizedUi * TangibleObject::createSynchronizedUi ()
{
	return new CraftingToolSyncUi(*this);
}


// ======================================================================
// TangibleObject private static member functions
// ======================================================================

void TangibleObject::customizationDataModificationCallback(const CustomizationData & /* customizationData */, const void *context)
{
	if (!context)
		return;

	//-- turn context into TangibleObject
	TangibleObject *const tangibleObject = const_cast<TangibleObject*>(reinterpret_cast<const TangibleObject*>(context));

	//-- set the tangible object's customization data modification flag now.
	//   The reason we don't handle the customization data change now is that
	//   we want to lump up all the changes that occur during one server
	//   frame and handle the string conversion once.
	tangibleObject->setCustomizationDataModified(true);
}

// ======================================================================
// TangibleObject private member functions
// ======================================================================

void TangibleObject::handleContainerSetup()
{

}

//-----------------------------------------------------------------------

void TangibleObject::setOwnerId(NetworkId const &id)
{
	if(isAuthoritative())
	{
		// if we are changing owners, we are no longer insured
		bool ownerChanged = false;
		if (m_ownerId.get() != id)
		{
			ownerChanged = true;
			setInsured(false);
		}

		m_ownerId = id;
		Container* container = ContainerInterface::getContainer(*this);
		if (container)
		{
			ContainerIterator i = container->begin();
			for (; i != container->end(); ++i)
			{
				ServerObject* content = safe_cast<ServerObject*>((*i).getObject());
				if (content && !dynamic_cast<CreatureObject*>(content))
				{
					content->setOwnerId(id);
				}
			}
		}

		// if we're hidden, who can see us has changed
		if (ownerChanged && isInWorld() && isHidden())
			visibilityDataModified();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setOwnerId, new MessageQueueNetworkId(id));
	}
}
//-----------------------------------------------------------------------

bool TangibleObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{
	const int got = getGameObjectType ();
	if (got == SharedObjectTemplate::GOT_misc_crafting_station && destination && !destination->asCellObject())
	{
		//Only allow stations to be trasferred if their hoppers are empty
		Container* container = ContainerInterface::getContainer(*this);
		if (container)
		{
			ContainerIterator i = container->begin();
			for (; i != container->end(); ++i)
			{
				ServerObject* content = safe_cast<ServerObject*>((*i).getObject());
				if (!isEmpty(*content))
				{

					if (transferer && transferer->getClient())
						ContainerInterface::sendContainerMessageToClient(*transferer, Container::CEC_HopperNotEmpty);
					return false;
				}
			}
		}
	}
	if (destination != nullptr && getObjVars().hasItem(OBJVAR_BIO_LINK_ID))
	{
		// if this item is bio-linked and a player is trying to equip it, make
		// sure the link id matches the player's id
		if (destination->asCreatureObject() != nullptr)
		{
			// allow holograms to have biolinked items transferred to them
			int hologramVal = 0;
			destination->getObjVars().getItem("hologram_performer", hologramVal);
			
			NetworkId linkId(getBioLink());
			if (linkId != destination->getNetworkId() && hologramVal == 0)
			{
				if (transferer && transferer->getClient())
					ContainerInterface::sendContainerMessageToClient(*transferer, Container::CEC_BioLinkedToOtherPlayer);
				return false;
			}
		}

	    // Check to see if we're trying to appearance equip a biolinked item.
		Object * containedBy = ContainerInterface::getContainedByObject(*destination);
		if(containedBy && containedBy->asServerObject() && containedBy->asServerObject()->asCreatureObject())
		{
			CreatureObject * creatureContainer = containedBy->asServerObject()->asCreatureObject();
			if(creatureContainer && creatureContainer->getAppearanceInventory() && creatureContainer->getAppearanceInventory() == destination)
			{
				// We're trying to appearance equip a biolinked item. Make sure the BioLink ID matches our player ID.
				NetworkId linkId(getBioLink());
				if (linkId != creatureContainer->getNetworkId())
				{
					if (transferer && transferer->getClient())
						ContainerInterface::sendContainerMessageToClient(*transferer, Container::CEC_BioLinkedToOtherPlayer);
					return false;
				}
			}
		}
	}
	return ServerObject::onContainerAboutToTransfer(destination, transferer);
}

//-----------------------------------------------------------------------

bool TangibleObject::onContainerChildAboutToLoseItem(ServerObject *destination, ServerObject &item, ServerObject *transferer)
{
	int const got = getGameObjectType();
	if (transferer && got == SharedObjectTemplate::GOT_misc_crafting_station && !isInWorld())
		return false;

	if (getPortalProperty() && !onPobChildAboutToLoseItem(destination, item, transferer))
		return false;

	return ServerObject::onContainerChildAboutToLoseItem(destination, item, transferer);
}

//-----------------------------------------------------------------------

bool TangibleObject::onContainerChildAboutToGainItem(ServerObject &item, ServerObject *destination, ServerObject *transferer)
{
	int const got = getGameObjectType();
	if (transferer && got == SharedObjectTemplate::GOT_misc_crafting_station && !isInWorld())
		return false;

	if (getPortalProperty() && !onPobChildAboutToGainItem(item, destination, transferer))
		return false;

	return ServerObject::onContainerChildAboutToGainItem(item, destination, transferer);
}

// ----------------------------------------------------------------------

void TangibleObject::onContainerChildLostItem(ServerObject *destination, ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	if (getPortalProperty())
		onPobChildLostItem(destination, item, source, transferer);
	ServerObject::onContainerChildLostItem(destination, item, source, transferer);
}

// ----------------------------------------------------------------------

void TangibleObject::onContainerChildGainItem(ServerObject &item, ServerObject *source, ServerObject *transferer)
{
	if (getPortalProperty())
		onPobChildGainItem(item, source, transferer);
	ServerObject::onContainerChildGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

int TangibleObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	if (transferer && isCraftingTool())
	{
		return Container::CEC_BlockedByDestinationContainer;
	}
	return ServerObject::onContainerAboutToGainItem(item, transferer);
}

//----------------------------------------------------------------------

void TangibleObject::onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	ServerObject::onContainerGainItem(item, source, transferer);
	//do not set owners of creatures
	//and do not set the owner of items going into the bazaar
	if (getOwnerId() != NetworkId::cms_invalid && !item.asCreatureObject() && !isBazaarTerminal())
		item.setOwnerId(getOwnerId());
}

// ----------------------------------------------------------------------

Footprint *TangibleObject::getFootprint()
{
	CollisionProperty *const property = getCollisionProperty();
	if (property)
		return property->getFootprint();
	else
		return nullptr;
}

//-----------------------------------------------------------------------

Footprint const *TangibleObject::getFootprint() const
{
	CollisionProperty const *const property = getCollisionProperty();
	if (property)
		return property->getFootprint();
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Sets up the object for being in or out of combat.
 *
 * @param inCombat		flag to put the object in combat-ready state or not
 *
 * @return true if state was set, false if something went wrong
 */
void TangibleObject::setInCombat(bool inCombat)
{
	m_inCombat = inCombat;
}

//-----------------------------------------------------------------------

/**
 * If this object is in combat, adds an aim to it.
 */
void TangibleObject::addAim(void)
{
	if(isAuthoritative())
	{
		if (isInCombat())
		{
			CombatEngineData::AttackData & attackData = NON_NULL(getCombatData())->attackData;

			if (attackData.aims < ConfigCombatEngine::getMaxAims())
			{
				++attackData.aims;
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addAim, 0);
	}
}	// TangibleObject::addAim

//-----------------------------------------------------------------------

/**
 * If this object is in combat, clear's it's aims.
 */
void TangibleObject::clearAims(void)
{
	if(isAuthoritative())
	{
		if (isInCombat())
		{
			CombatEngineData::AttackData & attackData = NON_NULL(getCombatData())->attackData;
			attackData.aims = 0;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearAims, 0);
	}
}	// TangibleObject::clearAims

//-----------------------------------------------------------------------

/**
 * Applies damage to the object. No other function should modify m_damageTaken.
 *
 * @param delta					amount of damage to apply
 * @param ignoreInvulnerable	flag to ignore the invulnerable setting of this object
 * @param source				who is doing the damage
 *
 * @return the amount of damage actually applied
 */
int TangibleObject::alterHitPoints(int delta, bool ignoreInvulnerable,
	const NetworkId & source)
{
	if(isAuthoritative())
	{
#ifdef _DEBUG
		char debugBuffer[1024];
		const char * craftedString = "";
		Client *client = nullptr;
		ServerObject * sourceObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(source));
		if (source != NetworkId::cms_invalid && sourceObject != nullptr)
			client = sourceObject->getClient();
#endif

		// Only consider tangible invulnerability rules for this case
		if (isDisabled() || (!ignoreInvulnerable && TangibleObject::isInvulnerable()))
			return 0;

		int totalHitPoints = getMaxHitPoints();
		int totalDamageTaken = m_damageTaken.get() - delta;

#ifdef _DEBUG
		if (ConfigCombatEngine::getDebugDamage())
		{
			snprintf(debugBuffer, sizeof(debugBuffer), "%s object %s (hp=%d) damaged "
				"from %d to %d\n", craftedString, getNetworkId().getValueString().c_str(),
				totalHitPoints,	m_damageTaken.get(), totalDamageTaken);
			DEBUG_REPORT_LOG (true, ("%s", debugBuffer));
			if (source != NetworkId::cms_invalid && client != nullptr)
				ConsoleMgr::broadcastString(debugBuffer, client);
		}
#endif

		if (totalDamageTaken >= totalHitPoints)
		{
			// clip delta and damage taken
			if (isDisabled())
				delta = 0;
			else
				delta += totalDamageTaken - totalHitPoints;
			totalDamageTaken = totalHitPoints;
		}
		else if (totalDamageTaken < 0)
			totalDamageTaken = 0;
		if (totalDamageTaken == totalHitPoints)
			setDisabled(true);

		m_damageTaken = totalDamageTaken;

		if (isDisabled())
		{
#ifdef _DEBUG
			if (ConfigCombatEngine::getDebugDamage())
			{
				snprintf(debugBuffer, sizeof(debugBuffer), "%s object %s has been "
					"disabled\n", craftedString, getNetworkId().getValueString().c_str());
				DEBUG_REPORT_LOG (true, ("%s", debugBuffer));
				if (source != NetworkId::cms_invalid && client != nullptr)
					ConsoleMgr::broadcastString(debugBuffer, client);
			}
#endif
			ScriptParams params;
			params.addParam(source);
			IGNORE_RETURN(getScriptObject()->trigAllScripts(
				Scripting::TRIG_OBJECT_DISABLED, params));
		}
		scheduleForAlter();
		return delta;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_alterHitPoints, new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(delta, source)));
		return 0;
	}
}	// TangibleObject::alterHitpoints

//-----------------------------------------------------------------------

/**
 * Applies damage done during combat to the object.
 *
 * @param damageData		the damage
 */
void TangibleObject::applyDamage(const CombatEngineData::DamageData &damageData)
{
	int totalDamage = 0;

	WARNING_STRICT_FATAL(! isAuthoritative(), ("TangibleObject::applyDamage() invoked on a NON-AUTHORITATIVE object %s:%s.", getObjectTemplateName(), getNetworkId().getValueString().c_str()));

	// it is possible for damageData to go bad if this object is being damaged from
	// combat and it goes out of combat, so we copy the items we need off of it
	const NetworkId attackerId(damageData.attackerId);
	const NetworkId weaponId(damageData.weaponId);

	std::vector<AttribMod::AttribMod>::const_iterator iter;
	for (iter = damageData.damage.begin(); !isDisabled() && iter != damageData.damage.end();
		++iter)
	{
		const AttribMod::AttribMod &mod = *iter;
		if (mod.value == 0)
			continue;

		if (AttribMod::isDirectDamage(mod))
		{
			// apply instantanious damage directly
			totalDamage -= alterHitPoints(mod.value, damageData.ignoreInvulnerable,
				attackerId);
		}
		else
		{
			WARNING(true, ("Trying to apply non-immediate damage to tangible "
				"object %s. This isn't supported, the damage won't be applied",
				getNetworkId().getValueString().c_str()));
		}
	}

	// tell scripts we have taken combat damage
	if (attackerId != NetworkId::cms_invalid && totalDamage > 0)
	{
		NOT_NULL(getScriptObject());
		ScriptParams params;
		params.addParam(attackerId);
		params.addParam(weaponId);
		params.addParam(totalDamage);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_OBJECT_DAMAGED,
			params));
	}
}

// ----------------------------------------------------------------------

/**
 * Sends a message to the Planet Server to update the position of the object.
 */
void TangibleObject::updatePlanetServerInternal(const bool forceUpdate) const
{
	// Don't send updates for contained tangibles without an interest radius, and only send when forced or moving
	if ((forceUpdate || getPositionChanged()) && (!ContainerInterface::getContainedByObject(*this) || getInterestRadius() > 0))
	{
		Object const * const topmostContainer = ContainerInterface::getTopmostContainer(*this);
		FATAL(!topmostContainer, ("Object %s was contained by something, but getTopmostContainer() returned nullptr\n",getNetworkId().getValueString().c_str()));
		Vector const &position = topmostContainer->getPosition_p();

		UpdateObjectOnPlanetMessage const msg(
			getNetworkId(),
			topmostContainer == this ? NetworkId::cms_invalid : topmostContainer->getNetworkId(),
			static_cast<int>(position.x),
			static_cast<int>(position.y),
			static_cast<int>(position.z),
			getInterestRadius(),
			static_cast<int> (getLocationReservationRadius()),
			false,
			false,
			static_cast<int>(getObjectType()),
			0,
			false,
			getTemplateCrc(),
			-1,
			-1);
		GameServer::getInstance().sendToPlanetServer(msg);
	}
}

//----------------------------------------------------------------------

/**
 * Clears our damage list.
 */
void TangibleObject::clearDamageList(void)
{
	if (m_combatData != nullptr)
	{
		m_combatData->defenseData.damage.clear();
	}
}

//----------------------------------------------------------------------

bool TangibleObject::addHate(NetworkId const & target, float const hate)
{
	bool result = false;

	if (isAuthoritative())
	{
		if (m_hateList.addHate(target, hate))
		{
			result = true;

			CombatTracker::addDefender(this);
			setInCombat(true);
			scheduleForAlter();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addHate, new MessageQueueGenericValueType<std::pair<NetworkId, float> >(std::make_pair(target, hate)));
	}

	return result;
}

//----------------------------------------------------------------------

void TangibleObject::addHateOverTime(NetworkId const & target, float const hate, int const seconds)
{
	if (seconds <= 0)
		return;

	if (isAuthoritative())
	{
		unsigned long const currentGameTime =  ServerClock::getInstance().getGameTimeSeconds();
		unsigned long const endGameTime = currentGameTime + seconds;

		IGNORE_RETURN(addHate(target, hate));

		bool const firstHateOverTime = m_hateOverTime.empty();
		m_hateOverTime.set(target, std::make_pair(hate, std::make_pair(currentGameTime, endGameTime)));

		if (firstHateOverTime)
			MessageToQueue::sendMessageToC(getNetworkId(), "C++ProcessHateOverTime", std::string(), 1, false);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addHateOverTime, new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > >(std::make_pair(target, std::make_pair(seconds, hate))));
	}
}

//----------------------------------------------------------------------

bool TangibleObject::setHate(NetworkId const & target, float const hate)
{
	bool result = false;

	if (isAuthoritative())
	{
		if (m_hateList.setHate(target, hate))
		{
			result = true;

			CombatTracker::addDefender(this);
			setInCombat(true);
			scheduleForAlter();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setHate, new MessageQueueGenericValueType<std::pair<NetworkId, float> >(std::make_pair(target, hate)));
	}

	return result;
}

//----------------------------------------------------------------------

float TangibleObject::getHate(NetworkId const & target) const
{
	return m_hateList.getHate(target);
}

//----------------------------------------------------------------------

float TangibleObject::getMaxHate() const
{
	return m_hateList.getMaxHate();
}

//----------------------------------------------------------------------

CachedNetworkId const & TangibleObject::getHateTarget() const
{
	return m_hateList.getTarget();
}

//----------------------------------------------------------------------

void TangibleObject::removeHateTarget(NetworkId const & target)
{
	if (isAuthoritative())
	{
		m_hateList.removeTarget(target);
		scheduleForAlter();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeHateTarget, new MessageQueueGenericValueType<NetworkId>(target));
	}
}

//----------------------------------------------------------------------

void TangibleObject::clearHateList()
{
	if (isAuthoritative())
	{
		m_hateList.clear();
		scheduleForAlter();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearHateList, nullptr);
	}
}

//-----------------------------------------------------------------------

HateList::UnSortedList const & TangibleObject::getUnSortedHateList() const
{
	return m_hateList.getUnSortedList();
}

//-----------------------------------------------------------------------

void TangibleObject::getSortedHateList(HateList::SortedList & sortedHateList) const
{
	m_hateList.getSortedList(sortedHateList);
}

//-----------------------------------------------------------------------

void TangibleObject::verifyHateList()
{
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("TangibleObject::verifyHateList() owner(%s:%s)", getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(getEncodedObjectName()).c_str()));

	m_hateList.alter();
}

//-----------------------------------------------------------------------

bool TangibleObject::isHatedBy(Object * const object)
{
	bool result = false;
	TangibleObject * const hatedByTangibleObject = TangibleObject::asTangibleObject(object);

	if (hatedByTangibleObject != nullptr)
	{
		HateList::UnSortedList const & hateList = hatedByTangibleObject->getUnSortedHateList();

		if (hateList.find(CachedNetworkId(*this)) != hateList.end())
		{
			result = true;
		}
	}

	return result;
}

//-----------------------------------------------------------------------

bool TangibleObject::isOnHateList(NetworkId const & target) const
{
	return m_hateList.isOnList(target);
}

//-----------------------------------------------------------------------

int TangibleObject::getTimeSinceLastHateListUpdate() const
{
	return m_hateList.getTimeSinceLastUpdate();
}

//-----------------------------------------------------------------------

bool TangibleObject::isHateListEmpty() const
{
	return m_hateList.isEmpty();
}

//-----------------------------------------------------------------------
HateList::RecentList const & TangibleObject::getRecentHateList() const
{
	return m_hateList.getRecentList();
}

//-----------------------------------------------------------------------
void TangibleObject::clearRecentHateList()
{
	m_hateList.clearRecentList();
	scheduleForAlter();
}

//-----------------------------------------------------------------------

void TangibleObject::resetHateTimer()
{
	if (isAuthoritative())
	{
		m_hateList.resetHateTimer();
		scheduleForAlter();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_resetHateTimer, nullptr);
	}
}

//-----------------------------------------------------------------------

void TangibleObject::setHateListAutoExpireTargetEnabled(bool const enabled)
{
	m_hateList.setAutoExpireTargetEnabled(enabled);

	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_autoExpireHateListTargetEnabled, new MessageQueueGenericValueType<bool>(enabled));
	}
}

//-----------------------------------------------------------------------

bool TangibleObject::isHateListAutoExpireTargetEnabled() const
{
	return m_hateList.isAutoExpireTargetEnabled();
}

//-----------------------------------------------------------------------

int TangibleObject::getHateListAutoExpireTargetDuration() const
{
	return m_hateList.getAutoExpireTargetDuration();
}

//-----------------------------------------------------------------------
float TangibleObject::getAILeashTime() const
{
	return m_hateList.getAILeashTime();
}


//-----------------------------------------------------------------------
void  TangibleObject::setAILeashTime( float time )
{
	m_hateList.setAILeashTime( time );
}


//-----------------------------------------------------------------------

/**
 * @return true if we are crafted, false if not
 */
bool TangibleObject::isCrafted(void) const
{
	return hasCondition(C_crafted);
}	// TangibleObject::isCrafted

//-----------------------------------------------------------------------

/**
 * Returns the id of the manufacturing schematic that was used to craft this
 * item, or NetworkId::cms_invalid if the item wasn't crafted.
 *
 * @return the schematic id
 */
const NetworkId TangibleObject::getCraftedId(void) const
{
	NetworkId craftedId(NetworkId::cms_invalid);
	getObjVars().getItem(OBJVAR_CRAFTING_SCHEMATIC,craftedId);
	return craftedId;
}	// TangibleObject::getCraftedId

//-----------------------------------------------------------------------

/**
 * Sets the id of the manufacturing schematic used to create this object.
 *
 * @param id		the schematic id
 */
void TangibleObject::setCraftedId(const NetworkId & id)
{
	if (isAuthoritative())
	{
		if (id == NetworkId::cms_invalid)
		{
			IGNORE_RETURN(removeObjVarItem(OBJVAR_CRAFTING_SCHEMATIC));
			m_sourceDraftSchematic = 0;
			clearCondition(C_crafted);
		}
		else
		{
			IGNORE_RETURN(setObjVarItem(OBJVAR_CRAFTING_SCHEMATIC, id));
			setCondition(C_crafted);
			const Object * object = NetworkIdManager::getObjectById(id);
			if (object != nullptr)
			{
				const ManufactureSchematicObject * schematic = dynamic_cast<
					const ManufactureSchematicObject *>(object);
				if (schematic != nullptr)
					m_sourceDraftSchematic = schematic->getDraftSchematic();
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCraftedId,
			new MessageQueueGenericValueType<NetworkId>(id));
	}
}	// TangibleObject::setCraftedId

//-----------------------------------------------------------------------

/**
 * Returns the crafting type of a crafting/repair tool.
 *
 * @return the crafting type
 */
int TangibleObject::getCraftingType(void) const
{
	int type = 0;
	getObjVars().getItem(OBJVAR_CRAFTING_TYPE, type);
	return type;
}	// TangibleObject::getCraftingType

//-----------------------------------------------------------------------

/**
 * Returns the crafted type of a crafted object.
 *
 * @return the crafted type
 */
int TangibleObject::getCraftedType(void) const
{
	int craftedType = 0;
	getObjVars().getItem(OBJVAR_CRAFTING_REPAIR_TYPE,craftedType);
	return craftedType;
}	// TangibleObject::getCraftedType

//-----------------------------------------------------------------------

/**
 * Sets the crafted type of a crafted object.
 *
 * @param type		the crafted type
 */
void TangibleObject::setCraftedType(int type)
{
	if (isAuthoritative())
	{
		IGNORE_RETURN(setObjVarItem(OBJVAR_CRAFTING_REPAIR_TYPE, type));
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCraftedType,
			new MessageQueueGenericValueType<int>(type));
	}
}	// TangibleObject::setCraftedType

//-----------------------------------------------------------------------

/**
 * Returns the id of the object's creator's id.
 *
 * @return the object's creator's id
 */
const NetworkId TangibleObject::getCreatorId(void) const
{
	if (m_creatorId.get() == NetworkId::cms_invalid)
	{
		// @todo remove this code when no object has the objvar
		// OBJVAR_CREATOR_ID
		NetworkId creatorId(NetworkId::cms_invalid);
		if (getObjVars().getItem(OBJVAR_CREATOR_ID, creatorId))
		{
			const_cast<TangibleObject*>(this)->setCreatorId(creatorId);
			return creatorId;
		}
	}
	return m_creatorId.get();
}	// TangibleObject::getCreatorId

//-----------------------------------------------------------------------

/**
 * Sets the creator id of this object.
 *
 * @param creatorId		the creator id
 */
void TangibleObject::setCreatorId(const NetworkId & creatorId)
{
	if (isAuthoritative())
	{
		m_creatorId = creatorId;
		if (getObjVars().hasItem(OBJVAR_CREATOR_ID))
			removeObjVarItem(OBJVAR_CREATOR_ID);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCreatorId,
			new MessageQueueGenericValueType<NetworkId>(creatorId));
	}
}	// TangibleObject::setCreatorId

//-----------------------------------------------------------------------

/**
 * Returns the id of the object's creator's name.
 *
 * @return the object's creator's name
 */
const Unicode::String TangibleObject::getCreatorName(void) const
{
	return Unicode::narrowToWide(NameManager::getInstance().getPlayerName(
		getCreatorId()));
}	// TangibleObject::getCreatorName

//-----------------------------------------------------------------------

/**
 * Returns the xp amount the creator of this object will get when it is "used".
 *
 * @return the xp amount
 */
int TangibleObject::getCreatorXp(void) const
{
	int creatorXp=0;
	getObjVars().getItem(OBJVAR_CREATOR_XP,creatorXp);
	return creatorXp;
}	// TangibleObject::getCreatorXp

//-----------------------------------------------------------------------

/**
 * Sets the xp amount the creator of this object will get when it is "used".
 *
 * @param xp		the xp amount
 */
void TangibleObject::setCreatorXp(int xp)
{
	if (isAuthoritative())
	{
		setObjVarItem(OBJVAR_CREATOR_XP, xp);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCreatorXp, new MessageQueueGenericValueType<int>(xp));
	}
}	// TangibleObject::setCreatorXp

//-----------------------------------------------------------------------

/**
 * Returns the xp type the creator of this object will get when it is "used".
 *
 * @return the xp type
 */
int TangibleObject::getCreatorXpType(void) const
{
	int creatorXpType = -1;
	getObjVars().getItem(OBJVAR_CREATOR_XP_TYPE,creatorXpType);
	return creatorXpType;
}	// TangibleObject::getCreatorXpType

//-----------------------------------------------------------------------

/**
 * Sets the xp type the creator of this object will get when it is "used".
 *
 * @param type		the xp type
 */
void TangibleObject::setCreatorXpType(int type)
{
	if (isAuthoritative())
	{
		setObjVarItem(OBJVAR_CREATOR_XP_TYPE, type);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCreatorXpType, new MessageQueueGenericValueType<int>(type));
	}
}	// TangibleObject::setCreatorXpType

//-----------------------------------------------------------------------

/**
 * Returns if this object is a crafting tool or not.
 *
 * @return true if we are a tool, false if not
 */
bool TangibleObject::isCraftingTool(void) const
{
	return getObjVars().hasItem(OBJVAR_CRAFTING_TOOL);
}	// TangibleObject::isCraftingTool

//-----------------------------------------------------------------------

/**
 * Returns if this object is a crafting station or not.
 *
 * @return true if we are a station, false if not
 */
bool TangibleObject::isCraftingStation(void) const
{
	return getObjVars().hasItem(OBJVAR_CRAFTING_STATION);
}	// TangibleObject::isCraftingStation

//-----------------------------------------------------------------------

/**
 * Returns if this object is a repair tool or not.
 *
 * @return true if we are a repair tool, false if not
 */
bool TangibleObject::isRepairTool(void) const
{
	return getObjVars().hasItem(OBJVAR_REPAIR_TOOL);
}	// TangibleObject::isRepairTool

//-----------------------------------------------------------------------

/**
* If this is a crafting tool, tries to start a session with a player.
*
* @param crafter		the player who wants to craft something
*
* @return true if the session was started, false if not
*/
bool TangibleObject::startCraftingSession(CreatureObject & crafter)
{
	static const SlotId outputSlotId(SlotIdManager::findSlotId(MANF_OUTPUT_SLOT_NAME));

	if(isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		PlayerObject * crafterPlayer = PlayerCreatureController::getPlayerObject(&crafter);
		if (crafterPlayer == nullptr)
			return false;

		// make sure we are a crafting tool
		if (!isCraftingTool())
		{
			DEBUG_WARNING(true, ("Tried to start crafting session on non-tool "
				"object %s", myIdString));
			return false;
		}

		if (isDisabled())
		{
			DEBUG_WARNING(true, ("Tried to start crafting session on disabled "
				"tool %s", myIdString));
			return false;
		}

		// make sure the tool isn't creating a prototype
		float realTimeVar = 0;
		getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_TIME,realTimeVar);
		if (realTimeVar > 0)
		{
			StringId message("system_msg", "crafting_tool_creating_prototype");
			Unicode::String outOfBand;
			Chat::sendSystemMessage(crafter, message, outOfBand);
			return false;
		}

		// make sure the tool isn't already being used
		if (getObjVars().hasItem(OBJVAR_CRAFTING_CRAFTER))
		{
			StringId message("system_msg", "crafting_tool_in_use");
			Unicode::String outOfBand;
			Chat::sendSystemMessage(crafter, message, outOfBand);
			return false;
		}

		// make sure the output slot is empty
		SlottedContainer const * const slotContainer = ContainerInterface::getSlottedContainer(*this);
		if (slotContainer == nullptr)
		{
			DEBUG_WARNING(true, ("Crafting tool %s does not have a container!", myIdString));
			return false;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (slotContainer->getObjectInSlot(outputSlotId, tmp) != Container::ContainedItem::cms_invalid)
		{
			StringId message("system_msg", "crafting_tool_full");
			Unicode::String outOfBand;
			Chat::sendSystemMessage(crafter, message, outOfBand);
			return false;
		}

		// get the schematics list from the player and filter for the type of
		// schematics we can make
		int myType = getCraftingType();

		bool sendComplexities = true;
		if (!crafter.isSchematicFilteringEnabled())
			sendComplexities = false;

		std::vector<int> goodSchematics;
		std::vector<float>  scriptComplexities;
		const std::map<std::pair<uint32, uint32>,int> & schematics = crafter.getDraftSchematics();
		std::map<std::pair<uint32, uint32>,int>::const_iterator iter;
		for (iter = schematics.begin(); iter != schematics.end(); ++iter)
		{
			const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic((*iter).first.first);
			if (schematic != nullptr && ((myType & ServerObjectTemplate::CT_genericItem) != 0 ||
				((schematic->getCategory() & myType) != 0)))
			{
				// make sure the player has all the skill commands needed for the
				// schematic
				int skillCount = schematic->getSkillCommandsCount();
				int i;
				for (i = 0; i < skillCount; ++i)
				{
					if (!crafter.hasCommand(schematic->getSkillCommands(i)))
						break;
				}
				if (i == skillCount)
				{
					goodSchematics.push_back((*iter).first.first);
					if (sendComplexities)
						scriptComplexities.push_back(schematic->getComplexity());
					else
						scriptComplexities.push_back(1.0f);
				}
			}
		}

		crafterPlayer->setCraftingTool(*this);

		if (goodSchematics.size() > 0)
		{
			ScriptParams params;
			params.addParam(crafter.getNetworkId());
			params.addParam(goodSchematics);
			params.addParam(scriptComplexities);
			if (getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_DRAFT_SCHEMATICS, params) != SCRIPT_CONTINUE)
			{
				std::vector<uint32> empty;
				crafterPlayer->sendUseableDraftSchematics (empty);
				crafterPlayer->stopCrafting(true);

				return false;
			}
			setObjVarItem(OBJVAR_CRAFTING_CRAFTER, crafter.getNetworkId());
		}
		else
		{
			std::vector<uint32> empty;
			crafterPlayer->sendUseableDraftSchematics (empty);
			crafterPlayer->stopCrafting(true);

			StringId message("system_msg", "no_valid_schematics");
			Unicode::String outOfBand;
			Chat::sendSystemMessage(crafter, message, outOfBand);
			return false;
		}

		// the following section to send the ingredients to the player is only needed for
		// testing crafting via the console
#ifdef _DEBUG

		// send the player info about the ingredients in the tool's hopper;
		// the actual sending of the messages could be spread out over a few
		// frames
//		listIngredientsInHopper(crafter);
#endif
	}
	else
	{
		sendControllerMessageToAuthServer(CM_startCraftingSession, new MessageQueueGenericValueType<NetworkId>(crafter.getNetworkId()));
	}
	return true;
}	// TangibleObject::startCraftingSession

//-----------------------------------------------------------------------

/**
 * If this is a crafting tool, tries to stop a session with a player.
 *
 * @return true if the session was stopped, false if not
 */
bool TangibleObject::stopCraftingSession(void)
{
	if (isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// make sure we are a crafting tool
		if (!isCraftingTool())
		{
			DEBUG_WARNING(true, ("Tried to stop crafting session on non-tool "
				"object %s", myIdString));
			return false;
		}

		// make sure a prototype object isn't being made (the session will be
		// stopped when the prototype is done)
		float realTimeVar = 0;
		if (getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_TIME, realTimeVar) && realTimeVar > 0)
			return false;

		// make sure the tool isn't already being used
		NetworkId crafterId;
		if (!getObjVars().getItem(OBJVAR_CRAFTING_CRAFTER,crafterId))
		{
			DEBUG_REPORT_LOG(true, ("Crafting tool %s in not in use\n", myIdString));
			return false;
		}

		clearCraftingManufactureSchematic();
		clearCraftingPrototype();
		removeObjVarItem(OBJVAR_CRAFTING_CRAFTER);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_stopCraftingSession, 0);
	}
	return true;
}	// TangibleObject::stopCraftingSession

//-----------------------------------------------------------------------

/**
 * Test to see if an object is in a crafting tool's ingredient hopper.
 *
 * @param ingredientId		object we want to test
 *
 * @return true if the object is in the hopper, false if not
 */
bool TangibleObject::isIngredientInHopper(const NetworkId & ingredientId) const
{
	ServerObject const * const hopper = getIngredientHopper();
	if (hopper == nullptr)
		return false;

	Object const * const ingredient = CachedNetworkId(ingredientId).getObject();
	if (ingredient == nullptr)
		return false;

	return ContainerInterface::isNestedWithin(*ingredient, getNetworkId());
}	// TangibleObject::isIngredientInHopper

//-----------------------------------------------------------------------

/**
 * Returns this crafting station's ingredient hopper.
 *
 * @return the hopper
 */
ServerObject * TangibleObject::getIngredientHopper(void) const
{
static const SlotId hopperSlotId(SlotIdManager::findSlotId(INGREDIENT_HOPPER_SLOT_NAME));

	const std::string myId(getNetworkId().getValueString());
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// make sure we are a crafting station
	if (!isCraftingStation())
	{
		DEBUG_WARNING(true, ("Tried to get input hopper for non-crafting station "
			"object %s", myIdString));
		return nullptr;
	}

	// get the ingredient hopper
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
	if (container == nullptr)
	{
		DEBUG_WARNING(true, ("Crafting tool %s does not have a container!", myIdString));
		return nullptr;
	}
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	Container::ContainedItem hopperId = container->getObjectInSlot(hopperSlotId, tmp);
	if (hopperId == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Crafting tool %s does not have a ingredient "
			"hopper!", myIdString));
		return nullptr;
	}
	ServerObject * hopper = safe_cast<ServerObject *>(hopperId.getObject());
	if (hopper == nullptr)
	{
		DEBUG_WARNING(true, ("Can't find object for ingredient hopper id %s",
			hopperId.getValueString().c_str()));
		return nullptr;
	}

	return hopper;
}	// TangibleObject::getIngredientHopper

//----------------------------------------------------------------------

/**
 * Adds an object to this tool's output slot.
 *
 * @param object			the object to add
 * @param transferer        who is transferring the ingredient
 *
 * @return true on success, false on fail
 */
bool TangibleObject::addObjectToOutputSlot(ServerObject & object,
	ServerObject * transferer)
{
static const SlotId outputSlotId(SlotIdManager::findSlotId(MANF_OUTPUT_SLOT_NAME));

	if(isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// get output slot
		SlottedContainer const * const slotContainer = ContainerInterface::getSlottedContainer(*this);
		if (slotContainer == nullptr)
		{
			DEBUG_WARNING(true, ("Crafting tool %s does not have a container!", myIdString));
			return false;
		}

		// make sure the transferer is in the final stage of crafting
		if (transferer == nullptr)
		{
			DEBUG_WARNING(true, ("addObjectToOutputSlot, nullptr transferer"));
			return false;
		}
		NetworkId crafterVarId(NetworkId::cms_invalid);
		if (!getObjVars().getItem(OBJVAR_CRAFTING_CRAFTER, crafterVarId))
		{
			// @todo: send a message to the player saying this tool is in use
			DEBUG_WARNING(true, ("addObjectToOutputSlot, crafting tool %s has "
				"no crafter", myIdString));
			return false;
		}
		if (crafterVarId != transferer->getNetworkId())
		{
			DEBUG_WARNING(true, ("addObjectToOutputSlot, transferer %s is not the "
				"crafter %s", transferer->getNetworkId().getValueString().c_str(),
				crafterVarId.getValueString().c_str()));
			return false;
		}
		CreatureObject const * const creatureTransferer = transferer->asCreatureObject();
		if (creatureTransferer == nullptr)
		{
			DEBUG_WARNING(true, ("addObjectToOutputSlot, transferer %s is not a "
				"creature", transferer->getNetworkId().getValueString().c_str()));
			return false;
		}
		PlayerObject const * const creaturePlayer = PlayerCreatureController::getPlayerObject(creatureTransferer);
		if (creaturePlayer == nullptr)
			return false;

		if (creaturePlayer->getCraftingStage() != Crafting::CS_finish)
		{
			DEBUG_WARNING(true, ("addObjectToOutputSlot, crafter %s is in the "
				"wrong stage %d", transferer->getNetworkId().getValueString().c_str(),
				static_cast<int>(creaturePlayer->getCraftingStage())));
			return false;
		}

		// if the object is our prototype, clear the prototype
		if (getSynchronizedUi() != nullptr)
		{
			CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
				getSynchronizedUi());
			if (sync != nullptr)
			{
				if (sync->getPrototype() == object.getNetworkId())
				{
					sync->setPrototype(CachedNetworkId::cms_cachedInvalid);
				}
			}
		}

		// put the object in the slot
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (!ContainerInterface::transferItemToSlottedContainer(*this, object, outputSlotId, nullptr, tmp))
		{
			DEBUG_WARNING(true, ("Failed to transfer object %s to crafting "
				"tool %s", object.getNetworkId().getValueString().c_str(),
				myIdString));
			return false;
		}
	}
	else
	{
		NetworkId transfererId;
		if(transferer)
			transfererId = transferer->getNetworkId();

		sendControllerMessageToAuthServer(CM_addObjectToOutputSlot, new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(std::make_pair(object.getNetworkId(), transfererId)));
	}

	return true;
}	// TangibleObject::addObjectToOutputSlot

//----------------------------------------------------------------------

/**
 * Gets the temporary manufacturing schematic used during a crafting session.
 *
 * @return the schematic
 */
ManufactureSchematicObject * TangibleObject::getCraftingManufactureSchematic(void) const
{
	const std::string myId(getNetworkId().getValueString());
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// make sure we are a crafting tool
	if (!isCraftingTool())
	{
		DEBUG_WARNING(true, ("Tried to get non-crafting tool "
			"object %s", myIdString));
		return nullptr;
	}

	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
	if (container)
	{
		Container::ContainerErrorCode error;
		const CachedNetworkId & schematicId =
			container->getObjectInSlot(getCraftingManufactureSchematicSlotId(), error);
		return safe_cast<ManufactureSchematicObject *>(schematicId.getObject());
	}
	else
	{
		DEBUG_WARNING(true, ("ManufactureSchematicObject doesn't have a slotted container."));
		return nullptr;
	}
}	// TangibleObject::getCraftingManufactureSchematic

//----------------------------------------------------------------------

/**
 * Gives the schematic to the player at the end of the session.
 *
 * @return the schematic
 */
ManufactureSchematicObject * TangibleObject::removeCraftingManufactureSchematic(void)
{
	const std::string myId(getNetworkId().getValueString());
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// make sure we are a crafting tool
	if (!isCraftingTool() || getSynchronizedUi() == nullptr)
	{
		DEBUG_WARNING(true, ("Tried to get non-crafting tool "
			"object %s", myIdString));
		return nullptr;
	}

	CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
		getSynchronizedUi());
	if (sync == nullptr)
		return nullptr;

	ManufactureSchematicObject * schematic = safe_cast<ManufactureSchematicObject *>(
		sync->getManfSchematic().getObject());

	sync->setManfSchematic(CachedNetworkId::cms_cachedInvalid,
		CachedNetworkId::cms_cachedInvalid, false);

	return schematic;
}	// TangibleObject::removeCraftingManufactureSchematic

//----------------------------------------------------------------------

/**
 * Stores the temporary manufacturing schematic used during crafting in this
 * crafting tool.
 *
 * @param schematic		the schematic
 */
void TangibleObject::setCraftingManufactureSchematic(ManufactureSchematicObject & schematic)
{
	if(isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// make sure we are a crafting tool
		if (!isCraftingTool() || getSynchronizedUi() == nullptr)
		{
			DEBUG_WARNING(true, ("Tried to get non-crafting tool "
				"object %s", myIdString));
			return;
		}

		CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
			getSynchronizedUi());
		if (sync == nullptr)
			return;

		// see if there is already a schematic object set
		const CachedNetworkId & schematicId = sync->getManfSchematic();
		if (schematicId == schematic.getNetworkId())
			return;

		// remove the old schematic
		ServerObject * object = safe_cast<ServerObject *>(schematicId.getObject());
		if (object == nullptr)
		{
			if (schematicId != NetworkId::cms_invalid)
			{
				DEBUG_WARNING(true, ("Can't find object for schematic id %s",
					schematicId.getValueString().c_str()));
				// go on anyway
			}
		}
		else
		{
			sync->setManfSchematic(CachedNetworkId::cms_cachedInvalid,
				CachedNetworkId::cms_cachedInvalid, true);
			object->permanentlyDestroy(DeleteReasons::Consumed);
		}

		// the schematic isn't in the world or in a container, so we need to
		// tell the client about it
		int stationBonus = 0;
		ServerObject * crafter = nullptr;
		NetworkId crafterId;
		if (getObjVars().getItem(OBJVAR_CRAFTING_CRAFTER,crafterId))
		{
			crafter = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(crafterId));
		}
		if (crafter != nullptr)
		{
			sync->setManfSchematic(CachedNetworkId(schematic), CachedNetworkId(*crafter),
				true);

			// if the crafter is near a crafting station, get any crafting bonus
			// it has
			const PlayerObject * player = PlayerCreatureController::getPlayerObject(
				crafter->asCreatureObject());
			if (player != nullptr && player->getCraftingStation().getObject() != nullptr)
			{
				player->getCraftingStation().getObject()->asServerObject()->
					getObjVars().getItem(OBJVAR_CRAFTING_STATIONMOD, stationBonus);
			}
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("TangibleObject::setCraftingManufactureSchematic "
				"no crafter id on crafting tool %s!", myIdString));
		}

		// set any crafting bonus we give on the schematic
		int myBonus = 0;
		getObjVars().getItem(OBJVAR_CRAFTING_STATIONMOD, myBonus);
		if (myBonus + stationBonus != 0)
		{
			schematic.setObjVarItem(OBJVAR_CRAFTING_STATIONMOD, myBonus + stationBonus);
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCraftingManufactureSchematic, new MessageQueueGenericValueType<NetworkId>(schematic.getNetworkId()));
	}
}	// TangibleObject::setCraftingManufactureSchematic

//----------------------------------------------------------------------

/**
 * Removes the temporary manufacturing schematic from the crafting tool and
 * deletes it.
 */
void TangibleObject::clearCraftingManufactureSchematic(void)
{
	if(isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// make sure we are a crafting tool
		if (!isCraftingTool())
		{
			DEBUG_WARNING(true, ("Tried to get non-crafting tool "
				"object %s", myIdString));
			return;
		}

		// see if we have a schematic in our schematic slot
		CachedNetworkId schematicId = CachedNetworkId::cms_cachedInvalid;
		SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
		if (container)
		{
			Container::ContainerErrorCode error = Container::CEC_Success;
			schematicId = container->getObjectInSlot(
				getCraftingManufactureSchematicSlotId(), error);
		}
		else
		{
			DEBUG_WARNING(true, ("failed to get slotted container for schematic lookup."));
		}

		// get our ui
		CraftingToolSyncUi * sync = nullptr;
		if (getSynchronizedUi() != nullptr)
			sync = dynamic_cast<CraftingToolSyncUi *>(getSynchronizedUi());

		if (schematicId == CachedNetworkId::cms_cachedInvalid && sync == nullptr)
		{
			// no schematic
			return;
		}

		if (schematicId == CachedNetworkId::cms_invalid)
			schematicId = sync->getManfSchematic();
		else if (sync != nullptr && sync->getManfSchematic() != CachedNetworkId::cms_invalid &&
			schematicId != sync->getManfSchematic())
		{
			WARNING(true, ("TangibleObject::clearCraftingManufactureSchematic "
				"crafting tool %s has schematic %s in its slot but schematic %s "
				"in its ui!!!", getNetworkId().getValueString().c_str(),
				schematicId.getValueString().c_str(),
				sync->getManfSchematic().getValueString().c_str()));
			if (schematicId.getObject() != nullptr)
			{
				schematicId.getObject()->asServerObject()->permanentlyDestroy(
					DeleteReasons::Consumed);
			}
			else
			{
				WARNING_STRICT_FATAL(true, ("TangibleObject::clearCraftingManufactureSchematic "
					"crafting tool %s has slotted schematic %s with no object!",
					getNetworkId().getValueString().c_str(),
					schematicId.getValueString().c_str()));
			}
			schematicId = sync->getManfSchematic();
		}

		// get the schematic
		ManufactureSchematicObject * manfSchematic = nullptr;
		ServerObject * object = safe_cast<ServerObject *>(schematicId.getObject());
		if (object == nullptr && schematicId != CachedNetworkId::cms_invalid)
		{
			WARNING_STRICT_FATAL(true, ("Can't find object for manufactring schematic "
				"id %s", schematicId.getValueString().c_str()));
		}
		else
			manfSchematic = safe_cast<ManufactureSchematicObject *>(object);

		if (manfSchematic != nullptr)
		{
			NetworkId crafterId;
			if (getObjVars().getItem(OBJVAR_CRAFTING_CRAFTER,crafterId))
			{
				// if we are at the assembly stage of crafting, or the draft schematic says
				// not to destroy the ingredients, put the ingredients in the schematic back
				// in the tool
				CreatureObject * const crafter = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(crafterId));
				PlayerObject * const crafterPlayer = PlayerCreatureController::getPlayerObject(crafter);
				if (!manfSchematic->mustDestroyIngredients() || (crafterPlayer != nullptr &&
					crafterPlayer->getCraftingStage() == Crafting::CS_assembly))
				{
					crafterPlayer->setAllowEmptySlot(true);
					int count = manfSchematic->getSlotsCount();
					// note this loop is odd because when we clear a slot it may
					// actually be removed if it was optional
					Crafting::IngredientSlot manfSlot;
					for (int i = 0; i < count;)
					{
						if (manfSchematic->getSlot(i, manfSlot, false))
						{
							if (manfSlot.ingredientType != Crafting::IT_none &&
								!manfSlot.ingredients.empty())
							{
								if (crafterPlayer->emptySlot(i, NetworkId::cms_invalid))
								{
									if (count != manfSchematic->getSlotsCount())
									{
										count = manfSchematic->getSlotsCount();
										continue;
									}
								}
							}
						}
						++i;
					}
					crafterPlayer->setAllowEmptySlot(false);
				}
				else
					manfSchematic->destroyAllIngredients();
			}
		}

		if (sync != nullptr)
			sync->setManfSchematic(CachedNetworkId::cms_cachedInvalid, CachedNetworkId::cms_cachedInvalid, true);
		if (object != nullptr)
		{
			object->permanentlyDestroy(DeleteReasons::Consumed);
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearCraftingManufactureSchematic, 0);
	}
}	// TangibleObject::clearCraftingManufactureSchematic

//----------------------------------------------------------------------

/**
 * Gets the temporary prototype used during a crafting session. May also be used
 * to give the prototype to the player at the end of the session.
 *
 * @return the prototype
 */
ServerObject * TangibleObject::getCraftingPrototype(void) const
{
	const std::string myId(getNetworkId().getValueString());
	const char * myIdString = myId.c_str();
	UNREF(myIdString);		// needed for release mode

	// make sure we are a crafting tool
	if (!isCraftingTool() || getSynchronizedUi() == nullptr)
	{
		DEBUG_WARNING(true, ("Tried to get non-crafting tool "
			"object %s", myIdString));
		return nullptr;
	}

	const CraftingToolSyncUi * sync = dynamic_cast<const CraftingToolSyncUi *>(
		getSynchronizedUi());
	if (sync == nullptr)
		return nullptr;

	return safe_cast<ServerObject *>(sync->getPrototype().getObject());
}	// TangibleObject::getCraftingPrototype

//----------------------------------------------------------------------

/**
 * Stores the temporary prototype object used during crafting in this
 * crafting tool.
 *
 * @param prototype		the prototype
 */
void TangibleObject::setCraftingPrototype(ServerObject & prototype)
{
	if (isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// make sure we are a crafting tool
		if (!isCraftingTool() || getSynchronizedUi() == nullptr)
		{
			DEBUG_WARNING(true, ("Tried to get non-crafting tool "
				"object %s", myIdString));
			return;
		}

		CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
			getSynchronizedUi());
		if (sync == nullptr)
			return;

		// see if there is already a prototype object set
		const CachedNetworkId & prototypeId = sync->getPrototype();
		if (prototypeId == prototype.getNetworkId())
			return;

		// remove the old prototype
		ServerObject * object = safe_cast<ServerObject *>(prototypeId.getObject());
		if (object == nullptr)
		{
			if (prototypeId != NetworkId::cms_invalid)
			{
				DEBUG_WARNING(true, ("Can't find object for prototype id %s",
					prototypeId.getValueString().c_str()));
				// go on anyway
			}
		}
		else
		{
			sync->setPrototype(CachedNetworkId::cms_cachedInvalid);
			object->permanentlyDestroy(DeleteReasons::Consumed);
		}

		// put the prototype in the tool's slot so the crafter will get updates
		// about it
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (!ContainerInterface::transferItemToSlottedContainer(*this, prototype,
			getCraftingPrototypeSlotId(), nullptr, tmp))
		{
			// see if there is something in the slot, and delete it if there is
			bool failed = true;
			SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
			NOT_NULL(container);
			const Container::ContainedItem & oldItem = container->getObjectInSlot(
				getCraftingPrototypeSlotId(), tmp);
			if (oldItem != prototype.getNetworkId())
			{
				if (oldItem.getObject() != nullptr)
				{
					safe_cast<ServerObject*>(oldItem.getObject())->permanentlyDestroy(
						DeleteReasons::Consumed);
					// try the transfer again
					if (ContainerInterface::transferItemToSlottedContainer(*this,
						prototype, getCraftingPrototypeSlotId(), nullptr, tmp))
					{
						failed = false;
					}
				}
				if (failed)
				{
					DEBUG_WARNING(true, ("Failed to transfer prototype %s to crafting "
						"tool %s", prototype.getNetworkId().getValueString().c_str(),
						myIdString));
					return;
				}
			}
		}

		sync->setPrototype(CachedNetworkId(prototype));
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCraftingPrototype, new MessageQueueGenericValueType<NetworkId>(prototype.getNetworkId()));
	}
}	// TangibleObject::setCraftingPrototype

//----------------------------------------------------------------------

/**
 * Removes the temporary prototype from the crafting tool and
 * deletes it.
 */
void TangibleObject::clearCraftingPrototype(void)
{
	if(isAuthoritative())
	{
		const std::string myId(getNetworkId().getValueString());
		const char * myIdString = myId.c_str();
		UNREF(myIdString);		// needed for release mode

		// make sure we are a crafting tool
		if (!isCraftingTool() || getSynchronizedUi() == nullptr)
		{
			DEBUG_WARNING(true, ("Tried to get non-crafting tool "
				"object %s", myIdString));
			return;
		}

		CraftingToolSyncUi * sync = dynamic_cast<CraftingToolSyncUi *>(
			getSynchronizedUi());
		if (sync == nullptr)
			return;

		const CachedNetworkId & prototypeId = sync->getPrototype();
		ServerObject * object = safe_cast<ServerObject *>(prototypeId.getObject());
		if (object != nullptr)
		{
			object->permanentlyDestroy(DeleteReasons::Consumed);
		}
		else
		{
			// double-check the prototype slot to make sure nothing is in it
			SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);
			NOT_NULL(container);
			Container::ContainerErrorCode error;
			const Container::ContainedItem & contents = container->getObjectInSlot(
				getCraftingPrototypeSlotId(), error);
			if (contents.getObject() != nullptr)
			{
				safe_cast<ServerObject *>(contents.getObject())->permanentlyDestroy(
					DeleteReasons::Consumed);
			}

		}
		sync->setPrototype(CachedNetworkId::cms_cachedInvalid);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearCraftingPrototype, 0);
	}
}	// TangibleObject::clearCraftingPrototype

// ----------------------------------------------------------------------

/**
 * Sets the visible component string for this object. Note m_components is
 * write-only on the server because this variable exists only to provide
 * visual data to the client.
 *
 * @param components		component table ids of visible components attached to this object
 */
void TangibleObject::setVisibleComponents(const std::vector<int> & components)
{
	if(isAuthoritative())
	{
		m_components.clear ();

		for (uint i = 0; i < components.size (); ++i)
			m_components.insert(components [i]);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setVisibleComponents, new MessageQueueGenericValueType<std::vector<int> >(components));
	}
}

// ----------------------------------------------------------------------

void TangibleObject::forceExecuteCommand(Command const &command, NetworkId const &targetId, Unicode::String const &params, Command::ErrorCode &status, bool commandIsFromCommandQueue)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			safe_cast<TangibleController *>(controller)->appendMessage(
				CM_commandQueueForceExecuteCommandFwd,
				0.0f,
				new MessageQueueCommandQueueForceExecuteCommandFwd(command.m_commandHash, targetId, params, status, commandIsFromCommandQueue),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
		return;
	}

	if (status == Command::CEC_Success && ConfigServerGame::getLogAllCommands())
	{
		LOGU(
			"Command",
			("%s>%s %s",
				getNetworkId().getValueString().c_str(),
				targetId.getValueString().c_str(),
				command.m_commandName.c_str()),
				params);
	}

	if (getClient() && getClient()->isGod())
	{
		CreatureObject* creatureObject = asCreatureObject();
		if (creatureObject)
		{
			LOGU(
				"CustomerService",
				("Avatar: %s executed command: (%s) %s",
				 PlayerObject::getAccountDescription(creatureObject).c_str(),
				 targetId.getValueString().c_str(),
				 command.m_commandName.c_str()),
				params);
		}
	}

	if (getObjVars().hasItem("cheater"))
	{
		CreatureObject* creatureObject = asCreatureObject();
		if (creatureObject)
		{
			LOGU(
				"CustomerService",
				("SuspectedCheaterChannel: %s executed command: (%s) %s",
				 PlayerObject::getAccountDescription(creatureObject).c_str(),
				 targetId.getValueString().c_str(),
				 command.m_commandName.c_str()),
				params);
		}
	}

	PROFILER_BLOCK_DEFINE(profileBlockCommandIsFromCommandQueue, "TangibleObject::forceExecuteCommand commandIsFromCommandQueue");
	if (commandIsFromCommandQueue)
		PROFILER_BLOCK_ENTER(profileBlockCommandIsFromCommandQueue);

	// attach temp script
	bool needDetach = false;
	if (command.m_tempScript.length())
	{
		int ret = getScriptObject()->attachScript(command.m_tempScript, true);
		if (ret != SCRIPT_OVERRIDE)
			needDetach = true;
	}

	// do script call
	bool commandOverride = false;
	if (status == Command::CEC_Success)
	{
		if (!command.m_scriptHook.empty())
		{
			ScriptParams scriptParams;
			scriptParams.addParam(targetId);
			scriptParams.addParam(params);
			scriptParams.addParam(command.m_defaultTime);
			if (getScriptObject()->callScriptCommandHandler(command.m_scriptHook, scriptParams) == SCRIPT_OVERRIDE)
				commandOverride = true;
		}
	}
	else
	{
		if (!command.m_failScriptHook.empty())
		{
			ScriptParams scriptParams;
			scriptParams.addParam(targetId);
			scriptParams.addParam(params);
			scriptParams.addParam(command.m_defaultTime);
			if (getScriptObject()->callScriptCommandHandler(command.m_failScriptHook, scriptParams) == SCRIPT_OVERRIDE)
			{
				commandOverride = true;
				// an override in a failure script handler should mean to mark it as a success (used for standing from feign death)
				status = Command::CEC_Success;
			}
		}
	}

	// detach temp scripts
	if (needDetach)
		getScriptObject()->detachScript(command.m_tempScript);

	if ( commandOverride && commandIsFromCommandQueue )
	{
		CommandQueue * const queue = getCommandQueue();

		if (queue != nullptr)
		{
			queue->cancelCurrentCommand();
		}
	}

	// do c function call if not overridden by script
	if (!commandOverride)
	{
		if (status == Command::CEC_Success)
		{
			if (command.m_cppHook)
				(*command.m_cppHook)(command, getNetworkId(), targetId, params);
		}
		else
		{
			if (command.m_failCppHook)
				(*command.m_failCppHook)(command, getNetworkId(), targetId, params);
		}
	}

	if (commandIsFromCommandQueue)
		PROFILER_BLOCK_LEAVE(profileBlockCommandIsFromCommandQueue);
}

// ----------------------------------------------------------------------

void TangibleObject::setCondition(int condition)
{
	if (isAuthoritative())
		m_condition = (m_condition.get() | condition);
	else
		sendControllerMessageToAuthServer(CM_setCondition, new MessageQueueGenericValueType<int>(condition));
}

// ----------------------------------------------------------------------

void TangibleObject::clearCondition(int condition)
{
	if (isAuthoritative())
		m_condition = (m_condition.get() & (~condition));
	else
		sendControllerMessageToAuthServer(CM_clearCondition, new MessageQueueGenericValueType<int>(condition));
}

// ----------------------------------------------------------------------

bool TangibleObject::isDisabled() const
{
	return hasCondition(ServerTangibleObjectTemplate::C_disabled);
}

// ----------------------------------------------------------------------

void TangibleObject::setDisabled(bool disabled)
{
	if (disabled)
		setCondition(ServerTangibleObjectTemplate::C_disabled);
	else
		clearCondition(ServerTangibleObjectTemplate::C_disabled);
}

// ----------------------------------------------------------------------


const NetworkId & TangibleObject::getOwnerId() const
{
	return m_ownerId.get();
}

// ----------------------------------------------------------------------

bool TangibleObject::isOwner(NetworkId const &id, Client const *client) const
{
	if (id == getOwnerId())
		return true;
	if (client && client->isGod())
		return true;
	return false;
}

// ----------------------------------------------------------------------

/**
 * Determine the initial visibility state of an object when it is created.
 */
void TangibleObject::initializeVisibility()
{
	if (isAuthoritative())
	{
		const ServerTangibleObjectTemplate * myTemplate = safe_cast<
			const ServerTangibleObjectTemplate *>(getObjectTemplate());
		if (myTemplate != nullptr)
		{
			bool visible = false;
			for (size_t i = 0; i < myTemplate->getVisibleFlagsCount(); ++i)
			{
				if (myTemplate->getVisibleFlags(i) == ServerObjectTemplate::VF_player)
				{
					visible = true;
					break;
				}
			}
			setVisible(visible);
		}
	}
	else
	{
		WARNING(true, ("TangibleObject::initializeVisibility called on non-authoritative object %s", 
			getNetworkId().getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

void TangibleObject::setVisible(bool isVisible)
{
	if (isAuthoritative())
		m_visible = isVisible;
	else
		sendControllerMessageToAuthServer(CM_setVisible, new MessageQueueGenericValueType<bool>(isVisible));
}

// ----------------------------------------------------------------------

void TangibleObject::setHidden(bool const hide)
{
	if (isAuthoritative())
		m_hideFromClient = hide;
	else
		sendControllerMessageToAuthServer(CM_hideFromClient, new MessageQueueGenericValueType<bool>(hide));
}

// ----------------------------------------------------------------------

void TangibleObject::visibilityDataModified()
{
	if (isInWorld())
	{
		if (isVisible() && !isHidden())
		{
			// show the object
			const TriggerVolume * triggerVolume = getTriggerVolume(NetworkTriggerVolumeNamespace::NetworkTriggerVolumeName);
			if (triggerVolume != nullptr)
			{
				std::vector<ServerObject *> observers;
				ServerWorld::findPlayerCreaturesInRange(getPosition_w(), triggerVolume->getRadius(), observers);
				if (!observers.empty())
					ObserveTracker::onObjectMadeVisibleTo(*this, observers);
			}
		}
		else
		{
			// hide the object
			ObserveTracker::onObjectMadeInvisible(*this);

			// if the object is hidden, show the object to
			// players who have passively revealed the object
			if (isVisible() && isHidden() && !m_passiveRevealPlayerCharacter.empty())
			{
				const TriggerVolume * triggerVolume = getTriggerVolume(NetworkTriggerVolumeNamespace::NetworkTriggerVolumeName);
				if (triggerVolume != nullptr)
				{
					std::vector<ServerObject *> possibleObservers;
					ServerWorld::findPlayerCreaturesInRange(getPosition_w(), triggerVolume->getRadius(), possibleObservers);
					if (!possibleObservers.empty())
					{
						std::vector<ServerObject *> allowedObservers;
						for (std::vector<ServerObject *>::const_iterator i = possibleObservers.begin(); i != possibleObservers.end(); ++i)
						{
							if (m_passiveRevealPlayerCharacter.contains((*i)->getNetworkId()))
								allowedObservers.push_back(*i);
						}

						if (!allowedObservers.empty())
							ObserveTracker::onObjectMadeVisibleTo(*this, allowedObservers);
					}
				}
			}
		}
	}
}
bool TangibleObject::isVisibleOnClient(Client const &client) const
{
	// Everyone can see visible objects.
	// Gods and clients on the buildcluster can see everything.
	if (client.isGod() || ConfigServerGame::getBuildCluster() || (isVisible() && !isHidden()))
		return true;
	
	// fix for yet another buyback container exploit
	if(this->getScriptObject()->hasScript("object.buyback"))
	{
		return false;
	}

	// Invisible objects are visible to a non-god client if the client's
	// creature contains (or indirectly contains) the invisible object.

	ServerObject const * const characterObject = client.getCharacterObject();

	{
		for (ServerObject const *o = this; o; o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o)))
		{
			if (o == characterObject)
				return true;
			if (o->isInWorld() || o->asCellObject())
				break;
		}
	}

	// Invisible objects are visible to a non-god client if they are in
	// the client's character's containment chain.
	{
		for (ServerObject const *o = characterObject; o; o = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*o)))
			if (o == this)
				return true;
	}

	if (isVisible())
	{
		// if the client's character is the owner, then he can see it.
		const CachedNetworkId ownerId(getOwnerId());
		if (ownerId == characterObject->getNetworkId())
			return true;

		// if the client's character is grouped with the the owner, then he can see it.
		const CreatureObject * owner = safe_cast<const CreatureObject *>(ownerId.getObject());
		if (owner != nullptr)
		{
			const GroupObject * group = owner->getGroup();
			if (group != nullptr)
			{
				if (group->isGroupMember(characterObject->getNetworkId()))
					return true;
			}
		}

		// if the client's character has passively revealed the hidden object, then he can see it.
		if (m_passiveRevealPlayerCharacter.contains(characterObject->getNetworkId()))
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

/**
 * Tests if this item will encumber a player when equipped.
 *
 * @return true if the item has encumbrance data
 */
bool TangibleObject::hasEncumbrances() const
	{
	return getObjVars().hasItem(OBJVAR_ARMOR_BASE + '.' + OBJVAR_ARMOR_ENCUMBRANCE);
}	// TangibleObject::hasEncumbrances

// ----------------------------------------------------------------------

/**
 * Returns the encumbrance values this object applies to a player when equipped.
 *
 * @param encumbrances	vector to be filled in with encumbrance values for the health,
 *						action, and mind groups; will be empty if this object does not
 *						encumber a creature
 *
 * @return true if encumbrances has good data, false if not
 */
bool TangibleObject::getEncumbrances(std::vector<int> & encumbrances) const
		{
static int datatable_type_col            = -1;
static int datatable_min_encumbrance_col = -1;
static int datatable_max_encumbrance_col = -1;

	// NOTE: it might be better if we do all this via script, to reduce
	// duplicate functionality

	encumbrances.clear();

	DataTable * dt = DataTableManager::getTable(DATATABLE_ARMOR, true);
	if (dt == nullptr)
		return false;
	else if (datatable_type_col == -1)
	{
		// cache the column indexes
		datatable_type_col            = dt->findColumnNumber(DATATABLE_TYPE_COL);
		datatable_min_encumbrance_col = dt->findColumnNumber(DATATABLE_MIN_ENCUMBRANCE_COL);
		datatable_max_encumbrance_col = dt->findColumnNumber(DATATABLE_MAX_ENCUMBRANCE_COL);
		if (datatable_type_col < 0 ||
			datatable_min_encumbrance_col < 0 ||
			datatable_max_encumbrance_col < 0)
		{
			WARNING_STRICT_FATAL(true, ("TangibleObject::getEncumbrances: armor datatable missing expected columns"));
			return false;
		}
	}

	// get the encumbrance values from the objvars
	float baseEncumbrance = 0;
	if (!getObjVars().getItem(OBJVAR_ARMOR_BASE + '.' + OBJVAR_ARMOR_ENCUMBRANCE, baseEncumbrance))
			return false;
	std::vector<float> encumbranceSplit;
	if (!getObjVars().getItem(OBJVAR_ARMOR_BASE + '.' + OBJVAR_ENCUMBRANCE_SPLIT, encumbranceSplit))
			return false;
	if (encumbranceSplit.size() != 3)
		return false;

	// get the actual encumbrance range from the armor datatable
	int armorLevel;
	if (!getObjVars().getItem(OBJVAR_ARMOR_BASE + "." + OBJVAR_ARMOR_LEVEL, armorLevel))
			return false;
	char armorLevelRowName[64];
	snprintf(armorLevelRowName, sizeof(armorLevelRowName), "%s%d", DATATABLE_FINAL_ROW.c_str(), armorLevel);
	int armorLevelRow = dt->searchColumnInt(datatable_type_col, Crc::calculate(armorLevelRowName));
	if (armorLevelRow < 0)
		return false;
	int minEncumbrance = dt->getIntValue(datatable_min_encumbrance_col, armorLevelRow);
	int maxEncumbrance = dt->getIntValue(datatable_max_encumbrance_col, armorLevelRow);

	// scale the base encumbrance by the actual encumbrance range
	baseEncumbrance = baseEncumbrance * (maxEncumbrance - minEncumbrance) + minEncumbrance;

	// split the base encumbrace into the three attrib groups
	encumbrances.resize(encumbranceSplit.size());
	std::vector<int>::iterator j = encumbrances.begin();
	for (std::vector<float>::const_iterator i = encumbranceSplit.begin(); i != encumbranceSplit.end(); ++i, ++j)
	{
		*j = static_cast<int>((*i) * baseEncumbrance);
		}
		return true;
	}

// ----------------------------------------------------------------------

/**
 * Returns the slot id of the slot we use to hold temporary crafting prototypes.
 *
 * @return the prototype slot id
 */
const SlotId & TangibleObject::getCraftingPrototypeSlotId()
{
static const SlotId prototypeSlotId(SlotIdManager::findSlotId(TEMP_PROTOTYPE_SLOT_NAME));

	return prototypeSlotId;
}

// ----------------------------------------------------------------------

/**
 * Returns the slot id of the slot we use to hold temporary manf schematics.
 *
 * @return the schematic slot id
 */
const SlotId & TangibleObject::getCraftingManufactureSchematicSlotId()
{
static const SlotId schematicSlotId(SlotIdManager::findSlotId(TEMP_SCHEMATIC_SLOT_NAME));

	return schematicSlotId;
}

// ----------------------------------------------------------------------

/**
 * Generates an id to allow players to identify a crafted component. NOTE: this
 * returns a pointer to a static buffer, so make sure you save off the number
 * before calling this function again.
 *
 * @param id		network id that the serial number will be generated from
 *
 * @return the serial number
 */
const char * TangibleObject::generateSerialNumber(const NetworkId & id)
{
static char buffer[11] = {'(', 0, 0, 0, 0, 0, 0, 0, 0, ')', 0};
int i;
char c;
uint64 mask;

#if defined(WIN32)
	uint64 value = (id.getValue() & 0x000000FFFFFFFFFFi64);
#elif defined(linux)
	uint64 value = (id.getValue() & 0x000000FFFFFFFFFFULL);
#else
#error unsupported OS
#endif

	for (i = 0; i < 10; ++i)
	{
		mask = ((value >> 4) | ((value & 0x0F) << 36));
		value ^= mask;
	}

	for (i = 1; i < 9; ++i)
	{
		c = static_cast<char>(value & 0x1F);
		value >>= 5;
		if (c < 10)
			c += '0';
		else
			c += ('a' - 10);
		buffer[i] = c;
	}
	return buffer;
}	// TangibleObject::generateSerialNumber

//----------------------------------------------------------------------

/**
 * Returns the attribute bonuses this object grants when equipped.
 *
 * @param attribBonuses		list to be filled in with the bonuses
 */
void TangibleObject::getAttribBonuses(std::vector<std::pair<int, int> > & attribBonuses) const
{
	attribBonuses.clear();

	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_ATTRIBUTE_BONUS);

	for (DynamicVariableList::NestedList::const_iterator i = mods.begin();
		i != mods.end(); ++i)
	{
		int bonus = 0;
		i.getValue(bonus);
		int attribute = atoi(i.getName().c_str());
		if (isdigit(i.getName()[0]) && attribute >= 0 &&
			attribute < Attributes::NumberOfAttributes)
		{
			if (bonus > ConfigServerGame::getMaxItemAttribBonus())
				bonus = ConfigServerGame::getMaxItemAttribBonus();
			else if (bonus < -ConfigServerGame::getMaxItemAttribBonus())
				bonus = -ConfigServerGame::getMaxItemAttribBonus();
			attribBonuses.push_back(std::make_pair(attribute, bonus));
		}
		else
		{
			WARNING(true, ("Object %s has invalid attribute bonus objvar name of %s",
				getNetworkId().getValueString().c_str(), i.getName().c_str()));
		}
	}
}	// TangibleObject::getAttribBonuses

/**
 * Returns the attribute bonuses this object grants when equipped.
 *
 * @param attribBonuses		list of length 9 to be filled in with the bonuses
 *
 * @return true if we have bonuses, false if not
 */
bool TangibleObject::getAttribBonuses(std::vector<int> & attribBonuses) const
{
	attribBonuses.clear();
	attribBonuses.resize(Attributes::NumberOfAttributes, 0);

	bool hasBonues = false;
	typedef std::vector<std::pair<int, int> > BonusPairs;
	BonusPairs pairedBonuses;
	getAttribBonuses(pairedBonuses);
	for (BonusPairs::const_iterator i = pairedBonuses.begin(); i != pairedBonuses.end(); ++i)
	{
		int index = (*i).first;
		if (index >=0 && index < Attributes::NumberOfAttributes)
		{
			attribBonuses[index] = (*i).second;
			hasBonues = true;
		}
	}
	return hasBonues;
}	// TangibleObject::getAttribBonuses

//----------------------------------------------------------------------

/**
 * Returns the attribute bonus of this object for a given attribute.
 *
 * @param attribute		the attribute
 *
 * @return the bonus for the attribute
 */
int TangibleObject::getAttribBonus(int attribute) const
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return 0;

	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_ATTRIBUTE_BONUS);
	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", attribute);
	int bonus = 0;
	IGNORE_RETURN(mods.getItem(buffer, bonus));

	if (bonus > ConfigServerGame::getMaxItemAttribBonus())
		bonus = ConfigServerGame::getMaxItemAttribBonus();
	else if (bonus < -ConfigServerGame::getMaxItemAttribBonus())
		bonus = -ConfigServerGame::getMaxItemAttribBonus();
	return bonus;
}

//----------------------------------------------------------------------

/**
 * Sets the attribute bonus this object applies when equipped.
 *
 * @param attribute		the attribute
 * @param bonus			the bonus
 */
void TangibleObject::setAttribBonus(int attribute, int bonus)
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return;

	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", attribute);
	if (bonus != 0)
		setObjVarItem(OBJVAR_ATTRIBUTE_BONUS + '.' + buffer, bonus);
	else
		removeObjVarItem(OBJVAR_ATTRIBUTE_BONUS + '.' + buffer);
}

//----------------------------------------------------------------------

/**
 * Returns the skill mod bonuses this object grants when equipped.
 *
 * @param skillModBonuses		list to be filled in with the bonuses
 */
void TangibleObject::getSkillModBonuses(std::vector<std::pair<std::string, int> > & skillModBonuses, bool includeCategorizedSkillmod /*= true*/) const
{
	static std::map<std::string, int> mapSkillModBonuses;
	mapSkillModBonuses.clear();

	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_SKILLMOD_BONUS);
	for (DynamicVariableList::NestedList::const_iterator i = mods.begin();
		i != mods.end(); ++i)
	{
		int bonus = 0;
		i.getValue(bonus);

		std::string const name = i.getName();
		std::map<std::string, int>::iterator iterFind = mapSkillModBonuses.find(name);
		if (iterFind != mapSkillModBonuses.end())
			iterFind->second += bonus;
		else
			IGNORE_RETURN(mapSkillModBonuses.insert(std::make_pair(name, bonus)));
	}

	if (includeCategorizedSkillmod)
		getCategorizedSkillModBonuses(*this, OBJVAR_CATEGORIZED_SKILLMOD_BONUS, mapSkillModBonuses);

	skillModBonuses.clear();
	for (std::map<std::string, int>::const_iterator iterMapSkillModBonuses = mapSkillModBonuses.begin(); iterMapSkillModBonuses != mapSkillModBonuses.end(); ++iterMapSkillModBonuses)
	{
		if (iterMapSkillModBonuses->second != 0)
			skillModBonuses.push_back(std::make_pair(iterMapSkillModBonuses->first, iterMapSkillModBonuses->second));
	}
}	// TangibleObject::getSkillModBonuses

//----------------------------------------------------------------------

/**
 * Returns the skill mod bonus of this object for a given skill mod.
 *
 * @param skillMod		the skill mod name
 *
 * @return the bonus for the skill mod
 */
int TangibleObject::getSkillModBonus(const std::string & skillMod, bool includeCategorizedSkillmod /*= true*/) const
{
	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_SKILLMOD_BONUS);
	int value = 0;
	IGNORE_RETURN(mods.getItem(skillMod, value));

	if (includeCategorizedSkillmod)
		value += getCategorizedSkillModBonus(*this, OBJVAR_CATEGORIZED_SKILLMOD_BONUS, skillMod);

	return value;
}	// TangibleObject::getSkillModBonus

//----------------------------------------------------------------------

/**
 * Sets the base skill mod bonus this object applies when equipped.
 *
 * @param skillMod		the skill mod name
 * @param bonus			the bonus
 */
void TangibleObject::setSkillModBonus(const std::string & skillMod, int bonus)
{
	std::string const skillModObjvar = OBJVAR_SKILLMOD_BONUS + '.' + skillMod;

	if (bonus < 0)
	{
		setObjVarItem(skillModObjvar, bonus);
		return;
	}
	else if (bonus == 0)
	{
		removeObjVarItem(skillModObjvar);
		return;
	}

	int const currentTotalBonus = getSkillModBonus(skillMod);
	int const currentBonus = getSkillModBonus(skillMod, false);
	if ((currentTotalBonus - currentBonus + bonus) > ConfigServerGame::getMaxObjectSkillModBonus())
		bonus = std::max(0, ConfigServerGame::getMaxObjectSkillModBonus() - currentTotalBonus + currentBonus);

	if (bonus != 0)
		setObjVarItem(skillModObjvar, bonus);
	else
		removeObjVarItem(skillModObjvar);
}	// TangibleObject::setSkillModBonus

//----------------------------------------------------------------------

void TangibleObject::setSkillModBonus(const std::string & category, const std::string & skillMod, int bonus)
{
	if (category.empty())
		return;

	std::string const skillModObjvar = OBJVAR_CATEGORIZED_SKILLMOD_BONUS + '.' + category + '.' + skillMod;
	if (bonus < 0)
	{
		setObjVarItem(skillModObjvar, bonus);
		return;
	}
	else if (bonus == 0)
	{
		removeObjVarItem(skillModObjvar);
		return;
	}

	int const currentTotalBonus = getSkillModBonus(skillMod);
	int currentBonus = 0;
	IGNORE_RETURN(getObjVars().getItem(skillModObjvar, currentBonus));

	if ((currentTotalBonus - currentBonus + bonus) > ConfigServerGame::getMaxObjectSkillModBonus())
		bonus = std::max(0, ConfigServerGame::getMaxObjectSkillModBonus() - currentTotalBonus + currentBonus);

	if (bonus != 0)
		setObjVarItem(skillModObjvar, bonus);
	else
		removeObjVarItem(skillModObjvar);	
}	// TangibleObject::setSkillModBonus

//----------------------------------------------------------------------

void TangibleObject::removeCategorizedSkillModBonuses(const std::string & category)
{
	if (category.empty())
		return;

	std::string const skillModObjvar = OBJVAR_CATEGORIZED_SKILLMOD_BONUS + '.' + category;
	if (getObjVars().hasItem(skillModObjvar) && (getObjVars().getType(skillModObjvar) == DynamicVariable::LIST))
		removeObjVarItem(skillModObjvar);
}

//----------------------------------------------------------------------

/**
 * Adds a skill mod bonus to a skill mod socket, if the object has an open socket.
 *
 * @param skillMod		the skill mod name
 * @param bonus			the bonus
 *
 * @return true if the bonus was added, false if the object didn't have an open socket
 */
bool TangibleObject::addSkillModSocketBonus(const std::string & skillMod, int bonus)
{
	if (bonus > ConfigServerGame::getMaxSocketSkillModBonus())
		bonus = ConfigServerGame::getMaxSocketSkillModBonus();

	int sockets = getSkillModSockets();
	if (sockets > 0)
	{
		int currentBonus = getSkillModBonus(skillMod, false);

		// only apply the bonus if it's greater than the current bonus
		/*if (bonus <= currentBonus) // This breaks multi-socketed items, so now we combine all values.
			return false;*/          // Not sure if this code was ever hit in the first place.

		setSkillModBonus(skillMod, currentBonus + bonus);

		// trigger scripts
		ScriptParams params;
		std::vector<const char *> skillModParam;
		skillModParam.push_back(skillMod.c_str());
		std::vector<int> bonusParam;
		bonusParam.push_back(bonus);
		params.addParam(skillModParam);
		params.addParam(bonusParam);
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SOCKET_USED, params));
	}
	return sockets > 0;
}	// TangibleObject::addSkillModSocketBonus

//----------------------------------------------------------------------

/**
 * Adds multiple skill mod bonuses to a skill mod socket, if the object has an
 * open socket. Note that this call will only use up one socket, no matter how
 * many bonuses are added.
 *
 * @param skillModBonuses		the bonuses
 *
 * @return true if the bonus was added, false if the object didn't have an open socket
 */
bool TangibleObject::addSkillModSocketBonuses(const std::vector<std::pair<std::string,
	int> > & skillModBonuses)
{
	int sockets = getSkillModSockets();
	if (sockets > 0)
	{
		int i;
		int count = skillModBonuses.size();
		std::vector<int> bonusOrder(1, 0);
		for (i = 1; i < count; ++i)
		{
			std::vector<int>::iterator j;
			for (j = bonusOrder.begin(); j != bonusOrder.end(); ++j)
			{
				if (skillModBonuses[i].second > skillModBonuses[*j].second)
				{
					bonusOrder.insert(j, i);
					break;
				}
			}
			if (j == bonusOrder.end())
				bonusOrder.push_back(i);
		}

		bool someSucceeded = false;
		for (i = 0; i < count; ++i)
		{
			int index = bonusOrder[i];
			if(addSkillModSocketBonus(skillModBonuses[index].first,
				skillModBonuses[index].second))
				someSucceeded = true;
		}
		if(someSucceeded)
		{
			setSkillModSockets(sockets - 1);
			return true;
		}
	}
	return sockets > 0;
}	// TangibleObject::addSkillModSocketBonuses

//----------------------------------------------------------------------

/**
 * Returns the number of available skill mod sockets.
 *
 * @return the number of sockets
 */
int TangibleObject::getSkillModSockets(void) const
{
	int sockets = 0;

	if (getGameObjectType() == SharedObjectTemplate::GOT_misc_container_wearable)
		return 0;

	bool const isCybernetic = getGameObjectType() == SharedObjectTemplate::GOT_cybernetic_arm || getGameObjectType() == SharedObjectTemplate::GOT_cybernetic_legs;

	IGNORE_RETURN(getObjVars().getItem(OBJVAR_SKILLMOD_SOCKETS, sockets));
	return isCybernetic ? sockets : std::min(sockets, ConfigServerGame::getMaxSkillModSockets());
}	// TangibleObject::getSkillModSockets

//----------------------------------------------------------------------

/**
 * Sets the number of available skill mod sockets.
 *
 * @param sockets		the number of sockets
 */
void TangibleObject::setSkillModSockets(int sockets)
{
	bool const isCybernetic = getGameObjectType() == SharedObjectTemplate::GOT_cybernetic_arm || getGameObjectType() == SharedObjectTemplate::GOT_cybernetic_legs;

	if (sockets > ConfigServerGame::getMaxSkillModSockets() && !isCybernetic)
		sockets = ConfigServerGame::getMaxSkillModSockets();
	if (sockets > 0)
		setObjVarItem(OBJVAR_SKILLMOD_SOCKETS, sockets);
	else
		removeObjVarItem(OBJVAR_SKILLMOD_SOCKETS);
}	// TangibleObject::setSkillModSockets

//----------------------------------------------------------------------

/**
 * Returns the amount of time left before a crafted prototype object will be done.
 *
 * @return the time left, in secs
 */
int TangibleObject::getCraftingToolPrototypeTime() const
{
	int timeLeft = -1;
	float prototypeTime = 0;
	getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_TIME,prototypeTime);

	if (prototypeTime > 0)
	{
		int startTime =0;
		if (getObjVars().getItem(OBJVAR_CRAFTING_PROTOTYPE_START_TIME,startTime))
		{
			const int currentTime = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
			timeLeft = static_cast<int>(prototypeTime + 0.5f) - (currentTime - startTime);
		}
	}
	return timeLeft;
}	// TangibleObject::getCraftingToolPrototypeTime

//----------------------------------------------------------------------

void TangibleObject::getAttributesForCraftingTool (AttributeVector & data) const
{
	char valueBuffer[32];

	static const Unicode::String CRAFT_TOOL_WORKING  (Unicode::narrowToWide("@crafting:tool_status_working"));
	static const Unicode::String CRAFT_TOOL_FINISHED (Unicode::narrowToWide("@crafting:tool_status_finished"));
	static const Unicode::String CRAFT_TOOL_READY    (Unicode::narrowToWide("@crafting:tool_status_ready"));
	static const SlotId outputSlotId(SlotIdManager::findSlotId(MANF_OUTPUT_SLOT_NAME));

	float craft_effectiveness = 0.0f;
	getObjVars ().getItem(OBJVAR_CRAFTING_STATIONMOD, craft_effectiveness);
	snprintf (valueBuffer, sizeof (valueBuffer), "%.2f", craft_effectiveness);
	data.push_back (std::make_pair(SharedObjectAttributes::craft_tool_effectiveness, Unicode::narrowToWide(valueBuffer)));

	// add some internal attribs for the crafting station state

	// see if we are making a prototype

	int prototypeTimeLeft = getCraftingToolPrototypeTime();
	if (prototypeTimeLeft > 0)
	{
		data.push_back(std::make_pair(SharedObjectAttributes::craft_tool_status, CRAFT_TOOL_WORKING));
		_itoa(prototypeTimeLeft, valueBuffer, 10);
		data.push_back(std::make_pair(SharedObjectAttributes::craft_tool_time, Unicode::narrowToWide(valueBuffer)));
	}
	else
	{
		// see if there is an object in the output hopper
		bool hasPrototype = false;
		SlottedContainer const * const slotContainer = ContainerInterface::getSlottedContainer(*this);
		if (slotContainer != nullptr)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			if (slotContainer->getObjectInSlot(outputSlotId, tmp) != Container::ContainedItem::cms_invalid)
			{
				hasPrototype = true;
			}
		}
		if (hasPrototype)
			data.push_back(std::make_pair(SharedObjectAttributes::craft_tool_status, CRAFT_TOOL_FINISHED));
		else
			data.push_back(std::make_pair(SharedObjectAttributes::craft_tool_status, CRAFT_TOOL_READY));
	}
}

// ----------------------------------------------------------------------

void TangibleObject::getAttributes(const NetworkId & playerId, AttributeVector &data) const
{
	ServerObject::getAttributes(playerId, data);
}

// ----------------------------------------------------------------------

void TangibleObject::getAttributesForShipComponent (AttributeVector & data) const
{
	ShipComponentData * const componentData = ShipComponentDataManager::create(*this);
	if(componentData)
	{
		componentData->getAttributes(data);
		delete componentData;
	}
}

// ----------------------------------------------------------------------

void TangibleObject::getAttributes(AttributeVector & data) const
{
	ServerObject::getAttributes(data);

	char valueBuffer[32];

	CreatureObject const * const thisCreature = asCreatureObject();

	if (!thisCreature)
	{
		const int maxHit = getMaxHitPoints ();
		// Only consider tangible invulnerability rules for this case
		if (maxHit > 0 && !TangibleObject::isInvulnerable())
		{
			snprintf (valueBuffer, sizeof (valueBuffer), "%d/%d", maxHit - getDamageTaken (), maxHit);
			data.push_back (std::make_pair (SharedObjectAttributes::condition, Unicode::narrowToWide (valueBuffer)));
		}

		const ServerObjectTemplate * const targetTemplate = safe_cast<const ServerObjectTemplate *>(getObjectTemplate());
		if (targetTemplate)
		{
			bool playerCanMove = false;

			for (size_t i = 0; i < targetTemplate->getMoveFlagsCount(); ++i)
			{
				if (targetTemplate->getMoveFlags(i) == ServerObjectTemplate::MF_player)
				{
					playerCanMove = true;
					break;
				}
			}

			if (getCacheVersion() <= 0 && playerCanMove)
			{
				snprintf (valueBuffer, sizeof (valueBuffer), "%d", std::max (1, getVolume ()));
				data.push_back (std::make_pair (SharedObjectAttributes::volume, Unicode::narrowToWide (valueBuffer)));
			}
		}
	}

	const int got = getGameObjectType ();

	switch (got)
	{
	case SharedObjectTemplate::GOT_misc_drink:
	case SharedObjectTemplate::GOT_misc_food:
	case SharedObjectTemplate::GOT_misc_pharmaceutical:
	case SharedObjectTemplate::GOT_weapon_ranged_thrown:
	case SharedObjectTemplate::GOT_weapon_heavy_misc:
	case SharedObjectTemplate::GOT_misc_fishing_bait:
		{
			const int count = m_count.get ();
			if (count)
			{
				snprintf (valueBuffer, sizeof (valueBuffer), "%d", m_count.get ());
				data.push_back (std::make_pair (SharedObjectAttributes::counter_uses_remaining, Unicode::narrowToWide (valueBuffer)));
			}
		}
		break;
	case SharedObjectTemplate::GOT_resource_container_pseudo:
		{
			int resourceValue = 0;
			for (int i = 0; i < Crafting::RA_numResourceAttributes; ++i)
			{

				const std::string ovName(OBJVAR_COMPONENT_DATA + "." +
					Crafting::getResourceAttributeName(i));
				if (getObjVars().getItem(ovName, resourceValue))
				{
					char buffer[24];
					snprintf(buffer, sizeof(buffer), "%i", resourceValue);
					data.push_back(std::make_pair(Crafting::getResourceAttributeName(i),Unicode::narrowToWide(buffer)));
				}
			}
		}
		break;
	default:
		break;
	}

	const VolumeContainer * const volumeContainer = ContainerInterface::getVolumeContainer (*this);
	if (volumeContainer)
	{
		if (got != SharedObjectTemplate::GOT_misc_factory_crate)
		{
			const int curVol = volumeContainer->getCurrentVolume ();
			const int totVol = volumeContainer->getTotalVolume ();

			if (curVol >= 0 && totVol >= 0)
			{
				snprintf (valueBuffer, sizeof (valueBuffer), "%d/%d", curVol, totVol);
				data.push_back (std::make_pair (SharedObjectAttributes::contents, Unicode::narrowToWide (valueBuffer)));
			}
		}
	}

	std::vector<std::pair<std::string, int> > skillModBonuses;
	getSkillModBonuses(skillModBonuses);
	if (!skillModBonuses.empty())
	{
		char buffer[16];
		int count = skillModBonuses.size();
		for (int i = 0; i < count; ++i)
		{
			_itoa(skillModBonuses[i].second, buffer, 10);
			data.push_back(std::make_pair(SharedObjectAttributes::cat_skill_mod_bonus + "." + "@stat_n:" + skillModBonuses[i].first, Unicode::narrowToWide(buffer)));
		}
	}

	std::vector<int> attribBonuses;
	if (getAttribBonuses(attribBonuses) && !attribBonuses.empty())
	{
		char buffer[16];
		int const attribCount = attribBonuses.size();
		for (int attrib = 0; attrib < attribCount; ++attrib)
		{
			int const bonus = attribBonuses[attrib];
			if (bonus != 0)
			{
				_itoa(bonus, buffer, 10);
				data.push_back(std::make_pair(SharedObjectAttributes::cat_attrib_mod_bonus + "." + "@att_bonus_n:" + s_attributeBonusNames[attrib], Unicode::narrowToWide(buffer)));
			}
		}
	}

	int sockets = getSkillModSockets();
	if (sockets > 0)
	{
		char buffer[16];
		_itoa(sockets, buffer, 10);
		data.push_back(std::make_pair(SharedObjectAttributes::sockets, Unicode::narrowToWide(buffer)));
	}

	if (isCraftingTool())
		getAttributesForCraftingTool (data);

	if (getCreatorId() != NetworkId::cms_invalid)
		data.push_back(std::make_pair(SharedObjectAttributes::crafter,       getCreatorName()));

	if (isCrafted())
		data.push_back(std::make_pair(SharedObjectAttributes::serial_number, Unicode::narrowToWide(generateSerialNumber(getCraftedId()))));

	if(GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_ship_component))
	{
		getAttributesForShipComponent(data);
	}

	{
		// If this is a fake ship component in the interior of a ship, get its attributes from the ship

		ServerObject const * const cell = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
		if (cell && cell->asCellObject())
		{
			ServerObject const * const shipAsObject = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*cell));
			if (shipAsObject)
			{
				ShipObject const * const ship = shipAsObject->asShipObject();
				if (ship)
				{
					int slot;
					if (getObjVars().getItem("intSlot",slot))
					{
						ShipComponentData const * const componentData = ship->createShipComponentData(slot);
						if (componentData)
						{
							componentData->getAttributes(data);
							delete componentData;
						}
					}
				}
			}
		}
	}

	if (getObjVars().hasItem(OBJVAR_BIO_LINK_ID))
	{
		// display the bio-link info
		if (getBioLink() == PENDING_BIO_LINK_NETWORK_ID)
		{
			data.push_back(std::make_pair(SharedObjectAttributes::bio_link, Unicode::narrowToWide("@"+SharedStringIds::bio_link_pending.getCanonicalRepresentation())));
		}
		else
		{
			const std::string & bioLinkName = NameManager::getInstance().getPlayerFullName(getBioLink());
			if (!bioLinkName.empty())
			{
				data.push_back(std::make_pair(SharedObjectAttributes::bio_link, Unicode::narrowToWide(bioLinkName)));
			}
			else
			{
				LOG("CustomerService", ("Biolink: object %s is flagged as bio-linked "
					"to player %s that we can't find a name for",
					getNetworkId().getValueString().c_str(),
					PlayerObject::getAccountDescription(getBioLink()).c_str()));
				data.push_back(std::make_pair(SharedObjectAttributes::bio_link, Unicode::narrowToWide("@"+SharedStringIds::bio_link_unknown.getCanonicalRepresentation())));
			}
		}
	}

	// No Trade
	{
		if (markedNoTrade())
		{
			data.push_back(std::make_pair(SharedObjectAttributes::no_trade, Unicode::emptyString));

			if (markedNoTradeRemovable())
			{
				data.push_back(std::make_pair(SharedObjectAttributes::no_trade_removable, Unicode::emptyString));
			}

			if (markedNoTradeShared(true))
			{
				data.push_back(std::make_pair(SharedObjectAttributes::no_trade_shared, Unicode::emptyString));
			}
		}
		else
		{
			GameScriptObject * const gso = const_cast<GameScriptObject *>(getScriptObject());
			if (gso && gso->hasScript(NOMOVE_SCRIPT))
			{
				data.push_back(std::make_pair(SharedObjectAttributes::no_trade, Unicode::emptyString));

				if (markedNoTradeRemovable())
				{
					data.push_back(std::make_pair(SharedObjectAttributes::no_trade_removable, Unicode::emptyString));
				}

				if (markedNoTradeShared(false))
				{
					data.push_back(std::make_pair(SharedObjectAttributes::no_trade_shared, Unicode::emptyString));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void TangibleObject::getAttributesForAuction(AttributeVector &data) const
{
	static const std::string attributeAppearanceData("AppearanceData");

	DEBUG_FATAL(getAppearanceData().length() > 1000,("Appearance data was more than 1000 characters long, too long to save."));
	data.push_back(std::make_pair(attributeAppearanceData, Unicode::narrowToWide(getAppearanceData())));
	ServerObject::getAttributesForAuction(data);
}

// ----------------------------------------------------------------------

void TangibleObject::showFlyText(const StringId &outputText, float scale, int r, int g, int b)
{
	//-- Get the controller.
	Controller *const controller = getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyText(): object [%s] has no controller.", getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Send the showFlyText message to all clients.
	uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_CLIENT;
	if (isPlayerControlled())
		flags |= GameControllerMessageFlags::DEST_AUTH_CLIENT;
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, new MessageQueueShowFlyText(getNetworkId(), outputText, scale, r, g, b, 0), flags);
}

// ----------------------------------------------------------------------

void TangibleObject::showFlyText(const Unicode::String &outputTextOOB, float scale, int r, int g, int b)
{
	//-- Get the controller.
	Controller *const controller = getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyText(): object [%s] has no controller.", getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Send the showFlyText message to all clients.
	uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_CLIENT;
	if (isPlayerControlled())
		flags |= GameControllerMessageFlags::DEST_AUTH_CLIENT;
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, new MessageQueueShowFlyText(getNetworkId(), outputTextOOB, scale, r, g, b, 0), flags);
}

// ----------------------------------------------------------------------

void TangibleObject::showFlyText(const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags)
{
	//-- Get the controller.
	Controller *const controller = getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyText(): object [%s] has no controller.", getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Send the showFlyText message to all clients.
	uint32 netFlags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_CLIENT;
	if (isPlayerControlled())
		flags |= GameControllerMessageFlags::DEST_AUTH_CLIENT;
	controller->appendMessage(static_cast<int>(CM_showFlyText), 0.0f, new MessageQueueShowFlyText(getNetworkId(), outputTextOOB, scale, r, g, b,  flags), netFlags);
}

// ----------------------------------------------------------------------

void TangibleObject::showCombatText(const TangibleObject & attackerObject, const StringId &outputText, float scale, int r, int g, int b)
{
	//-- Get the controller.
	Controller *const controller = getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyText(): object [%s] has no controller.", getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Send the showFlyText message to all clients.
	uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_CLIENT;
	if (isPlayerControlled())
		flags |= GameControllerMessageFlags::DEST_AUTH_CLIENT;
	controller->appendMessage(static_cast<int>(CM_showCombatText), 0.0f, new MessageQueueShowCombatText(getNetworkId(), attackerObject.getNetworkId(), outputText, scale, r, g, b, 0), flags);
}

// ----------------------------------------------------------------------

void TangibleObject::showCombatText(const TangibleObject & attackerObject, const Unicode::String &outputTextOOB, float scale, int r, int g, int b)
{
	//-- Get the controller.
	Controller *const controller = getController();
	if (!controller)
	{
		DEBUG_WARNING(true, ("showFlyText(): object [%s] has no controller.", getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Send the showFlyText message to all clients.
	uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_CLIENT;
	if (isPlayerControlled())
		flags |= GameControllerMessageFlags::DEST_AUTH_CLIENT;
	controller->appendMessage(static_cast<int>(CM_showCombatText), 0.0f, new MessageQueueShowCombatText(getNetworkId(), attackerObject.getNetworkId(), outputTextOOB, scale, r, g, b, 0), flags);
}

// ----------------------------------------------------------------------

void TangibleObject::handleCMessageTo(const MessageToPayload &message)
{
	static const std::string methodname_updateCounter = "C++updateCounter";

	if (message.getMethod() == methodname_updateCounter)
	{
		if (isCraftingTool())
		{
			int timeLeft = getCraftingToolPrototypeTime();
			if (timeLeft <= 0)
			{
				// done
				timeLeft = 0;
			}
			else
			{
				// update the count 5 sec from now
				MessageToQueue::getInstance().sendMessageToC (getNetworkId(), methodname_updateCounter, std::string (), 5, false);
			}
			m_count = timeLeft;
		}
	}
	else if (message.getMethod() == "C++UpdatePvp")
	{
		Pvp::forceStatusUpdate(*this);
	}
	else if (message.getMethod() == "C++PurgeStructure")
	{
		LOG("CustomerService",("Purge:  Purging structure %s",getNetworkId().getValueString().c_str()));
		rollupStructure(NetworkId(message.getDataAsString()),false);
	}
	else if (message.getMethod() == "C++WarnAboutPurge")
	{
		LOG("CustomerService",("Purge:  Sending warning emails to the admins of structure %s",getNetworkId().getValueString().c_str()));
		rollupStructure(NetworkId(message.getDataAsString()),true);
	}
	else if (message.getMethod() == "C++PurgeVendor")
	{
		LOG("CustomerService",("Purge:  Purging vendor %s",getNetworkId().getValueString().c_str()));
		permanentlyDestroy(DeleteReasons::Decay); // Script's onDestroy() method will handle cleanup
	}
	else if (message.getMethod() == "C++CheckPvpRegion")
	{
		// check for pvp region change
		if (isAuthoritative() && !isInvulnerable() && !isNonPvpObject())
		{
			Pvp::checkForRegionChange(*this);
		}

		if (isPlayerControlled())
		{
			std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
			std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getNetworkId());
			if (iterFind != connectedCharacterLfgData.end())
			{
				std::string regionName;
				std::string playerCityName;

				std::string const & sceneId = ServerWorld::getSceneId();
				Vector const &position = findPosition_w();
				Region const * const region = RegionMaster::getSmallestVisibleRegionAtPoint(sceneId, position.x, position.z);
				if (region != nullptr)
					regionName = Unicode::wideToNarrow(region->getName());

				int const cityId = CityInterface::getCityAtLocation(sceneId, static_cast<int>(position.x), static_cast<int>(position.z), 0);
				if (cityId > 0)
					playerCityName = CityInterface::getCityInfo(cityId).getCityName();

				if (iterFind->second.locationRegion != regionName)
					ServerUniverse::setConnectedCharacterRegionData(getNetworkId(), regionName);

				if (iterFind->second.locationPlayerCity != playerCityName)
					ServerUniverse::setConnectedCharacterPlayerCityData(getNetworkId(), playerCityName);

				PlayerObject * const player = PlayerCreatureController::getPlayerObject(asCreatureObject());
				if (player)
					player->updateGcwRegionInfo();
			}
		}
	}
	else if (message.getMethod() == "C++ProcessHateOverTime")
	{
		if (!m_hateOverTime.empty())
		{
			std::list<NetworkId> expired;
			unsigned long const currentGameTime =  ServerClock::getInstance().getGameTimeSeconds();

			for (std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > >::const_iterator iter = m_hateOverTime.begin(); iter != m_hateOverTime.end(); ++iter)
			{
				if (iter->second.second.first < currentGameTime)
				{
					IGNORE_RETURN(addHate(iter->first, iter->second.first * static_cast<float>(std::min(currentGameTime, iter->second.second.second) - iter->second.second.first)));

					if (iter->second.second.second <= currentGameTime)
					{
						expired.push_back(iter->first);
					}
					else
					{
						m_hateOverTime.set(iter->first, std::make_pair(iter->second.first, std::make_pair(currentGameTime, iter->second.second.second)));
					}
				}
			}

			if (!expired.empty())
			{
				for (std::list<NetworkId>::const_iterator iterExpired = expired.begin(); iterExpired != expired.end(); ++iterExpired)
				{
					IGNORE_RETURN(m_hateOverTime.erase(*iterExpired));
				}
			}

			if (!m_hateOverTime.empty())
			{
				MessageToQueue::sendMessageToC(getNetworkId(), "C++ProcessHateOverTime", std::string(), 1, false);
			}
		}
	}
	else if (message.getMethod() == "C++DestroyDupedTcg5AtAtHeadItv")
	{
		LOG("CustomerService",("VeteranRewards:  destroying duped tcg5_atat_head_itv item (%s) owned by (%s)", getNetworkId().getValueString().c_str(), getOwnerId().getValueString().c_str()));
		permanentlyDestroy(DeleteReasons::Replaced);
	}
	else
		ServerObject::handleCMessageTo(message);
}

//----------------------------------------------------------------------

CustomizationData *TangibleObject::fetchCustomizationData ()
{
	CustomizationDataProperty * const cdprop = safe_cast<CustomizationDataProperty *>(getProperty (CustomizationDataProperty::getClassPropertyId()));
	return cdprop ? cdprop->fetchCustomizationData () : 0;
}

//----------------------------------------------------------------------

const CustomizationData *TangibleObject::fetchCustomizationData () const
{
	const CustomizationDataProperty * const cdprop = safe_cast<const CustomizationDataProperty *>(getProperty (CustomizationDataProperty::getClassPropertyId()));
	return cdprop ? cdprop->fetchCustomizationData () : 0;
}

// ----------------------------------------------------------------------

void TangibleObject::calcPvpableState()
{
	// a tangible is pvpable if it has hit points, has a shared template, does not have an arrangement descriptor, and is not player movable
	// or m_attackableOverride is true
	if (m_attackableOverride.get())
	{
		setPvpable(true);
		return;
	}

	setPvpable(false);
	if (getMaxHitPoints() && getSharedTemplate() && !getSharedTemplate()->getArrangementDescriptor())
	{
		ServerObjectTemplate const *objTemplate = safe_cast<ServerObjectTemplate const *>(getObjectTemplate());
		for (size_t i = 0; i < objTemplate->getMoveFlagsCount(); ++i)
			if (objTemplate->getMoveFlags(i) == ServerObjectTemplate::MF_player)
				return;
		setPvpable(true);
	}
}

// ----------------------------------------------------------------------

bool TangibleObject::isInvulnerable() const
{
	return hasCondition(ServerTangibleObjectTemplate::C_invulnerable);
}

// ----------------------------------------------------------------------

void TangibleObject::setInvulnerable(bool invulnerable)
{
	if (invulnerable)
	{
		setCondition(ServerTangibleObjectTemplate::C_invulnerable);
	}
	else
	{
		clearCondition(ServerTangibleObjectTemplate::C_invulnerable);
	}

	GameScriptObject * const gameScriptObject = getScriptObject();

	if (gameScriptObject != nullptr)
	{
		ScriptParams scriptParams;
		scriptParams.addParam(hasCondition(ServerTangibleObjectTemplate::C_invulnerable));
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_INVULNERABLE_CHANGED, scriptParams));
	}
}

// ----------------------------------------------------------------------

bool TangibleObject::isInsured(void) const
{
	return hasCondition(ServerTangibleObjectTemplate::C_insured);
}

// ----------------------------------------------------------------------

void TangibleObject::setInsured(bool insured)
{
	if (insured)
		setCondition(ServerTangibleObjectTemplate::C_insured);
	else
		clearCondition(ServerTangibleObjectTemplate::C_insured);
}

// ----------------------------------------------------------------------

bool TangibleObject::isUninsurable(void) const
{
	return hasCondition(ServerTangibleObjectTemplate::C_uninsurable);
}

// ----------------------------------------------------------------------

void TangibleObject::setUninsurable(bool uninsurable)
{
	if (uninsurable)
		setCondition(ServerTangibleObjectTemplate::C_uninsurable);
	else
		clearCondition(ServerTangibleObjectTemplate::C_uninsurable);
}

// ----------------------------------------------------------------------

bool TangibleObject::wantSawAttackTriggers() const
{
	if (isAuthoritative())
		return hasCondition(ServerTangibleObjectTemplate::C_wantSawAttackTrigger);
	return false;
}

// ----------------------------------------------------------------------

void TangibleObject::setWantSawAttackTriggers(bool enable)
{
	if (enable)
		setCondition(ServerTangibleObjectTemplate::C_wantSawAttackTrigger);
	else
		clearCondition(ServerTangibleObjectTemplate::C_wantSawAttackTrigger);
}

//----------------------------------------------------------------------

int TangibleObject::getCombatSkeleton() const
{
	return static_cast<int>((safe_cast<const ServerTangibleObjectTemplate *>(getObjectTemplate()))->getCombatSkeleton());
}	// TangibleObject::getCombatSkeleton()

//-----------------------------------------------------------------------

bool TangibleObject::canDropInWorld() const
{
	return true;
}

//------------------------------------------------------------------------------------------

int TangibleObject::getNumberOfLots() const
{

//-- fetch the shared object template
	const SharedTangibleObjectTemplate* const tangibleObjectTemplate = safe_cast<const SharedTangibleObjectTemplate*> (getSharedTemplate());
	if (!tangibleObjectTemplate)
	{
		return 0;
	}

	//-- get the structure footprint
	const StructureFootprint* const structureFootprint = tangibleObjectTemplate->getStructureFootprint ();
	if (!structureFootprint)
	{
		return 0;
	}

	//-- return the number of lots
	return structureFootprint->getNumberOfStructureReservations ();
}

//------------------------------------------------------------------------------------------

bool TangibleObject::isOnAdminList(const CreatureObject& player) const
{
	//shortcut for owners
	if (getOwnerId() == player.getNetworkId())
		return true;

	if(player.getClient() && player.getClient()->isGod())
		return true;

	if(getObjVars().hasItem("player_structure.admin_all_characters"))
    {
	    static int suid;
	    getObjVars().getItem("player_structure.admin_all_characters", suid);
	    if(NameManager::getInstance().getPlayerStationId(player.getNetworkId()) == suid)
        {
	        return true;
        }
    }

	// In script, the admin list is stored as a list of strings.
	// The format of the list is:
	// 1) All player are stored as network IDs represented as a string
	// 2) All guilds are stored as guild IDs represted as a string and prefixed with "guild:"

	typedef std::vector<Unicode::String> StringVector;
	static StringVector sv;
	sv.clear ();

	if (getObjVars().getItem("player_structure.admin.adminList", sv))
	{
		const NetworkId requestorNetworkId = player.getNetworkId();

		// Convert the network ID to a Unicode string
		Unicode::String requestorNetworkIdString = Unicode::narrowToWide( requestorNetworkId.getValueString() );

		// Try to match the string
		for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
		{
			const Unicode::String & adminString = *it;
			if (Unicode::caseInsensitiveCompare(adminString, requestorNetworkIdString))
			{
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------

bool TangibleObject::isOnHopperList(const CreatureObject& player) const
{
	if (getOwnerId() == player.getNetworkId())
		return true;

	if(player.getClient() && player.getClient()->isGod())
		return true;

	// In script, the hopper list is stored as a list of strings.
	// The format of the list is:
	// 1) All player are stored as network IDs represented as a string
	// 2) All guilds are stored as guild IDs represted as a string and prefixed with "guild:"

	typedef std::vector<Unicode::String> StringVector;
	static StringVector sv;
	sv.clear ();

	if (getObjVars().getItem("player_structure.hopper.hopperList", sv))
	{
		const NetworkId requestorNetworkId = player.getNetworkId();

		// Convert the network ID to a Unicode string
		Unicode::String requestorNetworkIdString = Unicode::narrowToWide( requestorNetworkId.getValueString() );

		// Try to match the string
		for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
		{
			const Unicode::String & adminString = *it;
			if (Unicode::caseInsensitiveCompare(adminString, requestorNetworkIdString))
			{
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------

bool TangibleObject::isCustomizationDataModified() const
{
	return getLocalFlag(LocalObjectFlags::TangibleObject_CustomizationDataModified);
}

// ----------------------------------------------------------------------

void TangibleObject::setCustomizationDataModified(bool modified)
{
	setLocalFlag(LocalObjectFlags::TangibleObject_CustomizationDataModified, modified);
	scheduleForAlter();
}

// ----------------------------------------------------------------------

void TangibleObject::setPvpable(bool pvpable)
{
	bool oldPvpable = !isNonPvpObject();
	setLocalFlag(LocalObjectFlags::TangibleObject_Pvpable, pvpable);

	if ((oldPvpable != pvpable) && !getObservers().empty())
	{
		// did the object's "pvp sync" status change because of the Pvpable change?
		bool const wasPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(!oldPvpable, hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (asCreatureObject() != nullptr), getPvpFaction());
		bool const isPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(!pvpable, hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (asCreatureObject() != nullptr), getPvpFaction());

		if (wasPvpSync != isPvpSync)
		{
			// force pvp status update
			Pvp::forceStatusUpdate(*this);

			const std::set<Client *> &observers = getObservers();
			for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
			{
				if (isPvpSync)
					(*i)->addObservingPvpSync(this);
				else
					(*i)->removeObservingPvpSync(this);
			}
		}
	}

	if (isAuthoritative())
	{
		if (pvpable && !oldPvpable)
			MessageToQueue::sendRecurringMessageToC(getNetworkId(), "C++CheckPvpRegion", std::string(), static_cast<int>(getPvpRegionCheckTime()));
		else if (!pvpable && oldPvpable)
			MessageToQueue::cancelRecurringMessageTo(getNetworkId(), "C++CheckPvpRegion");
	}
}

// ----------------------------------------------------------------------

int TangibleObject::getInterestRadius() const
{
	if (isPlayerControlled())
		return ConfigServerGame::getPlayerInterestRange();
	ServerTangibleObjectTemplate const *myTemplate = safe_cast<ServerTangibleObjectTemplate const *>(getObjectTemplate());
	return myTemplate->getInterestRadius();
}

// ----------------------------------------------------------------------

void TangibleObject::getRequiredCertifications(std::vector<std::string> & results) const
{
	const SharedTangibleObjectTemplate *const sharedObjectTemplate = safe_cast<const SharedTangibleObjectTemplate*>(getSharedTemplate());
	if (sharedObjectTemplate)
	{
		const int numRequired = sharedObjectTemplate->getCertificationsRequiredCount();
		for(int i=0; i<numRequired; ++i)
		{
			results.push_back(sharedObjectTemplate->getCertificationsRequired(i));
		}
	}
}

// ======================================================================

void TangibleObject::MaxHitPointsCallback::modified(TangibleObject &target, int oldValue, int value, bool) const
{
	// if the values actually changed, and if it changed from 0 or to 0, then we need to recalculate pvp state
	if ((oldValue != value) && ((oldValue == 0) || (value == 0)))
	{
		bool oldNonPvpState = target.isNonPvpObject();
		target.calcPvpableState();

		// if pvp state changed, need to update clients
		if (oldNonPvpState != target.isNonPvpObject())
			Pvp::forceStatusUpdate(target);
	}
}

// ----------------------------------------------------------------------

void TangibleObject::AttackableOverrideCallback::modified(TangibleObject &target, bool oldValue, bool value, bool) const
{
	// if the values actually changed, then we need to recalculate pvp state
	if (oldValue != value)
	{
		bool oldNonPvpState = target.isNonPvpObject();
		target.calcPvpableState();

		// if pvp state changed, need to update clients
		if (oldNonPvpState != target.isNonPvpObject())
			Pvp::forceStatusUpdate(target);
	}
}

// ----------------------------------------------------------------------

void TangibleObject::AppearanceDataCallback::modified(TangibleObject &target, const std::string &oldValue, const std::string &value, bool isLocal) const
{
	UNREF(isLocal);

	//-- Log any time an authoritative, player-controlled object has customizations set to the empty string.
	//   This most likely is something very wrong since the player is going to get reset to default cusotmizations.
	if (value.empty() && !oldValue.empty())
	{
		if (target.isPlayerControlled() && target.isAuthoritative())
		{
			std::string const oldAppearanceDataDump = CustomizationData::makeSelectDumpFormat(oldValue.c_str());
			LOG("customization-data-loss", ("non-empty old customization data getting wiped, id=[%s],template=[%s],old m_appearanceData in SELECT DUMP format:[%s]", target.getNetworkId().getValueString().c_str(), target.getObjectTemplateName(), oldAppearanceDataDump.c_str() ));
		}
	}

	//-- Notify the object that its appearance data has been modified.
	target.appearanceDataModified(value);

	Object * const objectContainer = ContainerInterface::getContainedByObject(target);
	ServerObject * const serverContainer = objectContainer ? objectContainer->asServerObject() : nullptr;
	TangibleObject * const tangibleContainer = serverContainer ? serverContainer->asTangibleObject() : nullptr;
	if(tangibleContainer)
		tangibleContainer->onContainedItemAppearanceDataModified(target, oldValue, value);
}

// ----------------------------------------------------------------------

void TangibleObject::onContainedItemAppearanceDataModified(TangibleObject const & , std::string const & , std::string const & )
{
	//do nothing.  Tangibles don't care if their contained objects have their appearance data change
}

// ----------------------------------------------------------------------

void TangibleObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if(name == "customAppearance")
	{
		m_customAppearance.pack(target);
	}
	else if(name == "appearanceData")
	{
		m_appearanceData.pack(target);
	}
	else if(name == "incubatorResourceData")
	{
		std::string resourceData;

		static const uint32 crcIncubator = CrcLowerString::calculateCrc("object/tangible/crafting/station/incubator_station.iff");
		if (getTemplateCrc() == crcIncubator)
		{
			DynamicVariableList const & objvars = getObjVars();
			if (objvars.hasItem("power.name") && (DynamicVariable::STRING == objvars.getType("power.name")))
			{
				std::string resourceName;
				IGNORE_RETURN(objvars.getItem("power.name", resourceName));

				ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeByName(resourceName);
				if (rto && !rto->getParentClass().isRecycled() && !rto->getParentClass().isPermanent())
					resourceData = rto->getResourceTypeDataForExport();
			}
		}

		Archive::put(target, resourceData);
	}
	else
	{
		ServerObject::getByteStreamFromAutoVariable(name, target);
	}
}

// ----------------------------------------------------------------------

void TangibleObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
    Archive::ReadIterator ri(source);
    if(name == "customAppearance")
    {
        m_customAppearance.unpackDelta(ri);
    }
	else if(name == "appearanceData")
	{
		m_appearanceData.unpackDelta(ri);
	}
	else if(name == "incubatorResourceData")
	{
		std::string resourceData;
		Archive::get(ri, resourceData);

		static const uint32 crcIncubator = CrcLowerString::calculateCrc("object/tangible/crafting/station/incubator_station.iff");
		if ((getTemplateCrc() == crcIncubator) && !resourceData.empty())
		{
			DynamicVariableList const & objvars = getObjVars();
			if (objvars.hasItem("power.name") && (DynamicVariable::STRING == objvars.getType("power.name")))
			{
				NetworkId const importedResourceType = ResourceTypeObject::addImportedResourceType(resourceData);
				if (importedResourceType.isValid())
				{
					if (importedResourceType.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId)
					{
						// need to tell all proxy game servers, if any, to create the imported
						// resource type, if it hasn't already been created on that game server
						ProxyList const &proxyList = getExposedProxyList();
						if (!proxyList.empty())
						{
							ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

							GenericValueTypeMessage<std::pair<NetworkId, std::string> > const addImportedResourceTypeMsg("AddImportedResourceType", std::make_pair(importedResourceType, resourceData));
							ServerMessageForwarding::send(addImportedResourceTypeMsg);

							ServerMessageForwarding::end();
						}
					}

					ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeById(importedResourceType);
					if (rto)
					{
						IGNORE_RETURN(setObjVarItem("power.name", rto->getResourceName()));

						if (importedResourceType.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId)
							IGNORE_RETURN(setObjVarItem("power.resourceData", resourceData));
						else
							removeObjVarItem("power.resourceData");
					}
				}
			}
		}
	}
    else
    {
        ServerObject::setAutoVariableFromByteStream(name, source);
    }
	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

/**
 * Sets a bio-link on this object. This object will only be able to be equipped by
 * the player who's id matches the bio-link id.
 *
 * @param playerId		the player to link this object to
 */
void TangibleObject::setBioLink(const NetworkId & playerId)
{
	if (playerId != NetworkId::cms_invalid)
	{
		if (isAuthoritative())
		{
			setObjVarItem(OBJVAR_BIO_LINK_ID, playerId);
		}
		else
		{
			sendControllerMessageToAuthServer(CM_setBioLink, new MessageQueueGenericValueType<NetworkId>(playerId));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Sets the pending bio-link on this object. This will prevent the object from
 * being equipped by any player until a real bio-link is set.
 */
void TangibleObject::setPendingBioLink()
{
	setBioLink(PENDING_BIO_LINK_NETWORK_ID);
}

// ----------------------------------------------------------------------

/**
 * Removes the bio-link on this object.
 */
void TangibleObject::clearBioLink()
{
	if (isAuthoritative())
	{
		removeObjVarItem(OBJVAR_BIO_LINK);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setBioLink, new MessageQueueGenericValueType<NetworkId>(NetworkId::cms_invalid));
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the bio-link id of this object.
 *
 * @return the bio-link player id (NetworkId::cms_invalid if the object isn't bio-linked)
 */
NetworkId TangibleObject::getBioLink() const
{
	NetworkId linkId;
	if (getObjVars().getItem(OBJVAR_BIO_LINK_ID, linkId))
		return linkId;
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

/**
 * Returns whether an object, or any contained object, is biolinked
 *
 * @return true if a biolinked object is found
 */
bool TangibleObject::isBioLinked() const
{
	NetworkId const linkId = getBioLink();

	if( (linkId != NetworkId::cms_invalid) && (linkId != PENDING_BIO_LINK_NETWORK_ID) )
	{
		// If the network ID is valid and does not correspond to a pending bio-link
		// then we will assume that the tangible object is bio-linked
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void TangibleObject::rollupStructure(NetworkId const & owner, bool warnOnly)
{
	GameScriptObject* gso = getScriptObject();
	if (gso)
	{
		gso->attachScript("structure.structure_rollup", true);
		ScriptParams params;
		params.addParam(owner);
		params.addParam(warnOnly);
		gso->trigAllScripts(Scripting::TRIG_DO_STRUCTURE_ROLLUP, params);
		gso->detachScript("structure.structure_rollup");
	}
}

// ----------------------------------------------------------------------

CommandQueue * TangibleObject::getCommandQueue() const
{
	return CommandQueue::getCommandQueue(*const_cast<TangibleObject *>(this));
}

//-----------------------------------------------------------------------

void TangibleObject::commandQueueEnqueue(Command const &command, NetworkId const &targetId, Unicode::String const &params, uint32 const sequenceId, bool const clearable, Command::Priority const priority, bool fromServer)
{
	if(!fromServer)
	{
		if(command.m_fromServerOnly)
		{
			LOG("CustomerService", ("CheatChannel: attempt to queue server-only command %s from client on object %s", command.m_commandName.c_str(), getNetworkId().getValueString().c_str()));
			return;
		}
	}
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_commandQueueEnqueueFwd, new MessageQueueCommandQueueEnqueueFwd(sequenceId, command.m_commandHash, targetId, params, clearable, static_cast<int>(priority), fromServer));
	}
	else
	{
		CommandQueue * const queue = getCommandQueue();
		if (queue != nullptr)
		{
			queue->enqueue(command, targetId, params, sequenceId, clearable, priority);
		}
	}
}

//-----------------------------------------------------------------------

void TangibleObject::commandQueueRemove(uint32 const sequenceId)
{
	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_commandQueueRemove, new MessageQueueCommandQueueRemove(sequenceId, 0.0f, static_cast<int>(Command::CEC_Success), 0));
	}
	else
	{
		CommandQueue * const queue = getCommandQueue();
		if (queue != nullptr)
		{
			queue->remove(sequenceId);
		}
	}
}

//-----------------------------------------------------------------------

void TangibleObject::commandQueueClear()
{
	// commandQueueRemove clears the queue when passed a 0 sequenceId
	commandQueueRemove(0);
}

//-----------------------------------------------------------------------

bool TangibleObject::commandQueueHasCommandFromGroup(uint32 groupHash) const
{
	CommandQueue * const queue = getCommandQueue();
	if (queue != nullptr)
	{
		return queue->hasCommandFromGroup(groupHash);
	}
	return false;
}

//-----------------------------------------------------------------------

void TangibleObject::commandQueueClearCommandsFromGroup(uint32 groupHash, bool force)
{
	CommandQueue * const queue = getCommandQueue();
	if (queue != nullptr)
	{
		queue->clearCommandsFromGroup(groupHash, force);
	}
}

//-----------------------------------------------------------------------

CombatEngineData::CombatData * TangibleObject::getCombatData()
{
	return m_combatData;
}

//-----------------------------------------------------------------------

CombatEngineData::CombatData const * TangibleObject::getCombatData() const
{
	return m_combatData;
}

//-----------------------------------------------------------------------

void TangibleObject::CombatStateChangedCallback::modified(TangibleObject & target, bool /*oldValue*/, bool newValue, bool /*local*/) const
{
	if (newValue)
	{
		// Starting Combat

		LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("TangibleObject::Callbacks::CombatStateChanged::modified() STARTING COMBAT owner(%s:%s)", target.getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target.getEncodedObjectName()).c_str()));

		if (target.isAuthoritative())
		{
			target.createCombatData();

			target.m_combatStartTime = Os::getRealSystemTime();

			if (!target.isPlayerControlled())
			{
				ObjectTracker::addCombatAI();
			}

			AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(target.getController());

			if (aiCreatureController != nullptr)
			{
				aiCreatureController->markCombatStartLocation();
			}

			if (target.getScriptObject() != nullptr)
			{
				ScriptParams params;
				IGNORE_RETURN(target.getScriptObject()->trigAllScripts(Scripting::TRIG_ENTERED_COMBAT, params));
			}

			target.scheduleForAlter();
		}
	}
	else
	{
		// Exiting Combat

		LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("TangibleObject::Callbacks::CombatStateChanged::modified() EXITING COMBAT owner(%s:%s)", target.getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(target.getEncodedObjectName()).c_str()));

		if (target.isAuthoritative())
		{
			if (!target.isPlayerControlled())
			{
				ObjectTracker::removeCombatAI();
			}

			//target.clearHateList();

			target.commandQueueClearCommandsFromGroup(COMMAND_GROUP_COMBAT.getCrc());

			if (target.getScriptObject() != nullptr)
			{
				ScriptParams params;
				IGNORE_RETURN(target.getScriptObject()->trigAllScripts(Scripting::TRIG_EXITED_COMBAT, params));
			}

			target.scheduleForAlter();
		}
	}
}

//-----------------------------------------------------------------------
int TangibleObject::getCombatDuration() const
{
	int result = 0;

	if (isInCombat())
	{
		result = static_cast<int>(Os::getRealSystemTime() - m_combatStartTime.get());
	}

	return result;
}

//-----------------------------------------------------------------------
void TangibleObject::setAttackableOverride(bool attackable)
{
	if (isAuthoritative())
	{
		if (m_attackableOverride.get() != attackable)
			m_attackableOverride = attackable;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setAttackableOverride, new MessageQueueGenericValueType<bool>(attackable));
	}
}

//-----------------------------------------------------------------------
bool TangibleObject::hasAutoDeclineDuel() const
{
	return getObjVars().hasItem(OBJVAR_DECLINE_DUEL);
}

//----------------------------------------------------------------------

int TangibleObject::getPassiveRevealRange(NetworkId const & target) const
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator i = m_passiveReveal.find(target);
	if (i == m_passiveReveal.end())
		return -1;

	return i->second;
}

//----------------------------------------------------------------------

void TangibleObject::addPassiveReveal(TangibleObject const & target, int range)
{
	addPassiveReveal(target.getNetworkId(), range, (nullptr != PlayerCreatureController::getPlayerObject(target.asCreatureObject())));
}

//----------------------------------------------------------------------

void TangibleObject::addPassiveReveal(NetworkId const & target, int range, bool isTargetPlayerCharacter)
{
	// and object cannot passively reveal itself
	if (target == getNetworkId())
		return;

	// range must be > 0
	if (range <= 0)
		return;

	if(isAuthoritative())
	{
		m_passiveReveal.set(target, range);

		if (isTargetPlayerCharacter)
			m_passiveRevealPlayerCharacter.insert(target);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addPassiveReveal, new MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> >(std::make_pair(std::make_pair(range, (isTargetPlayerCharacter ? 1 : 0)), target)));
	}
}

//----------------------------------------------------------------------

void TangibleObject::removePassiveReveal(NetworkId const & target)
{
	if(isAuthoritative())
	{
		IGNORE_RETURN(m_passiveReveal.erase(target));
		IGNORE_RETURN(m_passiveRevealPlayerCharacter.erase(target));
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removePassiveReveal, new MessageQueueGenericValueType<NetworkId>(target));
	}
}

//----------------------------------------------------------------------

void TangibleObject::removeAllPassiveReveal()
{
	if(isAuthoritative())
	{
		m_passiveReveal.clear();
		m_passiveRevealPlayerCharacter.clear();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllPassiveReveal, 0);
	}
}

//----------------------------------------------------------------------

bool TangibleObject::setOverrideMapColor(unsigned char r, unsigned char g, unsigned char b)
{
	if(!isAuthoritative())
	{
		return false;
	}

	PackedArgb color(255, r, g, b);
	m_mapColorOverride.set(color.getArgb());
	return true;
}

//----------------------------------------------------------------------

bool TangibleObject::clearOverrideMapColor()
{
	if(!isAuthoritative())
	{
		return false;
	}

	m_mapColorOverride.set(0);
	return true;
}

//----------------------------------------------------------------------

bool TangibleObject::getOverrideMapColor(unsigned char & r, unsigned char & g, unsigned char & b) const
{
	PackedArgb color(m_mapColorOverride.get());
	if(color.getA() == 0)
	{
		return false;
	}

	r = color.getR();
	g = color.getG();
	b = color.getB();
	return true;
}

//----------------------------------------------------------------------

void TangibleObject::handleIncubatorCommit(CreatureObject const & owner, IncubatorCommitMessage const & msg)
{
	// this operation *REQUIRES* that both the incubator and the
	// character manipulating the incubator be authoritative on
	// the same game server, so it's really not possible to
	// pass this handler over to the authoritative incubator,
	// since there's no guarantee that the character will be
	// authoritative on that game server either
	if (!isAuthoritative() || !owner.isAuthoritative())
		return;

	GameScriptObject * const scriptObject = getScriptObject();
	if (scriptObject)
	{
		ScriptParams params;
		params.addParam(owner.getNetworkId(), "playerId");
		params.addParam(getNetworkId(), "terminalId");
		params.addParam(msg.getSlot1Id(), "slot1Id");
		params.addParam(msg.getSlot2Id(), "slot2Id");
		params.addParam(msg.getSlot3Id(), "slot3Id");
		params.addParam(msg.getSlot4Id(), "slot4Id");
		params.addParam(msg.getInitialPointsSurvival(), "initialPointsSurvival");
		params.addParam(msg.getInitialPointsBeastialResilience(),"initialPointsBeastialResilience");
		params.addParam(msg.getInitialPointsCunning(),"initialPointsCunning");
		params.addParam(msg.getInitialPointsIntelligence(),"initialPointsIntelligence");
		params.addParam(msg.getInitialPointsAggression(),"initialPointsAggression");
		params.addParam(msg.getInitialPointsHuntersInstinct(),"initialPointsHuntersInstinct");
		params.addParam(msg.getTotalPointsSurvival(),"totalPointsSurvival");
		params.addParam(msg.getTotalPointsBeastialResilience(),"totalPointsBeastialResilience");
		params.addParam(msg.getTotalPointsCunning(),"totalPointsCunning");
		params.addParam(msg.getTotalPointsIntelligence(),"totalPointsIntelligence");
		params.addParam(msg.getTotalPointsAggression(),"totalPointsAggression");
		params.addParam(msg.getTotalPointsHuntersInstinct(),"totalPointsHuntersInstinct");
		params.addParam(msg.getTemperatureGauge(),"temperatureGauge");
		params.addParam(msg.getNutrientGauge(),"nutrientGauge");
		params.addParam(msg.getNewCreatureColorIndex(), "newCreatureColorIndex");

		IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_INCUBATOR_COMMITTED, params));	
	}
}

void TangibleObject::handleIncubatorCancel(CreatureObject const & owner)
{
	// this operation *REQUIRES* that both the incubator and the
	// character manipulating the incubator be authoritative on
	// the same game server, so it's really not possible to
	// pass this handler over to the authoritative incubator,
	// since there's no guarantee that the character will be
	// authoritative on that game server either
	if (!isAuthoritative() || !owner.isAuthoritative())
		return;

	GameScriptObject * const scriptObject = getScriptObject();
	if (scriptObject)
	{
		ScriptParams params;
		params.addParam(owner.getNetworkId(), "playerId");
		params.addParam(getNetworkId(), "terminalId");

		IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_INCUBATOR_CANCELED, params));	
	}
}

void TangibleObject::forceHateTarget(NetworkId const & target)
{
	if(isAuthoritative())
	{
		m_hateList.forceHateTarget(target);
		scheduleForAlter();
	}
	else
	{
		sendControllerMessageToAuthServer(CM_forceHateTarget, new MessageQueueGenericValueType<NetworkId>(target));
	}

}

bool TangibleObject::isUserOnAccessList(NetworkId const user) const
{
    ServerObject * sourceObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(user));
    if(sourceObject)
    {
        if(sourceObject->getClient()->isGod())
        {
            return true;
        }
    }
	return (std::find(m_accessList.begin(), m_accessList.end(), user) != m_accessList.end());
}

bool TangibleObject::isGuildOnAccessList(int guildId) const
{
	return (std::find(m_guildAccessList.begin(), m_guildAccessList.end(), guildId) != m_guildAccessList.end());
}

void TangibleObject::addUserToAccessList(const NetworkId user)
{
	if(isAuthoritative())
	{
		if(m_accessList.find(user) == m_accessList.end())
		{
			if(!hasCondition(C_locked)) // If we weren't locked before, we are now.
				setCondition(C_locked);

			m_accessList.insert(user);
			
			copyUserAccessListToObjVars();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addUserToAccessList, new MessageQueueGenericValueType< std::pair<int, NetworkId> >(std::make_pair(-1, user) ) );
	}
}

void TangibleObject::addGuildToAccessList(int guildId)
{
	if(isAuthoritative())
	{
		if(m_guildAccessList.find(guildId) == m_guildAccessList.end())
		{
			if(!hasCondition(C_locked)) // If we weren't locked before, we are now.
				setCondition(C_locked);

			m_guildAccessList.insert(guildId);

			copyGuildAccessListToObjVars();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addUserToAccessList, new MessageQueueGenericValueType< std::pair<int, NetworkId> >(std::make_pair(guildId, NetworkId::cms_invalid) ) );
	}
}


void TangibleObject::removeUserFromAccessList(const NetworkId user)
{
	if(isAuthoritative())
	{
		Archive::AutoDeltaSet<NetworkId>::const_iterator iter = m_accessList.find(user);
		if(iter != m_accessList.end())
		{
			m_accessList.erase(iter);
			if(m_accessList.empty())
			{
				if(hasCondition(C_locked))
					clearCondition(C_locked); // Containers with no access list are open to whomever.

				removeObjVarItem(s_accessListScriptVar); // Remove our objvars, no need for them anymore.

				return;
			}

			copyUserAccessListToObjVars();

		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeUserFromAccessList, new MessageQueueGenericValueType< std::pair<int, NetworkId> >(std::make_pair(-1, user)));
	}
}

void TangibleObject::removeGuildFromAccessList(int guildId)
{
	if(isAuthoritative())
	{
		Archive::AutoDeltaSet<int>::const_iterator iter = m_guildAccessList.find(guildId);
		if(iter != m_guildAccessList.end())
		{
			m_guildAccessList.erase(iter);
			if(m_guildAccessList.empty())
			{
				if(hasCondition(C_locked) && m_accessList.empty())
					clearCondition(C_locked); // Containers with no access list are open to whomever.

				removeObjVarItem(s_guildAccessListScriptVar); // Remove our objvars, no need for them anymore.

				return;
			}

			copyGuildAccessListToObjVars();

		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeUserFromAccessList, new MessageQueueGenericValueType< std::pair<int, NetworkId> >(std::make_pair(guildId, NetworkId::cms_invalid)));
	}
}

void TangibleObject::copyUserAccessListToObjVars()
{
	std::vector<NetworkId> newAccessList;
	Archive::AutoDeltaSet<NetworkId>::const_iterator iter = m_accessList.begin();
	for(; iter != m_accessList.end(); ++iter)
		newAccessList.push_back((*iter));

	setObjVarItem(s_accessListScriptVar, newAccessList);
}

void TangibleObject::copyGuildAccessListToObjVars()
{
	std::vector<int> newAccessList;
	Archive::AutoDeltaSet<int>::const_iterator iter = m_guildAccessList.begin();
	for(; iter != m_guildAccessList.end(); ++iter)
		newAccessList.push_back((*iter));

	setObjVarItem(s_guildAccessListScriptVar, newAccessList);
}


void TangibleObject::readInUserAccessListObjVars()
{
	std::vector<NetworkId> inputVector;
	if(getObjVars().getItem(s_accessListScriptVar, inputVector))
	{
		// We have some obj vars to add to our list!
		std::vector<NetworkId>::size_type i = 0;
		for( ; i < inputVector.size(); ++i)
		{
			m_accessList.insert(inputVector[i]);
		}
	}
}

void TangibleObject::readInGuildAccessListObjVars()
{
	std::vector<int> inputVector;
	if(getObjVars().getItem(s_guildAccessListScriptVar, inputVector))
	{
		// We have some obj vars to add to our list!
		std::vector<int>::size_type i = 0;
		for( ; i < inputVector.size(); ++i)
		{
			m_guildAccessList.insert(inputVector[i]);
		}
	}
}

void TangibleObject::clearUserAccessList()
{
	if(isAuthoritative())
	{
		if(hasCondition(C_locked) && m_guildAccessList.empty()) // No access list = total unlock.
			clearCondition(C_locked);

		m_accessList.clear();

		removeObjVarItem(s_accessListScriptVar); // Remove our objvars, no need for them anymore.

	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearUserAccessList, new MessageQueueGenericValueType<bool>(false));
	}
}

void TangibleObject::clearGuildAccessList()
{
	if(isAuthoritative())
	{
		if(hasCondition(C_locked) && m_accessList.empty()) // No access list = total unlock.
			clearCondition(C_locked);

		m_guildAccessList.clear();

		removeObjVarItem(s_guildAccessListScriptVar); // Remove our objvars, no need for them anymore.

	}
	else
	{
		sendControllerMessageToAuthServer(CM_clearUserAccessList, new MessageQueueGenericValueType<bool>(true));
	}
}

void TangibleObject::getUserAccessList(std::vector<NetworkId> & ids)
{
	getObjVars().getItem(s_accessListScriptVar, ids);
}

void TangibleObject::getGuildAccessList(std::vector<int> & ids)
{
	getObjVars().getItem(s_guildAccessListScriptVar, ids);
}

void TangibleObject::addObjectEffect(std::string const & filename, std::string const & hardpoint, Vector const & offset, float scale, std::string const & label)
{
	if(isAuthoritative())
	{
		if(m_effectsMap.find(label) != m_effectsMap.end())
			return;

		m_effectsMap.insert(label, std::make_pair(filename, std::make_pair( hardpoint, std::make_pair(offset, scale ))));
	}
	else
		sendControllerMessageToAuthServer(CM_addObjectEffect, 
		new MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float > > > > >(std::make_pair(label, std::make_pair(filename, std::make_pair(hardpoint, std::make_pair(offset, scale))))));
}

void TangibleObject::removeObjectEffect(std::string const & label)
{
	if(isAuthoritative())
	{
		Archive::AutoDeltaMap<std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > >::const_iterator iter = m_effectsMap.find(label);
		if(iter == m_effectsMap.end())
			return;

		m_effectsMap.erase(iter);
	}
	else
		sendControllerMessageToAuthServer(CM_removeObjectEffect, new MessageQueueGenericValueType<std::string>(label));
}

bool TangibleObject::hasObjectEffect(std::string const & label)
{
	return m_effectsMap.find(label) != m_effectsMap.end();
}

void TangibleObject::removeAllObjectEffects()
{
	if(isAuthoritative())
	{
		m_effectsMap.clear();
	}
	else
		sendControllerMessageToAuthServer(CM_removeAllObjectEffect, new MessageQueueGenericValueType<bool>(true));
}
// ======================================================================
