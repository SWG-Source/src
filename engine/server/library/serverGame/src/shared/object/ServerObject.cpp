// ======================================================================
//
//ServerObject.cpp
//
//Copyright 2000-2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObject.h"

#include "UnicodeUtils.h"
#include "serverGame/AuthTransferTracker.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/ContainmentMessageManager.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DistributionListStack.h"
#include "serverGame/GameServer.h"
#include "serverGame/InstantDeleteList.h"
#include "serverGame/IntangibleVolumeContainer.h"
#include "serverGame/LineOfSightCache.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NetworkTriggerVolume.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PatrolPathNodeProperty.h"
#include "serverGame/PatrolPathRootProperty.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerQuestObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerSanityChecker.h"
#include "serverGame/PlayerShipController.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ServerClearCollidableFloraNotification.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerNoBuildNotification.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerObjectTerrainModificationNotification.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerSynchronizedUi.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "serverGame/SceneGlobalData.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/TangibleVolumeContainer.h"
#include "serverGame/TeleportFixupHandler.h"
#include "serverGame/TriggerVolume.h"
#include "serverGame/VeteranRewardManager.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"
#include "serverNetworkMessages/FlagObjectForDeleteMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/LoadContainedObjectMessage.h"
#include "serverNetworkMessages/LoadContentsMessage.h"
#include "serverNetworkMessages/MessageQueueTeleportObject.h"
#include "serverNetworkMessages/MessageToAckMessage.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverPathfinding/ServerPathfindingConstants.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ChatLogManager.h"
#include "serverUtility/ServerClock.h"
#include "Session/CommonAPI/CommonAPI.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/SocialsManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/LogoutMessage.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdAndTransform.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/LayerProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/SlotDescriptor.h"
#include "sharedObject/SlotDescriptorList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgServerNetworkMessages/MessageQueueScriptTransferMoney.h"
#include <algorithm>

// ======================================================================

namespace DeleteReasons
{
	const std::string & describeDeleteReason(const int reason)
	{
		switch(reason)
		{
		case NotDeleted:
			{
				static const std::string r = "NotDeleted";
				return r;
			}
			break;
		case Decay:
			{
				static const std::string r = "Decay";
				return r;
			}
			break;
		case God:
			{
				static const std::string r = "God";
				return r;
			}
			break;
		case Player:
			{
				static const std::string r = "Player";
				return r;
			}
			break;
		case Script:
			{
				static const std::string r = "Script";
				return r;
			}
			break;
		case CharacterDeleted:
			{
				static const std::string r = "CharacterDeleted";
				return r;
			}
			break;
		case ContainerDeleted:
			{
				static const std::string r = "ContainerDeleted";
				return r;
			}
			break;
		case Publish:
			{
				static const std::string r = "Publish";
				return r;
			}
			break;
		case BadContainerTransfer:
			{
				static const std::string r = "BadContainerTransfer";
				return r;
			}
			break;
		case Consumed:
			{
				static const std::string r = "Consumed";
				return r;
			}
			break;
		case SetupFailed:
			{
				static const std::string r = "SetupFailed";
				return r;
			}
			break;
		case Replaced:
			{
				static const std::string r = "Replaced";
				return r;
			}
			break;
		}
		static const std::string unknown = "UNKNOWN";
		return unknown;
	}
}

using namespace DeleteReasons;

// ======================================================================

namespace ServerObjectNamespace
{
	const std::string OBJVAR_STRUCTURE_PLAYER_PACK("structure.player.pack");

	const std::string OBJVAR_OPEN_BANK_TERMINAL_ID("open_bank_location");

	unsigned long gs_objectCount = 0;

	const char * const portalPropertyCrcObjectVariableName = "portalProperty.crc";

	// ----------------------------------------------------------------------

	const std::string objvar_item_attribute_keys   = "item_attrib_keys";
	const std::string objvar_item_attribute_values = "item_attrib_values";

	// ----------------------------------------------------------------------

	const uint32 s_clearLocalFlagsOnConcludeMask =
		~(
		    (1u<<LocalObjectFlags::ServerObject_DirtyObjectMenuSent)
		  | (1u<<LocalObjectFlags::ServerObject_DirtyAttributesSent)
		  | (1u<<LocalObjectFlags::ServerObject_GoingToConclude));

	// ----------------------------------------------------------------------

	// sentinel to keep the messageTo current being
	// handled from getting removed from m_messageTos
	std::pair<std::pair<unsigned long, uint64>, MessageToId> s_currentMessageToBeingHandled = std::make_pair(std::make_pair(0, 0), MessageToId::cms_invalid);

	// ----------------------------------------------------------------------

	// list of object types that supports no trade "shared" behavior
	// (no trade item of those types can be picked up by another character
	// on the same account as the character who currently owns the item)
	std::set<uint32> s_noTradeSharedTemplateItemType;
	std::set<std::string> s_noTradeSharedStaticItemType;

	// list of object types that supports no trade "removable" behavior
	std::set<uint32> s_noTradeRemovableTemplateItemType;
	std::set<std::string> s_noTradeRemovableStaticItemType;

	// ----------------------------------------------------------------------

	void sendDeleteToDatabase(const ServerObject& obj, int reason)
	{
		bool const immediate = InstantDeleteList::isOnList(*obj.getObjectTemplate());
		bool const cascadeDeleteReason = !obj.getLoadContents() && reason == DeleteReasons::House; // Anything that is deleted by house deletion and is demanded-loaded (i.e. cells) should have the delete reason of its contents set to DeleteReasons::House also, instead of DeleteReasons::ContainerDeleted.  This is so the /restoreHouse command can identify these items

		FlagObjectForDeleteMessage const dm(obj.getNetworkId(), reason, immediate, !obj.getLoadContents(), cascadeDeleteReason);
		GameServer::getInstance().sendToDatabaseServer(dm);
	}

	//----------------------------------------------------------------------

	void flagChildObjectsForDeletion(ServerObject &obj, int reason)
	{
		Container * const container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				ServerObject * const containedObject = safe_cast<ServerObject *>((*containerIterator).getObject());
				if (containedObject && containedObject->isPersisted())
				{
					sendDeleteToDatabase(*containedObject, reason);
					LOG("CustomerService",("Deletion:%s deleted from game.  Reason: %s", ServerObject::getLogDescription(containedObject).c_str(), describeDeleteReason(reason).c_str()));
					flagChildObjectsForDeletion(*containedObject, reason);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	void endBaselinesInitializeScript(ServerObject& obj, bool doChildCheck)
	{
		if (doChildCheck)
		{
			ServerObject * parent = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(obj));
			if (parent && !parent->isInitialized())
				return;
		}

		obj.getScriptObject()->initScriptInstances();
		obj.getScriptObject()->setOwnerIsInitialized();

		Container* container = ContainerInterface::getContainer(obj);
		if (container)

		{
			std::vector<CachedNetworkId> initList;

			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				initList.push_back(*containerIterator);
			}
			for(std::vector<CachedNetworkId>::iterator i = initList.begin(); i != initList.end(); ++i)
			{
				ServerObject * containedObject = safe_cast<ServerObject*>((*i).getObject());
				if (!containedObject)
				{
					WARNING_STRICT_FATAL(true, ("Cannot initialize object %s because it did not resolve to an object from an id", (*i).getValueString().c_str()));
				}
				else
					endBaselinesInitializeScript(*containedObject, false);
			}
		}
	}

	//----------------------------------------------------------------------

	const std::string OBJVAR_BANK_NAME_ID = "banking_bankid";

	//----------------------------------------------------------------------

	bool checkBankTransfer(ServerObject* dest, const ServerObject& item, const ServerObject* transferer)
	{
		if (!dest || !transferer)
			return true;

		const CreatureObject * const creature = transferer->asCreatureObject();
		if (creature && dest == creature->getBankContainer())
		{
			float distance = 0.0f;
			//Banks are carried along with the player, so their distance
			//is always going to be 0.  This block changes the parentInWorld
			//to be the bank terminal that was opened rather than the bank
			//itself.
			const ServerObject * const bank = creature->getBankContainer();
			if (bank)
			{
				NetworkId bankTerminalId;
				if (bank->areContentsLoaded() && creature->getObjVars().getItem(OBJVAR_OPEN_BANK_TERMINAL_ID,bankTerminalId))
				{

					const TangibleObject * const bankTerminal = safe_cast<TangibleObject *>(NetworkIdManager::getObjectById(bankTerminalId));
					if (bankTerminal)
					{
						Unicode::String bankName;
						if (bankTerminal->getObjVars().getItem(OBJVAR_BANK_NAME_ID, bankName))
						{
							if (!creature->isBankMember(bankName))
							{
								return false;
							}
						}

						distance = creature->getPosition_w().magnitudeBetween(bankTerminal->getPosition_w());
						distance -= dest->getRadius();
						distance -= bankTerminal->getRadius();
						// Check range on the server with a 1m fudge factor in it.
						if (distance > 6.0f + 1.0f) //todo MLS make that range of 6.0 configurable
						{
							static const StringId message("system_msg", "out_of_range");
							Chat::sendSystemMessage(*creature, message, Unicode::emptyString);
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
		}

		return true;

	}

	//----------------------------------------------------------------------

	void getPlayerControlledContents(ServerObject &obj, std::vector<CachedNetworkId> &results)
	{
		if (obj.isPlayerControlled())
			results.push_back(CachedNetworkId(obj));

		Container *container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject *containedObject = safe_cast<ServerObject *>((*i).getObject());
				if (containedObject)
					getPlayerControlledContents(*containedObject, results);
			}
		}
	}

	// ----------------------------------------------------------------------

	void getContainedClients(ServerObject &obj, std::vector<Watcher<Client> > &containedClients)
	{
		// find all clients for primary controlled objects in the containment hierarchy under obj (including obj)

		Client * const client = obj.getClient();
		if (client && client->getCharacterObjectId() == obj.getNetworkId())
			containedClients.push_back(Watcher<Client>(client));

		Container * const container = ContainerInterface::getContainer(obj);
		if (container)
		{
			for (ContainerIterator i = container->begin(); i != container->end(); ++i)
			{
				ServerObject * const containedObject = safe_cast<ServerObject *>((*i).getObject());
				if (containedObject)
					getContainedClients(*containedObject, containedClients);
			}
		}
	}

	// ----------------------------------------------------------------------

	unsigned int getPackedObjVarField(std::string const &buf, unsigned int offset, std::string &result)
	{
		std::vector<char> s;
		while (offset < buf.size())
		{
			if (buf[offset] == '|')
				break;
			s.push_back(buf[offset++]);
		}
		s.push_back('\0');
		result = &(s[0]);
		return offset+1;
	}

	// ----------------------------------------------------------------------

	unsigned int getPackedObjVarField(std::string const &buf, unsigned int offset, int &result)
	{
		std::string s;
		offset = getPackedObjVarField(buf, offset, s);
		result = atoi(s.c_str());
		return offset;
	}

	// ----------------------------------------------------------------------

	void appendPackedObjVarField(std::vector<char> &buf, char const *s)
	{
		while (*s)
		{
			if ((*s) != '|')
				buf.push_back(*s);
			++s;
		}
		buf.push_back('|');
	}

	// ----------------------------------------------------------------------

	void appendPackedObjVarField(std::vector<char> &buf, int val)
	{
		char tempbuf[32];
		sprintf(tempbuf, "%d", val);
		appendPackedObjVarField(buf, tempbuf);
	}

	// ----------------------------------------------------------------------

	void remove()
	{
	}
}

using namespace ServerObjectNamespace;

// ======================================================================

const SharedObjectTemplate * ServerObject::m_defaultSharedTemplate = nullptr;
float ServerObject::ms_buildingUpdateRadiusMultiplier = 0;

// ======================================================================

class ServerObject::PobFixupNotification : public ObjectNotification
{
public:
	PobFixupNotification();
	~PobFixupNotification();

	virtual void pobFixupComplete(Object &object) const;
	
private:
	PobFixupNotification(const PobFixupNotification & );
	PobFixupNotification& operator= (const PobFixupNotification&);
};

//------------------------------------------------------------------------------------------

ServerObject::PobFixupNotification::PobFixupNotification() : ObjectNotification()
	{
	}

//------------------------------------------------------------------------------------------

ServerObject::PobFixupNotification::~PobFixupNotification()
{
}

//------------------------------------------------------------------------------------------

void ServerObject::PobFixupNotification::pobFixupComplete(Object &object) const
{
	static_cast<ServerObject*>(&object)->onPobFixupComplete();
//	object.removeNotification(*this);
}

//------------------------------------------------------------------------------------------

static ServerObject::PobFixupNotification pobFixupNotification;

//------------------------------------------------------------------------------------------

class ServerObject::MoveNotification : public ObjectNotification
{
public:
	MoveNotification();
	~MoveNotification();
	virtual void addToWorld(Object &object) const;
	virtual void cellChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	virtual void rotationChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	virtual void visibilityDataChanged(Object &object) const;

private:
	void checkForGoldObject(const Object &object) const;

private:
	MoveNotification(const MoveNotification &);
	MoveNotification & operator= (const MoveNotification &);
};

static ServerObject::MoveNotification moveNotification;


//------------------------------------------------------------------------------------------
ServerObject::MoveNotification::MoveNotification() :
		ObjectNotification()
{

}

//------------------------------------------------------------------------------------------

ServerObject::MoveNotification::~MoveNotification()
{

}

//------------------------------------------------------------------------------------------

void ServerObject::MoveNotification::addToWorld(Object &object) const
{
	static_cast<ServerObject &>(object).updateWorldSphere();
}

// ----------------------------------------------------------------------

void ServerObject::MoveNotification::checkForGoldObject(const Object &object) const
{
	if (ConfigServerGame::getMaxGoldNetworkId()!=NetworkId::cms_invalid && object.getNetworkId() < ConfigServerGame::getMaxGoldNetworkId())
	{
		DEBUG_FATAL(ConfigServerGame::getFatalOnMovingGoldObject(),("Attempted to move object %s, which is a gold object that should not be movable.", object.getNetworkId().getValueString().c_str()));
		DEBUG_WARNING(true,("Attempted to move object %s, which is a gold object that should not be movable.", object.getNetworkId().getValueString().c_str()));
	}
}

//------------------------------------------------------------------------------------------

void ServerObject::MoveNotification::cellChanged(Object &object, bool dueToParentChange) const
{
	checkForGoldObject(object);

	static_cast<ServerObject &>(object).updateWorldSphere();
	if (!dueToParentChange)
	{
		DEBUG_FATAL(object.getCellProperty() && !object.getKill(), ("Cell object changing cells"));
		static_cast<ServerObject &>(object).setTransformChanged(true);
	}
}

//------------------------------------------------------------------------------------------

bool ServerObject::MoveNotification::positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const
{
	checkForGoldObject(object);

	UNREF(oldPosition);
	static_cast<ServerObject &>(object).updateWorldSphere();
	if (!dueToParentChange)
	{
		DEBUG_FATAL(object.getCellProperty() && !object.getKill(), ("Cell object changing position"));
		static_cast<ServerObject &>(object).setTransformChanged(true);
	}
	return true;
}

//------------------------------------------------------------------------------------------

void ServerObject::MoveNotification::rotationChanged(Object &object, bool dueToParentChange) const
{
	checkForGoldObject(object);

	static_cast<ServerObject &>(object).updateWorldSphere();
	if (!dueToParentChange)
	{
		DEBUG_FATAL(object.getCellProperty() && !object.getKill(), ("Cell object changing rotation"));
		static_cast<ServerObject &>(object).setTransformChanged(true);
	}
}

//------------------------------------------------------------------------------------------

bool ServerObject::MoveNotification::positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const
{
	checkForGoldObject(object);

	UNREF(oldPosition);
	static_cast<ServerObject &>(object).updateWorldSphere();
	if (!dueToParentChange)
	{
		DEBUG_FATAL(object.getCellProperty() && !object.getKill(), ("Cell object changing position and rotation"));
		static_cast<ServerObject &>(object).setTransformChanged(true);
	}
	return true;
}

//------------------------------------------------------------------------------------------

void ServerObject::MoveNotification::visibilityDataChanged(Object &object) const
{
	static_cast<ServerObject &>(object).updateWorldSphere();
	static_cast<ServerObject &>(object).addObjectToConcludeList();
}

// ======================================================================

void ServerObject::AuthProcessIdCallback::modified(ServerObject &target, uint32 oldValue, uint32, bool) const
{
	AuthTransferTracker::sendConfirmAuthTransfer(target.getNetworkId(), oldValue);
}

void ServerObject::ObserversCountCallback::modified(ServerObject &target, int oldValue, int, bool) const
{
	target.observersCountChanged();
}

// ======================================================================

ServerObject::ServerObject(const ServerObjectTemplate* newTemplate, const ObjectNotification &notification, bool const hyperspaceOnCreate) :
Object                        (newTemplate, NetworkId::cms_invalid),
m_oldPosition                 (),
m_sharedTemplate              (nullptr),
m_client                      (0),
m_observers                   (),
m_localFlags                  (0),
m_scriptObject                (new GameScriptObject),
m_objVars                     (),
m_observersCount              (0),
m_volume                      (newTemplate->getVolume()),
m_complexity                  (newTemplate->getComplexity()),
m_sceneId                     (ServerWorld::getSceneId()),
m_objectName                  (),
m_includeInBuildout           (false),
m_nameStringId                (),
m_staticItemName              (),
m_staticItemVersion           (0),
m_conversionId                (0),
m_playerControlled            (false),
m_synchronizedUi              (),
m_pendingSyncUi               (0),
m_persisted                   (false),
m_authServerProcessId         (),
m_proxyServerProcessIds       (),
m_transformSequence           (),
m_cacheVersion                (0),
m_loadContents                (true),
m_contentsLoaded              (false),
m_contentsRequested           (false),
m_loadWith                    (NetworkId::cms_invalid),
m_cashBalance                 (0),
m_bankBalance                 (0),
m_defaultAlterTime            (AlterResult::cms_keepNoAlter),
m_broadcastListeners          (),
m_broadcastBroadcasters       (),
m_authClientServerPackage     (),
m_authClientServerPackage_np  (),
m_firstParentAuthClientServerPackage(),
m_firstParentAuthClientServerPackage_np(),
m_serverPackage               (),
m_serverPackage_np            (),
m_sharedPackage               (),
m_sharedPackage_np            (),
m_networkUpdateFar            (nullptr),
m_triggerVolumes              (),
m_attributesAttained          (),
m_attributesInterested        (),
m_triggerVolumeInfo           (),
m_messageTos                  (),
m_worldSphere                 (Sphere::unit),
m_dirtyArchiveCallback        (),
m_unreliableControllerMessageSequenceNumber(0),
m_gameObjectType              (0),
m_calledTriggerDestroy        (false),
m_calledTriggerRemovingFromWorld(false),
m_loadCTSBank(false),
m_loadCTSPackedHouses(false)
{
	synchronizationOnConstruct();

	m_observersCount.setSourceObject(this);
	m_authServerProcessId.setSourceObject(this);

	NOT_NULL(newTemplate);

	setLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnCreate, hyperspaceOnCreate);
	setLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnDestruct, false);

#ifdef _DEBUG
	setDebugName(getTemplateName());
#endif

	updateWorldSphere();

	const std::string & sharedTemplateName = newTemplate->getSharedTemplate();
	m_sharedTemplate = dynamic_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch(sharedTemplateName));
	if (m_sharedTemplate == nullptr)
	{
		WARNING_STRICT_FATAL(!sharedTemplateName.empty(), ("Template %s has an invalid shared template %s. We will use the default shared template for now.",
			newTemplate->getName(), sharedTemplateName.c_str()));
	}

	if (getSharedTemplate() != nullptr)
	{
		m_nameStringId = getSharedTemplate()->getObjectName();
		m_descriptionStringId = getSharedTemplate()->getDetailedDescription();
	}

	m_scriptObject->setOwner(this);

	ContainedByProperty *containedBy = new ContainedByProperty(*this, nullptr);
	addProperty(*containedBy);

	//-- create the SlottedContainment property
	SlottedContainmentProperty *slottedProperty = new SlottedContainmentProperty(*this);
	addProperty(*slottedProperty);

	//-- get ArrangementDescriptor if specified
	if (getSharedTemplate() != nullptr)
	{
		const ArrangementDescriptor *const arrangementDescriptor = getSharedTemplate()->getArrangementDescriptor();
		if (arrangementDescriptor)
		{
			const int arrangementCount = arrangementDescriptor->getArrangementCount();
			for (int i = 0; i < arrangementCount; ++i)
			{
				slottedProperty->addArrangement(arrangementDescriptor->getArrangement(i));
			}
		}
	}

	//set up containers on this object if it has any
	if (getSharedTemplate() != nullptr)
	{
		SharedObjectTemplate::ContainerType const containerType = getSharedTemplate()->getContainerType();

		switch (containerType)
		{
			//buildings (portal properties get added below
		case SharedObjectTemplate::CT_none:
			break;

		case SharedObjectTemplate::CT_slotted:
		case SharedObjectTemplate::CT_ridable:
		{
			const SlotDescriptor *const slotDescriptor = getSharedTemplate()->getSlotDescriptor();
			if (slotDescriptor)
			{
				//-- create a SlottedContainer with the given slots
				SlottedContainer* slotContainer = new SlottedContainer(*this, slotDescriptor->getSlots());
				addProperty(*slotContainer);
				slotContainer = 0;
			}
			else
			{
				DEBUG_WARNING(true, ("DATA ERROR: specified slotted container type but could not get slot descriptor"));
			}
			break;
		}
		case SharedObjectTemplate::CT_volume:
		{
			VolumeContainer * volumeContainer = 0;
			int maxVolume = getSharedTemplate()->getContainerVolumeLimit();
			if (maxVolume <= 0)
			{
				maxVolume = VolumeContainer::VL_NoVolumeLimit;
			}
			volumeContainer = new TangibleVolumeContainer(*this, maxVolume);
			addProperty(*volumeContainer);
			volumeContainer = 0;
			break;
		}
		case SharedObjectTemplate::CT_volumeIntangible:
		{
			int maxVolume = getSharedTemplate()->getContainerVolumeLimit();
			if (maxVolume <= 0)
			{
				maxVolume = VolumeContainer::VL_NoVolumeLimit;
			}
			VolumeContainer * volumeContainer = new IntangibleVolumeContainer(*this, maxVolume);
			addProperty(*volumeContainer);
			volumeContainer = 0;
			break;
		}
		case SharedObjectTemplate::CT_volumeGeneric:
		{
			int maxVolume = getSharedTemplate()->getContainerVolumeLimit();
			if (maxVolume <= 0)
			{
				maxVolume = VolumeContainer::VL_NoVolumeLimit;
			}
			VolumeContainer * volumeContainer = new VolumeContainer(*this, maxVolume);
			addProperty(*volumeContainer);
			volumeContainer = 0;
			break;
		}

		default:
			DEBUG_WARNING(true, ("Invalid container type specified."));
			break;
		}

		setLocalFlag(LocalObjectFlags::ServerObject_SendToClient, getSharedTemplate()->getSendToClient());
		m_gameObjectType = static_cast<int>(getSharedTemplate()->getGameObjectType());
	}

	//If the object template specifies a volume, give it a volumeContainment
	int vol = newTemplate->getVolume();
	VolumeContainmentProperty* volumeProperty = new VolumeContainmentProperty(*this, (vol < 1 ? 1 : vol) );
	addProperty(*volumeProperty);
	volumeProperty = 0;   //lint !e423 // creation of memory leak // no, managed by property list


	// add the portal property if one is requested
	if (getSharedTemplate() != nullptr)
	{
		const std::string &portalLayoutFileName = getSharedTemplate()->getPortalLayoutFilename();
		if (!portalLayoutFileName.empty())
		{
			PortalProperty *portalProperty = new PortalProperty(*this, portalLayoutFileName.c_str());
			addProperty(*portalProperty);
		}
	}

	m_triggerVolumeInfo.setOnChanged(this, &ServerObject::updateTriggerVolumes);

	addMembersToPackages();

	addNotification(notification);

	addNotification(moveNotification);

	gs_objectCount++;
	ObjectTracker::addObject();

	//-- add the no build notification
	{
		if (getSharedTemplate () && getSharedTemplate ()->getNoBuildRadius () > 0.f)
			addNotification (ServerNoBuildNotification::getInstance ());
	}

	//-- add the clear flora notification
	{
		if (getSharedTemplate () && getSharedTemplate ()->getClearFloraRadius () > 0.f)
			addNotification (ServerClearCollidableFloraNotification::getInstance ());
	}
}

//-----------------------------------------------------------------------

ServerObject::~ServerObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	ObserveTracker::onObjectDestroyed(*this, getLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnDestruct));
	PositionUpdateTracker::serverObjDestroyed(*this);

	synchronizationOnDestruct();

	if (getLayer())
		ObjectTracker::removeRunTimeRule();

	if (getClient())
		getClient()->removeControlledObject(*this);

	WARNING_STRICT_FATAL(getClient(),("Deleting server object %s which had a client.",getNetworkId().getValueString().c_str()));

	{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject removeFromWorld");
	//remove from world
	if (isInWorld())
		removeFromWorld();
	}

	WARNING_STRICT_FATAL(!m_triggerVolumes.empty(), ("Deleting object %s that still has trigger volumes attached", getNetworkId().getValueString().c_str()));

	{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject destroyTriggerVolumes");
	// make absolutely sure our trigger volumes are removed from the world
	destroyTriggerVolumes();
	}

	if (m_sharedTemplate != nullptr)
	{
		m_sharedTemplate->releaseReference();
		m_sharedTemplate = nullptr;
	}

	if (getClient())
	{
		PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject dropClient");
		//Object has a client!!
		WARNING(true, ("Destroying object %s which isAuth: %d", getNetworkId().getValueString().c_str(), isAuthoritative()));
		GameServer::getInstance().dropClient(getNetworkId());
	}

	{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject removeObjectFromGame");
	// clean ourselves off any of ServerWorlds lists.
	ServerWorld::removeObjectFromGame(*this);
	}

	delete m_pendingSyncUi;
	m_pendingSyncUi = 0;

	delete m_synchronizedUi;
	m_synchronizedUi = 0;

	{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject delete script object");
		delete m_scriptObject;
	}
	m_scriptObject = nullptr;

	gs_objectCount--;

	{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::~ServerObject ObjectTracker::removeObject");
	ObjectTracker::removeObject();
	}
}

// ----------------------------------------------------------------------

void ServerObject::install()
{
	// read in list of object types that supports no trade "shared" behavior
	// (no trade item of those types can be picked up by another character
	// on the same account as the character who currently owns the item)
	char const * const noTradeSharedDataTableName = "datatables/no_trade/no_trade_shared.iff";
	DataTable *noTradeSharedTable = DataTableManager::getTable(noTradeSharedDataTableName, true);
	if (noTradeSharedTable)
	{
		int const columnObjectTemplate = noTradeSharedTable->findColumnNumber("Object Template");
		if (columnObjectTemplate >= 0)
		{
			std::string objectTemplate;

			for (int i = 0, numRows = noTradeSharedTable->getNumRows(); i < numRows; ++i)
			{
				objectTemplate = noTradeSharedTable->getStringValue(columnObjectTemplate, i);
				if (objectTemplate.empty())
					continue;

				if (objectTemplate.find(".iff") != std::string::npos)
					s_noTradeSharedTemplateItemType.insert(CrcLowerString::calculateCrc(objectTemplate.c_str()));
				else
					s_noTradeSharedStaticItemType.insert(objectTemplate);
			}
		}

		DataTableManager::close(noTradeSharedDataTableName);
	}

	char const *const noTradeRemovableDataTableName = "datatables/no_trade/no_trade_removable.iff";
	DataTable *noTradeRemovableTable = DataTableManager::getTable(noTradeRemovableDataTableName, true);
	if (noTradeRemovableTable)
	{
		int const columnObjectTemplate = noTradeRemovableTable->findColumnNumber("Object Template");
		if (columnObjectTemplate >= 0)
		{
			std::string objectTemplate;

			for (int i = 0, numRows = noTradeRemovableTable->getNumRows(); i < numRows; ++i)
			{
				objectTemplate = noTradeRemovableTable->getStringValue(columnObjectTemplate, i);
				if (objectTemplate.empty())
					continue;

				if (objectTemplate.find(".iff") != std::string::npos)
					s_noTradeRemovableTemplateItemType.insert(CrcLowerString::calculateCrc(objectTemplate.c_str()));
				else
					s_noTradeRemovableStaticItemType.insert(objectTemplate);
			}
		}

		DataTableManager::close(noTradeSharedDataTableName);
	}

	ExitChain::add(ServerObjectNamespace::remove, "ServerObjectNamespace::remove");
}

//-----------------------------------------------------------------------

ServerObject * ServerObject::getServerObject(NetworkId const & networkId)
{
	return asServerObject(NetworkIdManager::getObjectById(networkId));
}

//-----------------------------------------------------------------------

ServerObject * ServerObject::asServerObject(Object * const object)
{
	return (object != nullptr) ? object->asServerObject() : nullptr;
}

//-----------------------------------------------------------------------

ServerObject const * ServerObject::asServerObject(Object const * const object)
{
	return (object != nullptr) ? object->asServerObject() : nullptr;
}

//-----------------------------------------------------------------------

ServerObject * ServerObject::asServerObject()
{
	return this;
}

//-----------------------------------------------------------------------

ServerObject const * ServerObject::asServerObject() const
{
	return this;
}

//-----------------------------------------------------------------------

CreatureObject * ServerObject::asCreatureObject()
{
	return 0;
}

//-----------------------------------------------------------------------

CreatureObject const * ServerObject::asCreatureObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

BuildingObject * ServerObject::asBuildingObject()
{
	return 0;
}

//-----------------------------------------------------------------------

BuildingObject const * ServerObject::asBuildingObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

StaticObject * ServerObject::asStaticObject()
{
	return 0;
}

//-----------------------------------------------------------------------

StaticObject const * ServerObject::asStaticObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

TangibleObject * ServerObject::asTangibleObject()
{
	return 0;
}

//-----------------------------------------------------------------------

TangibleObject const * ServerObject::asTangibleObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

IntangibleObject *ServerObject::asIntangibleObject()
{
	return 0;
}

//-----------------------------------------------------------------------

IntangibleObject const *ServerObject::asIntangibleObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

MissionObject * ServerObject::asMissionObject()
{
	return 0;
}

//-----------------------------------------------------------------------

MissionObject const * ServerObject::asMissionObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

CellObject * ServerObject::asCellObject()
{
	return 0;
}

//-----------------------------------------------------------------------

CellObject const * ServerObject::asCellObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

GroupObject * ServerObject::asGroupObject()
{
	return 0;
}

//-----------------------------------------------------------------------

GroupObject const * ServerObject::asGroupObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

ShipObject * ServerObject::asShipObject()
{
	return 0;
}

//-----------------------------------------------------------------------

ShipObject const * ServerObject::asShipObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

PlayerObject *ServerObject::asPlayerObject()
{
	return 0;
}

//-----------------------------------------------------------------------

PlayerObject const * ServerObject::asPlayerObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

WeaponObject *ServerObject::asWeaponObject()
{
	return 0;
}

//-----------------------------------------------------------------------

WeaponObject const * ServerObject::asWeaponObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

ManufactureSchematicObject *ServerObject::asManufactureSchematicObject()
{
	return 0;
}

//-----------------------------------------------------------------------

ManufactureSchematicObject const * ServerObject::asManufactureSchematicObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

PlayerQuestObject * ServerObject::asPlayerQuestObject()
{
	return 0;
}

//-----------------------------------------------------------------------

PlayerQuestObject const * ServerObject::asPlayerQuestObject() const
{
	return 0;
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * ServerObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/object/base/shared_object_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "ServerObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// ServerObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

const unsigned long ServerObject::getObjectCount()
{
	return gs_objectCount;
}

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void ServerObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// ServerObject::removeDefaultTemplate

// ----------------------------------------------------------------------

void ServerObject::addObjectToConcludeList()
{
	if (!isInitialized())
		return;
	if (!isGoingToConclude())
	{
		setGoingToConclude(true);
		ServerWorld::addObjectToConcludeList(this);
	}
}

// ----------------------------------------------------------------------

/**
 * Called during the alter phase of the game.
 *
 * @param time		time since last frame
 *
 * @return true or false
 */
float ServerObject::alter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::alter");

	//Check initialization
	WARNING_STRICT_FATAL(!isInitialized(), ("Alter called on unitialized object %s.", getNetworkId().getValueString().c_str()));
	if (!isInitialized())
		return 1.0f;

	// Alter the base class.
	float result = Object::alter(time);

	if (isPlacing())
	{
		TerrainObject* terrain = TerrainObject::getInstance();
		if (terrain && getSharedTemplate() && getSharedTemplate()->getSnapToTerrain() && isInWorldCell() && isInWorld())
		{
			terrain->placeObject(*this, false, true);
		}
		setPlacing(false);
	}

	if (result == AlterResult::cms_kill)
	{
		if (isAuthoritative())
		{
			//If we are being deleted this frame, make sure to update our deltas, position, etc.
			sendDeltasForSelfAndContents();
		}
		if (isUnloading())
		{
			Object * const containerObject = ContainerInterface::getContainedByObject(*this);
			//If we're contained, remove ourself from the container before destruction
			if (containerObject)
			{
				Container * const container = ContainerInterface::getContainer(*containerObject);
				NOT_NULL(container);
				container->internalItemRemoved(*this);
				DEBUG_REPORT_LOG(true, ("Removing %s from container %s because it is being unloaded\n",
										getNetworkId().getValueString().c_str(),
										containerObject->getNetworkId().getValueString().c_str()));
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ServerObject::canDropInWorld() const
{
	return false;
}

// ----------------------------------------------------------------------

bool ServerObject::canManipulateObject(ServerObject const &target, bool movingObject, bool checkPermissions, bool checkPermissionsOnParent, float maxDistance, Container::ContainerErrorCode&, bool skipNoTradeCheck, bool * allowedByGodMode) const
{
	//In general objects cannot manipulate other objects, unless they are creatures.  This may not be true of all
	//objects, however, which is why the function is implemented this way (ie a player object that derives from ServerObject).
	UNREF(target);
	UNREF(maxDistance);
	UNREF(checkPermissions);
	UNREF(checkPermissionsOnParent);
	UNREF(movingObject);
	UNREF(skipNoTradeCheck);
	UNREF(allowedByGodMode);
	return false;
}

//-----------------------------------------------------------------------

bool ServerObject::canTrade() const
{
	if( isInBazaarOrVendor() )
	{
		return false;
	}
	
	// aconite 3/21/22
	// if an item has the move flag MF_GM but *doesn't* have the MF_Player flag
	// that object shouldn't be considered tradeable because it isn't intended to
	// be moved by a player (fixes, e.g., trading your buyback container)
	bool hasGmFlag = false;
	bool hasPlayerFlag = false;
	auto tpf = dynamic_cast<const ServerObjectTemplate*>(getObjectTemplate());
	if(tpf)
	{
		const size_t flags = tpf->getMoveFlagsCount();
		if(flags > 0)
		{
			for (size_t i = 0; i < flags; i++)
			{
				if(!hasGmFlag && tpf->getMoveFlags(i) == ServerObjectTemplate::MF_gm)
				{
					hasGmFlag = true;
				}
				if (!hasPlayerFlag && tpf->getMoveFlags(i) == ServerObjectTemplate::MF_player)
				{
					hasPlayerFlag = true;
				}
			}
			if(hasGmFlag && !hasPlayerFlag)
			{
				return false;
			}
		}
	}

	return !markedNoTrade();
}

//-----------------------------------------------------------------------

bool ServerObject::canTradeRecursive(bool testPlayers) const
{
	if( testPlayers || !isPlayerControlled() )
	{
		if( !canTrade() )
		{
			return false;
		}
		else
		{
			// check the contained items
			const Container * const container = ContainerInterface::getContainer(*this);
			if( container )
			{
				for( ContainerConstIterator i = container->begin(); i != container->end(); ++i )
				{
					const ServerObject * const content = safe_cast<const ServerObject *>((*i).getObject());
					if( content )
					{
						if( !content->canTradeRecursive(testPlayers) )
							return false;
					}
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------

bool ServerObject::markedNoTrade() const
{
	return ( getObjVars().hasItem("noTrade") || getObjVars().hasItem("notrade") );
}

//-----------------------------------------------------------------------

bool ServerObject::markedNoTradeRecursive(bool testPlayers, bool testOnlyContainedItems) const
{
	if( testPlayers || !isPlayerControlled() )
	{
		if( !testOnlyContainedItems && markedNoTrade() )
		{
			return true;
		}
		else
		{
			// check the contained items
			const Container * const container = ContainerInterface::getContainer(*this);
			if( container )
			{
				for( ContainerConstIterator i = container->begin(); i != container->end(); ++i )
				{
					const ServerObject * const content = safe_cast<const ServerObject *>((*i).getObject());
					if( content )
					{
						if( content->markedNoTradeRecursive(testPlayers) )
							return true;
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------

bool ServerObject::markedNoTradeShared(bool includeCheckForNoTrade) const
{
	if (includeCheckForNoTrade)
	{
		if (!markedNoTrade())
			return false;
	}

	if (getObjVars().hasItem("noTradeShared"))
		return true;

	// see if the item type is no trade "shared"
	if (m_staticItemName.get().empty())
		return (s_noTradeSharedTemplateItemType.count(getTemplateCrc()) > 0);

	return (s_noTradeSharedStaticItemType.count(m_staticItemName.get()) > 0);
}

//-----------------------------------------------------------------------

bool ServerObject::markedNoTradeRemovable() const
{
	if (!markedNoTrade())
		return false;

	if (m_staticItemName.get().empty())
		return (s_noTradeRemovableTemplateItemType.count(getTemplateCrc()) > 0);

	return (s_noTradeRemovableStaticItemType.count(m_staticItemName.get()) > 0);
}

//-----------------------------------------------------------------------

bool ServerObject::isInSecureTrade() const
{
	const ServerObject *parent = safe_cast<const ServerObject*>(ContainerInterface::getFirstParentInWorld(*this));
	if( parent )
	{
		const CreatureObject *creatureObjParent = parent->asCreatureObject();
		if( creatureObjParent )
		{
			const CreatureController *cc = creatureObjParent->getCreatureController();
			if( cc )
			{
				const ServerSecureTrade *trade = cc->getSecureTrade();
				if( trade && trade->hasItemInSecureTrade(*creatureObjParent, *this) )
				{
					return true;
				}
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

/**
 * Returns whether an object is bio-linked
 *
 * @return true if a the object is bio-linked
 */
bool ServerObject::isBioLinked() const
{
	// By default, objects are not bio-linked
	return false;
}

// ----------------------------------------------------------------------

/**
 * Returns whether an object, or any contained object, is biolinked
 *
 * @return true if a biolinked object is found
 */
bool ServerObject::isBioLinkedRecursive() const
{
	if( isBioLinked() )
	{
		return true;
	}
	else
	{
		// If we are a container, see if any of the contained objects are bio-linked
		const Container * const container = ContainerInterface::getContainer(*this);
		if( container )
		{
			for( ContainerConstIterator i = container->begin(); i != container->end(); ++i )
			{
				const ServerObject * const content = safe_cast<const ServerObject *>((*i).getObject());
				if( content )
				{
					if( content->isBioLinkedRecursive() )
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------

bool ServerObject::isWaypoint() const
{
	return getObjectTemplate()->derivesFrom(SERVER_PATHFINDING_WAYPOINT_TEMPLATE_BASE);
}

//-----------------------------------------------------------------------

bool ServerObject::isInBazaarOrVendor() const
{
	bool inBazaarOrVendor = false;
	const ServerObject *parent = safe_cast<const ServerObject*>(ContainerInterface::getFirstParentInWorld(*this));
	const ServerObject *grandParent = nullptr;
	if( parent )
	{
		grandParent = safe_cast<const ServerObject*>(ContainerInterface::getFirstParentInWorld(*parent));
	}

	if( parent && (parent->isVendor() || parent->isBazaarTerminal() ) )
	{
		inBazaarOrVendor = true;
	}
	else if( grandParent && (grandParent->isVendor() || grandParent->isBazaarTerminal()) )
	{
		inBazaarOrVendor = true;
	}

	return inBazaarOrVendor;
}

void ServerObject::removeAllAuctions()
{
	if ( isVendor() )
	{
		ServerObject *inv = getBazaarContainer();
		NetworkId playerOwnerId;
		getObjVars().getItem("vendor_owner", playerOwnerId);
		if( inv && playerOwnerId.getValue()>0 )
		{
			// if this was an uninitialized vendor, remove objvar from owner
			ServerObject *owner = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(playerOwnerId));
			if (owner)
			{
				NetworkId notInitalizedVendor;
				if (owner->getObjVars().getItem("vendor_not_initialized", notInitalizedVendor))
				{
					if (notInitalizedVendor == Object::getNetworkId())
					{
						owner->removeObjVarItem("vendor_not_initialized");
					}
				}
			}
			CommoditiesMarket::destroyVendorMarket(playerOwnerId, *inv);
		}
	}
}

// ----------------------------------------------------------------------

bool ServerObject::checkLOSTo(ServerObject const &target) const
{
	return LineOfSightCache::checkLOS(*this, target);
}

//-----------------------------------------------------------------------

bool ServerObject::checkLOSTo(Location const &target) const
{
	return LineOfSightCache::checkLOS(*this, target);
}

//-----------------------------------------------------------------------

void ServerObject::conclude()
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::conclude");

	// everything has updated, check trigger volume

	if (getTransformChanged() && isInWorld())
	{
		Vector const &newPosition = getTransform_o2w().getPosition_p();
		// move the object and trigger volumes, detect volume enter/leave
		ServerWorld::moveObject(*this, m_oldPosition, newPosition);
		updatePositionOnPlanetServer();
		m_oldPosition = newPosition;
	}

	m_localFlags &= s_clearLocalFlagsOnConcludeMask;

	{
		PROFILER_AUTO_BLOCK_DEFINE("Object::conclude");
		Object::conclude();
	}
}

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 * @todo This function should be removed.  Use the controller factory instead.
 */
Controller* ServerObject::createDefaultController(void)
{
	Controller* controller = new ServerController(this);

	setController(controller);
	return controller;
}	// Object::createDefaultController

//-----------------------------------------------------------------------
/** Called by the container system when this object's contained By value is changed
 */

void ServerObject::containedByModified(NetworkId const &oldValue, NetworkId const &newValue, bool isLocal)
{
	UNREF(isLocal);

	if (oldValue == newValue)
		return;

	ServerObject * const oldContainer = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(oldValue));
	ServerObject * const newContainer = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(newValue));

	// TODO: reenable these fatals after container proxying changes are done
	//FATAL(oldValue.isValid() && !oldContainer, ("ServerObject::containedByModified: obj %s, could not resolve old container %s.", getDebugInformation().c_str(), oldValue.getValueString().c_str()));
	//FATAL(newValue.isValid() && !newContainer, ("ServerObject::containedByModified: obj %s, could not resolve new container %s.", getDebugInformation().c_str(), newValue.getValueString().c_str()));

	if (isAuthoritative())
	{
		FATAL((newContainer && !newContainer->isAuthoritative()), ("Invalid container transfer with auth object to/from non-auth containers."));
	}
	else if (isInitialized())
	{
		FATAL((oldContainer && oldContainer->isAuthoritative()) || (newContainer && newContainer->isAuthoritative()), ("Invalid container transfer with non-auth object to/from auth containers."));

		// For proxy objects and mounts, it is essential for this function
		// to setup new container state before calling
		// isContentItemExposedWith.  Failure to do this causes proxy
		// server objects of a rider to handle dismounting improperly
		// because wasInWorld will be set to false erroneously.
		// Also, it is essential to call this function prior to calling
		// containerHandleUpdateClients() below since that function
		// assumes the objects are in the new containment state.
		containerDepersistContents(oldContainer, newContainer);

		Container const * const newContainerProperty = newContainer ? ContainerInterface::getContainer(*newContainer) : 0;
		bool const wasInWorld = isInWorld();
		bool const nowInWorld = newValue == NetworkId::cms_invalid || (newContainerProperty && newContainer->isInWorld() && newContainerProperty->isContentItemExposedWith(*this));

//		DEBUG_REPORT_LOG(true, ("Updating %s with old container %s and new container %s\n",
//								getNetworkId().getValueString().c_str(),
//								oldValue.getValueString().c_str(),
//								newValue.getValueString().c_str()));
		containerHandleUpdateClients(oldContainer, newContainer, (wasInWorld && !nowInWorld), (!wasInWorld && nowInWorld));
	}

	scheduleForAlter();
}

//-----------------------------------------------------------------------

void ServerObject::arrangementModified(int oldValue, int newValue, bool isLocal)
{
	if (!isLocal && !isAuthoritative() && isInitialized())
	{
		ServerObject * const parent = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*this));
		if (parent)
		{
			SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(*parent);
			if (slotContainer)
			{
				SlottedContainmentProperty* const slotted = ContainerInterface::getSlottedContainmentProperty(*this);
				DEBUG_FATAL(!slotted, ("Error unpacking non slotted objects into a slotted container"));
				slotContainer->updateArrangement(*this, oldValue, slotted->getCurrentArrangement());
			}
		}
		ObserveTracker::onObjectArrangementChanged(*this);
	}
}

//-----------------------------------------------------------------------

void ServerObject::destroyTriggerVolumes()
{
	// get rid of any trigger volumes
	for (TriggerVolumeMap::iterator i = m_triggerVolumes.begin(); i != m_triggerVolumes.end(); ++i)
	{
		ServerWorld::removeObjectTriggerVolume((*i).second);
		delete (*i).second;
	}
	m_triggerVolumes.clear();
	m_networkUpdateFar = 0;
}

//-----------------------------------------------------------------------

void ServerObject::updateTriggerVolumes()
{
	ServerObject *topMost = safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(*this));
	if (!topMost || topMost != this || !topMost->isInWorld())
	{
		destroyTriggerVolumes();
		return;
	}

	// keep track of what trigger volumes should exist so we can destroy the rest afterwards
	std::vector<TriggerVolume*> newVolumes;

	// create any new volumes
	{
		for (std::vector<TriggerVolumeInfo>::const_iterator i = m_triggerVolumeInfo.begin(); i != m_triggerVolumeInfo.end(); ++i)
		{
			const TriggerVolumeMap::iterator j = m_triggerVolumes.find((*i).name);
			if (j != m_triggerVolumes.end())
			{
				TriggerVolume *t = (*j).second;
				// if we already have one that matches, leave it alone
				if (   t->getPromiscuous() == (*i).isPromiscuous
				    && t->getRadius() == (*i).radius)
				{
					newVolumes.push_back(t);
					continue;
				}
				// does not match, so destroy it and fall through to creating the new one
				ServerWorld::removeObjectTriggerVolume(t);
				delete t;
				m_triggerVolumes.erase(j);
			}

			TriggerVolume * t = new TriggerVolume(*this, (*i).radius, (*i).name, (*i).isPromiscuous);
			addTriggerVolume(t);
			newVolumes.push_back(t);
		}
	}

	// remove any volumes which no longer exist
	{
		TriggerVolumeMap::iterator i = m_triggerVolumes.begin();
		while (i != m_triggerVolumes.end())
		{
			TriggerVolume *t = (*i).second;
			if (   t != m_networkUpdateFar
			    && std::find(newVolumes.begin(), newVolumes.end(), t) == newVolumes.end())
			{
				ServerWorld::removeObjectTriggerVolume(t);
				delete t;
				m_triggerVolumes.erase(i++);
			}
			else
				++i;
		}
	}
}

//-----------------------------------------------------------------------

void ServerObject::addTriggerVolume(TriggerVolume * t)
{
	if (!t)
	{
		WARNING_STRICT_FATAL(true, ("Cannot add nullptr volume"));
		return;
	}
	m_triggerVolumes.insert(std::make_pair(t->getName(), t));
	ServerWorld::addObjectTriggerVolume(t);
}

//-----------------------------------------------------------------------

void ServerObject::createTriggerVolume(const real radius, const std::string & name, const bool isPromiscuous)
{
	removeTriggerVolume(name);

	TriggerVolumeInfo triggerVolumeInfo;
	triggerVolumeInfo.name = name;
	triggerVolumeInfo.radius = radius;
	triggerVolumeInfo.isPromiscuous = isPromiscuous;
	m_triggerVolumeInfo.push_back(triggerVolumeInfo);
}

//-----------------------------------------------------------------------

void ServerObject::serverObjectEndBaselines(bool fromDatabase)
{
	setInEndBaselines(true);

	// handle trying to create a player in an incorrect scene, or without a PlayerObject
	bool needSceneFix = false;

	if (isPlayerControlled())
	{
		bool doUnload = false;
		if (!PlayerCreatureController::getPlayerObject(asCreatureObject()))
		{
			WARNING(true, ("TRACE_LOGIN: Loading a player controlled creature object (%s) without a player object.", getNetworkId().getValueString().c_str()));
			doUnload = true;
		}
		else if (getSceneId() != ServerWorld::getSceneId())
		{
			Client* p_client = GameServer::getInstance().getClient( getNetworkId() );


			if ( (p_client) && ( p_client->getSendToStarport() ) )
			{
				needSceneFix = true;
				GameServer::getInstance().requestSceneWarp(CachedNetworkId(*this),"tatooine",Vector(3528,5,-4804),
					NetworkId::cms_invalid,Vector(3528,5,-4804));
			}
			else if (isAuthoritative() && ContainerInterface::getContainedByObject(*this))
			{
				needSceneFix = true;
			}

			else
			{
				WARNING(true,
					("Unloading object id=[%s] on game server id=[%d] due to incorrect scene during baselines: server scene=[%s], object scene=[%s], fromDatabase=[%s], authoritative=[%s])",
					getNetworkId().getValueString().c_str(),
					static_cast<int>(GameServer::getInstance().getProcessId()),
					ServerWorld::getSceneId().c_str(),
					getSceneId().c_str(),
					fromDatabase ? "true" : "false",
					isAuthoritative() ? "true" : "false"
					));

				//-- As a precautionary measure, force CentralServer to clear out its player id/scene id map since a bug in the updating of Central's map probably is the origin of this routing error.
				GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > const setSceneMsg("SetSceneForPlayer", std::make_pair(getNetworkId(), std::make_pair(std::string(""), false)));
				GameServer::getInstance().sendToCentralServer(setSceneMsg);

				doUnload = true;
			}
		}
		if (doUnload)
		{
			if (getClient())
				GameServer::getInstance().dropClient(getNetworkId());
			unload();
			delete this;
			return;
		}
	}

	//This is the public interface to end baselines.  ::endBaselines is a protected virtual function now
	// so that we can enforce order.
	endBaselines();

	if (needSceneFix)
	{
		WARNING(true, ("Fixing scene id for contained object %s due to incorrect scene id during baselines", getNetworkId().getValueString().c_str()));
		setSceneIdOnThisAndContents(ServerWorld::getSceneId());
	}

	if (fromDatabase)
		onLoadedFromDatabase();

	updateWorldSphere();
	if (getScriptObject() != nullptr)
	{
		endBaselinesInitializeScript(*this, true);
		// If the object was created authoritative, trigger if needed
		if (isAuthoritative())
			triggerMadeAuthoritative();
	}

	if (isPlayerControlled())
		PlayerSanityChecker::add(getNetworkId());

	//-- Validate the object transform.
	checkAndLogInvalidTransform();

	//-- Schedule for alter just in case something needs an alter.
	scheduleForAlter();
}

//-----------------------------------------------------------------------

void ServerObject::onLoadedFromDatabase()
{
}

//-----------------------------------------------------------------------

void ServerObject::endBaselines()
{
	// This should always go first: it ensures there are no deltas
	// accumulated for this object at the time we start processing
	// our baselined data. Only changes after this point will
	// generate deltas.
	clearDeltas();

	//Handle updating container info
	bool addToWorldFlag = handleContentsSetup();
	setInitialized(true);

	ServerController * c = safe_cast<ServerController*>(getController());
	c->endBaselines();
	setTransformChanged(false);

	// check for parent getting uncached
	if (isAuthoritative() && getCacheVersion() != 0)
	{
		ServerObject *parent = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
		if (parent && parent->getCacheVersion() == 0)
			setCacheVersion(0);
	}

	// handle version numbering for contents on portal properties
	PortalProperty *portalProperty = getPortalProperty();
	if (portalProperty)
	{
		int crcValue=0;
		getObjVars().getItem(portalPropertyCrcObjectVariableName,crcValue);
		std::vector<Object*> unfixables;
		if (portalProperty->serverEndBaselines(crcValue, unfixables, isAuthoritative()))
		{
			FATAL(ConfigServerGame::getFatalOnGoldPobChange() && getCacheVersion() != 0, 
				("Object %s is a client cached object and the POB changed from [%d] to [%d]", getNetworkId().getValueString().c_str(), crcValue, portalProperty->getCrc()));

			DEBUG_WARNING(getCacheVersion() != 0, ("Object %s is a client cached object and the POB changed from [%d] to [%d]", getNetworkId().getValueString().c_str(), crcValue, portalProperty->getCrc()));

			if (isAuthoritative())
				setCacheVersion(0);
		}
		if (isAuthoritative())
			setObjVarItem(portalPropertyCrcObjectVariableName, portalProperty->getCrc());

		if (!unfixables.empty())
		{
			WARNING(true, ("Object %s contained objects that could not be placed after pob fixup", getNetworkId().getValueString().c_str()));
		}
	}

	if (addToWorldFlag)
		addToWorld();

	if (isAuthoritative())
	{
		// don't allow login if the character is in the middle of CTS or has been successfully CTS(ed)
		Client * const client = GameServer::getInstance().getClient(getNetworkId());
		if (client && !getClient() && (client->getCharacterObjectId() == getNetworkId()))
		{
			bool kickRequested = false;

			DynamicVariableList const & objVars = getObjVars();
			int timeOut;
			if (objVars.getItem("disableLoginCtsInProgress", timeOut))
			{
				if (timeOut > static_cast<int>(ServerClock::getInstance().getGameTimeSeconds()))
				{
					MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++kickPlayerCtsCompletedOrInProgress", "", 0, false);
					kickRequested = true;
				}
				else
				{
					removeObjVarItem("disableLoginCtsInProgress");
				}
			}

			int transferredTime;
			if (!kickRequested && objVars.getItem("disableLoginCtsCompleted", transferredTime) && !client->isUsingAdminLogin())
			{
				MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++kickPlayerCtsCompletedOrInProgress", "", 0, false);
				kickRequested = true;
			}
		}

		// attach a pending client to this object if needed
		checkAndAddClientControl(client);

		// Make sure the PlanetServer knows about the new object if it needs to
		updatePositionOnPlanetServer(true);

		// for initialized, unpacked vendor, sync the vendor location with the commodities server
		if (isVendor())
			CommoditiesMarket::updateVendorLocation(*this);

		// if this is a reward item that supports trade in, attach the trade in script to it to provide the trade in functionality
		VeteranRewardManager::attachTradeInScriptToItem(*this);

		// sanity check - fix up any player character negative cash/bank balance
		if (client)
		{
			if (m_cashBalance.get() < 0)
				m_cashBalance = 0;

			if (m_bankBalance.get() < 0)
				m_bankBalance = 0;
		}
	}

	// If our immediate container is initialized, we were just added to the container so update the ObserveTracker
	ServerObject *immediateContainer = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
	if (immediateContainer && immediateContainer->isInitialized())
	{
		// handle the case of a player who logged out in a buildout cell
		// and that buildout id got changed to some other object, like a rock
		if (isPlayerControlled() && isAuthoritative() && (immediateContainer->getNetworkId().getValue() < static_cast<NetworkId::NetworkIdType>(0)))
		{
			if (immediateContainer->asCellObject() != nullptr || ContainerInterface::getCell(*immediateContainer) != nullptr)
				ObserveTracker::onObjectContainerChanged(*this);
			else
			{
				WARNING(true, ("ServerObject::endBaselines player (%s) is trying to be put into non-cell object (%s). We will put the player in the world instead.",
					PlayerObject::getAccountDescription(this).c_str(), immediateContainer->getDebugInformation().c_str()));
				ContainedByProperty * const cby = ContainerInterface::getContainedByProperty(*this);
				if (cby)
				{
					cby->setContainedBy(NetworkId::cms_invalid, true);
					if (!ServerWorld::isSpaceScene())
					{
						// if the player was in a building, try and put them at the building's ejection point
						DynamicVariableLocationData ejectionPoint;
						if (getObjVars().getItem("building_ejection_point", ejectionPoint) && (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid()))
						{
							removeObjVarItem("building_ejection_point");
							setPosition_p(ejectionPoint.pos);
						}
					}
					addToWorld();
				}
			}
		}
		else
		{
			//@todo: we may want to do something here if this call fails
			ObserveTracker::onObjectContainerChanged(*this);
		}
	}

	if (isAuthoritative() && (!isPlayerControlled() || getClient()) && getObjVars().hasItem("teleportFixup"))
		TeleportFixupHandler::add(*this);

	setInEndBaselines(false);
}

// ----------------------------------------------------------------------

void ServerObject::getAuthClients(std::set<Client const *> &authClients) const
{
	Client const * const client = getClient();
	if (client)
		IGNORE_RETURN(authClients.insert(client));
}

// ----------------------------------------------------------------------

bool ServerObject::handlePlayerInInteriorSetup(ContainedByProperty *containedBy)
{
	// If we are player-controlled, assume that it may have been a while since we logged in.
	// If we were in a POB, it might have changed.  So, ignore the persisted cell number and look
	// for the best cell.  If anything goes wrong looking for the cell, warp to a good location.

	bool retval = false;

	Object * const originalCell = containedBy->getContainedBy();
	if (originalCell)
	{
		bool fixOk = false;

		CellProperty * const originalCellProp = originalCell->getCellProperty();
		ServerObject * const pob = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*originalCell));
		BuildingObject * const building = pob ? pob->asBuildingObject() : 0;
		PortalProperty * const portal = pob ? pob->getPortalProperty() : 0;

		// Don't ever put the character into a packed house
		if ((building) && (!building->isInWorld()))
		{
			DEBUG_REPORT_LOG(true,("Ejecting player %s from building %s, because the building is packed\n",getNetworkId().getValueString().c_str(), building->getNetworkId().getValueString().c_str()));
			LOG("CustomerService",("Login:  Ejecting player %s from building %s, because the building is packed",getNetworkId().getValueString().c_str(), building->getNetworkId().getValueString().c_str()));

			// settting fixOk to false will cause the character to get moved
			// into the world at the building ejection point (see below)
			fixOk = false;
		}
		else if (portal)
		{
			originalCellProp->depersistContents(*this);

			if (pob->isInitialized())
			{
				if (isAuthoritative())
				{
					Transform const saveTransform(getTransform_o2p());
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					originalCellProp->remove(*this, tmp);
					containedBy->setContainedBy(NetworkId::cms_invalid);

					fixOk = portal->fixupObject(*this, saveTransform);
					containerHandleUpdateProxies(nullptr, safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*this)));

					if (building)
						building->gainedPlayer(*this);
				}
				else
				{
					fixOk = true;
					DEBUG_REPORT_LOG(true,("Skipping POB fixup on non-authoritative player %s\n",getNetworkId().getValueString().c_str()));
					retval = true;
				}
			}
			else
			{
				if (isAuthoritative())
				{
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					portal->queueObjectForFixup(*this);
					originalCellProp->remove(*this, tmp);
					containedBy->setContainedBy(NetworkId::cms_invalid);
					setNeedsPobFixup(true);
					addNotification(pobFixupNotification);
				}
				else
					DEBUG_REPORT_LOG(true,("Skipping POB fixup on non-authoritative player %s\n",getNetworkId().getValueString().c_str()));
				fixOk = true;
			}
		}

		if (!fixOk)
		{
			// warp to safe location
			DEBUG_WARNING(true,("Player-controlled object %s was warped outside of its container because a good cell could not be found.",getNetworkId().getValueString().c_str()));

			containedBy->setContainedBy(NetworkId::cms_invalid);

			// attempt to put the character at the ejection point of the building
			if (!ServerWorld::isSpaceScene())
			{
				DynamicVariableLocationData ejectionPoint;
				if (getObjVars().getItem("building_ejection_point", ejectionPoint) && (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid()))
				{
					removeObjVarItem("building_ejection_point");
					setPosition_p(ejectionPoint.pos);
				}
			}

			retval = true;
		}
	}
	else
	{
		// the container (most likely a building) we were in doesn't exist any more
		DEBUG_REPORT_LOG(true,("Ejecting player %s from building %s, because the building does not exist anymore\n",getNetworkId().getValueString().c_str(), containedBy->getContainedByNetworkId().getValueString().c_str()));
		LOG("CustomerService",("Login:  Ejecting player %s from building %s, because the building does not exist anymore",getNetworkId().getValueString().c_str(), containedBy->getContainedByNetworkId().getValueString().c_str()));

		containedBy->setContainedBy(NetworkId::cms_invalid);

		// attempt to put the character at the ejection point of the building
		if (!ServerWorld::isSpaceScene())
		{
			DynamicVariableLocationData ejectionPoint;
			if (getObjVars().getItem("building_ejection_point", ejectionPoint) && (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid()))
			{
				removeObjVarItem("building_ejection_point");
				setPosition_p(ejectionPoint.pos);
			}
			else
			// warp the player to the Mos Eisley Starport
			{
				DEBUG_REPORT_LOG(true, ("Tried to load creature %s into invalid container. Moving to Mos Eisley Starport.\n", getNetworkId().getValueString().c_str()));
				LOG("CustomerService",("Login:  Tried to load creature %s into invalid container. Moving to Mos Eisley Starport.\n", getNetworkId().getValueString().c_str()));
				GameServer::getInstance().requestSceneWarp(CachedNetworkId(*this),"tatooine",Vector(3528,5,-4804),NetworkId::cms_invalid,Vector(3528,5,-4804));
			}
		}

		retval = true;
	}
	return retval;
}

//-----------------------------------------------------------------------
/**
 * Handle setting up the contents of any containedby objects while de-persisting this object
 */

bool ServerObject::handleContentsSetup()
{
	ContainedByProperty * const containedBy = ContainerInterface::getContainedByProperty(*this);
	if (!containedBy || containedBy->getContainedByNetworkId() == NetworkId::cms_invalid)
		return true;

	ServerObject * const containingObject = safe_cast<ServerObject *>(containedBy->getContainedBy());
	DEBUG_REPORT_LOG(!containingObject, ("Could not depersist object %s into non-existant object %s.\n", getNetworkId().getValueString().c_str(), containedBy->getContainedByNetworkId().getValueString().c_str()));

	// handle special case of player controlled objects depersisting into interiors
	if (   isPlayerControlled()
	    && (   (containingObject && containingObject->getCellProperty())
	        || (containedBy->getContainedByNetworkId() != NetworkId::cms_invalid && !containingObject)))
		return handlePlayerInInteriorSetup(containedBy);

	if (containingObject)
	{
		Container * const container = ContainerInterface::getContainer(*containingObject);
		SlottedContainer * const slotContainer = dynamic_cast<SlottedContainer *>(container);
		if (slotContainer)
		{
			SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(*this);
			if (slottedProperty)
				slotContainer->depersistSlotContents(*this, slottedProperty->getCurrentArrangement());
			else
				WARNING_STRICT_FATAL(true, ("Tried to depersist an object with no arrangement into a slotted container"));
		}
		else if (container)
		{
			container->depersistContents(*this);
			if (containingObject->isInitialized() && containingObject->isInWorld() && container->isContentItemExposedWith(*this))
				addToWorld();
		}

		synchronizationOnLoadedInContainer();
		return false;
	}

	LOG("CustomerService", ("BadContainerDepersist: Tried to depersist non-player object %s into container %s which could not be found.", getDebugInformation().c_str(), containedBy->getContainedByNetworkId().getValueString().c_str()));
	unload();
	return false;
}

//-----------------------------------------------------------------------

unsigned long ServerObject::getAndIncrementMoveSequenceNumber()
{
	unsigned long newValue = m_transformSequence.get();
	newValue++;
	m_transformSequence = newValue;
	return newValue;
}

//-----------------------------------------------------------------------

const bool ServerObject::getPositionChanged() const
{
	return (getTransform_o2w().getPosition_p() != m_oldPosition);
}

//-----------------------------------------------------------------------

const int ServerObject::getCacheVersion() const
{
	return m_cacheVersion.get();
}

//-----------------------------------------------------------------------

const char * ServerObject::getSharedTemplateName() const
{
	if (getSharedTemplate() == nullptr)
		return nullptr;
	return getSharedTemplate()->ObjectTemplate::getName();
}

//-----------------------------------------------------------------------

const char * ServerObject::getClientSharedTemplateName () const
{
	return getSharedTemplateName();
}

//-----------------------------------------------------------------------

const Unicode::String ServerObject::getObjectName(void) const
{
	// try the player defined name first
	const Unicode::String & name = m_objectName.get();
	if (!name.empty ())
		return name;

	// try the stringId name
	const StringId & nameId = m_nameStringId.get();
	if (!nameId.isInvalid ())
		return Unicode::narrowToWide(nameId.getDebugString ());

	// use the template name
	return Unicode::narrowToWide(getTemplateName());
}

//-----------------------------------------------------------------------

const Sphere & ServerObject::getSphereExtent() const
{
	return m_worldSphere;
}

//-----------------------------------------------------------------------

const Sphere ServerObject::getLocalSphere() const
{
	Sphere sphere (Sphere::unit);
	const CollisionProperty * collision = getCollisionProperty();
	if (collision)
	{
		sphere = collision->getBoundingSphere_l();
	}
	else
	{
		const Appearance * appearance = getAppearance();
		if (appearance)
		{
			sphere = appearance->getSphere();
		}
	}

	if (sphere.getRadius() == 0.0f)
	{
		sphere.setRadius(1.0f);
	}
	return sphere;
}

//-----------------------------------------------------------------------

void ServerObject::updateWorldSphere()
{
	Sphere sphere (getLocalSphere());
	m_worldSphere.setCenter (rotateTranslate_o2w(sphere.getCenter()));
	m_worldSphere.setRadius(sphere.getRadius());
}

//-----------------------------------------------------------------------

/**
 * Gets the approximate height of the object.
 *
 * @return the object's height
 */
float ServerObject::getHeight() const
{
	return getSphereExtent().getRadius() * 2.0f * getScale().y;
}	// ServerObject::getHeight

//-----------------------------------------------------------------------

/**
 * Gets the approximate radius of the object.
 *
 * @return the object's radius
 */
float ServerObject::getRadius() const
{
	return getSphereExtent().getRadius() * std::max(getScale().x, getScale().z);
}	// ServerObject::getHeight

//-----------------------------------------------------------------------

/**
 * Returns the volume of this object (and it's contents, if it's a container)
 *
 * @return the object's volume
 */
int ServerObject::getVolume(void) const
{
	const VolumeContainmentProperty* volumeProperty = ContainerInterface::getVolumeContainmentProperty(*this);
	return volumeProperty ? volumeProperty->getVolume() : 1;
}

//-----------------------------------------------------------------------


const TriggerVolume * ServerObject::getTriggerVolume(const std::string & triggerVolumeName) const
{
	const TriggerVolume * result = 0;
	const TriggerVolumeMap::const_iterator i = m_triggerVolumes.find(triggerVolumeName);
	if(i != m_triggerVolumes.end())
	{
		result = (*i).second;
	}
	return result;
}

//-----------------------------------------------------------------------

TriggerVolume * ServerObject::getTriggerVolume(const std::string & triggerVolumeName)
{
	TriggerVolume * result = 0;
	const TriggerVolumeMap::iterator i = m_triggerVolumes.find(triggerVolumeName);
	if (i != m_triggerVolumes.end())
	{
		result = (*i).second;
	}
	return result;
}

//----------------------------------------------------------------------

void ServerObject::seeCombatSpam (const MessageQueueCombatSpam & spamMsg)
{
	if (!isPlayerControlled())
		return;

	if (!isAuthoritative ())
	{
		sendControllerMessageToAuthServer(CM_combatSpam, new MessageQueueCombatSpam(spamMsg));
		return;
	}

	if (getClient ())
	{
		bool shouldSend = (spamMsg.m_attacker == getNetworkId());
		if (!shouldSend)
		{
			ServerObject * sourceObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(spamMsg.m_attacker));
			if (   sourceObject
			    && ObserveTracker::isObserving(*getClient(), *sourceObject)
			    && !PlayerObject::isIgnoring(getNetworkId(), spamMsg.m_attacker))
			{
				shouldSend = true;
			}
		}

		if (shouldSend)
		{
			MessageQueueCombatSpam * const msg = new MessageQueueCombatSpam (spamMsg);
			Controller *const controller = NON_NULL (getController());
			controller->appendMessage(CM_combatSpam, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

//----------------------------------------------------------------------

void ServerObject::seeSocial (const MessageQueueSocial & socialMsg)
{
	Controller * const controller = NON_NULL (getController());

	if (!isAuthoritative ())
	{
		MessageQueueSocial * const msg = new MessageQueueSocial (socialMsg);
		controller->appendMessage(CM_socialReceive, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		return;
	}

	if (getClient ())
	{
		ServerObject *sourceObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(socialMsg.getSourceId()));
		if (   sourceObject
		    && (   sourceObject->getCacheVersion() > 0
		        || (   ObserveTracker::isObserving(*getClient(), *sourceObject)
		            && !PlayerObject::isIgnoring(getNetworkId(), socialMsg.getSourceId()))))
		{
			MessageQueueSocial * const msg = new MessageQueueSocial (socialMsg);
			controller->appendMessage(CM_socialReceive, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

//-----------------------------------------------------------------------

bool ServerObject::serverObjectInitializeFirstTimeObject(ServerObject *cell, Transform const &transform)
{
	//This is the public interface for initializeFirstTimeObject.  ::initializeFirstTimeObject has been made
	//protected virtual so that we could enforce order.

	initializeFirstTimeObject();

	// attach template scripts to this object
	NOT_NULL(getScriptObject());
	getScriptObject()->initScriptInstances();

	ServerObjectTemplate const * const newTemplate = safe_cast<ServerObjectTemplate const *>(getObjectTemplate());
	NOT_NULL(newTemplate);

	int count = newTemplate->getScriptsCount();
	for (int i = 0; i < count; ++i)
	{
		getScriptObject()->attachScript(newTemplate->getScripts(i), true);
	}
	if (markedNoTradeRemovable())
	{
		getScriptObject()->attachScript("item.special.no_trade_removable", true);
	}

	if (cell)
	{
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		if (!ContainerInterface::transferItemToCell(*cell, *this, transform, nullptr, tmp))
		{
			DEBUG_WARNING(true, ("ServerWorld::createNewObjectIntermediate tried to create a new object in a cell, but it failed."));
			return false;
		}
	}
	else if (transform != Transform::identity)
		setTransform_o2p(transform);

	getScriptObject()->setOwnerIsInitialized();

	addObjectToConcludeList();

	// validate the object transform
	checkAndLogInvalidTransform();

	//-- Initialize the controller
	{
		ServerController * const controller = safe_cast<ServerController *>(getController());
		controller->endBaselines();
	}

	return true;
}

//-----------------------------------------------------------------------

void ServerObject::initializeFirstTimeObject()
{
	DEBUG_FATAL(!isAuthoritative(), ("Calling initialize on non authoritative object %s", getNetworkId().getValueString().c_str()));
	setInitialized(true);
	setPlacing(true);

	const ServerObjectTemplate* newTemplate = safe_cast<const ServerObjectTemplate*>(getObjectTemplate());
	NOT_NULL(newTemplate);

	// set up objvars - we know we are authoritative so we can get by with this
	newTemplate->getObjvars(m_objVars);

	// record the crc of the pob used to create this object
	PortalProperty *portalProperty = getPortalProperty();
	if (portalProperty)
		m_objVars.setItem(portalPropertyCrcObjectVariableName, portalProperty->getCrc());

	// add template equipment to this object
	int i;
	int count = newTemplate->getContentsCount();
	for (i = 0; i < count; ++i)
	{
		//Don't move this declaration of contents out of the loop or you will leak a ref.
		ServerObjectTemplate::Contents contents;
		newTemplate->getContents(contents, i);
		if (contents.content == nullptr)
		{
			DEBUG_WARNING(true, ("No template for contents item %d", i));
			continue;
		}
		if (contents.equipObject)
		{
			// find out where the contents go
			const SlotId & slotId = SlotIdManager::findSlotId(CrcLowerString(contents.slotName.c_str()));
			if (slotId != SlotId::invalid)
			{
				//We'll persist the contents automatically if the parent object is persisted.
				ServerObject * newObject = ServerWorld::createNewObject(*(safe_cast<const ServerObjectTemplate*>(contents.content)), *this,slotId, false);
				if (newObject == nullptr)
				{
					DEBUG_WARNING(true, ("Can't create object from template %s",contents.content->getName()));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("Slot %s is not a valid slot name", contents.slotName.c_str()));
			}
		}
		else
		{
			// get the object (which we assume is a volume container) in the
			// desired slot
			SlottedContainer * const container = ContainerInterface::getSlottedContainer(*this);
			if (container != nullptr)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				CachedNetworkId equippedObjectId = container->getObjectInSlot(SlotIdManager::findSlotId(CrcLowerString(contents.slotName.c_str())), tmp);
				if (equippedObjectId != NetworkId::cms_invalid)
				{
					// put the contents in the volume container
					ServerObject * equippedObject = safe_cast<ServerObject *>(equippedObjectId.getObject());
					if (equippedObject != nullptr)
					{

						//We'll persist the contents automatically if the parent object is persisted.
						ServerObject * newObject = ServerWorld::createNewObject(*(safe_cast<const ServerObjectTemplate*>(contents.content)), *equippedObject,false);
						if (newObject == nullptr)
						{
							DEBUG_WARNING(true, ("Can't create object from template %s", contents.content->getName()));
						}
					}
					else
					{
						DEBUG_WARNING(true, ("Object in slot %s not loaded",contents.slotName.c_str()));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("No object in slot %s", contents.slotName.c_str()));
				}
			}
			else
				DEBUG_WARNING(true, ("No slotted container for object"));
		}
	}
	{
		const PropertyList::iterator iEnd = m_propertyList->end();
		for (PropertyList::iterator i = m_propertyList->begin(); i != iEnd; ++i)
			(*i)->initializeFirstTimeObject();
	}

	SharedObjectTemplate::GameObjectType got  =
		static_cast<SharedObjectTemplate::GameObjectType> (getGameObjectType());
	if (got == SharedObjectTemplate::GOT_terminal_bazaar)
	{
		setLoadContents(false);
	}

	//-- Assume this object wants to be altered.
	scheduleForAlter();
}

//-----------------------------------------------------------------------

void ServerObject::kill()
{
	Object::kill();
}

// ----------------------------------------------------------------------

/**
 * Mark all the children of this object as persisted.
 */
void ServerObject::markChildrenPersisted()
{
	if (!safe_cast<const ServerObjectTemplate *>(getObjectTemplate())->getPersistContents())
		return;

	Container *container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
		{
			ServerObject *containedObject = safe_cast<ServerObject *>((*containerIterator).getObject());
			if (containedObject)
			{
				containedObject->m_persisted=true;
				containedObject->markChildrenPersisted();
			}
		}
	}

}

//-----------------------------------------------------------------------

void ServerObject::addToWorld()
{
	if (getKill())
		return;

	if (isInWorld())
	{
		DEBUG_WARNING(true, ("Calling add to world on object already in it %s", getNetworkId().getValueString().c_str()));
		return;
	}
	updateWorldSphere();

	Object::addToWorld();
	updateTriggerVolumes();

	if (getScriptObject() != nullptr)
		getScriptObject()->setOwnerIsLoaded();

	onAddedToWorld();
}

//-----------------------------------------------------------------------

void ServerObject::onAddedToWorld()
{
	m_oldPosition = getTransform_o2w().getPosition_p();

	// sanity check whether we should really be in the world or not
	ContainedByProperty const * const containedByProperty = getContainedByProperty();
	if (containedByProperty && containedByProperty->getContainedByNetworkId().isValid())
	{
		Object const * const containedByObject = containedByProperty->getContainedBy();
		if (containedByObject)
		{
			WARNING_DEBUG_FATAL(
				!containedByObject->isInWorld(),
				(
					"Tried to add object %s to the world with container %s which is not in the world.",
					getDebugInformation().c_str(),
					containedByObject->getDebugInformation().c_str()));
			Container const * const containerProperty = ContainerInterface::getContainer(*containedByObject);
			if (containerProperty)
			{
				WARNING_DEBUG_FATAL(
					!containerProperty->isContentItemExposedWith(*this),
					(
						"Tried to add object %s to the world with container %s which does not expose it.",
						getDebugInformation().c_str(),
						containedByObject->getDebugInformation().c_str()));
			}
			else
			{
				WARNING_DEBUG_FATAL(
					true,
					(
						"Tried to add object %s to the world with container %s with no container property.",
						getDebugInformation().c_str(),
						containedByObject->getDebugInformation().c_str()));
			}
		}
		else
		{
			WARNING_DEBUG_FATAL(
				true,
				(
					"Tried to add object %s to the world with unresolvable container %s",
					getDebugInformation().c_str(),
					containedByProperty->getContainedByNetworkId().getValueString().c_str()));
		}
	}

	// Add networkUpdate volume
	if (isInWorldCell())
		onAddedToWorldCell();

	if (isAuthoritative())
	{
		//Tell scripts we are added to world
		ScriptParams params;
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_ADDED_TO_WORLD, params));

		//remove the vendor reinitialization flag if it exists
		removeObjVarItem(CommoditiesMarket::OBJVAR_VENDOR_REINITIALIZING);
	}
}

//-----------------------------------------------------------------------

bool ServerObject::checkAndAddClientControl(Client* client)
{
	if (isPlayerControlled())
	{
		CreatureObject* creature = asCreatureObject();
		if (client && creature && isInitialized() && isAuthoritative() && !isNeedingPobFixup())
		{
			client->onLoadPlayer(*creature);
			return true;
		}
		}
	return false;
	}

//-----------------------------------------------------------------------

void ServerObject::onPobFixupComplete()
{
	setNeedsPobFixup(false);
	checkAndAddClientControl(GameServer::getInstance().getClient(getNetworkId()));
	BuildingObject * const building = static_cast<ServerObject*>(ContainerInterface::getTopmostContainer(*this))->asBuildingObject();
	if (building)
		building->gainedPlayer(*this);
	
}

//-----------------------------------------------------------------------

void ServerObject::onClientReady(Client *c)
{
	NOT_NULL(c);
	CmdSceneReady const readyMessage;
	c->send(readyMessage, true);

	ScriptParams params;
	IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_LOGIN, params));

	int galacticReserve = 0;
	if (!getObjVars().getItem("galactic_reserve", galacticReserve))
	{
		galacticReserve = 0;
	}

	LOG("CustomerService", ("Money:%s logged in with %d money, %d galactic reserve", PlayerObject::getAccountDescription(getNetworkId()).c_str(), getTotalMoney(), galacticReserve));

	if (getClient() && getClient()->isFreeTrialAccount())
	{
		bool addToConcludeList = false;

		if ((m_cashBalance.get() + m_bankBalance.get()) > ConfigServerGame::getMaxFreeTrialMoney())
		{
			int amountOver = (m_cashBalance.get() + m_bankBalance.get()) - ConfigServerGame::getMaxFreeTrialMoney();
			if (amountOver <= m_bankBalance.get())
			{
				m_bankBalance = m_bankBalance.get() - amountOver;
				LOG("CustomerService", ("Money:%s is a free trial user and logged in with too much.  Deducting %d from bank.", PlayerObject::getAccountDescription(getNetworkId()).c_str(), amountOver));
			}
			else
			{
				m_cashBalance = m_cashBalance.get() - (amountOver - m_bankBalance.get());
				LOG("CustomerService", ("Money:%s is a free trial user and logged in with too much.  Deducting %d from bank and %d from cash.", PlayerObject::getAccountDescription(getNetworkId()).c_str(), m_bankBalance.get(), amountOver - m_bankBalance.get()));
				m_bankBalance = 0;			
			}

			// sanity check - fix up any negative cash/bank balance
			if (m_cashBalance.get() < 0)
				m_cashBalance = 0;

			if (m_bankBalance.get() < 0)
				m_bankBalance = 0;

			addToConcludeList = true;
		}

		// trial accounts cannot have galactic reserve
		if (galacticReserve > 0)
		{
			LOG("CustomerService", ("Money:%s is a free trial user and logged in with %d galactic reserve.  Setting galactic reserve to 0.", PlayerObject::getAccountDescription(getNetworkId()).c_str(), galacticReserve));
			removeObjVarItem("galactic_reserve");
			PlayerObject * const po = PlayerCreatureController::getPlayerObject(asCreatureObject());
			if (po)
			{
				po->depersistGalacticReserveDeposit();
			}

			addToConcludeList = true;
		}

		if (addToConcludeList)
			addObjectToConcludeList();
	}
}

// ----------------------------------------------------------------------

void ServerObject::onClientAboutToLoad()
{
}

// ----------------------------------------------------------------------

void ServerObject::onLoadingScreenComplete()
{
}

// ----------------------------------------------------------------------

bool ServerObject::onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	if (transferer && !checkBankTransfer(this, item, transferer))
		return false;

	Object *parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		return parent->asServerObject()->onContainerChildAboutToLoseItem(destination, item, transferer);

	return true;
}

//-----------------------------------------------------------------------

bool ServerObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{
	UNREF(destination);
	UNREF(transferer);
	return true;
}

//-----------------------------------------------------------------------

int ServerObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	if (transferer && !checkBankTransfer(this, item, transferer))
		return Container::CEC_BlockedByDestinationContainer;

	Object *parent = ContainerInterface::getContainedByObject(*this);
	//disallow putting crafting stations in other crafting stations
	if (transferer && parent && parent->asServerObject())
	{
		SharedObjectTemplate::GameObjectType parentGot = static_cast<SharedObjectTemplate::GameObjectType> (parent->asServerObject()->getGameObjectType());
		SharedObjectTemplate::GameObjectType itemGot = static_cast<SharedObjectTemplate::GameObjectType> (item.getGameObjectType());

		if (parentGot == SharedObjectTemplate::GOT_misc_crafting_station && itemGot == parentGot)
		{
			return Container::CEC_BlockedByDestinationContainer;
		}
	}

	if (parent && parent->asServerObject())
		return parent->asServerObject()->onContainerChildAboutToGainItem(item, this, transferer) ? Container::CEC_Success : Container::CEC_BlockedByDestinationContainer;

	return Container::CEC_Success;
}

//-----------------------------------------------------------------------

void ServerObject::onContainerLostItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	Object *parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		parent->asServerObject()->onContainerChildLostItem(destination, item, this, transferer);
}

//-----------------------------------------------------------------------

void ServerObject::onContainerTransfer(ServerObject * destination, ServerObject* transferer)
{
	UNREF(destination);
	UNREF(transferer);
}

//-----------------------------------------------------------------------

void  ServerObject::onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	Object *parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		parent->asServerObject()->onContainerChildGainItem(item, source, transferer);
	combineResourceContainers(item);
}

//-----------------------------------------------------------------------

void ServerObject::onContainerTransferComplete(ServerObject *oldContainer, ServerObject *newContainer)
{
	if (!isAuthoritative())
	{
		WARNING_STRICT_FATAL(true, ("onContainerTransferComplete called on non auth object %s", getNetworkId().getValueString().c_str()));
		return;
	}

	if (newContainer)
	{
		//If this object is not persisted, but it placed into a container that is
		// Then persist the object
		if (!isPersisted() && newContainer->isPersisted() && safe_cast<ServerObjectTemplate const *>(newContainer->getObjectTemplate())->getPersistContents())
			persist();
	}

	DEBUG_FATAL(getCellProperty(), ("Cell object container transfer complete"));

	setTransformChanged(true);

	if (   (oldContainer && !oldContainer->asCellObject())
	    || (newContainer && !newContainer->asCellObject()))
		PositionUpdateTracker::flushPositionUpdate(*this);

	containerHandleUpdateProxies(oldContainer, newContainer);

	Container const * const newContainerProperty = newContainer ? ContainerInterface::getContainer(*newContainer) : 0;
	bool const wasInWorld = isInWorld();
	bool const nowInWorld = !newContainer || (newContainerProperty && newContainer->isInWorld() && newContainerProperty->isContentItemExposedWith(*this));

	containerHandleUpdateClients(oldContainer, newContainer, (wasInWorld && !nowInWorld), (!wasInWorld && nowInWorld));

	ObserveTracker::onObjectArrangementChanged(*this);
	scheduleForAlter();

	updatePositionOnPlanetServer(true);
}

//-----------------------------------------------------------------------

void ServerObject::containerDepersistContents(ServerObject * oldParent, ServerObject * newParent)
{
	Container * const oldContainer = oldParent ? ContainerInterface::getContainer(*oldParent) : nullptr;
	Container * const newContainer = newParent ? ContainerInterface::getContainer(*newParent) : nullptr;

	if (oldContainer)
		oldContainer->internalItemRemoved(*this);
	if (newContainer)
	{
		SlottedContainer * const slotContainer = dynamic_cast<SlottedContainer*>(newContainer);
		if (slotContainer)
		{
			SlottedContainmentProperty* const slotted = ContainerInterface::getSlottedContainmentProperty(*this);
			DEBUG_FATAL(!slotted, ("Error unpacking non slotted objects into a slotted container"));
			slotContainer->depersistSlotContents(*this, -1);
			slotContainer->updateArrangement(*this, -1, slotted->getCurrentArrangement());
		}
		else
		{
			newContainer->depersistContents(*this);
		}
	}
}

//-----------------------------------------------------------------------

void ServerObject::containerHandleUpdateClients(ServerObject* oldContainer, ServerObject* newContainer, bool remFromWorld, bool addWorld)
{
	if (remFromWorld)
		removeFromWorld();

	// If we're moving in or out of the world cell, update as appropriate (adds or removes farUpdateVolume)
	if (oldContainer && !newContainer && !addWorld)
		onAddedToWorldCell();
	else if (newContainer && !oldContainer && !remFromWorld)
		onRemovedFromWorldCell();

	if (addWorld)
		addToWorld();

	ObserveTracker::onObjectContainerChanged(*this);
	synchronizationOnContainerChange(oldContainer, newContainer);
}

//-----------------------------------------------------------------------

void ServerObject::createFarNetworkUpdateVolume(float const overrideVal)
{
	if (isBeingDestroyed() || isUnloading())
	{
		DEBUG_REPORT_LOG(true, ("Not creating volume for object that is being destroyed\n"));
		return;
	}
	if (m_networkUpdateFar)
	{
		WARNING_STRICT_FATAL(true, ("Object %s already has a far update volume", getNetworkId().getValueString().c_str()));
		return;
	}

	// There's no reason for a contained item to have a far network update volume
	// if this object is contained by something else.
	Object const *const containedByObject = ContainerInterface::getContainedByObject(*this);
	if (containedByObject)
	{
		DEBUG_REPORT_LOG(true, ("SO::createFarNetworkUpdateVolume(): server id=[%lu],object id=[%s],container id=[%s]: skipping creation of far network update volume because item is contained.\n",
								GameServer::getInstance().getProcessId(),
								getNetworkId().getValueString().c_str(),
								containedByObject->getNetworkId().getValueString().c_str()));
		return;
	}

	if (getCacheVersion() == 0)
	{
		const ServerObjectTemplate * myTemplate = safe_cast<const ServerObjectTemplate *>(getObjectTemplate());
		UNREF(myTemplate);
		float const radius_far = overrideVal != 0.0f ? overrideVal : getFarNetworkUpdateRadius();
		if (radius_far > 0)
		{
			if (ServerWorld::isSpaceScene())
			{
				DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("creating far update volume (space version) for %s (template=%s, radius=%g)\n", getNetworkId().getValueString().c_str(), myTemplate->getName(), radius_far));
				// don't make link-dead creatures visible to other people at the top level in space
				if (getSendToClient() && (getClient() || !asCreatureObject()))
					SpaceVisibilityManager::addObject (*this, static_cast<int>(radius_far));
			}
			else
			{
				DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("creating far update volume for %s (template=%s, radius=%g)\n", getNetworkId().getValueString().c_str(), myTemplate->getName(), radius_far));
				TriggerVolume * t = new NetworkTriggerVolume(*this, radius_far);
				addTriggerVolume(t);
				m_networkUpdateFar = t;
			}
		}
	}
}

//-----------------------------------------------------------------------

void ServerObject::destroyFarNetworkUpdateVolume()
{
	if (!m_networkUpdateFar)
		return;

	DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("Removing far update volume for %s\n", getNetworkId().getValueString().c_str()));
	ServerWorld::removeObjectTriggerVolume(m_networkUpdateFar);
	TriggerVolumeMap::iterator i = m_triggerVolumes.find(NetworkTriggerVolumeNamespace::NetworkTriggerVolumeName);
	if (i == m_triggerVolumes.end() || i->second != m_networkUpdateFar)
	{
		WARNING_STRICT_FATAL(true, ("Called destroyFarNetworkUpdateVolume but could not find it in m_triggerVolumes"));
	}
	else
		m_triggerVolumes.erase(i);

	delete m_networkUpdateFar;
	m_networkUpdateFar = 0;
}

//-----------------------------------------------------------------------

bool ServerObject::isNetworkUpdateFarTriggerVolume(TriggerVolume const &triggerVolume) const
{
	return triggerVolume.isNetworkTriggerVolume();
}

// ----------------------------------------------------------------------

TriggerVolume *ServerObject::getNetworkTriggerVolume()
{
	return m_networkUpdateFar;
}

// ----------------------------------------------------------------------

float ServerObject::getFarNetworkUpdateRadius() const
{
	if (ServerWorld::getSceneId() == NewbieTutorial::getSceneId())
		return 0.f;

	// Overide update radius for all scenes for all objects gloablly with config entry
	int const overrideRadius = ConfigServerGame::getOverrideUpdateRadius();
	// Overide the update radius for a specific scene if the above global overide is not set
	// If Scene overide is disabled in ConfigServerGame::getEnableSceneGlobalData() it will return zero 
	float radius = overrideRadius > 0 ? static_cast<float>(overrideRadius) : SceneGlobalData::getUpdateRadius(ServerWorld::getSceneId()); 
        // Set the radius to the far update range of from the object template if it hasn't already been overiden
	radius = radius > 0 ? radius : safe_cast<ServerObjectTemplate const *>(getObjectTemplate())->getUpdateRanges(ServerObjectTemplate::UR_far);
	
	if (!ServerWorld::isSpaceScene())
	{
		if (ms_buildingUpdateRadiusMultiplier == 0)
			setBuildingUpdateRadiusMultiplier(ConfigServerGame::getBuildingUpdateRadiusMultiplier());

		// Potentially allow larger objects to come into range sooner if the cfg setting to do so is set
		if (radius >= 127.0f && radius < 512.0f && !asCreatureObject())
			radius *= ms_buildingUpdateRadiusMultiplier;
	}
	return radius;
}

// ----------------------------------------------------------------------

void ServerObject::onTriggerEnter(ServerObject & object, TriggerVolume & triggerVolume)
{
}

//-----------------------------------------------------------------------
// object is leaving this' volume
void ServerObject::onTriggerExit(ServerObject & object, TriggerVolume & triggerVolume)
{
}

// ----------------------------------------------------------------------
// this object got added to a trigger volume
void ServerObject::onAddedToTriggerVolume(TriggerVolume & triggerVolume)
{
	// only CreatureObject should ever be added to a TriggerVolume
	WARNING(true, ("ServerObject (%s) being added to TriggerVolume (%s) owned by (%s)",
		getDebugInformation().c_str(),
		triggerVolume.getName().c_str(),
		triggerVolume.getOwner().getDebugInformation().c_str()));
}

// ----------------------------------------------------------------------
// this object got removed from a trigger volume
void ServerObject::onRemovedFromTriggerVolume(TriggerVolume & triggerVolume)
{
	// NOTE: be careful about referencing the TriggerVolume because this
	// function can be called from the TriggerVolume's dtor

	// only CreatureObject should ever be added to a TriggerVolume
	WARNING(true, ("ServerObject (%s) being removed from TriggerVolume (%s) owned by (%s)",
		getDebugInformation().c_str(),
		triggerVolume.getName().c_str(),
		triggerVolume.getOwner().getDebugInformation().c_str()));
}

// ----------------------------------------------------------------------

/**
 * Send an object to the database process to be persisted, if it isn't already.
 * After this function is called, changes to this object will be sent to the database.
 */

void ServerObject::persist()
{
	// Do not persist buildout objects (buildout objects have negative networkIds)
	if (getNetworkId() < NetworkId::cms_invalid)
		return;

	if (isAuthoritative())
	{
		ContainedByProperty *containedBy = getContainedByProperty();

		bool const contained = containedBy && containedBy->getContainedByNetworkId().isValid();

		if (ServerWorld::isSpaceScene())
		{
			// in space, non-player objects are only allowed to persist at the origin
			if (!contained && !isPlayerControlled() && getPosition_p() != Vector::zero)
			{
				WARNING_STRICT_FATAL(true, ("Tried to persist non-player object %s in a space scene away from the origin (not persisting).", getDebugInformation().c_str()));
				return;
			}
		}
		else
		{
			WARNING_STRICT_FATAL((getPosition_w() == Vector::zero) && !contained && (getSceneId() != "universe"), ("Persisting object %s (%s) at (0,0,0)",getNetworkId().getValueString().c_str(),getTemplateName()));
		}

		if (!m_persisted.get())
		{
#ifdef _DEBUG
			ServerObject *parent = static_cast<ServerObject*>(ContainerInterface::getContainedByObject(*this));
			if (parent && !parent->isPersisted())
			{
				WARNING(true, ("Persisting %s into %s, which is not persisted!", getNetworkId().getValueString().c_str(), parent->getNetworkId().getValueString().c_str()));
			}
#endif
			m_persisted = true;
			markChildrenPersisted();
			sendCreateAndBaselinesToDatabaseServer();

			PositionUpdateTracker::sendPositionUpdate(*this);
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_persistObject, 0);
	}
}

// ----------------------------------------------------------------------

void ServerObject::removeTriggerVolume(const std::string & name)
{
	for (unsigned int i = 0; i < m_triggerVolumeInfo.size(); ++i)
	{
		if (m_triggerVolumeInfo[i].name == name)
		{
			m_triggerVolumeInfo.erase(i);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void ServerObject::sendControllerMessageToAuthServer(enum GameControllerMessage cm, MessageQueue::Data * msg, float value)
{
	DEBUG_WARNING(isInEndBaselines(), ("sendControllerMessageToAuthServer while ending baselines: message id=(%d) from object id=[%s], template=[%s], on server id=[%d]", static_cast<int>(cm), getNetworkId().getValueString().c_str(), getObjectTemplateName(), static_cast<int>(GameServer::getInstance().getProcessId())));

	//DEBUG_WARNING(!isInitialized(), ("sendControllerMessageToAuthServer while uninitialized: message id=(%d) from object id=[%s], template=[%s], on server id=[%d]", static_cast<int>(cm), getNetworkId().getValueString().c_str(), getObjectTemplateName(), static_cast<int>(GameServer::getInstance().getProcessId())));

	Controller * controller = getController();
	if(controller)
	{
		controller->appendMessage(cm, value, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
	else
	{
		DEBUG_WARNING(true, ("No controller for sendControllerMessageToAuthServer"));
		delete msg;
	}
}

//-----------------------------------------------------------------------

bool ServerObject::getSendToClient() const
{
	return getCacheVersion() == 0 && getLocalFlag(LocalObjectFlags::ServerObject_SendToClient);
}

//-----------------------------------------------------------------------

void ServerObject::sendToClientsInUpdateRange(const GameNetworkMessage & message, bool reliable, bool includeSelf) const
{
	std::set<Client *> const &clients = getObservers();
	std::set<Client *>::const_iterator i;
	std::map<ConnectionServerConnection *, std::vector<NetworkId> > &tmpDistributionList = DistributionListStack::alloc();

	Client const * const myClient = getClient();

	float minDistanceSquared = 0.0f;

	if (!reliable)
	{
		if (m_unreliableControllerMessageSequenceNumber == 0)
		{
			// Send reliable

			reliable = true;
			++m_unreliableControllerMessageSequenceNumber;
		}
		else
		{
			// Send un-reliable based on distance

			switch (m_unreliableControllerMessageSequenceNumber)
			{
				// The server is guaranteed to send one reliable transform update per 2 seconds (8:1 ratio for unreliable/reliable)

				case 1: { minDistanceSquared = ConfigServerGame::getUpdateRange7PacketsSquared(); } break; // 7 unreliable per 2 seconds
				case 2: { minDistanceSquared = ConfigServerGame::getUpdateRange3PacketsSquared(); } break; // 3 unreliable per 2 seconds
				case 3: { minDistanceSquared = ConfigServerGame::getUpdateRange5PacketsSquared(); } break; // 5 unreliable per 2 seconds
				case 4: { minDistanceSquared = ConfigServerGame::getUpdateRange1PacketsSquared(); } break; // 1 unreliable per 2 seconds
				case 5: { minDistanceSquared = ConfigServerGame::getUpdateRange4PacketsSquared(); } break; // 4 unreliable per 2 seconds
				case 6: { minDistanceSquared = ConfigServerGame::getUpdateRange2PacketsSquared(); } break; // 2 unreliable per 2 seconds
				case 7: { minDistanceSquared = ConfigServerGame::getUpdateRange6PacketsSquared(); } break; // 6 unreliable per 2 seconds
				default:
					{
						DEBUG_WARNING(true, ("All cases should be handled for the sequence numbers: %d", m_unreliableControllerMessageSequenceNumber));
					}

			}

			if (m_unreliableControllerMessageSequenceNumber >= 7)
			{
				m_unreliableControllerMessageSequenceNumber = 0;
			}
			else
			{
				++m_unreliableControllerMessageSequenceNumber;
			}
		}
	}

	// need to do special handling for CM_combatSpam controller
	// message to determine if the client should get the message
	bool isCombatSpamMessage = false;
	NetworkId combatSpamAttacker, combatSpamDefender;
	Vector combatSpamAttackerPosition_w, combatSpamDefenderPosition_w;
	if (!clients.empty())
	{
		static unsigned long int const controllerMessageCrc = MessageDispatch::MessageBase::makeMessageTypeFromString("ObjControllerMessage");
		if (controllerMessageCrc == message.getType())
		{
			ObjControllerMessage const & ocm = static_cast<ObjControllerMessage const &>(message);
			if (ocm.getMessage() == CM_combatSpam)
			{
				MessageQueueCombatSpam const * mqcs = static_cast<MessageQueueCombatSpam const *>(ocm.getData());

				if (mqcs)
				{
					isCombatSpamMessage = true;
					combatSpamAttacker = mqcs->m_attacker;
					combatSpamAttackerPosition_w = mqcs->m_attackerPosition_w;
					combatSpamDefender = mqcs->m_defender;
					combatSpamDefenderPosition_w = mqcs->m_defenderPosition_w;
				}
			}
		}
	}

	for (i = clients.begin(); i != clients.end(); ++i)
	{
		// build connection list
		Client * const c = (*i);

		if (isCombatSpamMessage && !c->shouldReceiveCombatSpam(combatSpamAttacker, combatSpamAttackerPosition_w, combatSpamDefender, combatSpamDefenderPosition_w))
			continue;

		if (!reliable)
		{
			// if it's time to try to send to this client
			// because it is close enough, add it to the
			// target list, otherwise continue looking
			// through clients.

			ServerObject * const obj = c->getCharacterObject();

			if (obj != nullptr)
			{
				if (   (ConfigServerGame::getSkipUnreliableTransformsForOtherCells() && obj->getAttachedTo() != getAttachedTo())
					|| (obj->getPosition_w().magnitudeBetweenSquared(getPosition_w()) > minDistanceSquared)
					|| (!CellProperty::areAdjacent(getParentCell(), obj->getParentCell())))
				{
					// Throw out this un-reliable packet due to distance

					continue;
				}
			}
		}

		if(includeSelf || ( (!includeSelf) && c != myClient) )
		{
			ConnectionServerConnection * const n = c->getConnection();
			if (n)
				tmpDistributionList[n].push_back(c->getCharacterObjectId());
		}
	}

	for (std::map<ConnectionServerConnection *, std::vector<NetworkId> >::const_iterator connIter = tmpDistributionList.begin(); connIter != tmpDistributionList.end(); ++connIter)
	{
		GameClientMessage const msg((*connIter).second, reliable, message);
		(*connIter).first->send(msg, reliable);
	}

	DistributionListStack::release();
}

//-----------------------------------------------------------------------

void ServerObject::sendToSpecifiedClients(const GameNetworkMessage & message, bool reliable, const std::vector<NetworkId> & clients)
{
	for (std::vector<NetworkId>::const_iterator i = clients.begin(); i != clients.end(); ++i)
	{
		ServerObject * o = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(*i));
		if (o != nullptr && o->getClient() != nullptr)
			o->getClient()->send(message, reliable);
	}
}

//-----------------------------------------------------------------------

void ServerObject::appendMessage(int const message, float const value, uint32 const flags)
{
	Controller * const co = getController();
	if (co)
		co->appendMessage(message, value, flags);
	else
		DEBUG_REPORT_LOG(true, ("ServerObject::appendMessage: %s called without a controller.\n", getDebugInformation().c_str()));
}

//-----------------------------------------------------------------------

void ServerObject::appendMessage(int const message, float const value, MessageQueue::Data * const data, uint32 const flags)
{
	Controller * const co = getController();
	if (co)
		co->appendMessage(message, 0.0f, data, flags);
	else
		DEBUG_REPORT_LOG(true, ("ServerObject::appendMessage: %s called without a controller.\n", getDebugInformation().c_str()));
}

//-----------------------------------------------------------------------

void ServerObject::forwardServerObjectSpecificBaselines() const
{
}

//-----------------------------------------------------------------------

void ServerObject::sendObjectSpecificBaselinesToClient(Client const &) const
{
}

//-----------------------------------------------------------------------

BaselinesMessage * ServerObject::createSharedBaselinesMessage() const
{
	return new BaselinesMessage(getNetworkId(), getObjectType(), m_sharedPackage, BaselinesMessage::BASELINES_SHARED);
}

//-----------------------------------------------------------------------

BaselinesMessage * ServerObject::createSharedNpBaselinesMessage() const
{
	return new BaselinesMessage(getNetworkId(), getObjectType(), m_sharedPackage_np, BaselinesMessage::BASELINES_SHARED_NP);
}

//-----------------------------------------------------------------------

void ServerObject::setAuthority()
{
	// We make sure to send out deltas before setting authority, so that we don't push out deltas at the
	// end of the frame which were intended to be nonauthoritative.
	if (isInitialized())
		sendDeltasForSelfAndContents();

	DEBUG_REPORT_LOG(ConfigServerGame::getLogObservers(), ("Called setAuthority on %s\n", getNetworkId().getValueString().c_str()));

	Container * const container = ContainerInterface::getContainer(*this);
	if (container)
	{
		std::vector<CachedNetworkId> idList;
		idList.reserve(container->getNumberOfItems());
		for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			idList.push_back(*containerIterator);

		for (std::vector<CachedNetworkId>::iterator i = idList.begin(); i != idList.end(); ++i)
		{
			ServerObject * const containedObject = safe_cast<ServerObject *>(i->getObject());
			if (containedObject)
				containedObject->setAuthority();
		}
	}

	safe_cast<ServerController*>(getController())->setAuthoritative(true);

	if (getScriptObject())
		getScriptObject()->setOwnerIsAuthoritative(true, GameServer::getInstance().getProcessId());

	setAuthServerProcessId(GameServer::getInstance().getProcessId());

	if (isInitialized())
		updatePositionOnPlanetServer(true);

	if (isAuthoritative() && (!isPlayerControlled() || getClient()) && getObjVars().hasItem("teleportFixup"))
		TeleportFixupHandler::add(*this);

	// Update loadWith so we don't depend on a stale value for objects that
	// drop authority and regain it later without unloading.
	setLoadWith(ContainerInterface::getLoadWithContainerId(*this));

	virtualOnSetAuthority();
	scheduleForAlter();
	if (!m_messageTos.empty())
		MessageToQueue::getInstance().addToScheduler(*this, m_messageTos.begin()->second.getCallTime());

	if (isPlayerControlled())
	{
		// Make sure Central knows the right sceneId for the player
		GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > const msg(
			"SetSceneForPlayer",
			std::make_pair(
				getNetworkId(),
				std::make_pair(ServerWorld::getSceneId(), false)));

		GameServer::getInstance().sendToCentralServer(msg);

		std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getNetworkId());
		if ((iterFind != connectedCharacterLfgData.end()) && (iterFind->second.locationPlanet != ServerWorld::getSceneId()))
			ServerUniverse::setConnectedCharacterPlanetData(getNetworkId(), ServerWorld::getSceneId());
	}

	// When a top level object or an object whose contiaining object is
	// authoritative gets to this point, all its contents have been
	// made authoritative, so it is now safe to trigger appropriate objects
	// to notify them of gaining authority.
	ServerObject * const containingObject = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
	if (!containingObject || containingObject->isAuthoritative())
		triggerMadeAuthoritative();
}

//-----------------------------------------------------------------------

void ServerObject::virtualOnSetClient()
{
	if (getObjVars().hasItem("teleportFixup"))
		TeleportFixupHandler::add(*this);

	if (m_pendingSyncUi)
	{
		for (std::vector<NetworkId>::const_iterator i = m_pendingSyncUi->begin(); i != m_pendingSyncUi->end(); ++i)
		{
			ServerObject *uiObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(*i));
			if (uiObject && uiObject->getSynchronizedUi())
				uiObject->addSynchronizedUiClient(*this);
		}
		delete m_pendingSyncUi;
		m_pendingSyncUi = 0;
	}
}

// ----------------------------------------------------------------------

void ServerObject::setClient(Client &newClient)
{
	m_client = &newClient;

	virtualOnSetClient();

	safe_cast<ServerController *>(getController())->onClientReady();
}

// ----------------------------------------------------------------------

void ServerObject::clearClient()
{
	if (m_client)
	{
		safe_cast<ServerController *>(getController())->onClientLost();
		m_client = 0;
	}
}

//-----------------------------------------------------------------------

void ServerObject::setCacheVersion(const int c)
{
	if (isAuthoritative())
	{
		bool const becomingUncached = c == 0 && m_cacheVersion.get() != 0;

		if (becomingUncached)
			ObserveTracker::onObjectDestroyed(*this, false);

		m_cacheVersion.set(c);
		if (c == 0)
		{
			Container* container = ContainerInterface::getContainer(*this);
			if (container)
			{
				for (ContainerIterator i = container->begin(); i != container->end(); ++i)
				{
					ServerObject *containedObject = safe_cast<ServerObject*>((*i).getObject());
					if (containedObject && containedObject->getCacheVersion() != 0)
						containedObject->setCacheVersion(0);
				}
			}

			if (becomingUncached && isInWorld() && !ContainerInterface::getContainedByObject(*this))
				createFarNetworkUpdateVolume();
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setCacheVersion, new MessageQueueGenericValueType<int>(c));
	}
}

// ----------------------------------------------------------------------

void ServerObject::setDefaultAlterTime(float time)
{
	// if the server is running in build mode, always alter, so that designers' changes
	// will be made even if there are no players around
	if (ConfigServerGame::getBuildoutAreaEditingEnabled() && time > AlterResult::cms_alterQuickly)
		time = AlterResult::cms_alterQuickly;
	m_defaultAlterTime = time;
}

// ----------------------------------------------------------------------
// CellProperty::addObject_w calls around this function (it calls Object::setParentCell directly)
// to ensure that server-side objects send cellChanged notifications correctly.

void ServerObject::setParentCell(CellProperty * newCell)
{
	// Uninitialized or proxy objects can set the parent cell directly

	if (!isInitialized() || !isAuthoritative())
	{
		Object::setParentCell(newCell);
		return;
	}

	// Everything else has to go through ContainerInterface

	Transform oldTransform = getTransform_o2p();

	bool result = false;

	CellProperty * oldCell = getParentCell();

	if(oldCell == nullptr) oldCell = CellProperty::getWorldCellProperty();
	if(newCell == nullptr) newCell = CellProperty::getWorldCellProperty();

	// ----------

	if(oldCell == newCell)
	{
		// Cells are the same - nothing to do
	}
	if(newCell == CellProperty::getWorldCellProperty())
	{
		// Object was in a cell and is moving to the world, transfer from cell container to world

		Object * oldCellObject = &oldCell->getOwner();

		Transform oldCellTransform = oldCellObject->getTransform_o2w();

		Transform newTransform;

		newTransform.multiply(oldCellTransform,oldTransform);

		Container::ContainerErrorCode tmp = Container::CEC_Success;
		result = ContainerInterface::transferItemToWorld(*this, newTransform, nullptr, tmp);

		DEBUG_REPORT_LOG(!result, ("Object %s (id=%s) transfer to world was denied via ContainerInterface::transferItemToWorld()\n", getObjectTemplateName(), getNetworkId().getValueString().c_str()));
	}
	else
	{
		// Object is moving to a different cell

		Object * newCellObject = &newCell->getOwner();

		ServerObject * newCellServerObject = safe_cast<ServerObject*>(newCellObject);

		Container::ContainerErrorCode tmp = Container::CEC_Success;
		result = ContainerInterface::transferItemToCell(*newCellServerObject, *this, nullptr, tmp);

		DEBUG_REPORT_LOG(!result, ("Object %s (id=%s) transfer to cell (id=%s) was denied via ContainerInterface::transferItemToCell()\n", getObjectTemplateName(), getNetworkId().getValueString().c_str(), newCellObject->getNetworkId().getValueString().c_str()));
	}

	// ----------

	if(!result)
	{
		// A failed transfer may have mucked up the transform, so restore it now.

		setTransform_o2p(oldTransform);
	}
}

//-----------------------------------------------------------------------

NetworkId const &ServerObject::getOwnerId() const
{
	return NetworkId::cms_invalid;
}

//-----------------------------------------------------------------------

bool ServerObject::isOwner(NetworkId const &, Client const *client) const
{
	return client && client->isGod();
}

//-----------------------------------------------------------------------

void ServerObject::setOwnerId(NetworkId const &)
{
}

//-----------------------------------------------------------------------

/**
 * Rename an object.
 * @param newName The new name for the object
 * You cannot rename a player-controlled object using this function.
 * @see PlayerCreationManagerServer::renamePlayer()
 */

void ServerObject::setObjectName(const Unicode::String &newName)
{
	if(isAuthoritative())
	{
		if (isPlayerControlled())
		{
			DEBUG_WARNING(true, ("You cannot set the name of a player controlled object directly.  Name:<%s>", Unicode::wideToNarrow(newName).c_str()));
			return;
		}
		
		std::string UTF8String = Unicode::wideToUTF8(newName);
		if (UTF8String.length() > 127)
		{
			DEBUG_WARNING(true, ("Tried to set object %s's name to something too long (truncating). [%s]", getNetworkId().getValueString().c_str(), UTF8String.c_str()));
			m_objectName = Unicode::utf8ToWide(UTF8String.substr(0, 127));
		}
		else
			m_objectName = newName;

		if (newName[0] == '@')
		{
			StringId id(Unicode::wideToNarrow(newName));
			setObjectNameStringId(id);
			setObjectName(Unicode::emptyString);
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setObjectName, new MessageQueueGenericValueType<Unicode::String>(newName));
	}
}

//-----------------------------------------------------------------

void ServerObject::setObjectNameStringId (const StringId & id)
{
	if(isAuthoritative())
	{
		m_nameStringId = id;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setObjectNameStringId, new MessageQueueGenericValueType<StringId>(id));
	}
}

//----------------------------------------------------------------------

void ServerObject::setSceneIdOnThisAndContents  (const std::string & sceneId)
{
	setSceneId(sceneId);
	Container *container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
		{
			ServerObject *containedObject = safe_cast<ServerObject *>((*containerIterator).getObject());
			if (containedObject)
			{
				containedObject->setSceneIdOnThisAndContents(sceneId);
			}
		}
	}
}

//----------------------------------------------------------------------

void ServerObject::speakText(NetworkId const &target, uint32 chatType, uint32 mood, uint32 flags, Unicode::String const &speech, int language, Unicode::String const &oob)
{
	if (speech.empty())
	{
		return;
	}

	if (chatType > 16384 || mood > 16384 || flags > 16384)
	{
		WARNING (true, ("ChatType, Mood, or Flags are out of 16 bit range. %d, %d, %d", chatType, mood, flags));
		return;
	}

	const bool isTypePrivate = SpatialChatManager::isPrivate (chatType);
	if (isTypePrivate)
		flags |= MessageQueueSpatialChat::F_isPrivate;

	const uint16 volume = SpatialChatManager::getVolume (chatType);

	MessageQueueSpatialChat const chatMessage(
		getNetworkId(),
		target,
		speech,
		volume,
		static_cast<uint16>(chatType),
		static_cast<uint16>(mood),
		flags,
		language,
		oob);
	speakText(chatMessage);
}

//-----------------------------------------------------------------------

void ServerObject::speakText(MessageQueueSpatialChat const &spatialChat)
{
	if (!isAuthoritative ())
	{
		Controller * const controller = getController();
		NOT_NULL (controller);

		MessageQueueSpatialChat * msg = new MessageQueueSpatialChat (spatialChat);
		controller->appendMessage(CM_spatialChatSend, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		return;
	}

	if (m_scriptObject)
	{
		ScriptParams params;
		params.addParam(spatialChat.getText ());
		if (m_scriptObject->trigAllScripts(Scripting::TRIG_SPEAKING, params) != SCRIPT_CONTINUE)
		{
			LOG("ScriptInvestigation", ("Chat could not speak for %s because scripts said not to", getNetworkId().getValueString().c_str()));
			return;
		}
	}

	const float distance         = static_cast<float>(spatialChat.getVolume ());
	const uint32 flags           = spatialChat.getFlags();
	const bool isPrivate         = (flags & MessageQueueSpatialChat::F_isPrivate) != 0;
	const bool isTargetOnly      = (flags & MessageQueueSpatialChat::F_targetOnly) != 0;
	const bool isTargetGroupOnly = (flags & MessageQueueSpatialChat::F_targetGroupOnly) != 0;
	const bool isTargetAndSourceGroup = (flags & MessageQueueSpatialChat::F_targetAndSourceGroup) != 0;

	const NetworkId & chatTargetId = spatialChat.getTargetId ();
	const NetworkId & chatSourceId = spatialChat.getSourceId ();

	typedef std::vector<ServerObject *> ServerObjectVector;
	ServerObjectVector results;
	ServerWorld::getSpatialChatListeners(*this, distance, results);

	const ServerObject * const chatTarget           = safe_cast<const ServerObject *>(NetworkIdManager::getObjectById (chatTargetId));
	const CreatureObject * const chatTargetCreature = chatTarget ? chatTarget->asCreatureObject () : 0;
	int const chatMessageIndex = ChatLogManager::getNextMessageIndex();

	const ServerObject * const chatSource = safe_cast<const ServerObject *>(NetworkIdManager::getObjectById (chatSourceId));
	const CreatureObject * const chatSourceCreature = chatSource ? chatSource->asCreatureObject () : 0;

	for(ServerObjectVector::iterator i = results.begin(); i != results.end(); ++i)
	{
		ServerObject * const target = NON_NULL (*i);

		const NetworkId & targetId = target->getNetworkId ();

		//-- see if the target is valid for the flags
		if ((spatialChat.getFlags() & MessageQueueSpatialChat::F_skipTarget) != 0 && target->getNetworkId() == spatialChat.getTargetId())
			continue;
		if ((spatialChat.getFlags() & MessageQueueSpatialChat::F_skipSource) != 0 && target->getNetworkId() == spatialChat.getSourceId())
			continue;

		if (isTargetAndSourceGroup)
		{
			const CreatureObject * const targetCreature = target->asCreatureObject ();
			if (!targetCreature)
			{
				continue;
			}

			const GroupObject * const chatSourceGroup = chatSourceCreature ? chatSourceCreature->getGroup () : 0;
			const GroupObject * const chatTargetGroup = chatTargetCreature ? chatTargetCreature->getGroup () : 0;
			const GroupObject * const targetGroup = targetCreature ? targetCreature->getGroup () : 0;

			//continue if targetCreature is not in a group with the chat source or chat target
			if (targetId != chatTargetId && targetId != chatSourceId)
			{
				if (!targetGroup || (targetGroup != chatSourceGroup && targetGroup != chatTargetGroup))
				{
					continue;
				}
			}

		}
		else if (isTargetGroupOnly)
		{
			if (!chatTargetCreature)
				continue;

			if (targetId != chatTargetId)
			{
				const CreatureObject * const targetCreature = target->asCreatureObject ();
				if (!targetCreature || !targetCreature->getGroup() || targetCreature->getGroup () != chatTargetCreature->getGroup ())
					continue;
			}
		}
		else if (isTargetOnly)
		{
			if (targetId != chatTargetId)
				continue;
		}

		if (isPrivate)
		{
			if (targetId == chatTargetId || getNetworkId () == chatTargetId || getNetworkId () == chatSourceId)
			{
				//-- don't send text to ourselves if we are an NPC
				if (isPlayerControlled () || chatSourceId != getNetworkId ())
				{
					target->hearText(*this, spatialChat, chatMessageIndex);
				}
			}
			else
			{
				const MessageQueueSpatialChat noTextMsg (spatialChat.getSourceId (),
					spatialChat.getTargetId  (),
					Unicode::emptyString,
					spatialChat.getVolume    (),
					spatialChat.getChatType  (),
					spatialChat.getMoodType  (),
					spatialChat.getFlags     (),
					spatialChat.getLanguage  (),
					Unicode::emptyString);

				target->hearText(*this, noTextMsg, chatMessageIndex);
			}
		}
		else
		{
			target->hearText(*this, spatialChat, chatMessageIndex);
		}
	}
}

//-----------------------------------------------------------------------

void ServerObject::hearText(ServerObject const &source, MessageQueueSpatialChat const &spatialChat, int chatMessageIndex)
{
	Controller * const controller = NON_NULL (getController());
	if (!isAuthoritative())
	{
		MessageQueueSpatialChat * const msg = new MessageQueueSpatialChat (spatialChat);
		controller->appendMessage(CM_spatialChatReceive, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		return;
	}

	if (m_scriptObject)
	{
		ScriptParams params;
		params.addParam(spatialChat.getSourceId());
		params.addParam(spatialChat.getText());
		if (!m_scriptObject->trigAllScripts(Scripting::TRIG_HEAR_SPEECH_STRING, params))
			return;
	}

	if (getClient ())
	{
		if (   source.getCacheVersion() > 0
		    || (   ObserveTracker::isObserving(*getClient(), source)
		        && !PlayerObject::isIgnoring(getNetworkId(), spatialChat.getSourceId())))
		{
			// Log the chat a player hears

			CreatureObject * const creatureObject = asCreatureObject();

			if (creatureObject != nullptr)
			{
				PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);

				if (playerObject != nullptr)
				{
					ChatAvatarId playerChatAvatarId(Chat::constructChatAvatarId(source));
					Unicode::String playerName(Unicode::narrowToWide(playerChatAvatarId.getFullName()));

					std::string chatTypeName;

					if (spatialChat.getChatType() == 0)
					{
						chatTypeName = "spatial";
					}
					else
					{
						if (!SpatialChatManager::getChatNameByType(spatialChat.getChatType(), chatTypeName))
						{
							chatTypeName = "error";
						}
					}

					ChatLogManager::logChat(playerName, Unicode::emptyString, spatialChat.getText(), Unicode::narrowToWide(chatTypeName), chatMessageIndex, Os::getRealSystemTime());

					playerObject->logChat(chatMessageIndex);
				}
			}

			MessageQueueSpatialChat * const msg = new MessageQueueSpatialChat (spatialChat);
			controller->appendMessage(CM_spatialChatReceive, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

//----------------------------------------------------------------------

void ServerObject::performSocial (const NetworkId & target, uint32 socialType, bool animationOk, bool textOk)
{
	const MessageQueueSocial socialMsg (CachedNetworkId (*this), CachedNetworkId (target), socialType, animationOk, textOk);
	performSocial (socialMsg);
}

//----------------------------------------------------------------------

void ServerObject::performSocial (const MessageQueueSocial & socialMsg)
{
	// a permanently squelched player cannot do any emote
	PlayerObject * p = PlayerCreatureController::getPlayerObject(asCreatureObject());
	if (p && (p->getSecondsUntilUnsquelched() < 0))
	{
		return;
	}

	if (!isAuthoritative ())
	{
		Controller *const controller = NON_NULL (getController());

		MessageQueueSocial * const msg = new MessageQueueSocial (socialMsg);
		controller->appendMessage(CM_socialSend, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		return;
	}

	// todo: this needs to have a better value sometime
	static const float distance = 50.0f;

	const Vector & pos_w = getPosition_w ();
	typedef std::vector<ServerObject *> ServerObjectVector;
	ServerObjectVector results;
	ServerWorld::findObjectsInRange (pos_w, distance, results);


	bool triggerOk = false;
	std::string socialTypeName;
	triggerOk = SocialsManager::getSocialNameByType (socialMsg.getSocialType(), socialTypeName);
	Unicode::String unicodeSocialTypeName(Unicode::narrowToWide(socialTypeName));

	// allow scripts to prevent the player from performing the emote
	ScriptParams params;

	if (getScriptObject() != nullptr)
	{
		params.addParam(unicodeSocialTypeName);
		if (getScriptObject()->trigAllScripts(Scripting::TRIG_PERFORM_EMOTE, params) != SCRIPT_CONTINUE)
			return;
		params.clear();
	}

	if (triggerOk)
	{
		params.addParam(getNetworkId());
		params.addParam(unicodeSocialTypeName);
	}

	static const float trigger_range = 4.0f;

	for (ServerObjectVector::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		ServerObject * const target = *it;
		NOT_NULL (target);

		if (triggerOk && this != target)
		{
			const float target_trigger_range_sqr = sqr (trigger_range + target->getSphereExtent ().getRadius ());
			const float distance_sqr = pos_w.magnitudeBetweenSquared (target->getPosition_w ());

			if (distance_sqr < target_trigger_range_sqr)
				IGNORE_RETURN(target->getScriptObject()->trigAllScripts(Scripting::TRIG_SAW_EMOTE, params));
		}

		(*it)->seeSocial (socialMsg);
	}
}

//----------------------------------------------------------------------

void ServerObject::performCombatSpam (const MessageQueueCombatSpam & spamMsg, bool sendToSelf, bool sendToTarget, bool sendToBystanders)
{
	if (!isAuthoritative ())
	{
		DEBUG_WARNING (true, ("Do not call performCombatSpam on non-authoritative objects"));
		return;
	}

	if (sendToSelf && !sendToBystanders)
		seeCombatSpam (spamMsg);

	if (sendToTarget && !sendToBystanders)
	{
		ServerObject * const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(spamMsg.m_defender));
		if (target) {
			if (target->getNetworkId() != spamMsg.m_attacker) {
				target->seeCombatSpam (spamMsg);
			}
		} else {
			DEBUG_WARNING (!sendToSelf && !sendToBystanders, ("nullptr target_obj in commandFuncCombatSpam, when sendToTarget was set true"));
		}
	}

	if (sendToBystanders)
	{
		Controller *const controller = NON_NULL (getController());
		controller->appendMessage (CM_combatSpam, 0, new MessageQueueCombatSpam(spamMsg), GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
	}
}

// ---------------------------------------------------------------------

void ServerObject::teleportObject(Vector const &position_w, NetworkId const &targetContainer, std::string const &targetCellName, Vector const &position_p, std::string const &scriptCallback, bool forceLoadScreen)
{
	Vector destPosition_w(position_w);

	// clamp destination if in space
	if (   ServerWorld::isSpaceScene()
	    && (   std::abs(position_w.x) > 8000.f
	        || std::abs(position_w.y) > 8000.f
	        || std::abs(position_w.z) > 8000.f))
	{
		WARNING(true, ("Tried to warp object %s outside of space zone boundaries (%g,%g,%g)", getDebugInformation().c_str(), position_w.x, position_w.y, position_w.z));
		destPosition_w.set(
			clamp(-8000.f, position_w.x, 8000.f),
			clamp(-8000.f, position_w.y, 8000.f),
			clamp(-8000.f, position_w.z, 8000.f));
	}



	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_teleportObject, new MessageQueueTeleportObject(destPosition_w, targetContainer, targetCellName, position_p, scriptCallback));
		return;
	}

	// make a copy of the destination position, as it might change if we are
	// warping to a cloning facility
	Vector finalPosition_p(position_p);

	bool doWarp = forceLoadScreen;
	bool ignoreContainer = true;

	Client * const client = m_client;
	std::vector<Watcher<Client> > containedClients;
	getContainedClients(*this, containedClients);

	ServerObject *targetContainerObject = 0;
	if (targetContainer != NetworkId::cms_invalid)
	{
		ignoreContainer = false;
		targetContainerObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(targetContainer));
		if (targetContainerObject && !targetCellName.empty())
		{
			PortalProperty *portalProp = targetContainerObject->getPortalProperty();
			targetContainerObject = 0; // clear in case we have a building that doesn't have the cell
			if (portalProp != nullptr)
			{
				CellProperty *cellProp = portalProp->getCell(targetCellName.c_str());
				if (cellProp != nullptr)
					targetContainerObject = safe_cast<ServerObject *>(&(cellProp->getOwner()));
			}
			if (!targetContainerObject)
			{
				WARNING(true, ("Tried to teleport to nonexistant cell '%s' of building %s.", targetCellName.c_str(), targetContainer.getValueString().c_str()));
				ignoreContainer = true;
			}
		}
	}

	Vector const oldPosition_w(NON_NULL(ContainerInterface::getTopmostContainer(*this))->getPosition_w());
	if (   destPosition_w.magnitudeBetweenSquared(oldPosition_w) >= sqr(ConfigServerGame::getMaxTeleportDistanceWithoutSceneWarp())
	    || (   targetContainer != NetworkId::cms_invalid
	        && (   !targetContainerObject
	            || (   getClient()
	                && !ObserveTracker::isObserving(*getClient(), *targetContainerObject)))))
		doWarp = true;

	if (doWarp && !containedClients.empty())
	{
		for (std::vector<Watcher<Client> >::const_iterator i = containedClients.begin(); i != containedClients.end(); ++i)
		{
			Client * const containedClient = (*i).getPointer();
			if (containedClient)
			{
				ServerObject * const characterObject = containedClient->getCharacterObject();
				if (characterObject)
					characterObject->onClientAboutToLoad();
				containedClient->clearControlledObjects();
			}
		}
	}

	bool moveNeeded = true;
	if (!ignoreContainer && targetContainer != NetworkId::cms_invalid)
	{
		if (!doWarp && targetContainerObject && targetContainerObject->isAuthoritative())
		{
			// allow the destination container to change the warp position
			float yaw = getObjectFrameK_p().theta();
			const ServerObject * targetObject = safe_cast<const ServerObject *>(
				ContainerInterface::getTopmostContainer(*targetContainerObject));
			if (targetObject && targetObject->isAuthoritative())
				targetObject->changeTeleportDestination(finalPosition_p, yaw);

			// teleporting to a container on this server
			if (targetContainerObject->asCellObject())
			{
				Transform worldToCell;
				worldToCell.invert(targetContainerObject->getTransform_o2w());
				Transform objectToCell;
				objectToCell.multiply(worldToCell, getTransform_o2w());
				objectToCell.setPosition_p(finalPosition_p);
				yaw -= getObjectFrameK_p().theta();
				if (yaw != 0.0f)
				{
					objectToCell.yaw_l(yaw);
					objectToCell.reorthonormalize();
				}
				safe_cast<ServerController *>(getController())->teleport(objectToCell, targetContainerObject);
			}
			else
			{
				if (isPlayerControlled())
				{
					WARNING(true, ("Tried to use ServerObject::teleportObject to teleport a player controlled object into a non-cell container!"));
					return;
				}

				Container::ContainerErrorCode err = Container::CEC_Success;
				ContainerInterface::transferItemToGeneralContainer(*targetContainerObject, *this, 0, err, false);
			}

			moveNeeded = false;
		}
		else
		{
			// container is on a different server - flag to move us into the container as soon as we change authority
			setInteriorTeleportDestination(targetContainer, targetCellName, position_p);
		}
	}

	//-- setup the script callback
	if (!scriptCallback.empty())
		setTeleportScriptCallback(scriptCallback);

	if (moveNeeded)
	{
		// Moves the object.  If the new area is on a different server, the PlanetServer will change our authority.

		// What we really want to check here is whether we want to use the ServerController's teleport
		// if a player has logged out, but we still have them loaded in game, their client will be nullptr, 
		// but we still want to use the ServerController's teleport method
		// so, we check against the player creature controller and player ship controller as well
		ServerController * controller = safe_cast<ServerController *>(getController());
		if (controller && (client || dynamic_cast<PlayerCreatureController *>(controller) || dynamic_cast<PlayerShipController *>(controller)))
		{
			Transform newTransform(getTransform_o2w());
			newTransform.setPosition_p(destPosition_w);
			controller->teleport(newTransform, 0);
		}
		else
		{
			if (ContainerInterface::getContainedByObject(*this))
			{
				Transform newTransform(getTransform_o2w());
				newTransform.setPosition_p(destPosition_w);
				Container::ContainerErrorCode err = Container::CEC_Success;
				ContainerInterface::transferItemToWorld(*this, newTransform, 0, err);
			}
			else
				setPosition_p(destPosition_w);
		}
	}

	if (doWarp && !containedClients.empty())
	{
		for (std::vector<Watcher<Client> >::const_iterator i = containedClients.begin(); i != containedClients.end(); ++i)
		{
			Client * const containedClient = (*i).getPointer();
			if (containedClient)
			{
				ServerObject * const so = NON_NULL(safe_cast<ServerObject *>(NetworkIdManager::getObjectById(containedClient->getCharacterObjectId())));
				CreatureObject * const primaryControlledCreature = NON_NULL(so->asCreatureObject());
				containedClient->assumeControl(*primaryControlledCreature);
			}
		}
	}

	CollisionWorld::objectWarped(this);

	if (isAuthoritative() && (!isPlayerControlled() || getClient()) && getObjVars().hasItem("teleportFixup"))
		TeleportFixupHandler::add(*this);
}

// ----------------------------------------------------------------------

void ServerObject::changeTeleportDestination(Vector &, float &) const
{
}

// ----------------------------------------------------------------------

void ServerObject::updatePositionOnPlanetServer(bool forceUpdate)
{
	PROFILER_AUTO_BLOCK_DEFINE("updatePositionOnPlanetServer");
	// In space, only send the forced updates
	if (isAuthoritative() && isInitialized() && (forceUpdate || !ServerWorld::isSpaceScene()))
		updatePlanetServerInternal(forceUpdate);
}

//----------------------------------------------------------------------

void ServerObject::updatePlanetServerInternal(bool) const
{
}

//----------------------------------------------------------------------

/**
* Attempts to create a synchronized ui object.  If the synchronized ui object
* does not exist, its creation is attempted via createSynchronizedUi.  The default implementation
* of createSynchronizedUi returns nullptr, and renders this method a no-op.
*/
void ServerObject::addSynchronizedUi(const std::vector<NetworkId> & clients)
{
	if (!m_synchronizedUi.getPointer())
		m_synchronizedUi = createSynchronizedUi ();

	if (m_synchronizedUi.getPointer())
	{
		for (std::vector<NetworkId>::const_iterator i = clients.begin(); i != clients.end(); ++i)
		{
			ServerObject * client = safe_cast<ServerObject *>(
				NetworkIdManager::getObjectById(*i));
			if (client != nullptr)
			{
				if (client->getClient() != nullptr)
					m_synchronizedUi->addClientObject (*client);
				else
				{
					client->addPendingSynchronizedUi(*this);
				}
			}
			else
			{
				WARNING(true, ("Object %s tried to add non-existant client %s to its sync ui",
					getNetworkId().getValueString().c_str(), i->getValueString().c_str()));
			}
		}
		scheduleForAlter();
	}
}

//----------------------------------------------------------------------

/**
* Attempts to add a client to the synchronized ui object.  If the synchronized ui object
* does not exist, its creation is attempted via createSynchronizedUi.  The default implementation
* of createSynchronizedUi returns nullptr, and renders this method a no-op.
*
*/
void ServerObject::addSynchronizedUiClient(ServerObject & client)
{
	if (isAuthoritative())
	{
		if (!m_synchronizedUi.getPointer())
			m_synchronizedUi = createSynchronizedUi ();

		if (!m_synchronizedUi.getPointer())
			return;

		m_synchronizedUi->addClientObject (client);
		scheduleForAlter();
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Non-authoritative object %s is being asked to create a synchronizedUi for client %s",
			getNetworkId().getValueString().c_str(), client.getNetworkId().getValueString().c_str()));
	}
}

//----------------------------------------------------------------------

void ServerObject::removeSynchronizedUiClient(const NetworkId & clientId)
{
	if (!m_synchronizedUi.getPointer())
		return;

	WARNING(!isAuthoritative(), ("ServerObject::removeSynchronizedUiClient being called on non-authoritative object %s for client %s",
		getNetworkId().getValueString().c_str(), clientId.getValueString().c_str()));

	m_synchronizedUi->removeClientObject (clientId);

	if (m_synchronizedUi->getNumClients () == 0)
	{
		delete m_synchronizedUi.getPointer();
	}
	scheduleForAlter();
}

//----------------------------------------------------------------------

/**
* Subclasses implement this if they have an appropriate synchronized ui object.
* Base class implementation returns nullptr.
*/
ServerSynchronizedUi * ServerObject::createSynchronizedUi ()
{
	return 0;
}

//----------------------------------------------------------------------

/**
 * Adds a object to the list of objects that have a synchronized ui and are
 * waiting for this object to have a client attached.
 *
 * @param uiObject		the object with the synchronized ui
 */
void ServerObject::addPendingSynchronizedUi(const ServerObject & uiObject)
{
	if (getClient() != nullptr)
	{
		WARNING(true, ("ServerObject::addPendingSynchronizedUi called on object %s that already has a client", getNetworkId().getValueString().c_str()));
		return;
	}

	if (m_pendingSyncUi == nullptr)
		m_pendingSyncUi = new std::vector<NetworkId>;
	m_pendingSyncUi->push_back(uiObject.getNetworkId());
}

//----------------------------------------------------------------------

void ServerObject::setPlayerControlled(bool newValue)
{
	if(isAuthoritative())
	{
		m_playerControlled=newValue;
		if (newValue)
			setDefaultAlterTime(0);
		else
			setDefaultAlterTime(AlterResult::cms_keepNoAlter);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setPlayerControlled, new MessageQueueGenericValueType<bool>(newValue));
	}
}

// ----------------------------------------------------------------------

void ServerObject::internalAdjustBankBalance(int const amount, bool makeRoom)
{
	if ((amount > 0) && ((amount + m_bankBalance.get()) > ConfigServerGame::getMaxMoney()) && makeRoom && isAuthoritative())
	{
		PlayerObject * const po = PlayerCreatureController::getPlayerObject(asCreatureObject());
		if (po && po->isAuthoritative())
		{
			int attempt = 0;
			do 
			{
				int amountToMove = amount + m_bankBalance.get() - ConfigServerGame::getMaxMoney();
				if (amountToMove > m_bankBalance.get())
				{
					amountToMove = m_bankBalance.get();
				}

				if (amountToMove <= 0)
					break;

				if ((amountToMove + m_cashBalance.get()) > ConfigServerGame::getMaxMoney())
				{
					// not enough room in cash, see if there's still room in galactic reserve
					int8 const galacticReserve = po->getGalacticReserveDeposit();
					if (static_cast<int>(galacticReserve) < ConfigServerGame::getMaxGalacticReserveDepositBillion())
					{
						// transfer 1 billion into galactic reserve, and try again, if necessary
						po->depositToGalacticReserve(true);

						// failed to transfer to galactic reserve, don't try again
						if (galacticReserve == po->getGalacticReserveDeposit())
						{
							break;
						}
					}
					else
					{
						// no more room in galactic reserve, move as much as possible into cash
						amountToMove = ConfigServerGame::getMaxMoney() - m_cashBalance.get();
						if (amountToMove > m_bankBalance.get())
						{
							amountToMove = m_bankBalance.get();
						}

						if (amountToMove > 0)
						{
							withdrawCashFromBank(amountToMove);
						}

						break;
					}
				}
				else
				{
					if (amountToMove > 0)
					{
						withdrawCashFromBank(amountToMove);
					}

					break;
				}

				++attempt;

			} while (((amount + m_bankBalance.get()) > ConfigServerGame::getMaxMoney()) && (attempt < 2));
		}
	}

	m_bankBalance = m_bankBalance.get() + amount;
	if (m_bankBalance.get() > ConfigServerGame::getMaxMoney())
	{
		LOG("CustomerService",("SuspectedCheaterChannel: %s has %i credits in the bank.", PlayerObject::getAccountDescription(this).c_str(), m_bankBalance.get()));
		m_bankBalance = ConfigServerGame::getMaxMoney();
	}

	if (getClient() && getClient()->isFreeTrialAccount()
		&& (m_cashBalance.get() + m_bankBalance.get()) > ConfigServerGame::getMaxFreeTrialMoney())
	{
		LOG("CustomerService",("FreeTrial: %s has exceeded max free trial balance of %d from a bank transfer amount of %d.  Money lost: %d.",PlayerObject::getAccountDescription(this).c_str(), ConfigServerGame::getMaxFreeTrialMoney(), amount, m_cashBalance.get() + m_bankBalance.get() - ConfigServerGame::getMaxFreeTrialMoney()));
		m_bankBalance = ConfigServerGame::getMaxFreeTrialMoney() - m_cashBalance.get();
		ProsePackage prosePackage;
		prosePackage.stringId = StringId("ui_trade", "err_max_free_trial_money");
		prosePackage.digitInteger = ConfigServerGame::getMaxFreeTrialMoney();
		Chat::sendSystemMessage(*this, prosePackage);
	}

	// sanity check - fix up any player character negative cash/bank balance
	if ((m_cashBalance.get() < 0) && PlayerCreatureController::getPlayerObject(asCreatureObject()))
		m_cashBalance = 0;

	if ((m_bankBalance.get() < 0) && PlayerCreatureController::getPlayerObject(asCreatureObject()))
		m_bankBalance = 0;

	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

void ServerObject::internalAdjustCashBalance(int const amount, bool makeRoom)
{
	if ((amount > 0) && ((amount + m_cashBalance.get()) > ConfigServerGame::getMaxMoney()) && makeRoom && isAuthoritative())
	{
		PlayerObject * const po = PlayerCreatureController::getPlayerObject(asCreatureObject());
		if (po && po->isAuthoritative())
		{
			int attempt = 0;
			do 
			{
				int amountToMove = amount + m_cashBalance.get() - ConfigServerGame::getMaxMoney();
				if (amountToMove > m_cashBalance.get())
				{
					amountToMove = m_cashBalance.get();
				}

				if (amountToMove <= 0)
					break;

				if ((amountToMove + m_bankBalance.get()) > ConfigServerGame::getMaxMoney())
				{
					// not enough room in bank, see if there's still room in galactic reserve
					int8 const galacticReserve = po->getGalacticReserveDeposit();
					if (static_cast<int>(galacticReserve) < ConfigServerGame::getMaxGalacticReserveDepositBillion())
					{
						// transfer 1 billion into galactic reserve, and try again, if necessary
						po->depositToGalacticReserve(true);

						// failed to transfer to galactic reserve, don't try again
						if (galacticReserve == po->getGalacticReserveDeposit())
						{
							break;
						}
					}
					else
					{
						// no more room in galactic reserve, move as much as possible into bank
						amountToMove = ConfigServerGame::getMaxMoney() - m_bankBalance.get();
						if (amountToMove > m_cashBalance.get())
						{
							amountToMove = m_cashBalance.get();
						}

						if (amountToMove > 0)
						{
							depositCashToBank(amountToMove);
						}

						break;
					}
				}
				else
				{
					if (amountToMove > 0)
					{
						depositCashToBank(amountToMove);
					}

					break;
				}

				++attempt;

			} while (((amount + m_cashBalance.get()) > ConfigServerGame::getMaxMoney()) && (attempt < 2));
		}
	}

	m_cashBalance = m_cashBalance.get() + amount;
	if (m_cashBalance.get() > ConfigServerGame::getMaxMoney())
	{
		LOG("CustomerService",("SuspectedCheaterChannel: %s has %i credits cash.",PlayerObject::getAccountDescription(this).c_str(), m_cashBalance.get()));
		m_cashBalance = ConfigServerGame::getMaxMoney();
	}

	if (getClient() && getClient()->isFreeTrialAccount()
		&& (m_cashBalance.get() + m_bankBalance.get()) > ConfigServerGame::getMaxFreeTrialMoney())
	{
		LOG("CustomerService",("FreeTrial: %s has exceeded max free trial balance of %d from a cash transfer amount of %d.  Money lost: %d.",PlayerObject::getAccountDescription(this).c_str(), ConfigServerGame::getMaxFreeTrialMoney(), amount, m_cashBalance.get() + m_bankBalance.get() - ConfigServerGame::getMaxFreeTrialMoney()));
		m_cashBalance = ConfigServerGame::getMaxFreeTrialMoney() - m_bankBalance.get();
		ProsePackage prosePackage;
		prosePackage.stringId = StringId("ui_trade", "err_max_free_trial_money");
		prosePackage.digitInteger = ConfigServerGame::getMaxFreeTrialMoney();
		Chat::sendSystemMessage(*this, prosePackage);
	}

	// sanity check - fix up any player character negative cash/bank balance
	if ((m_cashBalance.get() < 0) && PlayerCreatureController::getPlayerObject(asCreatureObject()))
		m_cashBalance = 0;

	if ((m_bankBalance.get() < 0) && PlayerCreatureController::getPlayerObject(asCreatureObject()))
		m_bankBalance = 0;

	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

bool ServerObject::transferCashTo(const NetworkId &target, int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}

	if (isAuthoritative())
	{
		if (amount > ConfigServerGame::getMaxMoneyTransfer())
		{
			DEBUG_REPORT_LOG(true,("Tried to transfer too much cash.  Source object %s, target %s, amount %i",
								   getNetworkId().getValueString().c_str(),target.getValueString().c_str(),amount));
			return false;
		}

		int sourceBalance=getCashBalance();
		if (sourceBalance < amount)
		{
			DEBUG_REPORT_LOG(true,("Tried to transfer too much cash.  Source object %s, target %s, amount %i",
								   getNetworkId().getValueString().c_str(),target.getValueString().c_str(),amount));
			return false;
		}
		internalAdjustCashBalance(-amount, true);

		// send persistent message to add money to target.
		char buffer[100];
		snprintf(buffer,100,"%i",amount);
		MessageToQueue::getInstance().sendMessageToC(target, "C++FinishCashTransfer", buffer, 0, true);

		LOG("CustomerService", ("Money: cash transfer from %s to %s, amount %i, total(from) %i", PlayerObject::getAccountDescription(getNetworkId()).c_str(),PlayerObject::getAccountDescription(target).c_str(),amount, getTotalMoney()));

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::transferBankCreditsTo(const NetworkId &target, int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}


	if (isAuthoritative())
	{
		if (amount > ConfigServerGame::getMaxMoneyTransfer())
		{
			DEBUG_REPORT_LOG(true,("Tried to transfer too many bank credits.  Source object %s, target %s, amount %i",
								   getNetworkId().getValueString().c_str(),target.getValueString().c_str(),amount));
			return false;
		}

		int sourceBalance=getBankBalance();
		if (sourceBalance < amount)
		{
			DEBUG_REPORT_LOG(true,("Tried to transfer too many bank credits.  Source object %s, target %s, amount %i",
								   getNetworkId().getValueString().c_str(),target.getValueString().c_str(),amount));
			return false;
		}
		internalAdjustBankBalance(-amount, true);

		// send persistent message to add money to target.
		char buffer[100];
		snprintf(buffer,100,"%i",amount);
		MessageToQueue::getInstance().sendMessageToC(target, "C++FinishBankTransfer", buffer, 0, true);

		LOG("CustomerService", ("Money: bank transfer from %s to %s, amount %i, total(from) %i", PlayerObject::getAccountDescription(getNetworkId()).c_str(),PlayerObject::getAccountDescription(target).c_str(),amount, getTotalMoney()));

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::transferBankCreditsTo(const std::string &target, int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}

	if (isAuthoritative())
	{
		// allow transfers to/from the commodities escrow account or galactic reserve to bypass
		// the max transfer limit because commodities item can be sold/bought for more than the
		// transfer limit and the galactic reserve transfers 1 billion credits at a time
		if ((target != CommoditiesMarket::getCommoditiesNamedEscrowAccount()) && (target != std::string("galactic_reserve")))
		{
			if (amount > ConfigServerGame::getMaxMoneyTransfer())
			{
				DEBUG_REPORT_LOG(true,("Tried to transfer too many bank credits.  Source object %s, target %s, amount %i",
									   getNetworkId().getValueString().c_str(),target.c_str(),amount));
				return false;
			}
		}

		int sourceBalance=getBankBalance();
		if (sourceBalance < amount)
		{
			DEBUG_REPORT_LOG(true,("Tried to transfer too many bank credits.  Source object %s, target %s, amount %i",
								   getNetworkId().getValueString().c_str(),target.c_str(),amount));
			return false;
		}
		internalAdjustBankBalance(-amount, true);

		LOG("CustomerService", ("Money: bank transfer from %s to named account %s, amount %i, total %i", PlayerObject::getAccountDescription(getNetworkId()).c_str(),target.c_str(), amount, getTotalMoney()));

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::transferBankCreditsFrom(const std::string &target, int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}

	if (isAuthoritative())
	{
		// allow transfers to/from the commodities escrow account or galactic reserve to bypass
		// the max transfer limit because commodities item can be sold/bought for more than the
		// transfer limit and the galactic reserve transfers 1 billion credits at a time
		if ((target != CommoditiesMarket::getCommoditiesNamedEscrowAccount()) && (target != std::string("galactic_reserve")))
		{
			if (amount > ConfigServerGame::getMaxMoneyTransfer())
			{
				DEBUG_REPORT_LOG(true,("Tried to transfer too many bank credits.  Source object %s, target %s, amount %i",
									   getNetworkId().getValueString().c_str(),target.c_str(),amount));
				return false;
			}
		}

		internalAdjustBankBalance(amount, true);

		LOG("CustomerService", ("Money: bank transfer from named account %s to %s, amount %i, total %i", target.c_str(),PlayerObject::getAccountDescription(getNetworkId()).c_str(), amount, getTotalMoney()));

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::withdrawCashFromBank(int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}
	if (isAuthoritative())
	{
		// adjust amount so it won't overflow the cash limit
		if ((getCashBalance() + amount) > ConfigServerGame::getMaxMoney())
		{
			amount = ConfigServerGame::getMaxMoney() - getCashBalance();
		}

		if (amount > 0)
		{
			int bankBalance=getBankBalance();

			if (bankBalance < amount)
			{
				DEBUG_REPORT_LOG(true,("Object %s tried to bounce a check.  Bank balance %i, requested %i.\n",getNetworkId().getValueString().c_str(),bankBalance,amount));
				return false;
			}

			internalAdjustBankBalance(-amount, false);
			internalAdjustCashBalance(amount, false);

			LOG("CustomerService", ("Money: cash withdraw from bank by %s, amount %i, total %i", PlayerObject::getAccountDescription(getNetworkId()).c_str(), amount, getTotalMoney()));
		}

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ServerObject::depositCashToBank(int amount, const NetworkId &transactionId)
{
	if (amount <= 0)
	{
		DEBUG_REPORT_LOG(true, ("Attempting to transfer 0 or negative money on object %s, amount %d\n", getNetworkId().getValueString().c_str(), amount));
		return false;
	}

	if (isAuthoritative())
	{
		// adjust amount so it won't overflow the bank limit
		if ((getBankBalance() + amount) > ConfigServerGame::getMaxMoney())
		{
			amount = ConfigServerGame::getMaxMoney() - getBankBalance();
		}

		if (amount > 0)
		{
			int cashBalance=getCashBalance();

			if (cashBalance < amount)
			{
				DEBUG_REPORT_LOG(true,("Object %s tried to deposit %i credits, but only has %i credits\n",getNetworkId().getValueString().c_str(),amount,cashBalance));
				return false;
			}

			internalAdjustCashBalance(-amount, false);
			internalAdjustBankBalance(amount, false);

			LOG("CustomerService", ("Money: cash deposit to bank by %s, amount %i, total %i", PlayerObject::getAccountDescription(getNetworkId()).c_str(), amount, getTotalMoney()));
		}

		return true;
	}
	else
	{
		//TODO: handle multi-server banking
		DEBUG_FATAL(true,("Banking across game servers is not yet implemented.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

char const *ServerObject::getTemplateName() const
{
	return getObjectTemplate()->getName();
}

// ----------------------------------------------------------------------

bool ServerObject::getHyperspaceOnCreate () const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnCreate);
}

// ----------------------------------------------------------------------

void ServerObject::setHyperspaceOnCreate (bool const hyperspaceOnCreate)
{
	setLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnCreate, hyperspaceOnCreate);
}

// ----------------------------------------------------------------------

bool ServerObject::getHyperspaceOnDestroy () const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnDestruct);
}

// ----------------------------------------------------------------------

void ServerObject::setHyperspaceOnDestroy (bool const hyperspaceOnDestroy)
{
	setLocalFlag(LocalObjectFlags::ServerObject_HyperspaceOnDestruct, hyperspaceOnDestroy);
}

// ----------------------------------------------------------------------

std::string const ServerObject::getPackedObjVars(std::string const &prefix) const
{
	std::vector<char> buf;
	for (DynamicVariableList::MapType::const_iterator i = getObjVars().begin(); i != getObjVars().end(); ++i)
	{
		if (!(*i).first.compare(0, prefix.size(), prefix))
	{
			appendPackedObjVarField(buf, (*i).first.c_str());
			appendPackedObjVarField(buf, (*i).second.getType());
			appendPackedObjVarField(buf, Unicode::wideToNarrow((*i).second.getPackedValueString()).c_str());
	}
	}
	appendPackedObjVarField(buf, "$");
	return std::string(buf.begin(), buf.end());
}

// ----------------------------------------------------------------------

bool ServerObject::setPackedObjVars(std::string const &packedVarString)
	{
	unsigned int offset = 0;

	while (offset < packedVarString.size())
	{
		std::string name;
		offset = getPackedObjVarField(packedVarString, offset, name);
		if (name == "$")
	return true;
		int typeId;
		offset = getPackedObjVarField(packedVarString, offset, typeId);
		std::string valueString;
		offset = getPackedObjVarField(packedVarString, offset, valueString);
		DynamicVariable value;
		value.load(0, typeId, Unicode::narrowToWide(valueString));
		setObjVarItem(name, value);
}
		return false;
}

// ----------------------------------------------------------------------

#ifdef _WIN32
#define atoll(a) _atoi64(a)
#endif

// ----------------------------------------------------------------------

std::string ServerObject::debugGetMessageToList() const
{
	unsigned long const now = ServerClock::getInstance().getGameTimeSeconds();
	std::string result;
	time_t const timeNow = ::time(nullptr);
	for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end(); ++i)
	{
		char temp[256];

		if (i->second.getCallTime() > now)
		{
			snprintf(temp,sizeof(temp)-1,"%s: %s [call time = %lu (%s) (%s) (%s), persisted = %s, recurring = %i]\n",i->second.getMessageId().getValueString().c_str(), i->second.getMethod().c_str(), i->second.getCallTime(), CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(i->second.getCallTime() - now)).c_str(), CalendarTime::convertEpochToTimeStringGMT(i->second.getCallTime() - now + timeNow).c_str(), CalendarTime::convertEpochToTimeStringLocal(i->second.getCallTime() - now + timeNow).c_str(), (i->second.getPersisted() ? "true" : "false"), i->second.getRecurringTime());
		}
		else
		{
			snprintf(temp,sizeof(temp)-1,"%s: %s [call time = %lu, persisted = %s, recurring = %i]\n",i->second.getMessageId().getValueString().c_str(), i->second.getMethod().c_str(), i->second.getCallTime(), (i->second.getPersisted() ? "true" : "false"), i->second.getRecurringTime());
		}

		temp[sizeof(temp)-1]='\0';
		result += temp;
	}
	return result;
}

// ----------------------------------------------------------------------

/**
 * Go through the list of messageTos and handle any whose time has been
 * reached.
 * @param effectiveMessageToTime The current Server Clock, for the purpose of MessageTos
 * @return The delivery time of the next message, or 0 if there are no more messages
 */

unsigned long ServerObject::processQueuedMessageTos(unsigned long effectiveMessageToTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::processQueuedMessageTos");

	if (isAuthoritative())
	{
		int handledMessageCount = 0;
		unsigned long startTime = Clock::timeMs();
		while (!m_messageTos.empty() && m_messageTos.begin()->second.getCallTime() <= effectiveMessageToTime)
		{
			++handledMessageCount;
			if ((ConfigServerGame::getMaxMessageTosPerObjectPerFrame() !=0 ) &&
				(handledMessageCount > ConfigServerGame::getMaxMessageTosPerObjectPerFrame()))
			{
				std::string nextTenMessages;
				{
					int count = 1;
					for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator iter = m_messageTos.begin(); ((iter != m_messageTos.end()) && (count <= 10)); ++iter, ++count)
					{
						nextTenMessages += iter->second.getMethod();
						nextTenMessages += ", ";
					}
				}

				WARNING(true,("MessageTo Backlog:  object %s had more than %i messages to be handled in one frame.  Remaining %d messages will be delayed until next frame.  Next ten messages are (%s).", getNetworkId().getValueString().c_str(), ConfigServerGame::getMaxMessageTosPerObjectPerFrame(), m_messageTos.size(), nextTenMessages.c_str()));
				MessageToQueue::getInstance().incrementBackloggedObjectCount();
				break;
			}
			if ((ConfigServerGame::getMaxMessageToTimePerObjectPerFrame() != 0) &&
				(static_cast<int>(Clock::timeMs() - startTime) > ConfigServerGame::getMaxMessageToTimePerObjectPerFrame()))
			{
				WARNING(true,("MessageTo Backlog:  object %s spent more than %i ms processing messages.  Remaining %d messages will be delayed until next frame.", getNetworkId().getValueString().c_str(), ConfigServerGame::getMaxMessageToTimePerObjectPerFrame(), m_messageTos.size()));
				MessageToQueue::getInstance().incrementBackloggedObjectCount();
				break;
			}

			Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator message = m_messageTos.begin();

			// if the message is going to be recurring, create a
			// new messageTo to reschedule the recurring message
			MessageToPayload * copyOfRecurringMessage = nullptr;
			if (message->second.getRecurringTime() > 0)
			{
				copyOfRecurringMessage = new MessageToPayload(
					getNetworkId(), 
					message->second.getMessageId(), 
					message->second.getMethod(), 
					message->second.getPackedDataVector(), 
					ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(message->second.getRecurringTime()), 
					false, 
					message->second.getDeliveryType(), 
					NetworkId::cms_invalid, 
					"", 
					message->second.getRecurringTime());
			}

			// set sentinel to prevent this message from being removed
			// from m_messageTos while it is being handled
			s_currentMessageToBeingHandled = message->first;
			handleMessageTo(message->second);

			// remove message that just got handled
			IGNORE_RETURN(m_messageTos.erase(s_currentMessageToBeingHandled));

			s_currentMessageToBeingHandled = std::make_pair(std::make_pair(0, 0), MessageToId::cms_invalid);

			// Reschedule the recurring message, if necessary
			if (copyOfRecurringMessage)
			{
				deliverMessageTo(*copyOfRecurringMessage);
				delete copyOfRecurringMessage;
			}
		}

		// Make sure object will handle messages again when the next message is due
		if (!m_messageTos.empty())
			return m_messageTos.begin()->second.getCallTime();
		else
			return 0;
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

void ServerObject::handleMessageTo(MessageToPayload const & message)
{
	if (message.getDeliveryType() == MessageToPayload::DT_c)
	{
		handleCMessageTo(message);
	}
	else if (message.getDeliveryType() == MessageToPayload::DT_java)
	{
		GameScriptObject *scriptObj=getScriptObject();
		if (scriptObj)
		{
			if (!scriptObj->handleMessage(message.getMethod(), message.getPackedDataVector()))
			{
				WARNING_STRICT_FATAL(true, ("ServerObject::handleMessageTo, message %s to object %s - script function was not called!",
										   message.getMethod().c_str(), message.getNetworkId().getValueString().c_str()));
			}
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("ServerObject::handleMessageTo, message %s to object %s - object does not have a ScriptObject attached!", 
										message.getMethod().c_str(), message.getNetworkId().getValueString().c_str()));
		}
	}
	else
		WARNING_STRICT_FATAL(true,("Called ServerObject::handleMessageTo with a message that had an unrecognized delivery type.  Message is %s, delivery type is %i", message.getMethod().c_str(), message.getDeliveryType()));

	if (message.getPersisted())
	{
		MessageToAckMessage const reply(message.getMessageId());
		GameServer::getInstance().sendToDatabaseServer(reply);		
	}
}

// ----------------------------------------------------------------------

void ServerObject::handleCMessageTo(const MessageToPayload &message)
{
	std::vector<int8> const &packedData = message.getPackedDataVector();

	if (message.getMethod() == "C++ForceAlter")
	{
		scheduleForAlter();
	}
	else if (message.getMethod() == "C++RenameCharacter")
	{
		//TODO: resolve Unicode issues with messageto's
		m_objectName=Unicode::narrowToWide(std::string(packedData.begin(), packedData.end()));

		// rename has been completed, remove the objvar so the system won't think there's a pending rename request
		Unicode::String previousRenameName;
		if (getObjVars().getItem("renameCharacterRequest.requestNewName", previousRenameName) && (previousRenameName == m_objectName.get()))
			removeObjVarItem("renameCharacterRequest.requestNewName");

		// if character is connected, update the client (character selection) with the new name
		Client * const client = getClient();
		if (client)
		{
			GenericValueTypeMessage<std::pair<std::pair<std::string, NetworkId>, Unicode::String> > const characterNameChanged("CharacterNameChanged", std::make_pair(std::make_pair(GameServer::getInstance().getClusterName(), getNetworkId()), m_objectName.get()));
			client->send(characterNameChanged, true);
		}
	}
	else if (message.getMethod() == "C++FinishBankTransfer")
	{
		std::string packedDataString(packedData.begin(), packedData.end());
		int m_amount=strtoul(packedDataString.c_str(), 0, 10);
		DEBUG_FATAL(m_amount < 0,("C++FinishBankTransaction had a negative amount.  This message should always have a positive amount.\n"));
		internalAdjustBankBalance(m_amount, true);
	}
	// transfers money from a named account to the bank
	else if( message.getMethod() == "C++FromNamedAccountToBank")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		//DEBUG_WARNING(true, ("Processing C++FromNamedAccountToBank.  Params = '%s' MessageId=%Ld", params, message.getMessageId().getValue() ));
		char *splitPos = strstr(params, " ");
		if( splitPos)
		{
			*splitPos = 0;
			std::string const named_account(params);
			int const amount = atoi(splitPos+1);
			DEBUG_FATAL(amount < 0,("C++FromNamedAccountToBank had a negative amount.  This message should always have a positive amount.\n"));
			transferBankCreditsFrom(named_account,  amount);
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++FinishCashTransfer")
	{
		std::string packedDataString(packedData.begin(), packedData.end());
		int m_amount=strtoul(packedDataString.c_str(), 0, 10);
		DEBUG_FATAL(m_amount < 0,("C++FinishCashTransaction had a negative amount.  This message should always have a positive amount.\n"));
		internalAdjustCashBalance(m_amount, true);
	}
	else if (message.getMethod() == "C++ModifyCash")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			NetworkId recipient(atoll(params));
			int amount = atoi(splitPos+1);
			transferCashTo(recipient, amount);
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++ModifyBank")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			NetworkId recipient(atoll(params));
			int amount = atoi(splitPos+1);
			transferBankCreditsTo(recipient, amount);
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++DestroySelf")
	{
		permanentlyDestroy(DeleteReasons::Script);
	}
	else if (message.getMethod() == "C++VendorStatusChange")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		int status =  atoi(params);
		CommoditiesMarket::vendorStatusChange(getNetworkId(), status);
		delete [] params;
	}
	else if (message.getMethod() == "C++Unstick")
	{
		CreatureObject* creatureObject = asCreatureObject();
		if (creatureObject && creatureObject->isInWorld())
		{
			PlayerObject * p = PlayerCreatureController::getPlayerObject(creatureObject);
			if (p && p->getIsUnsticking())
			{
				//Receive positional information with this unstick message.  If you change
				//the format of this positional information, you must also change it in the sender in CommandCppFuncs.cpp
				Vector position = getPosition_p();
				char buf[255];
				snprintf(buf, sizeof(buf), "%f:%f", position.x, position.z);
				buf[sizeof(buf) - 1] = '\0';
				
				char *params = new char[packedData.size()+1];
				memcpy(params, &(*packedData.begin()), packedData.size());
				params[packedData.size()] = 0;
				
				if (strcmp(buf, params) == 0)
				{
					//handle unstick
					static MessageDispatch::Emitter e;
					Chat::sendSystemMessageSimple(*this, SharedStringIds::unstick_request_complete, nullptr);
					RequestUnstick r;
					r.setClientId(getNetworkId());
					e.emitMessage(r);
					p->setIsUnsticking(false);
				}
				delete [] params;
			}
		}
	}
	else if (message.getMethod() == "C++WaitForPatrolPreload")
	{
		WARNING(true, ("Non-creature object %s received a C++WaitForPatrolPreload message",
			getNetworkId().getValueString().c_str()));
		MessageToQueue::cancelRecurringMessageTo(getNetworkId(), "C++WaitForPatrolPreload");
	}
	else if (message.getMethod() == "CancelRecurringMessageTo")
	{
		std::string const & methodName = message.getDataAsString();
		for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end(); ++i)
		{
			if ((i->second.getRecurringTime() != 0) && (i->second.getMethod() == methodName) && (i->first != s_currentMessageToBeingHandled))
			{
				IGNORE_RETURN(m_messageTos.erase(i));
				break;
			}
		}
	}
	else if (message.getMethod() == "C++updateObserversCount")
	{
		std::string const & buffer = message.getDataAsString();
		if (!buffer.empty())
		{
			int count = atoi(buffer.c_str());
			updateObserversCount(count);
		}
	}
	else if (message.getMethod() == "C++addBroadcastListener")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			addBroadcastListener(std::string(params), NetworkId(std::string(splitPos+1)));
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++removeBroadcastListener")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			removeBroadcastListener(std::string(params), NetworkId(std::string(splitPos+1)));
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++removeBroadcastListenerAllMessage")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		removeBroadcastListener(NetworkId(std::string(params)));
		delete [] params;
	}
	else if (message.getMethod() == "C++listenToBroadcastMessage")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			listenToBroadcastMessage(std::string(params), NetworkId(std::string(splitPos+1)));
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++stopListeningToBroadcastMessage")
	{
		char *params = new char[packedData.size()+1];
		memcpy(params, &(*packedData.begin()), packedData.size());
		params[packedData.size()] = 0;
		char *splitPos = strstr(params, " ");
		if (splitPos)
		{
			*splitPos = 0;
			stopListeningToBroadcastMessage(std::string(params), NetworkId(std::string(splitPos+1)));
		}
		delete [] params;
	}
	else if (message.getMethod() == "C++stopListeningToAllBroadcastMessage")
	{
		stopListeningToAllBroadcastMessages();
	}
	else if (message.getMethod() == "C++AdjustAccountFeatureIdResponse")
	{
		if (!packedData.empty())
		{
			Archive::ByteStream bs;
			bs.put(reinterpret_cast<const void *>(&packedData[0]), static_cast<unsigned int>(packedData.size()));

			Archive::ReadIterator ri = bs.begin();
			AdjustAccountFeatureIdResponse const msg(ri);

			if (msg.getTargetPlayer() == getNetworkId())
			{
				if (msg.getResultCode() == RESULT_SUCCESS)
				{
					Client * client = getClient();
					if (client)
					{
						if (msg.getGameCode() == PlatformGameCode::SWG)
						{
							// request an updated list of feature id
							AccountFeatureIdRequest const req(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), msg.getTargetPlayer(), msg.getTargetStationId(), msg.getGameCode(), AccountFeatureIdRequest::RR_Reload);
							client->sendToConnectionServer(req);

							// let player know that the reward was successfully traded in
							if (VeteranRewardManager::isTradeInRewardFeatureId(msg.getFeatureId()) && !msg.getTargetPlayerDescription().empty() && msg.getTargetItem().isValid() && !msg.getTargetItemDescription().empty())
								Chat::sendSystemMessage(*this, StringId("veteran_new", "trade_in_success"), Unicode::emptyString);
						}
						else if (msg.getGameCode() == PlatformGameCode::SWGTCG)
						{
							// let player know that the card was successfully redeemed
							Chat::sendSystemMessage(*this, StringId("veteran_new", "redeem_tcg_card_success"), Unicode::emptyString);
						}
					}
				}
				else if (msg.getGameCode() == PlatformGameCode::SWGTCG)
				{
					Client * client = getClient();
					if (client)
					{
						// let player know that the card failed to redeem
						Chat::sendSystemMessage(*this, StringId("veteran_new", "redeem_tcg_card_failure"), Unicode::emptyString);
					}
				}
				else if ((msg.getGameCode() == PlatformGameCode::SWG) && VeteranRewardManager::isTradeInRewardFeatureId(msg.getFeatureId()) && !msg.getTargetPlayerDescription().empty() && msg.getTargetItem().isValid() && !msg.getTargetItemDescription().empty())
				{
					Client * client = getClient();
					if (client)
					{
						// let player know that the reward trade in failed
						Chat::sendSystemMessage(*this, StringId("veteran_new", "trade_in_failure"), Unicode::emptyString);
					}
				}
			}

			if (msg.getRequestingPlayer() == getNetworkId())
			{
				if (msg.getResultCode() == RESULT_SUCCESS)
				{
					if (msg.getResultCameFromSession())
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("feature Id (%lu) on account (%lu) has been adjusted from (%d) to (%d); session/Platform reported successful result code (%u, %s:%s)", msg.getFeatureId(), msg.getTargetStationId(), msg.getOldValue(), msg.getNewValue(), msg.getResultCode(), msg.getSessionResultString().c_str(), msg.getSessionResultText().c_str()), getClient());
					else
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("feature Id (%lu) on account (%lu) has been adjusted from (%d) to (%d)", msg.getFeatureId(), msg.getTargetStationId(), msg.getOldValue(), msg.getNewValue()), getClient());
				}
				else
				{
					if (msg.getResultCameFromSession())
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("request to adjust feature Id for account (%lu) failed with error code (%u, %s:%s)", msg.getTargetStationId(), msg.getResultCode(), msg.getSessionResultString().c_str(), msg.getSessionResultText().c_str()), getClient());
					else
						ConsoleMgr::broadcastString(FormattedString<1024>().sprintf("request to adjust feature Id for account (%lu) failed", msg.getTargetStationId()), getClient());
				}
			}

			if (msg.getTargetItem() == getNetworkId())
			{
				if (msg.getGameCode() == PlatformGameCode::SWGTCG)
					VeteranRewardManager::handleTcgRedemptionResponse(*this, msg);
				else if ((msg.getGameCode() == PlatformGameCode::SWG) && VeteranRewardManager::isTradeInRewardFeatureId(msg.getFeatureId()) && !msg.getTargetPlayerDescription().empty() && !msg.getTargetItemDescription().empty())
					VeteranRewardManager::handleTradeInResponse(*this, msg);
			}
		}
	}
	else if (message.getMethod() == "C++kickPlayerCtsCompletedOrInProgress")
	{
		LOG("CustomerService",("Login: dropping %s because of CTS completed or in progress", PlayerObject::getAccountDescription(this).c_str()));

		KickPlayer const kickMessage(getNetworkId(), "CTS completed or in progress");
		GameServer::getInstance().sendToConnectionServers(kickMessage);
		GameServer::getInstance().dropClient(getNetworkId(), true);
	}
}

// ----------------------------------------------------------------------

bool ServerObject::handleTeleportFixup(bool force)
{
	if (getObjVars().hasItem("teleportFixup"))
	{
		LOG("TeleportFixup", ("Teleport fixup called for %s\n", getNetworkId().getValueString().c_str()));
		if (getObjVars().hasItem("teleportFixup.x"))
		{
			// Ignore the container portion of a teleport fixup if we are already contained
			if (!ContainerInterface::getContainedByObject(*this))
			{
				ServerObject *container = 0;
				NetworkId destContainer;
				std::string destCellName;
				Vector destPosition_p;

				getObjVars().getItem("teleportFixup.x", destPosition_p.x);
				getObjVars().getItem("teleportFixup.y", destPosition_p.y);
				getObjVars().getItem("teleportFixup.z", destPosition_p.z);
				getObjVars().getItem("teleportFixup.container", destContainer);


				if (getObjVars().getItem("teleportFixup.cellname", destCellName))
				{
						ServerObject * const destBuilding = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(destContainer));
					if (destBuilding)
					{
							PortalProperty * const portalProp = destBuilding->getPortalProperty();
						if (portalProp)
						{
								CellProperty * const cellProp = portalProp->getCell(destCellName.c_str());
							if (cellProp)
								container = safe_cast<ServerObject*>(&(cellProp->getOwner()));
						}
					}
				}
				else
					container = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(destContainer));

				if (container)
				{
					// allow the destination container to change the warp position
					float yaw = 0.0f;
						ServerObject * const topmost = safe_cast<ServerObject *>(ContainerInterface::getTopmostContainer(*container));
					if (topmost && topmost->isAuthoritative())
						topmost->changeTeleportDestination(destPosition_p, yaw);

					Transform t;
					t.setPosition_p(destPosition_p);
					yaw -= getObjectFrameK_p().theta();
					if (yaw != 0.0f)
					{
						t.yaw_l(yaw);
						t.reorthonormalize();
					}
					LOG("TeleportFixup", ("TeleportFixup from (%g,%g,%g) in world to (%g,%g,%g) in %s\n",
						getPosition_w().x, getPosition_w().y, getPosition_w().z,
						t.getPosition_p().x, t.getPosition_p().y, t.getPosition_p().z,
						container->getNetworkId().getValueString().c_str()));
					safe_cast<ServerController*>(getController())->teleport(t, container);
				}
				else if (destContainer != NetworkId::cms_invalid && !force)
				{
					LOG("TeleportFixup", ("Failing teleport fixup because object %s is in a nullptr dest container and force was passed in\n", getNetworkId().getValueString().c_str()));
					return false; // going to a container and it wasn't loaded, defer
				}
			}
			else
				LOG("TeleportFixup", ("Ignoring teleport fixup because object %s is already contained by %s\n", getNetworkId().getValueString().c_str(), ContainerInterface::getContainedByObject(*this)->getNetworkId().getValueString().c_str()));
		}

		// Teleporting may cause us to become nonauthoritative here, and if so we should not clear the fixup, so it is
		// reapplied on the new auth server.
		if (!isAuthoritative())
			return true;

		std::string scriptCallback;
		if (getObjVars().getItem("teleportFixup.callback", scriptCallback))
		{
			DEBUG_REPORT_LOG(true, ("Player warp making callback %s\n", scriptCallback.c_str()));
			MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), scriptCallback, std::vector<int8>(), 0, true);
		}

		LOG("TeleportFixup", ("Removing teleport fixup objvars for object %s\n", getNetworkId().getValueString().c_str()));
		removeObjVarItem("teleportFixup");
	}
	return true;
}

// ----------------------------------------------------------------------

/**
 * Sets customization data for this object.
 *
 * @param customName		name of the customization parameter
 * @param value				value to set the customization to
 */
void ServerObject::customize(const std::string & customName, int value)
{
	if(isAuthoritative())
	{
		CustomizationDataProperty *const cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (cdProperty != nullptr)
		{
			CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
			if (customizationData != nullptr)
			{
				RangedIntCustomizationVariable * variable = dynamic_cast<
					RangedIntCustomizationVariable*>(customizationData->findVariable(
					customName));
				if (variable != nullptr)
					variable->setValue(value);
				else
				{
					DEBUG_WARNING(true, ("ServerObject::customize() tried to access var [%s] as ranged int, but not right type.", customName.c_str()));
				}
				//-- release local references
				customizationData->release();
			}
			else
			{
				// this shouldn't happen
				DEBUG_WARNING(true, ("CustomizationDataProperty returned nullptr CustomizationData on fetch."));
			}
		}
		else
		{
			// this shouldn't happen
			DEBUG_WARNING(true, ("ServerObject::customize() object does not have customization property"));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_customize, new MessageQueueGenericValueType<std::pair<std::string, int> >(std::make_pair(customName, value)));
	}
}	// ServerObject::customize

// ----------------------------------------------------------------------

bool ServerObject::isInWorldCell() const
{
	return isInWorld() && (ContainerInterface::getTopmostContainer(*this) == this);
}

// ----------------------------------------------------------------------

bool ServerObject::canDestroy() const
{
	if (isPlayerControlled())
		return false;
	const Container * container = ContainerInterface::getContainer(*this);
	if (container)
	{
		//Check contents to make sure they can be destroyed.
		ContainerConstIterator i = container->begin();
		for (; i != container->end(); ++i)
		{
			const ServerObject* child = safe_cast<const ServerObject *>((*i).getObject());
			if (child && !child->canDestroy())
			{
				DEBUG_REPORT_LOG(true, ("Tried to delete object %s but could not delete child %s\n", getNetworkId().getValueString().c_str(), child->getNetworkId().getValueString().c_str()));
				return false;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::permanentlyDestroy(DeleteReasons::Enumerator reason)
{
	// prevent recursion
	if (isBeingDestroyed())
		return true;

	if (!isAuthoritative())
	{
		sendControllerMessageToAuthServer(CM_permanentlyDestroy, new MessageQueueGenericValueType<int>(reason));
		return false;
	}

	if (!canDestroy())
	{
		return false;
	}

	// can't destroy an initialized vendor unless the commodities server is up because
	// there is commodities server data that *MUST* be removed when the vendor is
	// destroyed, or else the commodities server will think the player has more
	// vendors than he actually has
	if ( isVendor() && getObjVars().hasItem("vendor_initialized") && !CommoditiesMarket::isCommoditiesServerAvailable() )
	{
		return false;
	}

	BuildingObject* buildingObject = asBuildingObject();
	const bool houseDestroyedByScript = (reason == DeleteReasons::Script && buildingObject && buildingObject->isPlayerPlaced());

	// tell scripts we want to destroy the object
	if (getScriptObject() != nullptr && !m_calledTriggerDestroy)
	{
		m_calledTriggerDestroy = true;
		ScriptParams params;
		if (getScriptObject()->trigAllScripts(Scripting::TRIG_DESTROY, params) == SCRIPT_OVERRIDE)
		{
			if(reason != DeleteReasons::God)
			{
				m_calledTriggerDestroy = false;
				return false;
			}
		}
	}

	if (isAuthoritative() && (getScriptObject() != nullptr) && !m_calledTriggerRemovingFromWorld)
	{
		ScriptParams params;
		m_calledTriggerRemovingFromWorld = true;
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_REMOVING_FROM_WORLD, params));
	}
	
	setBeingDestroyed(true);

	if (isInBazaarOrVendor())
	{
		CommoditiesMarket::auctionCancel(getOwnerId(), getNetworkId().getValue());
	}

	// need to remove all the items from the Commodities Market
	// TODO: how to remove an item that might get deleted from a vendor's inventory without the vendor being deleted? (GM deleting and item for instance)
	// TODO PROBLEM: contents of vendors aren't guaranteed to be loaded
	if ( isVendor() )
	{
		ServerObject *inv = getBazaarContainer();
		NetworkId playerOwnerId;
		getObjVars().getItem("vendor_owner", playerOwnerId);
		if( inv && playerOwnerId.getValue()>0 )
		{
			// if this was an uninitialized vendor, remove objvar from owner
			ServerObject *owner = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(playerOwnerId));
			if (owner)
			{
				NetworkId notInitalizedVendor;
				if (owner->getObjVars().getItem("vendor_not_initialized", notInitalizedVendor))
				{
					if (notInitalizedVendor == Object::getNetworkId())
					{
						owner->removeObjVarItem("vendor_not_initialized");
					}
				}
			}
			CommoditiesMarket::destroyVendorMarket(playerOwnerId, *inv);
		}
	}

	if (isPersisted())
	{
		sendDeleteToDatabase(*this, houseDestroyedByScript ? DeleteReasons::House : reason);
		ServerObject const * const oldContainer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
		LOG("CustomerService",("Deletion: %s in container: %s deleted from game.  Reason: %s", ServerObject::getLogDescription(this).c_str(), oldContainer ? ServerObject::getLogDescription(oldContainer).c_str() : "none", describeDeleteReason(static_cast<int>(reason)).c_str()));
		int childReason = DeleteReasons::ContainerDeleted;
		if (houseDestroyedByScript)
			childReason = DeleteReasons::House;
		flagChildObjectsForDeletion(*this, childReason);
	}

	// grab the proxy list prior to removing it from the container, because it's about to lose its proxy list
	ProxyList const &proxyList = getExposedProxyList();
	if (!proxyList.empty())
	{
		ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

	UnloadObjectMessage const unloadMessage(getNetworkId());
		ServerMessageForwarding::send(unloadMessage);

		ServerMessageForwarding::end();
	}

	onPermanentlyDestroyed();
	unload();

	//-- Ensure state is set to initialized (i.e. have received end baselines or
	//   created on this server).  This is required so that the AlterScheduler
	//   can accept the Object.  If changing initialization state, warn about it here.
	if (!isInitialized())
	{
		DEBUG_WARNING(true, ("ServerObject::permanentlyDestroy(): object is not initialized, setting to initialized to allow submission to alter scheduler for delete. id=[%s],template=[%s].", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		setInitialized(true);
	}

	//-- Ensure this object gets altered next frame so that kill() can take effect.
	AlterScheduler::submitForAlter(*this);

	return true;
}

// ----------------------------------------------------------------------

/**
 * Allows an object to do pre-deletion cleanup.
 */
void ServerObject::onRemovingFromWorld()
{
	// If we're being removed from the world, we may need to remove ourselves from the parent container list.
 	if (isAuthoritative() && isPlayerControlled() && isUnloading())
 	{
 		Object * const parent = ContainerInterface::getContainedByObject(*this);
 		if (parent)
 		{
 			Container * const container = parent->getCellProperty();
 			if (container)
 			{
 				container->internalItemRemoved(*this);
 			}
 		}
 	}

	if (m_networkUpdateFar)
		onRemovedFromWorldCell();

	destroyTriggerVolumes();
}

// ----------------------------------------------------------------------
/* Why would I unload something?
 * - It might be a player character logging out
 * - Or the planet server might be telling me to unload an object */
// ----------------------------------------------------------------------

void ServerObject::unload()
{
	scheduleForAlter();
	addObjectToConcludeList();

	// prevent recursion
	if (isUnloading())
	{
		kill();
		return;
	}

	setUnloading(true);

	DEBUG_REPORT_LOG(ConfigServerGame::getLogObjectLoading(), ("Unloading object (%s : %s)\n", getObjectTemplateName(), getNetworkId().getValueString().c_str()));

	// If the object has a client, uncontrol it.  When the primary controlled
	// object is unloaded, this prevents us from sending destroys to ourself,
	if (getClient())
		getClient()->removeControlledObject(*this);

	synchronizationOnUnload();

	if (isAuthoritative())
	{
		ScriptParams params;
		IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_UNLOADED_FROM_MEMORY, params));
	}

	kill();

	// if this object is in a slot in a slotted container, immediately remove
	// it from the slot, because another object may be immediately put into
	// the slot before this object is eventually destroyed and removed from the container
	Object * const containedBy = ContainerInterface::getContainedByObject(*this);
	if (containedBy)
	{
		Container * const container = ContainerInterface::getContainer(*containedBy);
		SlottedContainer * const slotContainer = dynamic_cast<SlottedContainer *>(container);
		if (slotContainer)
			slotContainer->removeItemFromSlotOnly(*this);
	}
}

// ----------------------------------------------------------------------

/**
 * Unload the object, and tell the database to move it to a specified player
 */
void ServerObject::moveToPlayerAndUnload(const NetworkId &player)
{
	GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const moveToPlayerMessage(
		"MoveToPlayer",
		std::make_pair(getNetworkId(), player));
	GameServer::getInstance().sendToDatabaseServer(moveToPlayerMessage);

	unload();
}


// ----------------------------------------------------------------------

/**
 * Unload the object, and tell the database to move it to a specified player's bank
 */
void ServerObject::moveToPlayerBankAndUnload(const NetworkId &player)
{
	GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const moveToPlayerMessage(
		"MoveToPlayerBankMessage",
		std::make_pair(getNetworkId(), player));
	GameServer::getInstance().sendToDatabaseServer(moveToPlayerMessage);

	unload();
}

// ----------------------------------------------------------------------

/**
 * Unload the object, and tell the database to move it to a specified player's bank
 */
void ServerObject::moveToPlayerDatapadAndUnload(const NetworkId &player, int maxDepth)
{
	GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, int> > const moveToPlayerMessage(
		"MoveToPlayerDatapadMessage",
		std::make_pair(std::make_pair(getNetworkId(), player), maxDepth));
	GameServer::getInstance().sendToDatabaseServer(moveToPlayerMessage);

	unload();
}

// ----------------------------------------------------------------------

void ServerObject::onPermanentlyDestroyed()
{
	IGNORE_RETURN(ContainerInterface::onObjectDestroy(*this));
}

// ------------------------------------------------------------------------------------------


/**
 * Request to load a specific object contained by this object.
 * (This object must be a demand-load container, i.e. m_loadContents==false.)
 */

void ServerObject::loadContainedObjectFromDB(const NetworkId &oid) const
{
	LoadContainedObjectMessage const msg(getNetworkId(), oid);
	GameServer::getInstance().sendToDatabaseServer(msg);
}

// ----------------------------------------------------------------------

bool ServerObject::areContentsLoaded() const
{
	return m_contentsLoaded.get();
}

// ----------------------------------------------------------------------

void ServerObject::loadAllContentsFromDB() const
{
	if (m_contentsLoaded.get() || m_contentsRequested.get())
	{
		DEBUG_REPORT_LOG(true,("Ignoring LoadAllContentsFromDB for object %s because they are already loaded\n",getNetworkId().getValueString().c_str()));
		return;
	}

	DEBUG_REPORT_LOG(true,("LoadAllContentsFromDB, object %s\n",getNetworkId().getValueString().c_str()));

	if (m_loadContents.get())
		WARNING_STRICT_FATAL(true,("Requested loading contents of container %s, but the container is not a demand-load container.",getNetworkId().getValueString().c_str()));
	else
	{
		m_contentsRequested = true;
		LoadContentsMessage const msg(getNetworkId());
		GameServer::getInstance().sendToDatabaseServer(msg);
	}
}

//----------------------------------------------------------------------

void ServerObject::setLoadContents(bool loadContents)
{
	if (isAuthoritative())
	{
		m_loadContents = loadContents;
	}
	else
	{
		// this function should probably never be called on a proxy object
		WARNING_STRICT_FATAL(true,("Setting loadContent across game servers is not allowed."));
	}
}

//----------------------------------------------------------------------

void ServerObject::unloadPersistedContents()
{
	if (!areContentsLoaded())
	{
		return;
	}

	Container *container = ContainerInterface::getContainer(*this);
	if (container)
	{
		for (ContainerIterator j = container->begin(); j != container->end(); ++j)
		{
			ServerObject * obj = safe_cast<ServerObject *>((*j).getObject());
			if (obj && obj->isPersisted())
			{
				obj->unload();
			}
		}
		m_contentsLoaded = false;
		m_contentsRequested = false;
	}
}

//----------------------------------------------------------------------

const Unicode::String ServerObject::getAssignedObjectFirstName () const
{
	const Unicode::String & name = m_objectName.get ();
	// split off the last name from toName
	std::string::size_type spacePos = name.find(' ');
	if (spacePos != std::string::npos)
		return name.substr(0, spacePos);

	return name;
}

//----------------------------------------------------------------------

/**
 * This function is used to obtain a client-displayable name regardless of whether
 * the object has an assigned name or string id.
 */

const Unicode::String ServerObject::getEncodedObjectName           () const
{
	static const Unicode::String at (1, '@');

	if (!m_objectName.get ().empty ())
		return m_objectName.get ();
	else
		return at + Unicode::narrowToWide (m_nameStringId.get().getCanonicalRepresentation ());
}

// ----------------------------------------------------------------------

/**
 * Do a money transfer and send the reply to a script
 * @param target Target of the money transfer
 * @param replyTo Object that wants the reply message (this object does not need to be involved in the transaction)
 * @param successCallback Script method on the object to call on success.
 * @param failCallback Script method on the object to call on failure.
 * @param packedDictionary Dictionary to be passed to the script (packed into a string)
 */
void ServerObject::scriptTransferCashTo(const NetworkId &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = transferCashTo(target, amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_cashTransfer, target, "", amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

/**
 * Do a money transfer and send the reply to a script
 * @param target Target of the money transfer
 * @param replyTo Object that wants the reply message (this object does not need to be involved in the transaction)
 * @param successCallback Script method on the object to call on success.
 * @param failCallback Script method on the object to call on failure.
 * @param packedDictionary Dictionary to be passed to the script (packed into a string)
 */
void ServerObject::scriptTransferBankCreditsTo(const NetworkId &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = transferBankCreditsTo(target, amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_bankTransfer, target, "", amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void ServerObject::scriptWithdrawCashFromBank(int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = withdrawCashFromBank(amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_bankWithdrawal, NetworkId::cms_invalid, "", amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void ServerObject::scriptDepositCashToBank(int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = depositCashToBank(amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_bankDeposit, NetworkId::cms_invalid, "", amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

/**
 * Do a money transfer and send the reply to a script
 * @param target Target of the money transfer
 * @param replyTo Object that wants the reply message (this object does not need to be involved in the transaction)
 * @param successCallback Script method on the object to call on success.
 * @param failCallback Script method on the object to call on failure.
 * @param packedDictionary Dictionary to be passed to the script (packed into a string)
 */
void ServerObject::scriptTransferBankCreditsTo(const std::string &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = transferBankCreditsTo(target, amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_bankTransferToNamedAccount, NetworkId::cms_invalid, target, amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void ServerObject::scriptTransferBankCreditsFrom(const std::string &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary)
{
	if (isAuthoritative())
	{
		bool result = transferBankCreditsFrom(target, amount);
		if (result)
		{
			if (!successCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,successCallback,packedDictionary,0,true);
		}
		else
		{
			if (!failCallback.empty())
			MessageToQueue::getInstance().sendMessageToJava(replyTo,failCallback,packedDictionary,0,true);
	}
	}
	else
	{
		MessageQueueScriptTransferMoney *msg = new MessageQueueScriptTransferMoney(MessageQueueScriptTransferMoney::TT_bankTransferFromNamedAccount, NetworkId::cms_invalid, target, amount, replyTo, successCallback, failCallback, packedDictionary);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(CM_scriptTransferMoney,0,msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

//----------------------------------------------------------------------

void ServerObject::getAttributes(const NetworkId & playerId, AttributeVector &data) const
{
	getAttributes (data);

	if (!isAuthoritative())
	{
		DEBUG_WARNING (true, ("ServerObject::getAttributes invoked on non-auth object %s", getNetworkId ().getValueString ().c_str ()));
		return;
	}

	GameScriptObject * const gso = const_cast<GameScriptObject *>(getScriptObject());

	if (gso->hasTrigger (Scripting::TRIG_GET_ATTRIBS))
	{
		const CreatureObject * player  = safe_cast<const CreatureObject *>(ServerWorld::findObjectByNetworkId(playerId));
		int MAX_ATTRIBS = 80;
		if(player)
		{
			if(player->getClient())
			{
				if(player->getClient()->isGod())
				{
					MAX_ATTRIBS = 2000;
				}
			}
		}

		// allow scripts to add attribute data to the vector
		static std::vector<const char *>            names   (MAX_ATTRIBS, static_cast<const char *>(0));
		static std::vector<const Unicode::String *> attribs (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

		//-- clear the vectors
		names.clear    ();
		attribs.clear  ();
		names.resize   (MAX_ATTRIBS, static_cast<const char *>(0));
		attribs.resize (MAX_ATTRIBS, static_cast<const Unicode::String *>(0));

		ScriptParams params;
		params.addParam (playerId);
		params.addParam (names);
		params.addParam (attribs);

		NOT_NULL (gso);

		IGNORE_RETURN(gso->trigAllScripts(Scripting::TRIG_GET_ATTRIBS, params));

		const std::vector<const char *> & newNames              = params.getStringArrayParam  (1);
		const std::vector<const Unicode::String *> & newAttribs = params.getUnicodeArrayParam (2);

		// copy the returned strings to data
		for (int i = 0; i < MAX_ATTRIBS; ++i)
		{
			const char * const            newName   = newNames   [i];
			const Unicode::String * const newAttrib = newAttribs [i];

			if (newName && *newName && newAttrib && !newAttrib->empty ())
			{
				data.push_back (std::make_pair (std::string (newName), *newAttrib));
			}
			else
				break;
		}

		// if the requestor is a gm, add objvars and scriptvars
		// to result data (ignore max attribs as well!)

	}
}

// ----------------------------------------------------------------------

void ServerObject::getAttributes(AttributeVector &) const
{
}

// ----------------------------------------------------------------------

/**
 * Return all the standard attributes for an item, plus special
 * auction-specific attributes.
 *
 * May eventually make this into a virtual function if we add more to it.
 */
void ServerObject::getAttributesForAuction(AttributeVector &data) const
{
	static const std::string attributeClientSharedTemplateName("ClientSharedTemplateName");

	DEBUG_FATAL(strlen(getClientSharedTemplateName()) > 1000,("Client shared template name was more than 1000 characters long, too long to save."));
	
	getAttributes(NetworkId::cms_invalid,data);
	data.push_back(std::make_pair(attributeClientSharedTemplateName, Unicode::narrowToWide(getClientSharedTemplateName())));
}

// ----------------------------------------------------------------------

bool ServerObject::alwaysSendReliableTransform() const
{
	return true;
}

// ----------------------------------------------------------------------

/**
 * Called when a request to load a contained object has been completed.
 *
 * Two Important Notes:
 * 1)  This is called in response to loadContainedObjectFromDB.  It is not
 *     called in response to loadAllContentsFromDB
 * 2)  There is no guarantee the requested object exists.  If the object
 *     id was wrong or the object wasn't in the container, it will not
 *     have been loaded.
 */
void ServerObject::onContainedObjectLoaded(NetworkId const &oid)
{
	CommoditiesMarket::checkPendingLoads(oid);
}

// ----------------------------------------------------------------------

bool ServerObject::isBazaarTerminal() const
{
	SharedObjectTemplate::GameObjectType got  =
		static_cast<SharedObjectTemplate::GameObjectType> (getGameObjectType());
	return (got == SharedObjectTemplate::GOT_terminal_bazaar);
}

// ----------------------------------------------------------------------

bool ServerObject::isVendor() const
{
	const TangibleObject *tangibleObject = asTangibleObject();
	if (tangibleObject)
	{
		return tangibleObject->hasCondition(ServerTangibleObjectTemplate::C_vendor);
	}
	return false;
}

// ----------------------------------------------------------------------

void ServerObject::makeVendor()
{
	CreatureObject *creatureObject = asCreatureObject();
	if (creatureObject)
	{
		const char* s_inventoryTemplate = "object/tangible/inventory/vendor_inventory.iff";
		std::set<Client *> inventoryObservers;
		ServerObject *inventory = creatureObject->getInventory();
		if (inventory)
		{
			// if there's already a vendor inventory on the creature,
			// then it means this creature is already a vendor; we
			// don't want to destroy that vendor inventory and create
			// another vendor inventory, because the vendor inventory
			// is already registered with the commodities server and
			// may have items in it, and deleting the vendor inventory
			// would cause those items to be orphaned; 
			
			// doing what we are doing here will cause the existing
			// vendor inventory to be registered with the commodities
			// server again, but it won't hurt, and it covers the
			// recovery case where if for some reason the first
			// registration failed, we will try the registration again
			if(strcmp(inventory->getTemplateName(), s_inventoryTemplate) == 0)
			{
				LOG("CustomerService", ("vendor: Calling makeVendor() for vendor (%s) who already has a vendor inventory object (%s)", getDebugInformation().c_str(), inventory->getDebugInformation().c_str()));
				return;
			}

			// make a copy of the current inventory observers
			inventoryObservers = inventory->getObservers();

			// destroy the existing inventory
			LOG("CustomerService", ("vendor: makeVendor() for vendor (%s), destroying existing inventory object (%s)", getDebugInformation().c_str(), inventory->getDebugInformation().c_str()));

			inventory->permanentlyDestroy(DeleteReasons::Replaced);
		}
		const SlotId invenSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("inventory"));
		ServerObject *vendorInventory = ServerWorld::createNewObject(std::string(s_inventoryTemplate), *this, invenSlot, true);
		if (vendorInventory)
		{
			LOG("CustomerService", ("vendor: makeVendor() for vendor (%s), created vendor inventory object (%s)", getDebugInformation().c_str(), vendorInventory->getDebugInformation().c_str()));

			vendorInventory->setLoadContents(false);

			// make sure that the new inventory object has
			// the same observers as the old inventory object
			ObserveTracker::onMakeVendorInventory(*vendorInventory, inventoryObservers);
		}
		else
		{
			LOG("CustomerService", ("vendor: makeVendor() for vendor (%s), failed to create vendor inventory object", getDebugInformation().c_str()));
		}
	}
	else
	{
		setLoadContents(false);
	}
}

// ----------------------------------------------------------------------

ServerObject *ServerObject::getBazaarContainer()
{
	CreatureObject *creatureObject = asCreatureObject();
	if (creatureObject)
	{
		return creatureObject->getInventory();
	}
	else
	{
		return this;
	}
}


// ----------------------------------------------------------------------

const ServerObject *ServerObject::getBazaarContainer() const
{
	const CreatureObject *creatureObject = asCreatureObject();
	if (creatureObject)
	{
		return creatureObject->getInventory();
	}
	else
	{
		return this;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a request to load a contained object has been completed.
 *
 * Two Important Notes:
 * 1)  This is called in response to loadAllContentsFromDB.  It is not
 *     called in response to loadContainedObjectFromDB
 * 2)  There is no guarantee the container now contains any objects.
 *     If there were no contained objects in the database, the container
 *     will be empty.
 */
void ServerObject::onAllContentsLoaded()
{
	GameServer::removePendingLoadRequest(getNetworkId());

	m_contentsLoaded=true;
	if (getObjVars().hasItem(CommoditiesMarket::OBJVAR_VENDOR_REINITIALIZING))
	{
		//vendor is re-initializing
		setLoadContents( false );
		CommoditiesMarket::reinitializeVendorContents(*this);
	}
	else if (getObjVars().hasItem(OBJVAR_STRUCTURE_PLAYER_PACK))
	{
		//Get player initiating load
		NetworkId playerId;

		if (getObjVars().getItem(OBJVAR_STRUCTURE_PLAYER_PACK, playerId))
		{
			bool failed = true;

			if (isAuthoritative())
			{
				ServerObject *player = nullptr;
				if ((player = getServerObject(playerId)) != nullptr)
				{
					//Tell scripts we are loaded
					ScriptParams params;
					params.addParam( getNetworkId() );
					IGNORE_RETURN(player->getScriptObject()->trigAllScripts(Scripting::TRIG_BUILDING_CONTENTS_LOADED, params));
					failed = false;
				}
			}

			if (failed)
			{
				// fail pack-up as authority lost on player - notify script
				MessageToQueue::getInstance().sendMessageToJava(playerId, "handleFailedStructurePackup", std::vector<int8>(), 0, false);
			}
		}

		//remove the vendor reinitialization flag if it exists
		removeObjVarItem(OBJVAR_STRUCTURE_PLAYER_PACK);
	}
	else
	{
		ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*this));
		if (containedBy)
		{
			Client * const client = containedBy->getClient();
			if (client)
			{
				CreatureObject const * const containedByCreature = containedBy->asCreatureObject();
				if (containedByCreature && containedByCreature->getBankContainer() == this)
				{
					if (containedBy->getLoadCTSBank())
					{
						LOG("CustomerService", ("CharacterTransfer: serverObject has LoadCTSBank flag set, sending ReplyBankCTSLoaded message"));
						GenericValueTypeMessage<NetworkId> setFromCTSLoadBank("ReplyBankCTSLoaded", containedBy->getNetworkId());
						client->sendToConnectionServer(setFromCTSLoadBank);
					}
					else
						client->openContainer(*this, 0, std::string());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool ServerObject::immediateLogoutAllowed()
{
	return true;
}

// ----------------------------------------------------------------------

void ServerObject::disconnect()
{
	if (!isAuthoritative())
		sendControllerMessageToAuthServer(CM_disconnect, 0);
	else if (getClient())
		GameServer::getInstance().dropClient(getNetworkId());
}

// ----------------------------------------------------------------------

void ServerObject::handleDisconnect(bool immediate)
{
	if (isPlayerControlled())
	{
		// client has disconnected, log pertinent information about the play session
		CreatureObject * const creatureObject = asCreatureObject();
		PlayerObject * playerObject = nullptr;
		if (creatureObject)
		{
			playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
			if (playerObject && (playerObject->getSessionStartPlayTime() != 0))
			{
				char buffer[128];
				std::string logText;

				// leading delimiter
				logText += "|";

				// character name
				logText += Unicode::wideToNarrow(getObjectName());
				logText += "|";

				// character id
				logText += getNetworkId().getValueString();
				logText += "|";

				// character birth date
				// get the base date of Jan 1st, 2001
				struct tm baseTimeData;
				baseTimeData.tm_hour = 0;
				baseTimeData.tm_isdst = 0;
				baseTimeData.tm_mday = 1;
				baseTimeData.tm_min = 0;
				baseTimeData.tm_mon = 0;
				baseTimeData.tm_sec = 0;
				baseTimeData.tm_wday = 0;
				baseTimeData.tm_yday = 0;
				baseTimeData.tm_year = 101;
				time_t baseTime = mktime(&baseTimeData);

				// the birth date is the number of days since Jan 1st, 2001
				baseTime += static_cast<time_t>(playerObject->getBornDate() * 60 * 60 * 24);

				struct tm * birthDateTimeData = gmtime(&baseTime);

				snprintf(buffer, sizeof(buffer)-1, "%d%02d%02d", (birthDateTimeData->tm_year + 1900), (birthDateTimeData->tm_mon + 1), birthDateTimeData->tm_mday);
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// station id
				snprintf(buffer, sizeof(buffer)-1, "%lu", playerObject->getStationId());
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// session play time
				logText += CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(playerObject->getSessionPlayTimeDuration()));
				logText += "|";

				// session active play time
				logText += CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(playerObject->getSessionActivePlayTimeDuration()));
				logText += "|";

				// session activity
				snprintf(buffer, sizeof(buffer)-1, "%lu", playerObject->getSessionActivity());
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// scene name
				logText += ServerWorld::getSceneId();
				logText += "|";

				// container id (0 if in world cell)
				Object const * const container  = ContainerInterface::getContainedByObject(*this);
				snprintf(buffer, sizeof(buffer)-1, "%s", (container ? container->getNetworkId().getValueString().c_str() : "0"));
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// x,y,z in container
				Vector const posContainer = getPosition_p();
				snprintf(buffer, sizeof(buffer)-1, "%d %d %d", static_cast<int>(posContainer.x), static_cast<int>(posContainer.y), static_cast<int>(posContainer.z));
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// x,y,z in world
				Vector const posWorld = getPosition_w();
				snprintf(buffer, sizeof(buffer)-1, "%d %d %d", static_cast<int>(posWorld.x), static_cast<int>(posWorld.y), static_cast<int>(posWorld.z));
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// profession
				logText += playerObject->getSkillTemplate();
				logText += "|";

				// level
				snprintf(buffer, sizeof(buffer)-1, "%d", creatureObject->getLevel());
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// current quest id
				snprintf(buffer, sizeof(buffer)-1, "%lu", playerObject->getCurrentQuest());
				buffer[sizeof(buffer)-1] = '\0';
				logText += buffer;
				logText += "|";

				// current quest name
				if (playerObject->getCurrentQuest() != 0)
				{
					logText += QuestManager::getQuestName(playerObject->getCurrentQuest());
				}
				else
				{
					logText += "none";
				}
				logText += "|";

				// xp
				bool hasLoggedFirstXp = false;
				std::map<std::string, int> const & xp = playerObject->getExperiencePoints();
				for (std::map<std::string, int>::const_iterator iter = xp.begin(); iter != xp.end(); ++iter)
				{
					if (iter->second == 0)
						continue;

					if (hasLoggedFirstXp)
						logText += ",";

					logText += iter->first;
					logText += "=";

					snprintf(buffer, sizeof(buffer)-1, "%d", iter->second);
					buffer[sizeof(buffer)-1] = '\0';
					logText += buffer;

					hasLoggedFirstXp = true;
				}
				logText += "|";

				// log it
				LOG("CustomerService", ("Logout:%s", logText.c_str()));

				// clear the session play time info
				playerObject->setSessionPlayTimeInfo(0, 0, 0);

				// clear the session activity info
				playerObject->clearSessionActivity();
			}
		}

		if (!immediate && !immediateLogoutAllowed())
			LogoutTracker::add(getNetworkId());
		else
		{
			virtualOnLogout();

			if (ConfigServerGame::getLogoutTriggerEnabled())
			{
				ScriptParams params;
				getScriptObject()->trigAllScripts(Scripting::TRIG_LOGOUT, params);
			}

			Client * const client = getClient();
			if (client)
			{
				LogoutMessage const logoutMessage;
				client->send(logoutMessage, true);
			}

			PositionUpdateTracker::flushPositionUpdate(*this);
			LogoutTracker::addPendingSave(this);

			// get rid of the proxies
			clearProxyList();

			// if we're inside a container, it's the topmost parent that contains our proxy
			// list, so the above clearProxyList() call will not get rid of the proxies
			synchronizationOnUnload();

			// don't allow anyone to continue observing this object (and associated PlayerObject)
			if (playerObject)
				ObserveTracker::onObjectDestroyed(*playerObject, false);

			ObserveTracker::onObjectDestroyed(*this, false);

			// remove from world and/or container as necessary but keep containment information available
			if (isInWorld())
				removeFromWorld();
			else
				WARNING(getTriggerVolumeEntered() && !getTriggerVolumeEntered()->empty(), ("ServerObject::handleDisconnect called for (%s) which is not in the world but is contained in one or more trigger volumes", getDebugInformation().c_str()));

			// the object should have been removed from all TriggerVolumes
			// (from removeFromWorld() above) by now, but as a safety measure,
			// we'll do a final check and remove the object from any
			// TriggerVolumes that it is still in
			if (getTriggerVolumeEntered())
			{
				std::set<TriggerVolume *> const triggerVolumeEntered = *(getTriggerVolumeEntered());
				for (std::set<TriggerVolume *>::const_iterator i = triggerVolumeEntered.begin(); i != triggerVolumeEntered.end(); ++i)
				{
					WARNING(true, ("Object (%s) added to LogoutTracker but hasn't been removed from TriggerVolume (%s) owned by (%s)",
						getDebugInformation().c_str(),
						(*i)->getName().c_str(),
						(*i)->getOwner().getDebugInformation().c_str()));

					(*i)->removeObject(*this);
				}
			}

			Object *containerObject = ContainerInterface::getContainedByObject(*this);
			if (containerObject)
			{
				NON_NULL(ContainerInterface::getContainer(*containerObject))->internalItemRemoved(*this);

				// if the character was in a building, tell the building it has lost
				// the player so the building demand load/unload system can work correctly
				Object *buildingObj = ContainerInterface::getTopmostContainer(*containerObject);
				if (buildingObj && buildingObj->isAuthoritative() && buildingObj->asServerObject() && buildingObj->asServerObject()->asBuildingObject())
					buildingObj->asServerObject()->asBuildingObject()->lostPlayer(*this);
			}

			// remove all sui pages for character
			ServerUIManager::onPlayerLogout(getNetworkId());
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::setAttributeAttained(int attribute)
{
	BitArray tmp(m_attributesAttained.get());
	tmp.setBit(attribute);
	m_attributesAttained = tmp;
}

// ----------------------------------------------------------------------

void ServerObject::clearAttributeAttained(int attribute)
{
	BitArray tmp(m_attributesAttained.get());
	tmp.clearBit(attribute);
	m_attributesAttained = tmp;
}

// ----------------------------------------------------------------------

bool ServerObject::hasAttributeAttained(int attribute) const
{
	return m_attributesAttained.get().testBit(attribute);
}

// ----------------------------------------------------------------------

void ServerObject::setAttributeInterested(int attribute)
{
	BitArray tmp(m_attributesInterested.get());
	tmp.setBit(attribute);
	m_attributesInterested = tmp;
}

// ----------------------------------------------------------------------

void ServerObject::clearAttributeInterested(int attribute)
{
	BitArray tmp(m_attributesInterested.get());
	tmp.clearBit(attribute);
	m_attributesInterested = tmp;
}

// ----------------------------------------------------------------------

bool ServerObject::hasAttributeInterest(int attribute) const
{
	return m_attributesInterested.get().testBit(attribute);
}

// ----------------------------------------------------------------------

bool ServerObject::isInterested(const ServerObject &rhs) const
{
	return m_attributesInterested.get().matchAnyBit(rhs.m_attributesAttained.get());
}

// ----------------------------------------------------------------------

std::string ServerObject::getItemLog() const
{
	return getNetworkId().getValueString() + ' ' + Unicode::wideToNarrow(getObjectName());
}

// ----------------------------------------------------------------------

void ServerObject::checkAndLogInvalidTransform() const
{
	Transform const &transform_o2p = getTransform_o2p();
	bool const isValid = transform_o2p.validate(true);

	if (!isValid)
	{
		Object const *const container  = ContainerInterface::getContainedByObject(*this);
		Vector const position          = transform_o2p.getPosition_p();
		Vector const containerPosition = container ? container->getTransform_o2w().getPosition_p() : Vector::zero;

		WARNING(!isValid,
			("transform is invalid for object id=[%s], object template name=[%s], server id=[%d], server scene id=[%s], position=[%.2f,%.2f,%.2f], container id=[%s], container position=[%.2f,%.2f,%.2f]",
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(),
			static_cast<int>(GameServer::getInstance().getProcessId()),
			ServerWorld::getSceneId().c_str(),
			position.x,
			position.y,
			position.z,
			container ? container->getNetworkId().getValueString().c_str() : "<not contained>",
			containerPosition.x,
			containerPosition.y,
			containerPosition.z
			));
	}
}

// ----------------------------------------------------------------------

ServerObject * ServerObject::combineResourceContainers(ServerObject &item)
{
	if (isVendor() || isBazaarTerminal() || isInBazaarOrVendor() )
	{
		//Do not combine resources on vendors or bazaars, as they need
		//to remain separate to sell separately.
		return &item;
	}

	ServerObject * returnedItem = &item;
	typedef std::vector<ResourceContainerObject *> SimilarContainers;
	static SimilarContainers similarContainers;

	if (item.getObjectType() == ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag)
	{
		ResourceContainerObject *resourceContainerObject = safe_cast<ResourceContainerObject *>(&item);

		if (resourceContainerObject != nullptr)
		{
			// If this is a container, see if it contains another resource container of the same type

			Container *container = ContainerInterface::getContainer(*this);

			if (container != nullptr)
			{
				ContainerIterator iterContainer = container->begin();

				for (; iterContainer != container->end(); ++iterContainer)
				{
					Object *containedObject = NetworkIdManager::getObjectById(*iterContainer);

					// Make sure the contained item is not the item we are gaining and that
					// the item is a resource container

					if ((containedObject != &item) &&
					    (containedObject != nullptr) &&
					    (containedObject->getObjectType() == ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag))
					{
						ResourceContainerObject *containedResourceContainerObject = safe_cast<ResourceContainerObject *>(containedObject);

						if ((containedResourceContainerObject != nullptr) &&
						    (resourceContainerObject->getResourceType() == containedResourceContainerObject->getResourceType()))
						{
							// This is a container of similar type, if it is not full, save it to the list

							int const quantity = containedResourceContainerObject->getQuantity();
							int const maxQuantity = containedResourceContainerObject->getMaxQuantity();

							if (quantity != maxQuantity)
							{
								similarContainers.push_back(containedResourceContainerObject);
							}
						}
					}
				}
			}
		}

		// Combine the resources as much as possible

		bool done = false;
		int resourcesLeftToAdd = resourceContainerObject->getQuantity();

		while (!done)
		{
			// Find the most full container

			SimilarContainers::iterator iterSimilarContainers = similarContainers.begin();
			SimilarContainers::iterator fullestContainer = similarContainers.end();

			for (; iterSimilarContainers != similarContainers.end(); ++iterSimilarContainers)
			{
				if (fullestContainer == similarContainers.end())
				{
					fullestContainer = iterSimilarContainers;
				}
				else
				{
					int const currentQuantity = (*iterSimilarContainers)->getQuantity();
					int const fullestQuantity = (*fullestContainer)->getQuantity();

					if (currentQuantity > fullestQuantity)
					{
						fullestContainer = iterSimilarContainers;
					}
				}
			}

			// Add as much as we can to the fullest container

			if (fullestContainer != similarContainers.end())
			{
				int const fullestQuantity = (*fullestContainer)->getQuantity();
				int const fullestMaxQuantity = (*fullestContainer)->getMaxQuantity();
				int const amountToAdd = std::min(resourcesLeftToAdd, fullestMaxQuantity - fullestQuantity);

				// Add the amount to the fullest container

				ResourceContainerObject *sourceObj = resourceContainerObject;
				ResourceContainerObject *destObj = *fullestContainer;

				if (sourceObj && destObj && (amountToAdd > 0))
				{
					sourceObj->transferTo(*destObj, amountToAdd);
					if (sourceObj->isBeingDestroyed())
						returnedItem = destObj;
				}

				resourcesLeftToAdd -= amountToAdd;

				// Remove this container from the list

				similarContainers.erase(fullestContainer);
			}
			else
			{
				// There are no more containers to combine to, so we are done

				done = true;
			}

			if (resourcesLeftToAdd <= 0)
			{
				// If there is no more resources left to combine, then we are done

				done = true;
			}
		}

		similarContainers.clear();
	}
	return returnedItem;
}

// ----------------------------------------------------------------------

/**
 * Determines if this object is contained by another object.
 *
 * @param container			the object that might contain this one
 * @param includeContents	if true, return true if one of the container's contents contains us
 *
 * @return true if we are contained by the container (or its contents)
 */
bool ServerObject::isContainedBy(const ServerObject & container, bool includeContents) const
{
	// we actually check what we are contained by instead of checking the
	// container's contents
	const Object * test = ContainerInterface::getContainedByObject(*this);
	if (test == &container)
	{
		// our container is the desired container
		return true;
	}
	else if (test != nullptr && test != this && includeContents)
	{
		// our container isn't the desired container, see if it is contained
		return safe_cast<const ServerObject *>(test)->isContainedBy(container, true);
	}
	return false;
}

// ----------------------------------------------------------------------

void ServerObject::concludeScriptVars()
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::concludeScriptVars");
	GameScriptObject::packAllScriptVarDeltas();
}

// ----------------------------------------------------------------------

void ServerObject::scheduleForAlter()
{
	// ServerObjects do not recurse into containers, so are always responsible for scheduling themselves directly.
	if (isInitialized())
	{
//		DEBUG_REPORT_LOG(true, ("[aitest] Scheduling %s for alter\n", getNetworkId().getValueString().c_str()));
		AlterScheduler::submitForAlter(*this);
	}
}

// ----------------------------------------------------------------------

void ServerObject::onAddedToWorldCell()
{
	createFarNetworkUpdateVolume();
	virtualOnAddedToWorldCell();
	updatePositionOnPlanetServer(true);
}


// ----------------------------------------------------------------------

void ServerObject::virtualOnAddedToWorldCell()
{
}

//------------------------------------------------------------------------------------------

void ServerObject::onRemovedFromWorldCell()
{
	virtualOnRemovedFromWorldCell();
}

//------------------------------------------------------------------------------------------

void ServerObject::virtualOnRemovedFromWorldCell()
{
	destroyFarNetworkUpdateVolume();
}

// ----------------------------------------------------------------------

void ServerObject::virtualOnSetAuthority()
{
}

// ----------------------------------------------------------------------

void ServerObject::virtualOnReleaseAuthority()
{
}

// ----------------------------------------------------------------------

void ServerObject::virtualOnLogout()
{
}

//------------------------------------------------------------------------------------------

bool ServerObject::wantSawAttackTriggers() const
{
	return false;
}

// ----------------------------------------------------------------------

void ServerObject::setTeleportScriptCallback(std::string const &scriptCallback)
{
	setObjVarItem("teleportFixup.callback", scriptCallback);
}

// ----------------------------------------------------------------------

void ServerObject::setInteriorTeleportDestination(NetworkId const &destContainerId, std::string const &destCellName, Vector const &destPosition_p)
{
	LOG("TeleportFixup", ("Setting interior telerport destination for %s: into %f %f %f %s %s\n", getNetworkId().getValueString().c_str(), destPosition_p.x, destPosition_p.y, destPosition_p.z, destCellName.c_str(), destContainerId.getValueString().c_str()));
	removeObjVarItem("teleportFixup");
	setObjVarItem("teleportFixup.x", destPosition_p.x);
	setObjVarItem("teleportFixup.y", destPosition_p.y);
	setObjVarItem("teleportFixup.z", destPosition_p.z);
	setObjVarItem("teleportFixup.container", destContainerId);
	if (!destCellName.empty())
		setObjVarItem("teleportFixup.cellname", destCellName);
}

// ----------------------------------------------------------------------

Vector const &ServerObject::getTriggerPosition() const
{
	return m_oldPosition;
}

// ----------------------------------------------------------------------

bool ServerObject::getTransformChanged() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_TransformChanged);
}

// ----------------------------------------------------------------------

void ServerObject::setTransformChanged(bool c)
{
	setLocalFlag(LocalObjectFlags::ServerObject_TransformChanged, c);
	if (c)
	{
		addObjectToConcludeList();
		if (isAuthoritative() && isPersisted())
			PositionUpdateTracker::positionChanged(*this);
	}
}

// ----------------------------------------------------------------------

bool ServerObject::onContainerChildAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	if (transferer && !checkBankTransfer(this, item, transferer))
		return false;

	Object * const parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		return parent->asServerObject()->onContainerChildAboutToLoseItem(destination, item, transferer);

	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::onContainerChildAboutToGainItem(ServerObject& item, ServerObject* destination, ServerObject* transferer)
{
	if (transferer && !checkBankTransfer(this, item, transferer))
		return false;

	Object * const parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		return parent->asServerObject()->onContainerChildAboutToGainItem(item, destination, transferer);

	return true;
}

// ----------------------------------------------------------------------

void ServerObject::onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	Object * const parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		parent->asServerObject()->onContainerChildLostItem(destination, item, source, transferer);
}

// ----------------------------------------------------------------------

void ServerObject::onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	Object * const parent = ContainerInterface::getContainedByObject(*this);
	if (parent && parent->asServerObject())
		parent->asServerObject()->onContainerChildGainItem(item, source, transferer);
}

// ----------------------------------------------------------------------

void ServerObject::setLoadCTSBank(bool isForCTS)
{
	m_loadCTSBank = isForCTS;
}

// ----------------------------------------------------------------------

void ServerObject::setLoadCTSPackedHouses(bool isForCTS)
{
	m_loadCTSPackedHouses = isForCTS;
}

// ----------------------------------------------------------------------

bool ServerObject::getLoadCTSBank() const
{
	return m_loadCTSBank;
}

// ----------------------------------------------------------------------

bool ServerObject::getLoadCTSPackedHouses() const
{
	return m_loadCTSPackedHouses;
}

// ----------------------------------------------------------------------

void ServerObject::packedHouseLoaded()
{
	// see if we have the objvar
	if (getLoadCTSPackedHouses())
	{
		if (getObjVars().hasItem("cts.packed_house_count"))
		{
			int count = 0;
			if (getObjVars().getItem("cts.packed_house_count", count))
			{
				--count;
				if (count <= 0)
				{
					LOG("CustomerService", ("CharacterTransfer: all houses loaded, responding to connection server."));
					removeObjVarItem("cts.packed_house_count");

					// tell the connection server.
					Client * client = getClient();
					if (client && client->getConnection())
					{
						GenericValueTypeMessage <NetworkId> loadedMsg("PackedHousesLoaded", getNetworkId());
						client->getConnection()->send(loadedMsg, true);
					}
				}
				else
				{
					LOG("CustomerService", ("CharacterTransfer: packed house is loaded, but we expect %d more.", count));
					setObjVarItem("cts.packed_house_count", count);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::setLoadWith(const NetworkId& id)
{
	m_loadWith = id;
}

//-----------------------------------------------------------------------

std::string ServerObject::getLogDescription(const NetworkId & id)
{
	std::string const result = getLogDescription(safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id)));
	if (result.empty())
		return id.getValueString();
	return result;
}

//-----------------------------------------------------------------------

std::string ServerObject::getLogDescription(const ServerObject * source)
{
	std::string desc = "";
	if(source)
	{
		desc += "NonPlayerObject ";
		desc += Unicode::wideToNarrow(source->getObjectName());
		desc += "(";
		desc += source->getNetworkId().getValueString();
		desc += ") ";
		desc += source->getObjectTemplateName();
	}
	return desc;
}

//----------------------------------------------------------------------

void ServerObject::retrieveStoredObjectAttributes (AttributeVector & av) const
{
	typedef std::vector<Unicode::String> UnicodeStringVector;

	static UnicodeStringVector kv;
	static UnicodeStringVector vv;

	kv.clear ();
	vv.clear ();

	getObjVars ().getItem (objvar_item_attribute_keys,   kv);
	getObjVars ().getItem (objvar_item_attribute_values, vv);

	if (kv.size () == vv.size ())
	{
		const size_t size = vv.size ();

		av.reserve (av.size () + kv.size ());

		for (size_t i = 0; i < size; ++i)
		{
			const std::string & key       = Unicode::wideToNarrow (kv [i]);
			const Unicode::String & value = vv [i];

			av.push_back (std::make_pair (key, value));
		}
	}
}

//----------------------------------------------------------------------

void ServerObject::storeObjectAttributes     (const ServerObject & prototype)
{
	typedef std::vector<Unicode::String> UnicodeStringVector;

	static UnicodeStringVector kv;
	static UnicodeStringVector vv;
	static AttributeVector     av;

	av.clear ();
	kv.clear ();
	vv.clear ();

	prototype.getAttributes (NetworkId::cms_invalid, av);

	kv.reserve (av.size());
	vv.reserve (av.size());

	for (AttributeVector::const_iterator it = av.begin (); it != av.end (); ++it)
	{
		const std::string & key       = (*it).first;
		const Unicode::String & value = (*it).second;

		kv.push_back (Unicode::narrowToWide (key));
		vv.push_back (value);
	}

	setObjVarItem (objvar_item_attribute_keys,   kv);
	setObjVarItem (objvar_item_attribute_values, vv);
}

//----------------------------------------------------------------------

void ServerObject::clearStoredObjectAttributes    ()
{
	m_objVars.removeItemByName (objvar_item_attribute_keys);
	m_objVars.removeItemByName (objvar_item_attribute_values);
}

//----------------------------------------------------------------------

void ServerObject::replaceStoredObjectAttribute(Unicode::String const & attribute, Unicode::String const & value)
{
	typedef std::vector<Unicode::String> UnicodeStringVector;

	static UnicodeStringVector kv;
	static UnicodeStringVector vv;

	kv.clear ();
	vv.clear ();

	getObjVars ().getItem (objvar_item_attribute_keys,   kv);
	getObjVars ().getItem (objvar_item_attribute_values, vv);

	if (kv.size () == vv.size ())
	{
		const size_t size = vv.size ();
		for (size_t i = 0; i < size; ++i)
		{
			if (kv [i] == attribute)
			{
				vv [i] = value;
				setObjVarItem (objvar_item_attribute_values, vv);

				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::setInitialized(bool initialized)
{
	setLocalFlag(LocalObjectFlags::ServerObject_Initialized, initialized);
}

// ----------------------------------------------------------------------

void ServerObject::setBeingDestroyed(bool beingDestroyed)
{
	setLocalFlag(LocalObjectFlags::ServerObject_BeingDestroyed, beingDestroyed);

	// set corresponding flag in script
	if (beingDestroyed && isAuthoritative() && getScriptObject())
		getScriptObject()->setOwnerDestroyed();

	// stop listening to broadcast messages
	if (beingDestroyed && isAuthoritative())
		stopListeningToAllBroadcastMessages();
}

// ----------------------------------------------------------------------

void ServerObject::setPlacing(bool placing)
{
	setLocalFlag(LocalObjectFlags::ServerObject_Placing, placing);
}

// ----------------------------------------------------------------------

void ServerObject::setUnloading(bool unloading)
{
	setLocalFlag(LocalObjectFlags::ServerObject_Unloading, unloading);
}

// ----------------------------------------------------------------------

void ServerObject::setGoingToConclude(bool goingToConclude)
{
	setLocalFlag(LocalObjectFlags::ServerObject_GoingToConclude, goingToConclude);
}

// ----------------------------------------------------------------------

void ServerObject::setInEndBaselines(bool inEndBaselines)
{
	setLocalFlag(LocalObjectFlags::ServerObject_InEndBaselines, inEndBaselines);
}

// ----------------------------------------------------------------------

void ServerObject::setNeedsPobFixup(bool needsFixup)
{
	setLocalFlag(LocalObjectFlags::ServerObject_NeedsPobFixup, needsFixup);
}

// ----------------------------------------------------------------------

bool ServerObject::isInitialized() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_Initialized);
}

// ----------------------------------------------------------------------

bool ServerObject::isPlacing() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_Placing);
}

// ----------------------------------------------------------------------

bool ServerObject::isUnloading() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_Unloading);
}

// ----------------------------------------------------------------------

bool ServerObject::isBeingDestroyed() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_BeingDestroyed);
}

// ----------------------------------------------------------------------

bool ServerObject::isGoingToConclude() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_GoingToConclude);
}

// ----------------------------------------------------------------------

bool ServerObject::isInEndBaselines() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_InEndBaselines);
}

// ----------------------------------------------------------------------

bool ServerObject::isNeedingPobFixup() const
{
	return getLocalFlag(LocalObjectFlags::ServerObject_NeedsPobFixup);
}

// ----------------------------------------------------------------------

float ServerObject::getLocationReservationRadius() const
{
	return getSharedTemplate() ? getSharedTemplate()->getLocationReservationRadius() : 0.f;
}

// ----------------------------------------------------------------------

void ServerObject::sendDirtyObjectMenuNotification()
{
	if (getLocalFlag(LocalObjectFlags::ServerObject_DirtyObjectMenuSent))
		return;
	setLocalFlag(LocalObjectFlags::ServerObject_DirtyObjectMenuSent, true);

	Controller * const controller = getController();
	if (controller)
		controller->appendMessage(CM_objectMenuDirty, 0, nullptr, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
}

// ----------------------------------------------------------------------

void ServerObject::sendDirtyAttributesNotification()
{
	if (getLocalFlag(LocalObjectFlags::ServerObject_DirtyAttributesSent))
		return;
	setLocalFlag(LocalObjectFlags::ServerObject_DirtyAttributesSent, true);

	Controller * const controller = getController();
	if (controller)
		controller->appendMessage(CM_attributesDirty, 0, nullptr, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_ALL_CLIENT);
}

//------------------------------------------------------------------------------------------

void ServerObject::setBuildingUpdateRadiusMultiplier(float m)
{
	if (m <= 0)
	{
		ms_buildingUpdateRadiusMultiplier = 1.0f;
		DEBUG_WARNING(true, ("Building Update Radius Multiplier was set to a value (%f) <= 0", m));
	}
	else
		ms_buildingUpdateRadiusMultiplier = m;

}

// ----------------------------------------------------------------------

void ServerObject::triggerMadeAuthoritative()
{
	std::vector<CachedNetworkId> trigObjs;
	getPlayerControlledContents(*this, trigObjs);
	for (std::vector<CachedNetworkId>::const_iterator i = trigObjs.begin(); i != trigObjs.end(); ++i)
	{
		ServerObject *trigObj = safe_cast<ServerObject *>((*i).getObject());
		if (trigObj && trigObj->isAuthoritative())
		{
			ScriptParams params;
			IGNORE_RETURN(trigObj->getScriptObject()->trigAllScripts(Scripting::TRIG_MADE_AUTHORITATIVE, params));
		}
	}
}

//------------------------------------------------------------------------------------------

void ServerObject::setLayer(TerrainGenerator::Layer* layer)
{
	LayerProperty * layerProperty = nullptr;
	Property * property = getProperty(LayerProperty::getClassPropertyId());
	if (property != nullptr)
		layerProperty = safe_cast<LayerProperty *>(property);
	else
		layerProperty = new LayerProperty(*this);

	layerProperty->setLayer(layer);
	if (property == nullptr)
	{
		addProperty(*layerProperty, true);
		ObjectTracker::addRunTimeRule();
		addNotification (ServerObjectTerrainModificationNotification::getInstance ());
	}
}

//------------------------------------------------------------------------------------------

TerrainGenerator::Layer* ServerObject::getLayer() const
{
	const Property * property = getProperty(LayerProperty::getClassPropertyId());
	if (property != nullptr)
	{
		const LayerProperty * layerProperty = safe_cast<const LayerProperty *>(property);
		return layerProperty->getLayer();
	}
	return nullptr;
}

//------------------------------------------------------------------------------------------

void ServerObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if(name == "complexity")
	{
		m_complexity.pack(target);
	}
	else if(name == "bankBalance")
	{
		m_bankBalance.pack(target);
	}
}

//------------------------------------------------------------------------------------------

void ServerObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if(name == "complexity")
	{
		m_complexity.unpackDelta(ri);
	}
	else if(name == "bankBalance")
	{
		m_bankBalance.unpackDelta(ri);
	}
	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

/**
 * Deliver a MessageTo to this object.  If the object is not
 * authoritative, send the message to the authoritative server.
 * If the MessageTo's time has expired, handle it.  If not, enqueue it
 * for later.
 */
void ServerObject::deliverMessageTo(MessageToPayload & message)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerObject::deliverMessageTo");
		
	if (message.getRecurringTime() != 0)
	{
		// Recurring messages can have only one instance each.  Ignore
		// this message if there is already a recurring one with the same method name.
		for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end(); ++i)
		{
			if ((i->second.getRecurringTime() != 0) && (i->second.getMethod() == message.getMethod()))
				return;
		}
	}
	
	if (isAuthoritative())
	{
		if (message.getCallTime() <= MessageToQueue::getInstance().getEffectiveMessageToTime())
		{
			// Message is already due, handle it right away
			handleMessageTo(message);
		}
		else
		{
			if (isPersisted() && message.getGuaranteed() && !message.getPersisted())
			{
				// Message is guaranteed.  Since it's being delayed, make sure the DB saves it
				// in case of a server crash
				message.setPersisted(true);
				MessageToMessage const msg(message, GameServer::getInstance().getProcessId());
				GameServer::getInstance().sendToDatabaseServer(msg);	
			}
			if (m_messageTos.empty() || message.getCallTime() < m_messageTos.begin()->second.getCallTime())
				MessageToQueue::getInstance().addToScheduler(*this, message.getCallTime());
			pushMessageTo(message);
		}
	}
	else
	{
		message.addBounceServer(GameServer::getInstance().getProcessId());
		ServerMessageForwarding::begin(getAuthServerProcessId());
		ServerMessageForwarding::send(MessageToMessage(message, GameServer::getInstance().getProcessId()));
		ServerMessageForwarding::end();
	}
}

// ----------------------------------------------------------------------

/**
 * Add a MessageTo to the sorted vector of pending messages
 *
 * Does a binary search to find the appropriate insertion point.
 */
void ServerObject::pushMessageTo(MessageToPayload const & newMessage)
{
	m_messageTos.set(std::make_pair(std::make_pair(newMessage.getCallTime(), newMessage.getCounterMessageToPayload()), newMessage.getMessageId()), newMessage);
}

// ----------------------------------------------------------------------

int ServerObject::cancelMessageTo(std::string const & messageName)
{
	int removeCount;

	if (isAuthoritative())
	{
		removeCount = 0;

		for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end();)
		{
			if ((i->second.getMethod() == messageName) && (i->first != s_currentMessageToBeingHandled))
			{
				i = m_messageTos.erase(i);
				++removeCount;
			}
			else
				++i;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_cancelMessageTo,
			new MessageQueueGenericValueType<std::string>(messageName));

		// -1 means request has been forward to authoritative server for processing
		removeCount = -1;
	}

	return removeCount;
}

// ----------------------------------------------------------------------

int ServerObject::cancelMessageToByMessageId(NetworkId const & messageId)
{
	int removeCount;

	if (isAuthoritative())
	{
		removeCount = 0;

		for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end();)
		{
			if ((i->second.getMessageId() == messageId) && (i->first != s_currentMessageToBeingHandled))
			{
				i = m_messageTos.erase(i);
				++removeCount;
				break;
			}
			else
				++i;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_cancelMessageTo,
			new MessageQueueGenericValueType<std::string>(messageId.getValueString()));

		// -1 means request has been forward to authoritative server for processing
		removeCount = -1;
	}

	return removeCount;
}

// ----------------------------------------------------------------------
// returns the number of seconds until the messageTo fires, which *CAN* be 0
// returns -1 if object doesn't have the messageTo
int ServerObject::timeUntilMessageTo(std::string const & messageName) const
{
	for (Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload>::const_iterator i=m_messageTos.begin(); i!=m_messageTos.end(); ++i)
	{
		if (i->second.getMethod() == messageName)
		{
			if ((i->first != s_currentMessageToBeingHandled) || (i->second.getRecurringTime() > 0))
			{
				unsigned long const now = ServerClock::getInstance().getGameTimeSeconds();
				unsigned long const callTime = i->second.getCallTime();
				if (callTime > now)
					return static_cast<int>(callTime - now);

				return 0;
			}
		}
	}

	return -1; // doesn't have the messageTo
}

// ----------------------------------------------------------------------

/**
 * Sets the root node for a patrol path node.
 *
 * @param root		the root node
 *
 * NOTE: Does not work multiserver!
 */
void ServerObject::setPatrolPathRoot(const ServerObject & root)
{
	if (!isAuthoritative())
	{
		WARNING(true, ("ServerObject::setPatrolPathRoot called on non-authoritative object %s for %s",
			getNetworkId().getValueString().c_str(), root.getNetworkId().getValueString().c_str()));
		return;
	}

	// make sure we aren't already a root node
	Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		if (root.getNetworkId() != getNetworkId())
		{
			WARNING(true, ("ServerObject::setPatrolPathRoot trying to set root node %s on node %s that is already a root",
				root.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str()));
		}
		return;
	}

	// see if we are going to be the root node
	if (root.getNetworkId() == getNetworkId())
	{
		p = new PatrolPathRootProperty(*this);
		addProperty(*p, true);
	}
	else
	{
		// add the root node to our node list
		p = getProperty(PatrolPathNodeProperty::getClassPropertyId());
		if (p == nullptr)
		{
			p = new PatrolPathNodeProperty(*this);
			addProperty(*p, true);
		}
		else if (safe_cast<PatrolPathNodeProperty *>(p)->hasRoot(root))
			return;
		safe_cast<PatrolPathNodeProperty *>(p)->addRoot(root);

		if (!root.isPatrolPathRoot())
		{
			// the root has not been set up, do it now
			WARNING(true, ("ServerObject::setPatrolPathRoot for node %s: root node %s was not initialized, doing it now (THIS SHOULD NEVER HAPPEN)",
				getNetworkId().getValueString().c_str(), root.getNetworkId().getValueString().c_str()));
			const_cast<ServerObject &>(root).setPatrolPathRoot(root);
		}
	}

	// we need to add any players in range to our path observer count
	TriggerVolume * triggerVolume = getNetworkTriggerVolume();
	if (triggerVolume != nullptr)
	{
		TriggerVolume::ContentsSet const & contents = triggerVolume->getContents();
		for (TriggerVolume::ContentsSet::const_iterator i = contents.begin(); i != contents.end(); ++i)
		{
			if (*i != nullptr)
			{
				Client * client = (*i)->getClient();
				if (client != nullptr)
				{
					if (!ObserveTracker::isObserving(*client, *this))
						ObserveTracker::onClientEnteredNetworkTriggerVolume(*client, *triggerVolume);
					else
					{
						// note: we call addPatrolPathObserver() on the root instead of us so 
						// that we don't update the count of any paths we already belong to
						const_cast<ServerObject &>(root).addPatrolPathObserver();
					}
				}
			}
		}
	}
}	// ServerObject::setPatrolPathRoot

// ----------------------------------------------------------------------

/**
 * Returns the root node for a patrol path node.
 *
 * @return the root node, or nullptr if this isn't a patrol path node
 */
const std::set<CachedNetworkId> & ServerObject::getPatrolPathRoots() const
{
static const std::set<CachedNetworkId> noRoots;

	const Property * p = getProperty(PatrolPathNodeProperty::getClassPropertyId());
	if (p != nullptr)
	{
		return safe_cast<const PatrolPathNodeProperty *>(p)->getRoots();
	}
	return noRoots;
}	// ServerObject::getPatrolPathRoot

// ----------------------------------------------------------------------

/**
 * Adds an object that is moving along a patrol path where this object is the 
 * root node. If our path goes from being unobserved to observed, all the objects 
 * using this path will be scheduled for alter.
 * 
 * @param ai	the object that's on our path
 *
 * NOTE: Does not work multiserver!
 */
void ServerObject::addPatrolPathingObject(const ServerObject & ai)
{
	if (!isAuthoritative())
	{
		WARNING(true, ("ServerObject::addPatrolPathingObject called on non-authoritative object %s for %s",
			getNetworkId().getValueString().c_str(), ai.getNetworkId().getValueString().c_str()));
		return;
	}

	Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		safe_cast<PatrolPathRootProperty *>(p)->addPatrollingObject(ai);
		DEBUG_REPORT_LOG(true, ("[patrolpath] add patrol ai %s to root %s\n", 
			ai.getNetworkId().getValueString().c_str(),
			getNetworkId().getValueString().c_str()));
	}
	else
	{
		// if we are not a root, try to add the ai to our root (but only do so if we have
		// one root)
		DEBUG_WARNING(true, ("ServerObject::addPatrolPathingObject trying to add an ai %s to a non-root path node %s, going to use our root",
			ai.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str()));
		const std::set<CachedNetworkId> & roots = getPatrolPathRoots();
		if (roots.size() == 1)
		{
			ServerObject * root = safe_cast<ServerObject *>(roots.begin()->getObject());
			if (root != nullptr)
				root->addPatrolPathingObject(ai);
		}
		else
		{
			WARNING(true, ("ServerObject::addPatrolPathingObject trying to add an ai %s to a non-root path node %s which has muliple roots.",
				ai.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str()));
		}
	}
}	// ServerObject::addPatrolPathingObject

// ----------------------------------------------------------------------

/**
 * Removes an object that is moving along our patrol path.
 *
 * @param ai	the object to remove
 *
 * NOTE: Does not work multiserver!
 */
void ServerObject::removePatrolPathingObject(const ServerObject & ai)
{
	if (!isAuthoritative())
	{
		WARNING(true, ("ServerObject::removePatrolPathingObject called on non-authoritative object %s for %s",
			getNetworkId().getValueString().c_str(), ai.getNetworkId().getValueString().c_str()));
		return;
	}

	Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		safe_cast<PatrolPathRootProperty *>(p)->removePatrollingObject(ai);
		DEBUG_REPORT_LOG(true, ("[patrolpath] remove patrol ai %s from root %s\n", 
			ai.getNetworkId().getValueString().c_str(),
			getNetworkId().getValueString().c_str()));
	}
	else
	{
		// if we are not a root, try to remove the ai to our root (but only do so if we have
		// one root)
		const std::set<CachedNetworkId> & roots = getPatrolPathRoots();
		if (roots.size() == 1)
		{
			ServerObject * root = safe_cast<ServerObject *>(roots.begin()->getObject());
			if (root != nullptr)
				root->removePatrolPathingObject(ai);
		}
		else
		{
			WARNING(true, ("ServerObject::removePatrolPathingObject trying to remove an ai %s from a non-root path node %s which has muliple roots.",
				ai.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str()));
		}
	}
}	// ServerObject::removePatrolPathingObject

// ----------------------------------------------------------------------

/**
 * Increments the observer count for this patrol path. If this is the first observer,
 * schedules any patrolling objects for alter.
 *
 * NOTE: Does not work multiserver!
 */
void ServerObject::addPatrolPathObserver()
{
	if (!isAuthoritative())
	{
		WARNING(true, ("ServerObject::addPatrolPathObserver called on non-authoritative object %s",
			getNetworkId().getValueString().c_str()));
		return;
	}

	// if we are a root node, increment our observer count
	Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		PatrolPathRootProperty * pprp = safe_cast<PatrolPathRootProperty *>(p);
		pprp->incrementObserverCount();
		DEBUG_REPORT_LOG(true, ("[patrolpath] add observer to %s, count = %d\n", 
			getNetworkId().getValueString().c_str(), pprp->getObserverCount()));
		if (pprp->getObserverCount() == 1)
		{
			// we need to wake up hibernating ai
			const std::set<ConstWatcher<ServerObject> > & ai = pprp->getPatrollingObjects();
			for (std::set<ConstWatcher<ServerObject> >::const_iterator i = ai.begin(); i != ai.end(); ++i)
			{
				if (*i != nullptr)
				{
					const ServerObject * ai = *i;
					if (ai->getController()->asCreatureController() != nullptr)
					{
						const CreatureController * controller = ai->getController()->asCreatureController();
						if (controller->getHibernate())
						{
							DEBUG_REPORT_LOG(true, ("[patrolpath] path root %s waking up ai %s",
								getNetworkId().getValueString().c_str(),
								ai->getNetworkId().getValueString().c_str()));
							const_cast<ServerObject *>(ai)->scheduleForAlter();
						}
					}
				}
			}
		}
	}
	else
	{
		// increment the count of the root nodes of any paths we belong to
		const std::set<CachedNetworkId> & roots = getPatrolPathRoots();
		for (std::set<CachedNetworkId>::const_iterator i = roots.begin(); i != roots.end(); ++i)
		{
			ServerObject * root = safe_cast<ServerObject *>(i->getObject());
			if (root != nullptr)
			{
				root->addPatrolPathObserver();
			}
		}
	}	
}	// ServerObject::addPatrolPathObserver

// ----------------------------------------------------------------------

/**
 * Decrements the observer count for this patrol path.
 *
 * NOTE: Does not work multiserver!
 */
void ServerObject::removePatrolPathObserver()
{
	if (!isAuthoritative())
	{
		WARNING(true, ("ServerObject::removePatrolPathObserver called on non-authoritative object %s",
			getNetworkId().getValueString().c_str()));
		return;
	}

	// if we are a root node, decrement our observer count
	Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		PatrolPathRootProperty * pprp = safe_cast<PatrolPathRootProperty *>(p);
		pprp->decrementObserverCount();
		DEBUG_REPORT_LOG(true, ("[patrolpath] remove observer from %s, count = %d\n", 
			getNetworkId().getValueString().c_str(), pprp->getObserverCount()));
		if (pprp->getObserverCount() < 0)
		{
			DEBUG_WARNING(true, ("ServerObject::removePatrolPathObserver %s has negative patrol path observer count of %d",
				getNetworkId().getValueString().c_str(), pprp->getObserverCount()));
		}
	}
	else
	{
		// decrement the count of the root nodes of any paths we belong to
		const std::set<CachedNetworkId> & roots = getPatrolPathRoots();
		for (std::set<CachedNetworkId>::const_iterator i = roots.begin(); i != roots.end(); ++i)
		{
			ServerObject * root = safe_cast<ServerObject *>(i->getObject());
			if (root != nullptr)
			{
				root->removePatrolPathObserver();
			}
		}
	}
}	// ServerObject::removePatrolPathObserver

// ----------------------------------------------------------------------

/**
 * Returns the number of observers of this patrol path. If this node connects to multiple
 * roots, we will return the total for all paths it connects to.
 *
 * @return the observer count
 */
int ServerObject::getPatrolPathObservers() const
{
	int observers = 0;

	const Property * p = getProperty(PatrolPathRootProperty::getClassPropertyId());
	if (p != nullptr)
	{
		observers = safe_cast<const PatrolPathRootProperty *>(p)->getObserverCount();
	}
	else
	{	
		const std::set<CachedNetworkId> & roots = getPatrolPathRoots();
		for (std::set<CachedNetworkId>::const_iterator i = roots.begin(); i != roots.end(); ++i)
		{
			const ServerObject * root = safe_cast<const ServerObject *>(i->getObject());
			if (root != nullptr && root->isPatrolPathRoot())
			{
				observers += root->getPatrolPathObservers();
			}
		}
	}
	return observers;
}	// ServerObject::getPatrolPathObservers

// ----------------------------------------------------------------------

bool ServerObject::isPatrolPathNode() const
{
	return (getProperty(PatrolPathNodeProperty::getClassPropertyId()) != nullptr) || isPatrolPathRoot();
}

// ----------------------------------------------------------------------

bool ServerObject::isPatrolPathRoot() const
{
	return getProperty(PatrolPathRootProperty::getClassPropertyId()) != nullptr;
}

// ----------------------------------------------------------------------

bool ServerObject::hasAttributeCaching() const
{
	// rls - if attribute caching breaks for an object, you can disable
	// caching by returning false here.
	return true;
}

// ----------------------------------------------------------------------

int ServerObject::getAttributeRevision() const
{
	return m_objVars.getRevision();
}

// ----------------------------------------------------------------------

void ServerObject::setAttributeRevisionDirty()
{
	m_objVars.updateRevision();
}

// ----------------------------------------------------------------------

bool ServerObject::getIncludeInBuildout() const
{
	return m_includeInBuildout.get();
}

// ----------------------------------------------------------------------

void ServerObject::setIncludeInBuildout( bool value )
{
	m_includeInBuildout = value;
}

// ----------------------------------------------------------------------

void ServerObject::setDescriptionStringId(const StringId & id)
{
	m_descriptionStringId.set(id);
}

// ----------------------------------------------------------------------

StringId const & ServerObject::getDescriptionStringId()
{
	return m_descriptionStringId.get();
}

// ----------------------------------------------------------------------

void ServerObject::addBroadcastListener(std::string const & messageName, NetworkId const & listener)
{
	if (!isAuthoritative())
		return;

	m_broadcastListeners.insert(std::make_pair(messageName, listener));
}

// ----------------------------------------------------------------------

void ServerObject::removeBroadcastListener(std::string const & messageName, NetworkId const & listener)
{
	if (!isAuthoritative())
		return;

	IGNORE_RETURN(m_broadcastListeners.erase(std::make_pair(messageName, listener)));
}

// ----------------------------------------------------------------------

void ServerObject::removeBroadcastListener(NetworkId const & listener)
{
	if (!isAuthoritative())
		return;

	for (BroadcastMap::const_iterator iter = m_broadcastListeners.begin(); iter != m_broadcastListeners.end();)
	{
		if (iter->second == listener)
		{
			// AutoDeltaSet's erase() will increment iter to point to the next item
			m_broadcastListeners.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::addBroadcastBroadcaster(std::string const & messageName, NetworkId const & broadcaster)
{
	if (!isAuthoritative())
		return;

	m_broadcastBroadcasters.insert(std::make_pair(messageName, broadcaster));
}

// ----------------------------------------------------------------------

void ServerObject::removeBroadcastBroadcaster(std::string const & messageName, NetworkId const & broadcaster)
{
	if (!isAuthoritative())
		return;

	IGNORE_RETURN(m_broadcastBroadcasters.erase(std::make_pair(messageName, broadcaster)));
}

// ----------------------------------------------------------------------

void ServerObject::removeBroadcastAllBroadcasters()
{
	if (!isAuthoritative())
		return;

	m_broadcastBroadcasters.clear();
}

// ----------------------------------------------------------------------

void ServerObject::listenToBroadcastMessage(std::string const & messageName, NetworkId const & broadcaster)
{
	if (messageName.empty() || !broadcaster.isValid())
		return;

	if (!isAuthoritative())
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer)-1, "%s %s", messageName.c_str(), broadcaster.getValueString().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++listenToBroadcastMessage", buffer, 0, false);
	}
	else
	{
		// need to send a message to the broadcaster if the broadcaster is not on this game server
		ServerObject * objBroadcaster = ServerWorld::findObjectByNetworkId(broadcaster);
		if (objBroadcaster && objBroadcaster->isAuthoritative())
		{
			// tell broadcaster that I want to listen
			objBroadcaster->addBroadcastListener(messageName, getNetworkId());
		}
		else
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "%s %s", messageName.c_str(), getNetworkId().getValueString().c_str());
			buffer[sizeof(buffer)-1] = '\0';
			MessageToQueue::getInstance().sendMessageToC(broadcaster, "C++addBroadcastListener", buffer, 0, false);
		}

		// this object is now listening to the broadcaster
		// @TODO: technically, we should have the remote broadcaster
		// send a message back confirming that this object has
		// been added to the broadcaster's listener list before
		// we mark this object as listening to the broadcaster
		addBroadcastBroadcaster(messageName, broadcaster);
	}
}

// ----------------------------------------------------------------------

void ServerObject::stopListeningToBroadcastMessage(std::string const & messageName, NetworkId const & broadcaster)
{
	if (messageName.empty() || !broadcaster.isValid())
		return;

	if (!isAuthoritative())
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer)-1, "%s %s", messageName.c_str(), broadcaster.getValueString().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++stopListeningToBroadcastMessage", buffer, 0, false);
	}
	else
	{
		// need to send a message to the broadcaster if the broadcaster is not on this game server
		ServerObject * objBroadcaster = ServerWorld::findObjectByNetworkId(broadcaster);
		if (objBroadcaster && objBroadcaster->isAuthoritative())
		{
			// tell broadcaster that I no longer want to listen
			objBroadcaster->removeBroadcastListener(messageName, getNetworkId());
		}
		else
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "%s %s", messageName.c_str(), getNetworkId().getValueString().c_str());
			buffer[sizeof(buffer)-1] = '\0';
			MessageToQueue::getInstance().sendMessageToC(broadcaster, "C++removeBroadcastListener", buffer, 0, false);
		}

		// this object is no longer listening to the broadcaster
		// @TODO: technically, we should have the remote broadcaster
		// send a message back confirming that this object has
		// been removed from the broadcaster's listener list before
		// we mark this object as no longer listening to the broadcaster
		removeBroadcastBroadcaster(messageName, broadcaster);
	}
}

// ----------------------------------------------------------------------

void ServerObject::stopListeningToAllBroadcastMessages()
{
	if (!isAuthoritative())
	{
		MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++stopListeningToAllBroadcastMessage", "", 0, false);
	}
	else
	{
		// build list of all broadcasters this object is currently listening to
		std::unordered_set<NetworkId> broadcasters;
		for (BroadcastMap::const_iterator iter = m_broadcastBroadcasters.begin(); iter != m_broadcastBroadcasters.end(); ++iter)
			IGNORE_RETURN(broadcasters.insert(iter->second));

		for (std::unordered_set<NetworkId>::const_iterator iter2 = broadcasters.begin(); iter2 != broadcasters.end(); ++iter2)
		{
			// need to send a message to the broadcaster if the broadcaster is not on this game server
			ServerObject * objBroadcaster = ServerWorld::findObjectByNetworkId(*iter2);
			if (objBroadcaster && objBroadcaster->isAuthoritative())
			{
				// tell broadcaster that I no longer want to listen
				objBroadcaster->removeBroadcastListener(getNetworkId());
			}
			else
			{
				char buffer[128];
				snprintf(buffer, sizeof(buffer)-1, "%s", getNetworkId().getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';
				MessageToQueue::getInstance().sendMessageToC(*iter2, "C++removeBroadcastListenerAllMessage", buffer, 0, false);
			}
		}

		// this object is no longer listening to any broadcasters
		// @TODO: technically, we should have the remote broadcaster
		// send a message back confirming that this object has
		// been removed from the broadcaster's listener list before
		// we mark this object as no longer listening to the broadcaster
		removeBroadcastAllBroadcasters();
	}
}

// ----------------------------------------------------------------------

void ServerObject::addObserver(Client * client)
{
	std::pair<std::set<Client *>::const_iterator, bool> result = m_observers.insert(client);
	
	if (result.second)
	{
		if (isAuthoritative())
		{
			m_observersCount = m_observersCount.get() + 1;
		}
		else
		{
			sendControllerMessageToAuthServer(CM_updateObserversCount, new MessageQueueGenericValueType<int>(1));
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::removeObserver(Client * client)
{
	int erased = m_observers.erase(client);
		
	if (erased > 0)
	{
		if (isAuthoritative())
		{
			m_observersCount = m_observersCount.get() - erased;
			if (m_observersCount.get() < 0)
			{
				WARNING(true, ("ServerObject::removeObserver for object %s has observer count of %d", getNetworkId().getValueString().c_str(), m_observersCount.get()));
				m_observersCount = 0;
			}
		}
		else
		{
			sendControllerMessageToAuthServer(CM_updateObserversCount, new MessageQueueGenericValueType<int>(-erased));
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::clearObservers()
{
	int erased = m_observers.size();
	m_observers.clear();

	if (erased > 0)
	{
		if (isAuthoritative())
		{
			m_observersCount = m_observersCount.get() - erased;
			if (m_observersCount.get() < 0)
			{
				WARNING(true, ("ServerObject::clearObservers for object %s has observer count of %d", getNetworkId().getValueString().c_str(), m_observersCount.get()));
				m_observersCount = 0;
			}
		}
		else
		{
			// if we aren't being deleted, send a controller message, otherwise send a messageTo
			if (!getKill())
				sendControllerMessageToAuthServer(CM_updateObserversCount, new MessageQueueGenericValueType<int>(-erased));
			else
			{
				char buffer[32];
				_itoa(-erased, buffer, 10);
				MessageToQueue::getInstance().sendMessageToC(getNetworkId(), "C++updateObserversCount", buffer, 0, false);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerObject::clearObserversCount()
{
	if (isAuthoritative())
	{
		m_observersCount = 0;
	}
}

// ----------------------------------------------------------------------

void ServerObject::updateObserversCount(int delta)
{
	if (isAuthoritative())
	{
		m_observersCount = m_observersCount.get() + delta;
		if (m_observersCount.get() < 0)
		{
			WARNING(m_observersCount.get() < 0, ("ServerObject::updateObserversCount for object %s has observer count of %d", getNetworkId().getValueString().c_str(), m_observersCount.get()));
			m_observersCount = 0;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Do nothing; this function is base virtual function so that creatures can go 
 * in and out of hibernation when observed.
 */
void ServerObject::observersCountChanged()
{
}

// ======================================================================

namespace Archive
{
	void put(Archive::ByteStream &target, TriggerVolumeInfo const &source)
	{
		put(target, source.name);
		put(target, source.radius);
		put(target, source.isPromiscuous);
	}

	void get(Archive::ReadIterator &source, TriggerVolumeInfo &target)
	{
		get(source, target.name);
		get(source, target.radius);
		get(source, target.isPromiscuous);
	}
}

// ======================================================================

