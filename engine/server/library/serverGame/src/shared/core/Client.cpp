// ======================================================================
//
// Client.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Client.h"

#include "Archive/ByteStream.h"
#include "LocalizationManager.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/Chat.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ConsentManager.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/GroupObject.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlanetMapManagerServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerShipController.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "serverGame/ShipClientUpdateTracker.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TriggerVolume.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AcceptAuctionMessage.h"
#include "sharedNetworkMessages/AddMapLocationMessage.h"
#include "sharedNetworkMessages/AddMapLocationResponseMessage.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"
#include "sharedNetworkMessages/AuctionQueryMessage.h"
#include "sharedNetworkMessages/BidAuctionMessage.h"
#include "sharedNetworkMessages/CancelLiveAuctionMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatOnInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/ConsentResponseMessage.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/CreateAuctionMessage.h"
#include "sharedNetworkMessages/CreateImmediateAuctionMessage.h"
#include "sharedNetworkMessages/CreateProjectileMessage.h"
#include "sharedNetworkMessages/ExpertiseRequestMessage.h"
#include "sharedNetworkMessages/FactionRequestMessage.h"
#include "sharedNetworkMessages/FactionResponseMessage.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetAuctionDetails.h"
#include "sharedNetworkMessages/GetMapLocationsMessage.h"
#include "sharedNetworkMessages/GuildRequestMessage.h"
#include "sharedNetworkMessages/GuildResponseMessage.h"
#include "sharedNetworkMessages/IsVendorOwnerMessage.h"
#include "sharedNetworkMessages/IsVendorOwnerResponseMessage.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ObjectMenuRequestDataArchive.h"
#include "sharedNetworkMessages/ObjectMenuSelectMessage.h"
#include "sharedNetworkMessages/ParametersMessage.h"
#include "sharedNetworkMessages/PlanetTravelPointListRequest.h"
#include "sharedNetworkMessages/PlanetTravelPointListResponse.h"
#include "sharedNetworkMessages/PlayerMoneyRequest.h"
#include "sharedNetworkMessages/PlayerMoneyResponse.h"
#include "sharedNetworkMessages/RequestGalaxyLoopTimes.h"
#include "sharedNetworkMessages/RetrieveAuctionItemMessage.h"
#include "sharedNetworkMessages/RetrieveAuctionItemResponseMessage.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedNetworkMessages/SecureTradeMessages.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedNetworkMessages/StomachRequestMessage.h"
#include "sharedNetworkMessages/SuiEventNotification.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkController.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/Location.h"
#include "sharedSkillSystem/SkillManager.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/CrcConstexpr.hpp"

namespace ClientNamespace {
    // list of object types, radial menu action permission
    // override, to allow certain object types to have certain radial
    // menu action invoked on them by non-owners of the object
    std::set <std::pair<uint32, int>> s_radialMenuPermissionOverrideTemplateItemType;
    std::set <std::pair<std::string, int>> s_radialMenuPermissionOverrideStaticItemType;

    // ----------------------------------------------------------------------

    bool
    canManipulateObjectExceptionCheck(ServerObject const &target, int menuType, Container::ContainerErrorCode &errorCode) {
        if (errorCode == Container::CEC_NoPermission) {
            const std::string &objectStaticName = target.getStaticItemName();
            if (objectStaticName.empty()) {
                if (s_radialMenuPermissionOverrideTemplateItemType.count(std::make_pair(target.getTemplateCrc(), menuType)) >
                    0) {
                    errorCode = Container::CEC_Success;
                    return true;
                }
            } else if (s_radialMenuPermissionOverrideStaticItemType.count(std::make_pair(objectStaticName, menuType)) >
                       0) {
                errorCode = Container::CEC_Success;
                return true;
            }
        }

        return false;
    }

    // ----------------------------------------------------------------------

    void remove() {
    }
}

// ======================================================================

Client::ClientMessage::ClientMessage(const std::string &src) : MessageDispatch::MessageBase(src.c_str()) {
}

//-----------------------------------------------------------------------

Client::ClientMessage::~ClientMessage() {
}




//-----------------------------------------------------------------------

std::map <std::string, uint32> Client::sm_outgoingBytesMap_Working;  // working stats that will rotate after 1 minute
std::map <std::string, uint32> Client::sm_outgoingBytesMap_Stats;    // computed stats from the last minute
uint32                          Client::sm_outgoingBytesMap_Worktime = 0; // time we started filling in the working map


// ======================================================================

Client::Client(ConnectionServerConnection &connection, const NetworkId &characterObjectId, const std::string &accountName, const std::string &ipAddr, bool isSecure, bool isSkipLoadScreen, unsigned int stationId, std::set <NetworkId> const &observedObjects, uint32 gameFeatures, uint32 subscriptionFeatures, AccountFeatureIdList const &accountFeatureIds, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints, std::vector <std::pair<NetworkId, std::string>> const &consumedRewardEvents, std::vector <std::pair<NetworkId, std::string>> const &claimedRewardItems, bool usingAdminLogin, CombatDataTable::CombatSpamFilterType combatSpamFilter, int combatSpamRangeSquaredFilter, int furnitureRotationDegree, bool hasUnoccupiedJediSlot, bool isJediSlotCharacter, bool sendToStarport)
        : MessageDispatch::Receiver(), MessageDispatch::Emitter(), m_accountName(accountName), m_characterName(),
          m_characterObjectId(characterObjectId), m_connection(&connection), m_controlledObjects(), m_godLevel(0),
          m_godMode(false), m_godValidated(false), m_ipAddress(ipAddr), m_isReady(false), m_isSecure(isSecure),
          m_isSkipLoadScreen(isSkipLoadScreen), m_primaryControlledObject(NetworkId::cms_invalid), destroyNotifier(),
          m_observing(), m_openedContainers(), m_watchedByList(),
          m_lastNonIdleTime(ServerClock::getInstance().getGameTimeSeconds()), m_stationId(stationId),
          m_connectionServerLag(0), m_gameServerLag(0), m_gameFeatures(gameFeatures),
          m_subscriptionFeatures(subscriptionFeatures), m_accountFeatureIds(accountFeatureIds),
          m_entitlementTotalTime(entitlementTotalTime), m_entitlementEntitledTime(entitlementEntitledTime),
          m_entitlementTotalTimeSinceLastLogin(entitlementTotalTimeSinceLastLogin),
          m_entitlementEntitledTimeSinceLastLogin(entitlementEntitledTimeSinceLastLogin), m_buddyPoints(buddyPoints),
          m_previousObservedObjects(observedObjects), m_syncUIs(), m_consumedRewardEvents(consumedRewardEvents),
          m_claimedRewardItems(claimedRewardItems), m_usingAdminLogin(usingAdminLogin),
          m_combatSpamFilter(combatSpamFilter), m_combatSpamRangeSquaredFilter(combatSpamRangeSquaredFilter),
          m_furnitureRotationDegree(furnitureRotationDegree), m_hasUnoccupiedJediSlot(hasUnoccupiedJediSlot),
          m_isJediSlotCharacter(isJediSlotCharacter), m_sendToStarport(sendToStarport) {

    connectToEmitter(connection, "ConnectionServerConnectionClosed");
    connectToEmitter(connection, "ConnectionServerConnectionDestroyed");

    // See if our controlled object is ready yet.
    ServerObject *obj = LogoutTracker::findPendingCharacterSave(characterObjectId);
    if (obj) {
        // If we're waiting to be saved in the LogoutTracker, then we've been
        // removed from our container, so restore our containment if need be.
        Object *containerObject = ContainerInterface::getContainedByObject(*obj);
        if (containerObject) {
            // Don't ever put the character into a packed house
            ServerObject *const serverObject = containerObject->asServerObject();
            CellObject *const cellObject = (serverObject ? serverObject->asCellObject() : nullptr);
            BuildingObject *const buildingObject = (cellObject ? cellObject->getOwnerBuilding() : nullptr);

            if ((buildingObject) && (!buildingObject->isInWorld())) {
                ContainedByProperty *const cby = ContainerInterface::getContainedByProperty(*obj);
                if (cby) {
                    DEBUG_REPORT_LOG(true, ("Ejecting player %s from building %s, because the building is packed\n", characterObjectId.getValueString().c_str(), buildingObject->getNetworkId().getValueString().c_str()));
                    LOG("CustomerService", ("Login:  Ejecting player %s from building %s, because the building is packed", characterObjectId.getValueString().c_str(), buildingObject->getNetworkId().getValueString().c_str()));

                    cby->setContainedBy(NetworkId::cms_invalid, true);
                    containerObject = 0;

                    // attempt to put the character at the ejection point of the building
                    if (!ServerWorld::isSpaceScene()) {
                        DynamicVariableLocationData ejectionPoint;
                        if (obj->getObjVars().getItem("building_ejection_point", ejectionPoint) &&
                            (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid())) {
                            obj->removeObjVarItem("building_ejection_point");
                            obj->setPosition_p(ejectionPoint.pos);
                        }
                    }
                }
            } else if (containerObject->isAuthoritative()) {
                // The cell depersist step will try to set the parent cell of the player.
                // However, that procedure will fail because the "contained by" property
                // says the player is in a container when they really are not.
                ContainedByProperty *const cby = ContainerInterface::getContainedByProperty(*obj);
                if (cby) {
                    // We need to remove and then re-add the property so that the
                    // property is updated immediately (as opposed to waiting for an alter)
                    cby->setContainedBy(NetworkId::cms_invalid, true);
                    obj->removeProperty(cby->getPropertyId());
                    obj->addProperty(*cby, true);
                }

                // save and restore the original transform.  The existing transform should be in parent
                // space, but it is in world.  down through depersistContents there is an assumption that
                // it is in parent space and gets transformed incorrectly.
                Transform origTransform = obj->getTransform_o2p();

                // NOTE: Depersist works slightly differently than when the player
                //       is loaded from the database since in this case the player
                //       is initialized when depersist is called
                Container *const container = ContainerInterface::getContainer(*containerObject);
                NOT_NULL(container);
                IGNORE_RETURN(container->depersistContents(*obj));

                // if failed to put character back into the container, then clean up the
                // containment relationship so that the character is now out in the world
                if (ContainerInterface::getContainedByObject(*obj) != containerObject) {
                    IGNORE_RETURN(container->internalItemRemoved(*obj));

                    if (cby) {
                        // We need to remove and then re-add the property so that the
                        // property is updated immediately (as opposed to waiting for an alter)
                        cby->setContainedBy(NetworkId::cms_invalid, true);
                        obj->removeProperty(cby->getPropertyId());
                        obj->addProperty(*cby, true);
                    }

                    // attempt to put the character at the ejection point of the building
                    if (!ServerWorld::isSpaceScene()) {
                        DynamicVariableLocationData ejectionPoint;
                        if (obj->getObjVars().getItem("building_ejection_point", ejectionPoint) &&
                            (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid())) {
                            DEBUG_REPORT_LOG(true, ("Ejecting player %s from building %s, because could not put player back in building\n", characterObjectId.getValueString().c_str(), (buildingObject
                                                                                                                                                                                         ? buildingObject->getNetworkId().getValueString().c_str()
                                                                                                                                                                                         : containerObject->getNetworkId().getValueString().c_str())));
                            LOG("CustomerService", ("Login:  Ejecting player %s from building %s, because could not put player back in building", characterObjectId.getValueString().c_str(), (buildingObject
                                                                                                                                                                                               ? buildingObject->getNetworkId().getValueString().c_str()
                                                                                                                                                                                               : containerObject->getNetworkId().getValueString().c_str())));

                            obj->removeObjVarItem("building_ejection_point");
                            obj->setPosition_p(ejectionPoint.pos);
                        }
                    }

                    containerObject = 0;
                } else {
                    obj->setTransform_o2w(origTransform);

                    // if the character was in a building, tell the building it has gained
                    // the player so the building demand load/unload system can work correctly
                    Object *buildingObj = ContainerInterface::getTopmostContainer(*containerObject);
                    if (buildingObj && buildingObj->asServerObject() &&
                        buildingObj->asServerObject()->asBuildingObject()) {
                        buildingObj->asServerObject()->asBuildingObject()->gainedPlayer(*obj);
                    }
                }
            } else {
                // The container is no longer authoritative on this server.  Move us to a safe place
                // outside the container.
                Object *const topmostObject = ContainerInterface::getTopmostContainer(*obj);
                BuildingObject *const topmostBuilding = dynamic_cast<BuildingObject *>(topmostObject);
                if (topmostBuilding) {
                    DEBUG_REPORT_LOG(true, ("Ejecting player %s from building %s, because the building has changed authority\n", characterObjectId.getValueString().c_str(), topmostBuilding->getNetworkId().getValueString().c_str()));
                    LOG("CustomerService", ("Login:  Ejecting player %s from building %s, because the building has changed authority", characterObjectId.getValueString().c_str(), topmostBuilding->getNetworkId().getValueString().c_str()));
                    ContainedByProperty *const cby = ContainerInterface::getContainedByProperty(*obj);
                    if (cby) {
                        cby->setContainedBy(NetworkId::cms_invalid, true);
                        containerObject = 0;

                        PortalProperty *const portalProperty = topmostBuilding->getPortalProperty();
                        if (portalProperty) {
                            ServerController *const controller = dynamic_cast<ServerController *>(obj->getController());
                            if (controller) {
                                Transform destinationTransform(Transform::IF_none);
                                destinationTransform.multiply(topmostBuilding->getTransform_o2w(), portalProperty->getEjectionLocationTransform());
                                obj->setTransform_o2p(destinationTransform);
                            }
                        }
                    }
                } else {
                    WARNING(true, ("Expected player %s to be in a building, but he was in some other type of object.  Skipping authority fixup because something strange is going on.", characterObjectId.getValueString().c_str()));
                }
            }
        } else {
            // check for the case where we were in a container that isn't there anymore
            ContainedByProperty *const cby = ContainerInterface::getContainedByProperty(*obj);
            if ((cby) && (cby->getContainedByNetworkId().isValid())) {
                DEBUG_REPORT_LOG(true, ("Ejecting player %s from building %s, because the building does not exist anymore\n", characterObjectId.getValueString().c_str(), cby->getContainedByNetworkId().getValueString().c_str()));
                LOG("CustomerService", ("Login:  Ejecting player %s from building %s, because the building does not exist anymore", characterObjectId.getValueString().c_str(), cby->getContainedByNetworkId().getValueString().c_str()));

                cby->setContainedBy(NetworkId::cms_invalid, true);

                // attempt to put the character at the ejection point of the building
                if (!ServerWorld::isSpaceScene()) {
                    DynamicVariableLocationData ejectionPoint;
                    if (obj->getObjVars().getItem("building_ejection_point", ejectionPoint) &&
                        (ServerWorld::getSceneId() == ejectionPoint.scene) && (!ejectionPoint.cell.isValid())) {
                        obj->removeObjVarItem("building_ejection_point");
                        obj->setPosition_p(ejectionPoint.pos);
                    }
                }
            }
        }

        // Only add to the world if it in a location which should be in the world.
        if (!containerObject ||
            (containerObject->isInWorld() && containerObject->getContainerProperty()->isContentItemExposedWith(*obj))) {
            obj->addToWorld();
        }

        // Notify the object that it has a client about to load
        obj->onClientAboutToLoad();

        // We need to trigger having been made authoritative since we are an object which has been
        // disconnected from the world.
        obj->triggerMadeAuthoritative();

        // Create proxies as needed
        ProxyList const &proxyList = obj->getExposedProxyList();
        if (!proxyList.empty()) {
            ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

            obj->forwardServerCreateAndBaselines();

            ServerMessageForwarding::end();
        }
    } else {
        obj = ServerWorld::findObjectByNetworkId(characterObjectId);
    }

    if (obj) {
        obj->checkAndAddClientControl(this);
    }

    // Send server-specific settings to the client
    ParametersMessage const msg(ConfigServerGame::getWeatherUpdateSeconds());
    send(msg, true);

    ObjectTracker::addPlayer();
}

//-----------------------------------------------------------------------

Client::~Client() {
    ClientDestroy d(this);
    destroyNotifier.emitMessage(d);

    if (ServerWorld::isSpaceScene()) {
        SpaceVisibilityManager::removeClient(*this);
    }

    ObserveTracker::onClientDestroyed(*this);
    ShipClientUpdateTracker::onClientDestroyed(*this);

    DEBUG_FATAL(!m_controlledObjects.empty(), ("Deleting a client with a non-empty controlled objects list!\n"));
    m_primaryControlledObject = NetworkId::cms_invalid;

    //clean up any orphaned SynchronizedUIs
    for (std::vector < Watcher < ServerSynchronizedUi > > ::iterator i = m_syncUIs.begin(); i != m_syncUIs.end();
    ++i)
    {
        ServerSynchronizedUi *syncUI = i->getPointer();
        delete syncUI;
    }

    ObjectTracker::removePlayer();

    PvpUpdateObserver::removeClientFromPvpStatusCache(this);

    m_connection = 0;
}

// ----------------------------------------------------------------------

void Client::install() {
    // read in the list of object types, radial menu action permission
    // override, to allow certain object types to have certain radial
    // menu action invoked on them by non-owners of the object
    char const *const radialMenuPermissionOverrideDataTableName = "datatables/player/radial_menu_permission_override.iff";
    DataTable *table = DataTableManager::getTable(radialMenuPermissionOverrideDataTableName, true);
    if (table) {
        int const columnObjectTemplate = table->findColumnNumber("Object Template");
        int const columnCommand = table->findColumnNumber("Command");
        if ((columnObjectTemplate >= 0) && (columnCommand >= 0)) {
            std::string objectTemplate;
            std::string command;
            int commandMenuId;

            for (int i = 0, numRows = table->getNumRows(); i < numRows; ++i) {
                objectTemplate = table->getStringValue(columnObjectTemplate, i);
                if (objectTemplate.empty()) {
                    continue;
                }

                command = table->getStringValue(columnCommand, i);
                if (command.empty()) {
                    continue;
                }

                commandMenuId = RadialMenuManager::getMenuTypeByName(command);
                if (commandMenuId == 0) {
                    continue;
                }

                if (objectTemplate.find(".iff") != std::string::npos) {
                    ClientNamespace::s_radialMenuPermissionOverrideTemplateItemType.insert(std::make_pair(CrcLowerString::calculateCrc(objectTemplate.c_str()), commandMenuId));
                } else {
                    ClientNamespace::s_radialMenuPermissionOverrideStaticItemType.insert(std::make_pair(objectTemplate, commandMenuId));
                }
            }
        }

        DataTableManager::close(radialMenuPermissionOverrideDataTableName);
    }

    ExitChain::add(ClientNamespace::remove, "ClientNamespace::remove");
}

//-----------------------------------------------------------------------

void Client::addControlledObject(ServerObject &object) {
    // check to see if it is in the list already
    if (std::find(m_controlledObjects.begin(), m_controlledObjects.end(), &object) == m_controlledObjects.end()) {
        m_controlledObjects.push_back(&object);
    } else {
        DEBUG_WARNING(true, ("Client adding controlled object [%s] that is already controlled", object.getNetworkId().getValueString().c_str()));
    }

    //finally set a pointer to the Client object on the object
    object.setClient(*this);
    ObserveTracker::onObjectControlled(*this, object, m_previousObservedObjects);

    // Any objects to be re-controlled have been at this point, so clean up any old objects that are no longer observed
    ObserveTracker::onClientControlComplete(*this, m_previousObservedObjects);
    m_previousObservedObjects.clear();

    // If in space, add the client to SpaceVisibilityManager.  Only do this for the topmost object the client controls that is in the world.
    if (ServerWorld::isSpaceScene()) {
        Object *topmost = ContainerInterface::getTopmostContainer(object);
        NOT_NULL(topmost);
        NOT_NULL(topmost->asServerObject());
        if (topmost->isInWorld()) {
            SpaceVisibilityManager::addClient(*this, *(topmost->asServerObject()));
        }
    }

    // Restore god mode after changing servers
    CreatureObject *primaryControlledObject = safe_cast<CreatureObject *>(m_primaryControlledObject.getObject());
    if (primaryControlledObject) {
        if (primaryControlledObject->hasCommand(AdminAccountManager::getAdminCommandName())) {
            setGodMode(true);
        }
    }
}

// ----------------------------------------------------------------------

void Client::clearControlledObjects() {
    while (!m_controlledObjects.empty()) {
        removeControlledObject(**m_controlledObjects.begin());
    }

    PvpUpdateObserver::removeClientFromPvpStatusCache(this);
}

// ----------------------------------------------------------------------

ServerObject *Client::findControlledObject(NetworkId const &oid) const {
    for (std::vector<ServerObject *>::const_iterator i = m_controlledObjects.begin();
         i != m_controlledObjects.end(); ++i) {
        if ((*i) && (*i)->getNetworkId() == oid) {
            return *i;
        }
    }
    return 0;
}

// ----------------------------------------------------------------------

std::vector<ServerObject *> const &Client::getControlledObjects() const {
    return m_controlledObjects; //lint !e1536 //low access
}

//-----------------------------------------------------------------------

bool Client::isControlled(NetworkId const &id) const {
    std::vector<ServerObject *>::const_iterator i;
    for (i = m_controlledObjects.begin(); i != m_controlledObjects.end(); ++i) {
        ServerObject *object = *i;
        if (object && object->getNetworkId() == id) {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------

void Client::onLoadPlayer(CreatureObject &newCharacter) {
    if (m_primaryControlledObject != NetworkId::cms_invalid) {
        DEBUG_WARNING(true, ("Already have a primary object!"));
        if (m_primaryControlledObject == newCharacter.getNetworkId()) {
            return;
        }
    }

    m_primaryControlledObject = CachedNetworkId(newCharacter);

    assumeControl(newCharacter);
}

//-----------------------------------------------------------------------

void Client::assumeControl(CreatureObject &newCharacter) {
    DEBUG_FATAL(
            newCharacter.getNetworkId() != m_primaryControlledObject, ("Tried to assume control of the wrong object?"));

    // Add the object to the controlled object list
    addControlledObject(newCharacter);

    // Register player name with the global list
    std::string const &firstName = Unicode::wideToNarrow(Unicode::toLower(newCharacter.getAssignedObjectFirstName()));
    std::string const &fullName = Unicode::wideToNarrow(newCharacter.getAssignedObjectName());
    PlayerObject const *const playerObject = PlayerCreatureController::getPlayerObject(&newCharacter);
    NameManager::getInstance().addPlayer(newCharacter.getNetworkId(), (playerObject ? playerObject->getStationId()
                                                                                    : static_cast<uint32>(m_stationId)), firstName, fullName, 0, Os::getRealSystemTime(), true);

    static const std::string loginTrace("TRACE_LOGIN");
    LOG(loginTrace, ("sending ControlAssumed(%s, %s)", m_primaryControlledObject.getValueString().c_str(), ConfigServerGame::getGroundScene()));

    ControlAssumed const controlMessage(m_primaryControlledObject, ConfigServerGame::getGroundScene(), m_isSkipLoadScreen, newCharacter.getTransform_o2w().getPosition_p(), newCharacter.getObjectFrameK_w().theta(), newCharacter.getSharedTemplateName(), static_cast<int64>(ServerClock::getInstance().getGameTimeSeconds()));

    sendToConnectionServer(controlMessage);

    m_isSkipLoadScreen = false;
}

//-----------------------------------------------------------------------

void Client::receiveClientMessage(const GameNetworkMessage &message) {
    try {

        // Ignore any messages if the character is not loaded yet.
        if (m_primaryControlledObject == NetworkId::cms_invalid) {
            DEBUG_REPORT_LOG(true, ("Ignoring message for character %s because it is not loaded yet.\n", m_characterObjectId.getValueString().c_str()));
            return;
        }

        ServerObject *primaryControlledObject = safe_cast<ServerObject *>(m_primaryControlledObject.getObject());

        if (!primaryControlledObject) {
            DEBUG_REPORT_LOG(true, ("Ignoring message for character %s because object %s cannot be found.\n", m_characterObjectId.getValueString().c_str(), m_primaryControlledObject.getValueString().c_str()));
            return;
        }

        //-----------------------------------------------------------------

        const uint32 messageType = message.getType();

        switch (messageType) {
            case constcrc("ShipUpdateTransformMessage") : {
                Archive::ReadIterator readIterator = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
                ShipUpdateTransformMessage const shipUpdateTransformMessage(readIterator);

                for (std::vector<ServerObject *>::const_iterator i = m_controlledObjects.begin();
                     i != m_controlledObjects.end(); ++i) {
                    ShipObject *const ship = (*i)->asShipObject();
                    if (ship) {
                        PlayerShipController *const playerShipController = safe_cast<PlayerShipController *>(ship->getController());
                        if (playerShipController) {
                            playerShipController->receiveTransform(shipUpdateTransformMessage);
                        }
                    }
                }
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("CreateProjectileMessage") : {
                Archive::ReadIterator readIterator = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
                CreateProjectileMessage const createProjectileMessage(readIterator);

                // pilots can fire from non-turrets, gunners can fire their turret
                ServerObject *const characterObject = getCharacterObject();
                if (characterObject) {
                    ServerObject *const containedBy = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*characterObject));
                    if (containedBy) {
                        SlottedContainer const *const slottedContainer = ContainerInterface::getSlottedContainer(*containedBy);
                        ShipObject *const shipObject = ShipObject::getContainingShipObject(containedBy);

                        if ((shipObject != nullptr) && (slottedContainer != nullptr) &&
                            !shipObject->hasCondition(TangibleObject::C_docking)) {
                            bool shotOk = false;

                            int const weaponIndex = createProjectileMessage.getWeaponIndex();
                            if (weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices) {
                                Container::ContainerErrorCode err = Container::CEC_Success;
                                if (shipObject->isTurret(weaponIndex)) {
                                    if (slottedContainer->getObjectInSlot(ShipSlotIdManager::getPobShipGunnerSlotId(weaponIndex), err) ==
                                        characterObject->getNetworkId() ||
                                        slottedContainer->getObjectInSlot(ShipSlotIdManager::getShipGunnerSlotId(weaponIndex), err) ==
                                        characterObject->getNetworkId()) {
                                        shotOk = true;
                                    } // gunner firing his turret
                                } else {
                                    if (slottedContainer->getObjectInSlot(ShipSlotIdManager::getShipPilotSlotId(), err) ==
                                        characterObject->getNetworkId() ||
                                        slottedContainer->getObjectInSlot(ShipSlotIdManager::getPobShipPilotSlotId(), err) ==
                                        characterObject->getNetworkId()) {
                                        shotOk = true;
                                    } // pilot firing non-turret
                                }
                            }

                            // only gods can fire while invulnerable
                            if (shipObject->isInvulnerable() && !isGod()) {
                                shotOk = false;
                            }

                            if (shotOk) {
                                shipObject->enqueueFireShotClient(*this, weaponIndex, createProjectileMessage.getTransform_p(), createProjectileMessage.getTargetedComponent(), createProjectileMessage.getSyncStampLong());
                            }
                        }
                    }
                }
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("CmdSceneReady") : {
                m_isReady = true;
                primaryControlledObject->onClientReady(this);
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("SetCombatSpamFilter") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage<int> const msg(ri);
                m_combatSpamFilter = static_cast<CombatDataTable::CombatSpamFilterType>(msg.getValue());
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("SetCombatSpamRangeFilter") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage<int> const msg(ri);
                m_combatSpamRangeSquaredFilter = sqr(msg.getValue());
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("SetFurnitureRotationDegree") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage<int> const msg(ri);
                m_furnitureRotationDegree = msg.getValue();
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("GcwRegionsReq") : {
                GenericValueTypeMessage < std::map < std::string, std::map < std::string, std::pair < std::pair < float,
                        float >, float > > > >
                const rsp(
                "GcwRegionsRsp", Pvp::getGcwScoreCategoryRegions());
                send(rsp, true);

                GenericValueTypeMessage < std::map < std::string, std::map < std::string, int > > >
                const rsp2(
                "GcwGroupsRsp", Pvp::getGcwScoreCategoryGroups());
                send(rsp2, true);
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("SetJediSlotInfo") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::pair<bool, bool>> const msg(ri);

                // this comes from the client (via the LoginServer) so should not be trusted;
                // we cache them here to do preliminary checks to avoid unnecessarily sending
                // commands to the LoginServer (assuming the values haven't been hacked on the
                // client); when we do send the commands to the LoginServer, we'll check again there
                m_hasUnoccupiedJediSlot = msg.getValue().first;
                m_isJediSlotCharacter = msg.getValue().second;
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("RotateFurnitureSetQuaternion") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage < std::pair < NetworkId, std::pair < std::pair < float, float >, std::pair <
                                                                                                         float, float >
                                                                                                                > > >
                const msg(ri);

                CreatureObject *creatureOwner = safe_cast<CreatureObject *>(getCharacterObject());
                if (creatureOwner && creatureOwner->isAuthoritative()) {
                    ServerObject const *target = ServerWorld::findObjectByNetworkId(msg.getValue().first);
                    if (target && target->isAuthoritative()) {
                        static uint32 const commandHash = Crc::normalizeAndCalculate("rotateFurniture");
                        const Command &command = CommandTable::getCommand(commandHash);
                        if (!command.isNull()) {
                            // for obfuscation so players won't manually send us this command
                            Unicode::String params = Unicode::narrowToWide(FormattedString<512>().sprintf("(^-,=+_)internal_use_only_%s_quaternion(,+-=_^) %.10f %.10f %.10f %.10f %s", creatureOwner->getNetworkId().getValueString().c_str(), msg.getValue().second.first.first, msg.getValue().second.first.second, msg.getValue().second.second.first, msg.getValue().second.second.second, msg.getValue().first.getValueString().c_str()));
                            creatureOwner->commandQueueEnqueue(command, target->getNetworkId(), params, 0, false, static_cast<Command::Priority>(Command::CP_Default), true);
                        }
                    }
                }
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("SetLfgInterests") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <BitArray> const msg(ri);

                PlayerObject const *
                const playerObject = PlayerCreatureController::getPlayerObject(safe_cast < CreatureObject const * >
                                                                                                                (getCharacterObject()));
                if (playerObject) {
                    std::map <NetworkId, LfgCharacterData> const &connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
                    std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(getCharacterObjectId());
                    if (iterFind != connectedCharacterLfgData.end()) {
                        BitArray lfgInterests = msg.getValue();

                        MatchMakingId const &id = playerObject->getMatchMakingCharacterProfileId();
                        if (id.isBitSet(MatchMakingId::B_lookingForGroup)) {
                            LfgDataTable::setBit("lfg", lfgInterests);
                        } else {
                            LfgDataTable::clearBit("lfg", lfgInterests);
                        }

                        if (id.isBitSet(MatchMakingId::B_helper)) {
                            LfgDataTable::setBit("helper", lfgInterests);
                        } else {
                            LfgDataTable::clearBit("helper", lfgInterests);
                        }

                        if (id.isBitSet(MatchMakingId::B_rolePlay)) {
                            LfgDataTable::setBit("rp", lfgInterests);
                        } else {
                            LfgDataTable::clearBit("rp", lfgInterests);
                        }

                        if (id.isBitSet(MatchMakingId::B_lookingForWork)) {
                            LfgDataTable::setBit("lfw", lfgInterests);
                        } else {
                            LfgDataTable::clearBit("lfw", lfgInterests);
                        }

                        ServerUniverse::setConnectedCharacterInterestsData(getCharacterObjectId(), lfgInterests);
                    }
                }
                break;
            }

                //----------------------------------------------------------------------

            case constcrc("RequestGroundObjectPlacement") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage < std::pair < NetworkId, std::pair < std::pair < int, float >, std::pair <
                                                                                                       float, float > >
                                                                                                              > >
                const msg(ri);

                ServerObject *const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg.getValue().first));
                GameScriptObject *const scriptObject = target ? target->getScriptObject() : 0;

                if (!scriptObject) {
                    return;
                }

                int menuType = msg.getValue().second.first.first;
                Vector loc(msg.getValue().second.first.second, msg.getValue().second.second.first, msg.getValue().second.second.second);
                ScriptParams params;
                params.addParam(getCharacterObjectId());
                params.addParam(menuType);
                params.addParam(loc.x);
                params.addParam(loc.y);
                params.addParam(loc.z);

                IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_GROUND_TARGET_LOC, params));
                break;
            }
                //----------------------------------------------------------------------

            case constcrc("ShowBackpack") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::pair<NetworkId, bool>> const msg(ri);
                PlayerObject *player = dynamic_cast<PlayerObject *>(NetworkIdManager::getObjectById(msg.getValue().first));

                if (player) {
                    player->modifyShowBackpack(msg.getValue().second);
                }

                break;
            }
                //----------------------------------------------------------------------

            case constcrc("ShowHelmet") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::pair<NetworkId, bool>> const msg(ri);
                PlayerObject *player = dynamic_cast<PlayerObject *>(NetworkIdManager::getObjectById(msg.getValue().first));

                if (player) {
                    player->modifyShowHelmet(msg.getValue().second);
                }

                break;
            }
                //----------------------------------------------------------------------

            case constcrc("SetWaypointColor") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::pair<NetworkId, std::string>> const msg(ri);

                if ((msg.getValue().second == "blue") || (msg.getValue().second == "green") ||
                    (msg.getValue().second == "orange") || (msg.getValue().second == "yellow") ||
                    (msg.getValue().second == "purple") || (msg.getValue().second == "white")) {
                    Waypoint w = Waypoint::getWaypointById(msg.getValue().first);
                    if (w.isValid()) {
                        w.setColor(Waypoint::getColorIdByName(msg.getValue().second));
                    }
                }

                break;
            }

                //----------------------------------------------------------------------

            case constcrc("ConGenericMessage") : {

                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                ConGenericMessage c(ri);
                if (isGod()) {
                    LOG("CustomerService", ("Avatar:%s executing command: %s", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str(), c.getMsg().c_str()));
                    ConsoleMgr::processString(c.getMsg(), this, c.getMsgId());
                } else {
                    //Non-god user tried to use a console command
                    if (m_primaryControlledObject.getObject()) {
                        LOG("CustomerService", ("CheatChannel:%s tried to execute a console command but they are not a god", PlayerObject::getAccountDescription(m_primaryControlledObject.getObject()->getNetworkId()).c_str()));
                        ConsoleMgr::broadcastString("Only god users can execute console commands.", this, c.getMsgId());
                    }

                }
                break;
            }

                //-----------------------------------------------------------------

            case constcrc("ObjControllerMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                ObjControllerMessage o(ri);
                bool appended = false;

                // check to make sure the controller message is allowed from the client
                bool allowFromClient = ControllerMessageFactory::allowFromClient(o.getMessage());
#ifdef _DEBUG
                if (allowFromClient || isGod()) {
#else
                if (allowFromClient) {
#endif
                    LOG("AllowedObjControllerMsgs", ("Allowing player %s at %s controller message %d for object %s", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str(), getIpAddress().c_str(), o.getMessage(), o.getNetworkId().getValueString().c_str()));

                    ServerObject *target = findControlledObject(o.getNetworkId());
                    if (target != 0) {
                        // apply the controller message
                        ServerController *controller = dynamic_cast<ServerController *>(target->getController());
                        if (controller != nullptr) {
                            uint32 flags = o.getFlags();
                            flags &= ~GameControllerMessageFlags::SOURCE_REMOTE;
                            flags |= GameControllerMessageFlags::SOURCE_REMOTE_CLIENT;
                            controller->appendMessage(o.getMessage(), o.getValue(), o.getData(), flags);
                            appended = true;
                        }
                    } else if (isGod()) {
                        target = ServerWorld::findObjectByNetworkId(o.getNetworkId());

                        if (target) {
                            ServerController *controller = dynamic_cast<ServerController *>(target->getController());
                            if (controller != nullptr) {
                                uint32 flags = o.getFlags();
                                flags &= ~GameControllerMessageFlags::SOURCE_REMOTE;
                                flags |= GameControllerMessageFlags::SOURCE_REMOTE_CLIENT;
                                controller->appendMessage(o.getMessage(), o.getValue(), o.getData(), flags);
                                appended = true;
                            }
                        }
                    }
                } else {
                    // log as a likely hack
                    LOG("HackAttempts", ("Unauthorized Controller Message:  Player %s at %s sent an unauthorized controller message %d for object %s", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str(), getIpAddress().c_str(), o.getMessage(), o.getNetworkId().getValueString().c_str()));

                    KickPlayer const kickMessage(m_characterObjectId, "Hack Attempt");
                    GameServer::getInstance().sendToConnectionServers(kickMessage);
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }

		if (!appended) {
		    delete o.getData();
		}

                break;
            }
                //-----------------------------------------------------------------
                //Secure Trade Messages
                //-----------------------------------------------------------------
            case constcrc("AddItemMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                AddItemMessage m(ri);


                ServerObject *item = ServerWorld::findObjectByNetworkId(m.getNetworkId());
                if (!item) {
                    return;
                }

                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->addItem(*playerObject, *item);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("RemoveItemMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                RemoveItemMessage m(ri);

                ServerObject *item = ServerWorld::findObjectByNetworkId(m.getNetworkId());
                if (!item) {
                    return;
                }

                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->removeItem(*playerObject, *item);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("GiveMoneyMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GiveMoneyMessage m(ri);

                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->giveMoney(*playerObject, m.getAmount());
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("AcceptTransactionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                AcceptTransactionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->acceptOffer(*playerObject);
                    }

                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has accepted a secure trade. (Check trade logs for more detail.", PlayerObject::getAccountDescription(playerObject).c_str()));
                    }


                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("UnAcceptTransactionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                UnAcceptTransactionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->unacceptOffer(*playerObject);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("VerifyTradeMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                VerifyTradeMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->verifyTrade(*playerObject);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("DenyTradeMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                DenyTradeMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->rejectOffer(*playerObject);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }
            case constcrc("AbortTradeMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                AbortTradeMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                DEBUG_FATAL(!playerObject, ("No controller for player!\n"));
                if (playerObject) //lint !e774
                {
                    PlayerCreatureController *playerController = dynamic_cast<PlayerCreatureController *>(playerObject->getController());
                    if (playerController && playerController->getSecureTrade()) {
                        playerController->getSecureTrade()->cancelTrade(*playerObject);
                    }
                } else {
                    GameServer::getInstance().dropClient(m_characterObjectId);
                }
                break;
            }

            case constcrc("GuildRequestMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GuildRequestMessage const m(ri);

                NetworkId const &id = m.getTargetId();
                CreatureObject *const creatureObject = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(id));
                if (creatureObject) {
                    int const guildId = creatureObject->getGuildId();
                    GuildMemberInfo const *const info = GuildInterface::getGuildMemberInfo(guildId, id);
                    GuildResponseMessage const responseMessage(id, GuildInterface::getGuildName(guildId), info
                                                                                                          ? info->m_title
                                                                                                          : std::string());
                    send(responseMessage, true);
                }
                break;
            }

            case constcrc("StomachRequestMessage") : {
                CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(getCharacterObject());

                if (creatureObject != nullptr) {
                    GameScriptObject *gameScriptObject = creatureObject->getScriptObject();

                    if (gameScriptObject != nullptr) {
                        ScriptParams scriptParams;
                        IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_STOMACH_UPDATE, scriptParams));
                    }
                }
                break;
            }

            case constcrc("FactionRequestMessage") : {
                int rebel = 0;
                int imperial = 0;
                int criminal = 0;

                CreatureObject *playerObject = dynamic_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    float temp;
                    if (playerObject->getObjVars().getItem("faction.Rebel", temp)) {
                        rebel = static_cast<int>(temp);
                    }
                    if (playerObject->getObjVars().getItem("faction.Imperial", temp)) {
                        imperial = static_cast<int>(temp);
                    }
                    if (playerObject->getObjVars().getItem("faction.Hutt", temp)) {
                        criminal = static_cast<int>(temp);
                    }

                    std::vector <std::string> factionNames;
                    std::vector<float> factionValues;
                    std::string currentName;
                    float currentValue;
                    DynamicVariableList::NestedList factionList(playerObject->getObjVars(), "faction");
                    for (DynamicVariableList::NestedList::const_iterator i = factionList.begin();
                         i != factionList.end(); ++i) {
                        currentName = i.getName();
                        bool result = i.getValue(currentValue);
                        if (result) {
                            factionNames.push_back(currentName);
                            factionValues.push_back(currentValue);
                        }
                    }

                    FactionResponseMessage const msg(rebel, imperial, criminal, factionNames, factionValues);
                    send(msg, true);
                }
                break;
            }

                //-----------------------------------------------------------------

            case constcrc("AcceptAuctionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                AcceptAuctionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    CommoditiesMarket::auctionAccept(*playerObject, m.getItemId().getValue());
                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has accepted an auction for item %s", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str()));
                    }
                } else {
                    WARNING(true, ("AcceptAuctionMessage: the player was not found"));
                }
                break;
            }

            case constcrc("BidAuctionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                BidAuctionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has bid on an auction for item %s, amount %d", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str(), m.getBid()));
                    }

                    CommoditiesMarket::auctionBid(*playerObject, m.getItemId().getValue(), m.getBid(), m.getMaxProxyBid());
                } else {
                    WARNING(true, ("BidAuctionMessage: the player was not found"));
                }
                break;
            }

            case constcrc("CancelLiveAuctionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                CancelLiveAuctionMessage m(ri);

                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    CommoditiesMarket::auctionCancel(*playerObject, m.getItemId().getValue());
                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has cancelled an auction for item %s", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str()));
                    }

                } else {
                    WARNING(true, ("CancelLiveAuctionMessage: the player was not found"));
                }
                break;
            }

            case constcrc("CreateAuctionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                CreateAuctionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    NetworkId itemId = m.getItemId();
                    NetworkId containerId = m.getContainerId();
                    CommoditiesMarket::auctionCreate(*playerObject, itemId, m.getItemLocalizedName(), containerId, m.getMinimumBid(), m.getAuctionLength(), m.getUserDescription(), m.isPremium());

                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has created an auction for item %s on vendor %s", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str(), containerId.getValueString().c_str()));
                    }
                } else {
                    WARNING(true, ("CreateAuctionMessage: the player was not found"));
                }
                break;
            }

            case constcrc("CreateImmediateAuctionMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                CreateImmediateAuctionMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    NetworkId itemId = m.getItemId();
                    NetworkId containerId = m.getContainerId();
                    if (m.isVendorTransfer()) {
                        CommoditiesMarket::transferVendorItemFromStockroom(*playerObject, itemId, m.getItemLocalizedName(), m.getPrice(), m.getAuctionLength(), m.getUserDescription());
                    } else {
                        CommoditiesMarket::auctionCreateImmediate(*playerObject, itemId, m.getItemLocalizedName(), containerId, m.getPrice(), m.getAuctionLength(), m.getUserDescription(), m.isPremium());
                    }
                    if (playerObject->getObjVars().hasItem("cheater")) {
                        LOG("CustomerService", ("SuspectedCheaterChannel: %s has created an immediate auction for item %s on vendor %s", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str(), containerId.getValueString().c_str()));
                    }
                } else {
                    WARNING(true, ("CreateImmediateAuctionMessage: the player was not found"));
                }
                break;
            }

            case constcrc("RelistItemsFromStockMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage < std::pair < std::pair < std::vector < NetworkId > , std::vector <
                                                                                              Unicode::String > >, int >
                                                                                                                   >
                const m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    std::vector <NetworkId> const &itemIds = m.getValue().first.first;
                    std::vector <Unicode::String> const &itemLocalizedNames = m.getValue().first.second;
                    int const timer = m.getValue().second;
                    NetworkId itemId;
                    std::vector<NetworkId>::const_iterator iterId = itemIds.begin();
                    std::vector<Unicode::String>::const_iterator iterName = itemLocalizedNames.begin();
                    for (; ((iterId != itemIds.end()) &&
                            (iterName != itemLocalizedNames.end())); ++iterId, ++iterName) {
                        itemId = *iterId;
                        CommoditiesMarket::transferVendorItemFromStockroom(*playerObject, itemId, *iterName, -1, timer, Unicode::String());
                        if (playerObject->getObjVars().hasItem("cheater")) {
                            LOG("CustomerService", ("SuspectedCheaterChannel: %s has created an immediate auction (relist) for item %s", PlayerObject::getAccountDescription(playerObject).c_str(), itemId.getValueString().c_str()));
                        }
                    }
                } else {
                    WARNING(true, ("RelistItemsFromStockMessage: the player was not found"));
                }
                break;
            }

            case constcrc("RetrieveAuctionItemMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                RetrieveAuctionItemMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    NetworkId itemId = m.getItemId();
                    ServerObject *auctionContainer = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getContainerId()));
                    if (auctionContainer) {
                        if (playerObject->isAuthoritative() && auctionContainer->isAuthoritative()) {
                            CommoditiesMarket::auctionRetrieve(*playerObject, m.getItemId().getValue(), m.getItemId(), *auctionContainer);
                            if (playerObject->getObjVars().hasItem("cheater")) {
                                LOG("CustomerService", ("SuspectedCheaterChannel: %s has retrieved an auction for item %s on vendor %s", PlayerObject::getAccountDescription(playerObject).c_str(), m.getItemId().getValueString().c_str(), auctionContainer->getNetworkId().getValueString().c_str()));
                            }
                        }
                    } else {
                        LOG("CustomerService", ("Auction:Player %s attempted to retrieve an auction for container %Ld but failed.", PlayerObject::getAccountDescription(playerObject).c_str(), m.getContainerId().getValue()));

                        RetrieveAuctionItemResponseMessage const msg(itemId, ar_INVALID_CONTAINER_ID);;
                        Client *client = playerObject->getClient();
                        if (client) {
                            client->send(msg, true);
                        }
                    }
                } else {
                    WARNING(true, ("RetrieveAuctionMessage: the player was not found"));
                }
                break;
            }
            case constcrc("AuctionQueryHeadersMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                AuctionQueryHeadersMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    CommoditiesMarket::auctionQueryHeaders(*playerObject, m.getRequestId(), m.getSearchType(), m.getItemType(), m.getItemTypeExactMatch(), m.getItemTemplateId(), m.getTextFilterAll(), m.getTextFilterAny(), m.getPriceFilterMin(), m.getPriceFilterMax(), m.getPriceFilterIncludesFee(), m.getAdvancedSearch(), m.getAdvancedSearchMatchAllAny(), m.getContainer(), m.getLocationSearchType(), m.getMyVendorsOnly(), m.getQueryOffset());
                }
                break;
            }
            case constcrc("GetAuctionDetails") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GetAuctionDetails m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    CommoditiesMarket::getAuctionDetails(*playerObject, m.getItem());
                }
                break;
            }
            case constcrc("IsVendorOwnerMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                IsVendorOwnerMessage m(ri);
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    CommoditiesMarket::isVendorOwner(*playerObject, m.getContainerId());
                }
                break;
            }
            case constcrc("CommoditiesItemTypeListRequest") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::string> const m(ri);

                static char buffer[128];
                snprintf(buffer, sizeof(buffer) -
                                 1, "%s.%d", GameServer::getInstance().getClusterName().c_str(), CommoditiesMarket::getItemTypeMapVersionNumber());
                buffer[sizeof(buffer) - 1] = '\0';

                if (m.getValue() != std::string(buffer)) {
                    GenericValueTypeMessage < std::pair < std::string, std::map < int, std::map < int, std::pair < int,
                            StringId > > > > >
                    const rsp(
                    "CommoditiesItemTypeListResponse", std::make_pair(std::string(buffer), CommoditiesMarket::getItemTypeMap()));
                    send(rsp, true);
                }
                break;
            }
            case constcrc("CommoditiesResourceTypeListRequest") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::string> const m(ri);

                static char buffer[128];
                snprintf(buffer, sizeof(buffer) -
                                 1, "%s.%d", GameServer::getInstance().getClusterName().c_str(), CommoditiesMarket::getResourceTypeMapVersionNumber());
                buffer[sizeof(buffer) - 1] = '\0';

                if (m.getValue() != std::string(buffer)) {
                    GenericValueTypeMessage < std::pair < std::string, std::map < int, std::set < std::string > > > >
                    const rsp(
                    "CommoditiesResourceTypeListResponse", std::make_pair(std::string(buffer), CommoditiesMarket::getResourceTypeMap()));
                    send(rsp, true);
                }
                break;
            }
            case constcrc("CollectionServerFirstListRequest") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::string> const m(ri);

                PlanetObject const *const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
                if (planetObject) {
                    static char buffer[128];
                    snprintf(buffer, sizeof(buffer) -
                                     1, "%s.%d", GameServer::getInstance().getClusterName().c_str(), planetObject->getCollectionServerFirstUpdateNumber());
                    buffer[sizeof(buffer) - 1] = '\0';

                    if (m.getValue() != std::string(buffer)) {
                        GenericValueTypeMessage < std::pair < std::string, std::set < std::pair < std::pair < int32,
                                std::string >, std::pair < NetworkId, Unicode::String > > > > >
                        const rsp(
                        "CollectionServerFirstListResponse", std::make_pair(std::string(buffer), planetObject->getCollectionServerFirst()));
                        send(rsp, true);
                    }
                }
                break;
            }

                //-----------------------------------------------------------------

            case constcrc("AddMapLocationMessage") : {
                Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                const AddMapLocationMessage msg(readIterator);
                UNREF(msg);

                /*
                MapLocation mapLocation(msg.getLocationId(), msg.getLocationName(),
                                        Vector2d(msg.getLocationX(), msg.getLocationY()),
                                        msg.getCategory(), msg.getSubCategory());

                PlanetObject *planetObject = ServerUniverse::getInstance ().getPlanetByName (msg.getPlanetName ());
                if (planetObject)
                {
                    planetObject->addMapLocation(mapLocation);
                }
                AddMapLocationResponseMessage responseMsg(msg.getLocationId());
                send(responseMsg, true);
                */
                break;
            }

            case constcrc("GetSpecificMapLocationsMessage") : {
                Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                const GenericValueTypeMessage <std::set<std::pair < std::string, NetworkId>> > msg(readIterator);

                PlanetMapManagerServer::handleClientRequestGetSpecificMapLocationsMessage(*this, msg.getValue());
                break;
            }

            case constcrc("ConsentResponseMessage") : {
                Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                ConsentResponseMessage const msg(readIterator);
                ConsentManager::getInstance().handleResponse(msg);
                break;
            }

            case constcrc("RequestGalaxyLoopTimes") : {
                // obfuscate the information so it's not obvious on the client that this is the server loop time
                unsigned long loopTime = static_cast<unsigned long>(Clock::frameTime() *
                                                                    3193000.0f); // loop time (in ms) * 3193
                GalaxyLoopTimesResponse const msg(loopTime, loopTime);
                send(msg, true);
                break;
            }
            case constcrc("LagRequest") : {
                GameNetworkMessage const msg("GameServerLagResponse");
                send(msg, true);
                break;
            }
            case constcrc("LagReport") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                GenericValueTypeMessage <std::pair<int, int>> const report(ri);
                m_connectionServerLag = report.getValue().first;
                m_gameServerLag = report.getValue().second;
                break;
            }
            case constcrc("ChatInviteGroupToRoom") : {
                if (primaryControlledObject) {
                    CreatureObject *const playerCreature = primaryControlledObject->asCreatureObject();
                    if (playerCreature) {
                        Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                        ChatInviteGroupToRoom const chat(ri);

                        // We will need to notify the user if the group leader can not be found
                        bool foundGroupLeader = false;

                        // Get the name of the group leader being invited
                        ChatAvatarId const &groupLeaderChatAvatarId = chat.getAvatarId();

                        // Convert the name to a network ID
                        NetworkId const groupLeaderNetworkId = NameManager::getInstance().getPlayerId(NameManager::normalizeName(Unicode::wideToNarrow(groupLeaderChatAvatarId.getName())));

                        // Try to get a group object corresponding to the player network ID
                        ServerObject const *const groupServerObject = ServerWorld::findObjectByNetworkId(GroupObject::getGroupIdForLeader(groupLeaderNetworkId));
                        if (groupServerObject && groupServerObject->asGroupObject()) {
                            // We now know the player is a group leader of a group
                            GroupObject const *const groupLeaderGroupObject = groupServerObject->asGroupObject();

                            // Make sure we have the correct player since we only searched by name
                            // (i.e. we may have been looking for a player with the same name but on a different cluster)
                            ChatAvatarId const playerChatAvatarId = Chat::constructChatAvatarId(NameManager::normalizeName(groupLeaderGroupObject->getGroupLeaderName()));
                            if (playerChatAvatarId == groupLeaderChatAvatarId) {
                                // We found the group leader
                                foundGroupLeader = true;

                                // We need to get the network IDs for all the group members
                                std::vector <NetworkId> groupNetworkIds;

                                GroupObject::GroupMemberVector const &groupMemberVector = groupLeaderGroupObject->getGroupMembers();
                                for (unsigned i = 0; i < groupMemberVector.size(); ++i) {
                                    GroupObject::GroupMember const &groupMember = groupMemberVector[i];

                                    groupNetworkIds.push_back(groupMember.first);
                                }

                                // Pass the invitation to all the group members
                                Chat::inviteGroupMembers(m_primaryControlledObject, chat.getAvatarId(), chat.getRoomName(), groupNetworkIds);
                            }
                        }

                        // If we couldn't find the group leader, let the player know
                        if (!foundGroupLeader) {
                            ChatAvatarId const inviteeId = Chat::constructChatAvatarId(*playerCreature);
                            ChatOnInviteGroupToRoom msg(ERR_DESTAVATARDOESNTEXIST, chat.getRoomName(), inviteeId, chat.getAvatarId());
                            send(msg, false);
                        }
                    }
                }
                break;
            }
            case constcrc("ChatPersistentMessageToServer") : {
                if (primaryControlledObject) //lint !e774
                {
                    CreatureObject *const playerCreature = primaryControlledObject->asCreatureObject();
                    if (playerCreature) {
                        Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                        ChatPersistentMessageToServer const chat(ri);
                        std::string const &toName = chat.getToCharacterName().name;
                        if (!_stricmp(toName.c_str(), "guild")) {
                            std::pair<int, std::pair<std::string, std::string> > const count = GuildInterface::mailToGuild(*playerCreature, chat.getSubject(), chat.getMessage(), chat.getOutOfBand());
                            if (count.first < 0) {
                                Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide("You do not have sufficient permission to send mail to guild members."), Unicode::emptyString);
                            } else {
                                Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide(FormattedString<512>().sprintf("Your mail was sent to %d members of guild %s (%s).", count.first, count.second.first.c_str(), count.second.second.c_str())), Unicode::emptyString);
                            }
                        } else if (!_strnicmp(toName.c_str(), "guild ", 6)) {
                            // see if it's a valid guild rank
                            GuildRankDataTable::GuildRank const *guildRank = GuildRankDataTable::getRankForDisplayRankName(toName);
                            if (guildRank) {
                                std::pair<int, std::pair<std::string, std::string> > const count = GuildInterface::mailToGuildRank(*playerCreature, guildRank->slotId, chat.getSubject(), chat.getMessage(), chat.getOutOfBand());
                                if (count.first < 0) {
                                    Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide("You do not have sufficient permission to send mail to guild members."), Unicode::emptyString);
                                } else {
                                    Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide(FormattedString<512>().sprintf("Your mail was sent to %d members in rank %s of guild %s (%s).", count.first, guildRank->displayName.c_str(), count.second.first.c_str(), count.second.second.c_str())), Unicode::emptyString);
                                }
                            } else {
                                Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide(FormattedString<512>().sprintf("\"%s\" is not a valid name.", toName.c_str())), Unicode::emptyString);
                            }
                        } else if (!_stricmp(toName.c_str(), "citizens")) {
                            std::pair<int, std::string> const count = CityInterface::mailToCitizens(*playerCreature, chat.getSubject(), chat.getMessage(), chat.getOutOfBand());
                            if (count.first < 0) {
                                Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide("You do not have sufficient permission to send mail to citizens."), Unicode::emptyString);
                            } else {
                                Chat::sendSystemMessage(*playerCreature, Unicode::narrowToWide(FormattedString<512>().sprintf("Your mail was sent to %d citizens of city %s.", count.first, count.second.c_str())), Unicode::emptyString);
                            }
                        }

                        if (playerCreature->getObjVars().hasItem("cheater")) {
                            LOGU("CustomerService", ("SuspectedCheaterChannel: %s is broadcasting a %s message that says '%s'", PlayerObject::getAccountDescription(playerCreature).c_str(), toName.c_str()), chat.getMessage());
                        }
                    }
                }
                break;
            }
            case constcrc("ExpertiseRequestMessage") : {
                Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                ExpertiseRequestMessage const m(ri);

                std::vector <std::string> const &addExpertisesNamesList = m.getAddExpertisesList();
                bool clearAllExpertisesFirst = m.getClearAllExpertisesFirst();
                std::string addList;
                std::vector<std::string>::const_iterator i;
                for (i = addExpertisesNamesList.begin(); i != addExpertisesNamesList.end(); ++i) {
                    addList += " ";
                    addList += *i;
                }
                CreatureObject *playerObject = safe_cast<CreatureObject *>(getCharacterObject());
                if (playerObject) {
                    LOG("CustomerService", ("ExpertiseRequestMessage: %s has requested to add expertises [%s] %s", PlayerObject::getAccountDescription(playerObject).c_str(), addList.c_str(), clearAllExpertisesFirst
                                                                                                                                                                                               ? "(clearing first)"
                                                                                                                                                                                               : ""));
                    playerObject->processExpertiseRequest(addExpertisesNamesList, clearAllExpertisesFirst);
                }
                break;
            }
            case constcrc("UpdateSessionPlayTimeInfo") : {
                Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                GenericValueTypeMessage < std::pair < int32, std::pair < int32, unsigned
                long > > >
                const msgPlayTimeInfo(readIterator);

                PlayerObject *playerObject = PlayerCreatureController::getPlayerObject(safe_cast<CreatureObject *>(getCharacterObject()));
                if (playerObject != nullptr) {
                    playerObject->setSessionPlayTimeInfo(msgPlayTimeInfo.getValue().first, msgPlayTimeInfo.getValue().second.first, msgPlayTimeInfo.getValue().second.second);
                }
                break;
            }
            default : {
                // am too lazy to convert these to constexpr's
                if (message.isType(ObjectMenuSelectMessage::MESSAGE_TYPE)) {
                    Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
                    const ObjectMenuSelectMessage m(ri);
                    ServerObject *const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m.getNetworkId()));
                    GameScriptObject *const scriptObject = target ? target->getScriptObject() : 0;
                    Object *const targetContainedBy = target ? ContainerInterface::getContainedByObject(*target)
                                                             : nullptr;
                    const int menuType = m.getSelectedItemId();

                    static int examineMenuType = RadialMenuManager::getMenuTypeByName("EXAMINE");
                    static int tradeMenuType = RadialMenuManager::getMenuTypeByName("TRADE_START");

                    CreatureObject *cobj = dynamic_cast<CreatureObject *>(primaryControlledObject);
                    if (cobj && menuType != examineMenuType && menuType != tradeMenuType) {
                        CreatureController *const controller = cobj->getCreatureController();
                        if (controller && controller->getSecureTrade()) {
                            controller->getSecureTrade()->cancelTrade(*cobj);
                            DEBUG_REPORT_LOG(true, ("Client ObjectMenuSelectMessage [%d] for [%s] canceling trade.\n", menuType, cobj->getNetworkId().getValueString().c_str()));
                        }
                    }

                    if (!scriptObject) {
                        DEBUG_REPORT_LOG(true, ("Received an object menu select message from player %s for object %s, which does not exist or lacks a GameScriptObject.", m_characterObjectId.getValueString().c_str(), m.getNetworkId().getValueString().c_str()));
                        return;
                    }

                    NOT_NULL(target);

                    float range = 0.0f;
                    if (RadialMenuManager::getRangeForMenuType(menuType, range)) {
                        Container::ContainerErrorCode errorCode = Container::CEC_Success;
                        if (!target->isAuthoritative()) {
                            GenericValueTypeMessage <std::pair<NetworkId, NetworkId>> const rssMessage("RequestSameServer", std::make_pair(ContainerInterface::getTopmostContainer(*primaryControlledObject)->getNetworkId(), ContainerInterface::getTopmostContainer(*target)->getNetworkId()));
                            GameServer::getInstance().sendToPlanetServer(rssMessage);
                            errorCode = Container::CEC_TryAgain;
                        } else if (menuType != examineMenuType && targetContainedBy &&
                                   targetContainedBy->asServerObject() &&
                                   (targetContainedBy->asServerObject()->getGameObjectType() ==
                                    SharedObjectTemplate::GOT_chronicles_quest_holocron ||
                                    targetContainedBy->asServerObject()->getGameObjectType() ==
                                    SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe)) {
                            // Can only examine items that are contained in a holocron.
                            errorCode = Container::CEC_NoPermission;
                        } else if (
                                primaryControlledObject->canManipulateObject(*target, false, false, false, range, errorCode) ||
                                ClientNamespace::canManipulateObjectExceptionCheck(*target, menuType, errorCode)) {
                            ScriptParams params;
                            params.addParam(getCharacterObjectId());
                            params.addParam(menuType);

                            if (cobj && cobj->getObjVars().hasItem("cheater")) {
                                std::string menuDesc = "unknown";
                                bool tmp;
                                IGNORE_RETURN(RadialMenuManager::getCommandForMenuType(menuType, menuDesc, tmp));
                                LOG("CustomerService", ("SuspectedCheaterLog: %s has used a radial menu on object %s. Menu item %d (%s).", PlayerObject::getAccountDescription(cobj).c_str(), m.getNetworkId().getValueString().c_str(), menuType, menuDesc.c_str()));
                            }

                            IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_OBJECT_MENU_SELECT, params));
                        }
                        if (errorCode != Container::CEC_Success) {
                            ContainerInterface::sendContainerMessageToClient(*primaryControlledObject, errorCode);
                        }
                    } else {
                        DEBUG_REPORT_LOG(true, ("Received a message type %d that had no range in data (radial_menu.tab)\n", menuType));
                    }
                } else if (message.isType(SuiEventNotification::MessageType)) {
                    ServerUIManager::receiveMessage(message);
                } else if (message.isType(PlayerMoneyRequest::MessageType)) {
                    Archive::ReadIterator ri = (*NON_NULL(static_cast<const GameNetworkMessage *>(&message))).getByteStream().begin();
                    const PlayerMoneyRequest m(ri);

                    ServerObject *const playerObject = getCharacterObject();

                    WARNING(!playerObject, ("Got PlayerMoneyRequest for non-existant character (account=%s, networkid=%s)", m_accountName.c_str(), m_characterObjectId.getValueString().c_str()));

                    if (playerObject) {
                        PlayerMoneyResponse resp;
                        resp.m_balanceCash.set(playerObject->getCashBalance());
                        resp.m_balanceBank.set(playerObject->getBankBalance());
                        send(resp, true);
                    }
                } else if (message.isType(PlanetTravelPointListRequest::cms_name)) {
                    Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                    const PlanetTravelPointListRequest requestMessage(readIterator);

                    const std::string &planetName = requestMessage.getPlanetName();
                    const PlanetObject *const planetObject = ServerUniverse::getInstance().getPlanetByName(requestMessage.getPlanetName());
                    if (planetObject) {
                        std::vector <std::string> names;
                        planetObject->getTravelPointNameList(names);

                        std::vector <Vector> points;
                        planetObject->getTravelPointPointList(points);

                        std::vector<int> costs;
                        planetObject->getTravelPointCostList(costs);

                        std::vector<bool> interplanetary;
                        planetObject->getTravelPointInterplanetaryList(interplanetary);

                        PlanetTravelPointListResponse const rsp(planetName, names, points, costs, interplanetary, requestMessage.getSequenceId());
                        send(rsp, true);
                    } else {
                        DEBUG_WARNING(true, ("GameServer::receiveMessage: received request for travel point names from client [%s] for planet %s which does not exist", requestMessage.getPlanetName().c_str(), requestMessage.getNetworkId().getValueString().c_str()));
                    }

                } else if (message.isType(NewbieTutorialResponse::cms_name)) {
                    Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                    const NewbieTutorialResponse responseMessage(readIterator);

                    static const std::string clientReadyString("clientReady");

                    if (responseMessage.getResponse() == clientReadyString) {
                        for (std::vector<ServerObject *>::iterator i = m_controlledObjects.begin();
                             i != m_controlledObjects.end(); ++i) {
                            ServerObject *const so = *i;
                            if (so) {
                                so->onLoadingScreenComplete();
                            }
                        }
                    }

                    ScriptParams scriptParameters;
                    scriptParameters.addParam(responseMessage.getResponse().c_str());

                    if (primaryControlledObject->getScriptObject()->trigAllScripts(Scripting::TRIG_NEWBIE_TUTORIAL_RESPONSE, scriptParameters) !=
                        SCRIPT_CONTINUE) {
                        DEBUG_REPORT_LOG(true, ("OnNewbieTutorialResponse: did not return SCRIPT_CONTINUE\n"));
                    }

                } else if (message.isType(GetMapLocationsMessage::MessageType)) {
                    Archive::ReadIterator readIterator = static_cast<const GameNetworkMessage &> (message).getByteStream().begin();
                    const GetMapLocationsMessage msg(readIterator);

                    PlanetMapManagerServer::handleClientRequest(*this, msg);
                }
                break;
            }
        }
    } catch (Archive::ReadException &) {
        if (getCharacterObject()) {
            WARNING_STRICT_FATAL(true, ("A message was received from a client that could not be unpacked. Either the data was corrupt (intentionally or accidentally) or the client and server notion of the message are out of sync. The client will be disconnected. Character object is : %s :%s, IP=%s, accountname=%s", getCharacterObject()->getObjectTemplateName(), getCharacterObject()->getNetworkId().getValueString().c_str(), getIpAddress().c_str(), getAccountName().c_str()));
        } else {
            WARNING_STRICT_FATAL(true, ("A message was received from a client that could not be unpacked. Either the data was corrupt (intentionally or accidentally) or the client and server notion of the message are out of sync. The client will be disconnected. IP=%s, accountname=%s", getIpAddress().c_str(), getAccountName().c_str()));
        }
        if (getConnection()) {
            KickPlayer const kickMessage(m_characterObjectId, "Invalid Network Stream");
            GameServer::getInstance().sendToConnectionServers(kickMessage);
            GameServer::getInstance().dropClient(m_characterObjectId);
        }
    }

    //-----------------------------------------------------------------
}

//-----------------------------------------------------------------------

void Client::receiveMessage(const MessageDispatch::Emitter &source, const MessageDispatch::MessageBase &message) {
    UNREF(source);

    const uint32 messageType = message.getType();

    if (messageType == constcrc("ConnectionServerConnectionClosed") ||
        messageType == constcrc("ConnectionServerConnectionDestroyed")) {
        //Connection server crashed
        DEBUG_REPORT_LOG(true, ("Connection Server crashed.  Client dropping.\n"));
        m_connection = 0;
        GameServer::getInstance().dropClient(m_characterObjectId);
    } else {
        if (&source != m_connection) {
            const MessageDispatch::Message <std::string> *m = dynamic_cast<const MessageDispatch::Message <std::string> *>(&message);
            if (m) {
                ConGenericMessage const c(m->getValue(), 0);
                send(c, true);
            }
        }
    }
}

//-----------------------------------------------------------------------

void Client::removeControlledObject(ServerObject &object) {
    if (m_primaryControlledObject == object.getNetworkId()) {
        ObserveTracker::onClientDestroyed(*this);
        if (ServerWorld::isSpaceScene()) {
            SpaceVisibilityManager::removeClient(*this);
        }
    }

    std::vector<ServerObject *>::iterator iter = std::find(m_controlledObjects.begin(), m_controlledObjects.end(), &object);
    if (iter != m_controlledObjects.end()) {
        IGNORE_RETURN(m_controlledObjects.erase(iter));
    } else {
        DEBUG_WARNING(true, ("Client removing object [%s] that is not controlled", object.getNetworkId().getValueString().c_str()));
    }

    object.clearClient();
}

//-----------------------------------------------------------------------

void Client::selfDestruct() {
    selfDestruct(false);
}
//-----------------------------------------------------------------------

void Client::selfDestruct(const bool immediate) {
    // @todo : this will eventually need to attach ai controllers to
    // link dead objects.
    std::vector < ServerObject * > controlledObjects = m_controlledObjects;

    // the controlled object list on the client is affected by clearClient, so work on a copy of the vector
    for (std::vector<ServerObject *>::iterator i = controlledObjects.begin(); i != controlledObjects.end(); ++i) {
        if (*i) {
            if ((*i)->isAuthoritative()) {
                (*i)->handleDisconnect(immediate);
            }
            (*i)->clearClient();
        }
    }
    m_controlledObjects.clear();
    delete this;
}

//-----------------------------------------------------------------------

void Client::send(GameNetworkMessage const &outgoingMessage, bool reliable) const {
    if (m_connection) {
        static std::vector <NetworkId> v;
        v.clear();
        v.push_back(m_characterObjectId);
        GameClientMessage const msg(v, reliable, outgoingMessage);


        uint32 now = Clock::timeMs();
        if (sm_outgoingBytesMap_Worktime == 0) {
            sm_outgoingBytesMap_Worktime = now;
        } else if ((now - sm_outgoingBytesMap_Worktime) > 60000)   // 60 seconds
        {
            sm_outgoingBytesMap_Stats = sm_outgoingBytesMap_Working;
            std::map<std::string, uint32>::iterator iter;
            for (iter = sm_outgoingBytesMap_Working.begin(); iter != sm_outgoingBytesMap_Working.end(); ++iter) {
                iter->second = 0;
            }
            sm_outgoingBytesMap_Worktime = now;
        }
        sm_outgoingBytesMap_Working[outgoingMessage.getCmdName()] += msg.getByteStream().getSize();


        m_connection->send(msg, true);
    } else {
        DEBUG_REPORT_LOG(true, ("Tried to send message to a client without a connection server connection.\n"));
    }
}


//-----------------------------------------------------------------------

std::map <std::string, uint32> &Client::getPacketBytesPerMinStats() {
    uint32 now = Clock::timeMs();
    if (sm_outgoingBytesMap_Worktime == 0) {
        sm_outgoingBytesMap_Worktime = now;
    } else if ((now - sm_outgoingBytesMap_Worktime) > 60000)   // 60 seconds
    {
        sm_outgoingBytesMap_Stats = sm_outgoingBytesMap_Working;
        std::map<std::string, uint32>::iterator iter;
        for (iter = sm_outgoingBytesMap_Working.begin(); iter != sm_outgoingBytesMap_Working.end(); ++iter) {
            iter->second = 0;
        }
        sm_outgoingBytesMap_Worktime = now;
    }

    return sm_outgoingBytesMap_Stats;
}



//-----------------------------------------------------------------------

void Client::sendToConnectionServer(GameNetworkMessage const &msg) {
    if (m_connection) {
        m_connection->send(msg, true);
    } else {
        DEBUG_REPORT_LOG(true, ("Tried to send message to a client without a connection server connection.\n"));
    }
}

//-----------------------------------------------------------------------

uint16 Client::getServerSyncStampShort() const {
    NOT_NULL(m_connection);
    return m_connection->getSyncStampShort();
}

//-----------------------------------------------------------------------

uint32 Client::getServerSyncStampLong() const {
    NOT_NULL(m_connection);
    return m_connection->getSyncStampLong();
}

//-----------------------------------------------------------------------

float Client::computeDeltaTimeInSeconds(uint32 const syncStampLong) const {
    uint32 const deltaTimeMilliseconds = syncStampLong - getServerSyncStampLong();

    float deltaTime = 0.f;
    if (deltaTimeMilliseconds > 0x7fffffff) {
        deltaTime = static_cast<float>(static_cast<int>(0xffffffff - deltaTimeMilliseconds)) / 1000.0f;
    } else {
        deltaTime = static_cast<float>(static_cast<int>(deltaTimeMilliseconds)) / 1000.0f;
    }

    return deltaTime;
}

//-----------------------------------------------------------------------

bool Client::setGodMode(bool value) {
    // (re?) check god permissions
    m_godLevel = AdminAccountManager::getAdminLevel(m_accountName.c_str());

    if (ConfigServerGame::getAdminGodToAll() || (m_godLevel > 0)) {
        m_godValidated = true;
        if (ConfigServerGame::getAdminGodToAll()) {
            m_godLevel = ConfigServerGame::getAdminGodToAllGodLevel();
        }
    }

    bool wasInGodMode = m_godMode;
    m_godMode = value;

    if (value && !m_godValidated) {
        LOG("CustomerService", ("Avatar:%s denied god mode because it wasn't validated.", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str()));
        m_godMode = false;
    }

    CreatureObject *primaryControlledObject = safe_cast<CreatureObject *>(m_primaryControlledObject.getObject());
    if (!primaryControlledObject) {
        if (value && !wasInGodMode) {
            LOG("CustomerService", ("Avatar:%s denied god mode because it has no associated character.", m_accountName.c_str()));
        }
        m_godMode = false;
        return false;
    }

    if (m_godMode) {
        IGNORE_RETURN(primaryControlledObject->grantCommand(AdminAccountManager::getAdminCommandName(), false));
    } else {
        primaryControlledObject->revokeCommand(AdminAccountManager::getAdminCommandName(), false, true);
    }

    if (m_godMode != wasInGodMode) {
        if (m_godMode) {
            LOG("CustomerService", ("Avatar:%s granted god mode %s level %d.", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str(), AdminAccountManager::getAdminCommandName(), m_godLevel));
        } else {
            LOG("CustomerService", ("Avatar:%s dropped god mode.", PlayerObject::getAccountDescription(getCharacterObjectId()).c_str()));
        }

        // cell permissions may change for us for all cells, so observe all buildings in range for the change
        CellPermissions::ViewerChangeObserver o(primaryControlledObject);

        ObserveTracker::onGodModeChanged(*this);
    }
    return (value == m_godMode); // return true if the value was set to what was requested, false otherwise
}

//-----------------------------------------------------------------------

void Client::addObserving(ServerObject *o) {
    if (o) {
        IGNORE_RETURN(m_observing.insert(o));
        TangibleObject *to = o->asTangibleObject();
        if (to &&
            PvpUpdateObserver::satisfyPvpSyncCondition(to->isNonPvpObject(), to->hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (
                    o->asCreatureObject() != nullptr), to->getPvpFaction())) {
            addObservingPvpSync(to);
        }
    }
}

//-----------------------------------------------------------------------

void Client::addObservingPvpSync(TangibleObject *to) {
    if (to) {
        IGNORE_RETURN(m_observingPvpSync.insert(to));
        PvpUpdateObserver::startObservingPvpSyncNotification(this, *to);
    }
}

//-----------------------------------------------------------------------

void Client::removeObservingPvpSync(ServerObject *o) {
    if (o && m_observingPvpSync.erase(reinterpret_cast<TangibleObject *>(o))) {
        PvpUpdateObserver::stopObservingPvpSyncNotification(this, o->getNetworkId());
    }
}

//-----------------------------------------------------------------------

void Client::resetIdleTimeAfterCharacterTransfer() {
    // set m_lastNonIdleTime such that idledTooLong() will be true in ConfigServerGame::getIdleLogoutTimeAfterCharacterTransferSec()
    unsigned long const currentGameTime = ServerClock::getInstance().getGameTimeSeconds();

    if ((currentGameTime + ConfigServerGame::getIdleLogoutTimeAfterCharacterTransferSec()) >
        ConfigServerGame::getIdleLogoutTimeSec()) {
        m_lastNonIdleTime = currentGameTime + ConfigServerGame::getIdleLogoutTimeAfterCharacterTransferSec() -
                            ConfigServerGame::getIdleLogoutTimeSec();

        if (m_lastNonIdleTime > currentGameTime) {
            m_lastNonIdleTime = currentGameTime;
        }
    } else {
        m_lastNonIdleTime = 0;
    }
}

//-----------------------------------------------------------------------

void Client::resetIdleTime() {
    m_lastNonIdleTime = ServerClock::getInstance().getGameTimeSeconds();
}

//-----------------------------------------------------------------------

bool Client::idledTooLong() const {
    return ServerClock::getInstance().getGameTimeSeconds() - m_lastNonIdleTime >
           ConfigServerGame::getIdleLogoutTimeSec();
}

//-----------------------------------------------------------------------

const std::string &Client::getAccountName() const {
    return m_accountName;
}

//-----------------------------------------------------------------------

const std::string &Client::getIpAddress() const {
    return m_ipAddress;
}

//-----------------------------------------------------------------------

void Client::handleIdleDisconnect() const {
    DEBUG_REPORT_LOG(true, ("Disconnecting %s for being idle too long.\n", m_characterObjectId.getValueString().c_str()));
    KickPlayer const kickMessage(m_characterObjectId, "Idle Connection");
    GameServer::getInstance().sendToConnectionServers(kickMessage);
    GameServer::getInstance().dropClient(m_characterObjectId);
}

//-----------------------------------------------------------------------

const unsigned int Client::getStationId() const {
    return m_stationId;
}

//-----------------------------------------------------------------------

int Client::getConnectionServerLag() const {
    return m_connectionServerLag;
}

//-----------------------------------------------------------------------

int Client::getGameServerLag() const {
    return m_gameServerLag;
}

//-----------------------------------------------------------------------

void Client::addSynchronizedUi(ServerSynchronizedUi *syncUI) {
    m_syncUIs.push_back(Watcher<ServerSynchronizedUi>(syncUI));
}

//-----------------------------------------------------------------------

void Client::removeSynchronizedUi(ServerSynchronizedUi const *const syncUI) {
    for (std::vector < Watcher < ServerSynchronizedUi > > ::iterator i = m_syncUIs.begin(); i != m_syncUIs.end(); ++i)
    {
        if (i->getPointer() == syncUI) {
            IGNORE_RETURN(m_syncUIs.erase(i));
            return;
        }
    }
}

// ----------------------------------------------------------------------

void Client::openContainer(ServerObject &obj, int sequence, std::string const &slotDesc) {
    if (ObserveTracker::onClientOpenedContainer(*this, obj, sequence, slotDesc)) {
        ScriptParams params;
        params.addParam(getCharacterObjectId());
        IGNORE_RETURN(obj.getScriptObject()->trigAllScripts(Scripting::TRIG_OPENED_CONTAINER, params));
    }
}

// ----------------------------------------------------------------------

void Client::closeContainer(ServerObject &obj) {
    if (ObserveTracker::onClientClosedContainer(*this, obj)) {
        ScriptParams params;
        params.addParam(getCharacterObjectId());
        IGNORE_RETURN(obj.getScriptObject()->trigAllScripts(Scripting::TRIG_CLOSED_CONTAINER, params));
    }
}

//-----------------------------------------------------------------------

void Client::observeContainer(ServerObject &obj, int sequence, std::string const &slotDesc) {
    ObserveTracker::onClientOpenedContainer(*this, obj, sequence, slotDesc, false);
}

//-----------------------------------------------------------------------

bool Client::isFreeTrialAccount() const {
    return false;
}

//-----------------------------------------------------------------------

bool Client::isSecure() const {
    return m_isSecure;
}

// ----------------------------------------------------------------------

std::vector <std::pair<NetworkId, std::string>> const &Client::getConsumedRewardEvents() const {
    return m_consumedRewardEvents;
}

// ----------------------------------------------------------------------

std::vector <std::pair<NetworkId, std::string>> const &Client::getClaimedRewardItems() const {
    return m_claimedRewardItems;
}

// ----------------------------------------------------------------------

void Client::launchWebBrowser(std::string const &url) const {
    GenericValueTypeMessage <std::string> message("LaunchBrowserMessage", url);
    send(message, true);
}

// ----------------------------------------------------------------------

bool Client::isUsingAdminLogin() const {
    return m_usingAdminLogin;
}

// ----------------------------------------------------------------------

bool  Client::shouldReceiveCombatSpam(NetworkId const &attacker, Vector const &attackerPosition_w, NetworkId const &defender, Vector const &defenderPosition_w) const {
    PROFILER_AUTO_BLOCK_DEFINE("Client::shouldReceiveCombatSpam");

    // none means no combat spam, period!!!
    if (m_combatSpamFilter == CombatDataTable::CSFT_None) {
        return false;
    }

    // some combat spam messages don't have an attacker or defender, always allow those
    if (!attacker.isValid() && !defender.isValid()) {
        return true;
    }

    // always receive combat spam if I'm the attacker or the defender
    if (m_characterObjectId.isValid() && ((m_characterObjectId == attacker) || (m_characterObjectId == defender))) {
        return true;
    }

    bool passFilter = false;
    if (m_combatSpamFilter == CombatDataTable::CSFT_All) {
        passFilter = true;
    } else if (m_combatSpamFilter == CombatDataTable::CSFT_Group) {
        CreatureObject *primaryControlledObject = safe_cast<CreatureObject *>(m_primaryControlledObject.getObject());
        if (primaryControlledObject) {
            GroupObject const *group = primaryControlledObject->getGroup();
            if (group && (group->isGroupMember(attacker) || group->isGroupMember(defender))) {
                passFilter = true;
            }
        }
    }

    if (!passFilter) {
        return false;
    }

    // range check
    CreatureObject *primaryControlledObject = safe_cast<CreatureObject *>(m_primaryControlledObject.getObject());
    if (primaryControlledObject) {
        Vector const playerPosition = primaryControlledObject->getPosition_w();

        if ((attacker.isValid() &&
             (playerPosition.magnitudeBetweenSquared(attackerPosition_w) <= m_combatSpamRangeSquaredFilter)) ||
            (defender.isValid() &&
             (playerPosition.magnitudeBetweenSquared(defenderPosition_w) <= m_combatSpamRangeSquaredFilter))) {
            return true;
        }
    }

    return false;
}

// ----------------------------------------------------------------------

bool Client::getSendToStarport() const {
    return m_sendToStarport;
}


// ----------------------------------------------------------------------

int Client::getBuddyPoints() const {
    return m_buddyPoints;
}

// ======================================================================

