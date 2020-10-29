//ServerObject.h
//Copyright 2000-2003 Sony Online Entertainment

#ifndef	INCLUDED_ServerObject_H
#define	INCLUDED_ServerObject_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "Archive/AutoDeltaVector.h"
#include "StringId.h"
#include "Unicode.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverGame/Client.h"
#include "serverGame/ProxyList.h"
#include "serverGame/ServerWorldTangibleNotification.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedCollision/CollisionEnums.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMath/Sphere.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedObject/CachedNetworkIdArchive.h"
#include "sharedObject/Container.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedUtility/PooledString.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class BaselinesMessage;
class BuildingObject;
class CachedNetworkId;
class CellObject;
class Client;
class ConnectionServerConnection;
class ConsoleCommandParserObject;
class ContainedByProperty;
class CreatureObject;
class DeltasMessage;
class DynamicVariableLocationData;
class GameNetworkMessage;
class GameScriptObject;
class GroupObject;
class IntangibleObject;
class Location;
class ManufactureSchematicObject;
class MessageQueueCombatSpam;
class MessageQueueSocial;
class MessageQueueSpatialChat;
class MessageToPayload;
class MissionObject;
class PlayerQuestObject;
class PlayerObject;
class ServerObjectTemplate;
class ServerSynchronizedUi;
class SharedObjectTemplate;
class ShipObject;
class SlottedContainmentProperty;
class StaticObject;
class TangibleObject;
class TriggerVolume;
class UpdateObjectPositionMessage;
class WeaponObject;

template <typename T> class Watcher;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------


namespace DeleteReasons
{
	
	// NOTE NOTE NOTE !!!!
	// Do not change these without updating the data in the delete_reasons
	// table in the database.  I mean it.  The database may reject any
	// attempt to delete an object for a reason that's not on this list.
	// (see src/game/server/database/data/delete_reasons.sql)

	const int NotDeleted = 0;
//	const int Unknown = 1;
	const int Decay = 2;
	const int God = 3;  
	const int Player = 4;
	const int Script = 5;
	const int CharacterDeleted = 6;
	const int ContainerDeleted = 7;
	const int Publish = 8;
	const int BadContainerTransfer = 9;
	const int Consumed = 10;
	const int SetupFailed = 11;
	const int Replaced = 12;
	const int House = 13;
	
	typedef int Enumerator;

	const std::string & describeDeleteReason(const int reason);
}

//-----------------------------------------------------------------------

namespace LocalObjectFlags
{
	enum
	{
		ServerObject_Initialized          = 0, // Set once either endBaselines or initializeFirstTime has been called
		ServerObject_BeingDestroyed       = 1, // Set when the object is being destroyed
		ServerObject_Placing              = 2, // Set when the object should snap to terrain first alter
		ServerObject_TransformChanged     = 3, // Set when transform has changed this frame
		ServerObject_Unloading            = 4, // Set when the object is being unloaded from memory 
		ServerObject_GoingToConclude      = 5, // Set when the object is going to conclude this frame
		ServerObject_InEndBaselines       = 6, // Set during endBaselines
		ServerObject_AutoDeltaChanged     = 7, // Set when an AutoDeltaVariable on the object is modified
		ServerObject_SendToClient         = 8, // Cached sendToClient value from shared template
		ServerObject_HyperspaceOnCreate   = 9, // Set when an object should hyperspace on creation
		ServerObject_HyperspaceOnDestruct = 10, // Set when an object should hyperspace on destruction
		ServerObject_DirtyObjectMenuSent  = 11, // set to avoid multiple dirty object menu messages in a frame
		ServerObject_DirtyAttributesSent  = 12, // set to avoid multiple dirty attributes messages in a frame
		ServerObject_NeedsPobFixup        = 13, // set when added to a building that needs to re-add the object in case of pob changes
		ServerObject_Max                  = 14
	};
}

//-----------------------------------------------------------------------

struct TriggerVolumeInfo
{
	PooledString name;
	float radius;
	bool isPromiscuous;

	bool operator==(TriggerVolumeInfo const &rhs) const
	{
		return rhs.radius == radius && rhs.isPromiscuous == isPromiscuous && rhs.name == name;
	}
};

//-----------------------------------------------------------------------

class ServerObject : public Object
{
	friend class ConsoleCommandParserObject;
	friend class ServerObjectBaselinesManager;
	
public:

	struct Messages
	{
		static const char * const TRIGGER_VOLUME_REMOVE;
		static const char * const TRIGGER_VOLUME_CREATE;
	};

public:
	class PobFixupNotification;
	friend class PobFixupNotification;
	class MoveNotification;
	friend class MoveNotification;
	friend class ServerController;

	ServerObject                                                (const ServerObjectTemplate* newTemplate, const ObjectNotification &notifcation = ServerWorldTangibleNotification::getInstance(), bool hyperspaceOnCreate = false);
	virtual                     ~ServerObject                   ();

	static void install();

	static ServerObject * getServerObject(NetworkId const & networkId);
	static ServerObject * asServerObject(Object * object);
	static ServerObject const * asServerObject(Object const * object);

	virtual ServerObject *             asServerObject();
	virtual ServerObject const *       asServerObject() const;
	virtual CreatureObject *           asCreatureObject();
	virtual CreatureObject const *     asCreatureObject() const;
	virtual BuildingObject *           asBuildingObject();
	virtual BuildingObject const *     asBuildingObject() const;
	virtual StaticObject *             asStaticObject();
	virtual StaticObject const *       asStaticObject() const;
	virtual TangibleObject *           asTangibleObject();
	virtual TangibleObject const *     asTangibleObject() const;
	virtual IntangibleObject *         asIntangibleObject();
	virtual IntangibleObject const *   asIntangibleObject() const;
	virtual MissionObject *            asMissionObject();
	virtual MissionObject const *      asMissionObject() const;
	virtual CellObject *               asCellObject();
	virtual CellObject const *         asCellObject() const;
	virtual GroupObject *              asGroupObject();
	virtual GroupObject const *        asGroupObject() const;
	virtual ShipObject *               asShipObject();
	virtual ShipObject const *         asShipObject() const;
	virtual PlayerObject *             asPlayerObject();
	virtual PlayerObject const *       asPlayerObject() const;
	virtual WeaponObject *             asWeaponObject();
	virtual WeaponObject const *       asWeaponObject() const;
	virtual ManufactureSchematicObject *       asManufactureSchematicObject();
	virtual ManufactureSchematicObject const * asManufactureSchematicObject() const;
	virtual PlayerQuestObject *		   asPlayerQuestObject();
	virtual PlayerQuestObject const *  asPlayerQuestObject() const;

	static void                  removeDefaultTemplate          (void);
	static void                  concludeScriptVars             ();
	virtual void                 scheduleForAlter();
	virtual float                alter(float time);
	virtual bool                 alwaysSendReliableTransform    () const;
	BaselinesMessage *           createSharedBaselinesMessage   () const;
	BaselinesMessage *           createSharedNpBaselinesMessage () const;
	void                         applyBaselines                 (BaselinesMessage const &source);
	void                         applyBaselines                 (char type, Archive::ByteStream const & package);
	void                         applyDeltas                    (DeltasMessage const &source);
	void                         applyObjectPositionUpdate      (UpdateObjectPositionMessage const &source);
	void                         updateContainment              (NetworkId const &containerId, int slotArrangement);
	bool                         removeFromBaselineDistributionList(Client const &client) const;
	virtual bool                 canDestroy                     () const;
	virtual bool                 canDropInWorld                 () const;
	virtual bool                 canTrade                       () const;
	virtual bool                 canTradeRecursive              (bool testPlayers) const;
	virtual bool                 markedNoTrade                  () const;
	virtual bool                 markedNoTradeRecursive         (bool testPlayers, bool testOnlyContainedItems = false) const;
	virtual bool                 markedNoTradeShared            (bool includeCheckForNoTrade) const;
	virtual bool                 markedNoTradeRemovable         () const;
	virtual bool                 isInBazaarOrVendor             () const;
	virtual bool                 isInSecureTrade                () const;
	virtual void				 removeAllAuctions				();
	virtual bool                 isBioLinked                    () const;
	virtual bool                 isBioLinkedRecursive           () const;
	bool                         isWaypoint                     () const;
	bool                         checkLOSTo(ServerObject const &target) const;
	bool                         checkLOSTo(Location const &target) const;
	void                         clearProxyList                 ();
	virtual void                 conclude                       ();
	virtual Controller*          createDefaultController        ();
	void                         addMembersToPackages           ();
	const SharedObjectTemplate * getSharedTemplate              () const;
	const char *                 getSharedTemplateName          () const;
	virtual const char *         getClientSharedTemplateName    () const;

	const Unicode::String        getObjectName                  () const;
	const StringId &             getObjectNameStringId          () const;
	const Unicode::String &      getAssignedObjectName          () const;
	const Unicode::String        getAssignedObjectFirstName     () const;
	const Unicode::String        getEncodedObjectName           () const;

	const std::string &          getStaticItemName              () const;
	void                         setStaticItemName              (std::string const & newStaticName);
	int                          getStaticItemVersion           () const;
	void                         setStaticItemVersion           (int newStaticVersion);
	int                          getConversionId                () const;
	void                         setConversionId                (int newConversionId);

	static const unsigned long   getObjectCount                 ();
	const bool                   getPositionChanged             () const;
	const Sphere                 getLocalSphere                 () const;
	const Sphere &               getSphereExtent                () const;
	Vector const &               getTriggerPosition             () const;
	virtual float                getHeight                      () const;
	virtual float                getRadius                      () const;
	virtual int                  getVolume                      () const;
	const char *                 getTemplateName                () const;
	bool                         getHyperspaceOnCreate          () const;
	void                         setHyperspaceOnCreate          (bool hyperspaceOnCreate);
	bool                         getHyperspaceOnDestroy         () const;
	void                         setHyperspaceOnDestroy         (bool hyperspaceOnDestroy);
	uint32                       getTemplateCrc                 () const;
	bool                         isPlayerControlled             () const;
	bool                         isPersisted                    () const;
	float                        getComplexity                  () const;
	void                         setComplexity                  (float complexity);
	bool                         isBazaarTerminal               () const;
	bool                         isVendor                       () const;
	void                         makeVendor                     ();
	ServerObject *               getBazaarContainer             ();
	const ServerObject *         getBazaarContainer             () const;
	virtual void                 addToWorld();
	virtual void                 onAddedToWorld                 ();

	virtual bool                 checkAndAddClientControl(Client* client);
	virtual void                 onClientReady (Client *c);
	virtual void                 onClientAboutToLoad();
	virtual void                 onLoadingScreenComplete();
	virtual void                 onPobFixupComplete();
	bool                         isBeingDestroyed() const;
	bool                         isPlacing() const;
	bool                         isUnloading() const;
	bool                         isGoingToConclude() const;
	bool                         isInEndBaselines() const;
	bool                         isNeedingPobFixup() const;
	bool                         permanentlyDestroy             (DeleteReasons::Enumerator reason);
	virtual void                 persist                        ();
	virtual void                 onRemovingFromWorld            ();
	virtual void                 unload                         ();
	void                         moveToPlayerAndUnload          (const NetworkId &player);
	void                         moveToPlayerBankAndUnload      (const NetworkId &player);
	void                         moveToPlayerDatapadAndUnload   (const NetworkId &player, int maxDepth);
	virtual void                 onPermanentlyDestroyed         ();
	virtual bool                 immediateLogoutAllowed         ();
	void                         handleDisconnect               (bool immediate);
	void                         disconnect                     ();
	void                         triggerMadeAuthoritative       ();
	bool                         getIncludeInBuildout           () const;
	void                         setIncludeInBuildout           ( bool value );


//-----------------------------------------------------------------------
// Object manipulation
// Can this object manipulate other objects.  CreatureObject overrides.  Base class returns false.
	// generally an object is allowed to manipulate another object if it is container or "nearby".
public:
	virtual bool                 canManipulateObject(ServerObject const &target, bool movingObject, bool checkPermission, bool checkPermissionOnParent, float maxDistance, Container::ContainerErrorCode&, bool skipNoTradeCheck = false, bool * allowedByGodMode = nullptr) const;

//-----------------------------------------------------------------------
// container support
	virtual void                          arrangementModified(int oldValue, int newValue, bool isLocal);
	virtual void                          containedByModified(NetworkId const &oldValue, NetworkId const &newValue, bool isLocal);
	virtual void                          onContainerTransferComplete(ServerObject *oldContainer, ServerObject *newContainer);
	void                                  loadContainedObjectFromDB(const NetworkId& oid) const;
	void                                  loadAllContentsFromDB() const;
	virtual void                          onContainedObjectLoaded(const NetworkId &oid);
	virtual void                          onAllContentsLoaded();
	bool                                  areContentsLoaded() const;
	bool                                  getLoadContents() const;
	const NetworkId&                      getLoadWith() const;
	void                                  setLoadWith(const NetworkId& id);
	void                                  setLoadContents(bool loadContents);
	void                                  unloadPersistedContents();
	ServerObject *                        combineResourceContainers(ServerObject &item);
	bool                                  isContainedBy(const ServerObject & container, bool includeContents) const;

	//Container triggers
	virtual bool                          onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual bool                          onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	virtual int                           onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);
	virtual void                          onContainerLostItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual void                          onContainerTransfer(ServerObject * destination, ServerObject* transferer);
	virtual void                          onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);
	virtual bool                          onContainerChildAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual bool                          onContainerChildAboutToGainItem(ServerObject& item, ServerObject* destination, ServerObject* transferer);
	virtual void                          onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer);
	virtual void                          onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);
	void                                  containerDepersistContents(ServerObject *, ServerObject *);
	
	//CTS
	void                                  setLoadCTSBank(bool isForCTS);
	bool                                  getLoadCTSBank() const;

	void                                  setLoadCTSPackedHouses(bool isForCTS);
	bool                                  getLoadCTSPackedHouses() const;
	void								  packedHouseLoaded();

private:
	void                                  synchronizationOnConstruct();
	void                                  synchronizationOnDestruct();
	void                                  synchronizationOnContainerChange(ServerObject const *oldContainer, ServerObject const *newContainer) const;
	void                                  synchronizationOnLoadedInContainer();
	void                                  synchronizationOnUnload() const;
	void                                  containerHandleUpdateProxies(ServerObject* oldContainer, ServerObject* newContainer);
	void                                  containerHandleUpdateClients(ServerObject* oldContainer, ServerObject* newContainer, bool removeFromWorld, bool addToWorld);
	
// ----------------------------------------------------------------------
// Money functions
public:
	bool               transferCashTo               (const NetworkId &target, int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	bool               transferBankCreditsTo        (const NetworkId &target, int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	bool               transferBankCreditsTo        (const std::string &target, int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	bool               transferBankCreditsFrom      (const std::string &target, int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	bool               withdrawCashFromBank         (int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	bool               depositCashToBank            (int amount, const NetworkId &transactionId = NetworkId::cms_invalid);
	int                getCashBalance               () const;
	int                getBankBalance               () const;
	int                getTotalMoney                () const;

public:
	void               scriptTransferCashTo         (const NetworkId &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	void               scriptTransferBankCreditsTo  (const NetworkId &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	void               scriptWithdrawCashFromBank   (int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	void               scriptDepositCashToBank      (int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	void               scriptTransferBankCreditsTo  (const std::string &target, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);
	void               scriptTransferBankCreditsFrom(const std::string &source, int amount, const NetworkId &replyTo, const std::string &successCallback, const std::string &failCallback, const std::vector<int8> &packedDictionary);

	void        sendControllerMessageToAuthServer(GameControllerMessage cm, MessageQueue::Data *msg, float value = 0.0f);
	static std::string  getLogDescription(const ServerObject * object);
	static std::string  getLogDescription(const NetworkId &);

private:
	// makeRoom specifies that if the amount would cause the bank/cash limit to be exceeded,
	// move some credits from the bank/cash into cash/bank/galactic reserve (if there is room
	// there) to make room
	void               internalAdjustBankBalance(int amount, bool makeRoom);
	void               internalAdjustCashBalance(int amount, bool makeRoom);

public:
	void               sendAuthClientBaselines(Client const &client) const;
	void               sendFirstParentBaselines(Client const &client) const;

private:
	void               flushCreateMessages() const;
	void               onAddedToWorldCell();
	void               onRemovedFromWorldCell();

protected:
	virtual void       virtualOnAddedToWorldCell();
	virtual void       virtualOnRemovedFromWorldCell();
	virtual void       virtualOnSetAuthority();
	virtual void       virtualOnReleaseAuthority();
	virtual void       virtualOnSetClient();
	virtual void       virtualOnLogout();
	
//-----------------------------------------------------------------------
// trigger volume support

protected:
	void updateTriggerVolumes();
	void destroyTriggerVolumes();
	void                                      addTriggerVolume (TriggerVolume* t);
	
public:
	
	void                                      createTriggerVolume          (const real radius, const std::string & name, bool isPromiscuous);
	void                                      removeTriggerVolume          (const std::string & name);

	const TriggerVolume *                     getTriggerVolume             (const std::string & volumeName) const;
	TriggerVolume *                           getTriggerVolume             (const std::string & volumeName);

	typedef std::map<PooledString, TriggerVolume *> TriggerVolumeMap;
	TriggerVolumeMap &                        getTriggerVolumeMap          ();

	void                                      onTriggerEnter               (ServerObject & object, TriggerVolume & triggerVolume);
	void                                      onTriggerExit                (ServerObject & object, TriggerVolume & triggerVolume);

	virtual void                              onAddedToTriggerVolume       (TriggerVolume & triggerVolume);
	virtual void                              onRemovedFromTriggerVolume   (TriggerVolume & triggerVolume);

	virtual const std::set<TriggerVolume *> * getTriggerVolumeEntered      () const;

public:
	void                                      setAttributeAttained         (int attribute);
	void                                      clearAttributeAttained       (int attribute);
	bool                                      hasAttributeAttained         (int attribute) const;

	void                                      setAttributeInterested       (int attribute);
	void                                      clearAttributeInterested     (int attribute);
	bool                                      hasAttributeInterest         (int attribute) const;

	bool                                      isInterested                 (const ServerObject &rhs) const;

//-----------------------------------------------------------------------
// broadcast system

private:
	void                          addBroadcastListener      (std::string const & messageName, NetworkId const & listener);
	void                          removeBroadcastListener   (std::string const & messageName, NetworkId const & listener);
	void                          removeBroadcastListener   (NetworkId const & listener);
	void                          addBroadcastBroadcaster   (std::string const & messageName, NetworkId const & broadcaster);
	void                          removeBroadcastBroadcaster(std::string const & messageName, NetworkId const & broadcaster);
	void                          removeBroadcastAllBroadcasters();
	void                          stopListeningToAllBroadcastMessages();

public:
	typedef std::pair<std::string, NetworkId>                    BroadcastMapItem;
	typedef std::set<BroadcastMapItem>                           BroadcastMap;
	BroadcastMap const &          getBroadcastListeners          () const;
	BroadcastMap const &          getBroadcastBroadcasters       () const;
	void                          listenToBroadcastMessage       (std::string const & messageName, NetworkId const & broadcaster);
	void                          stopListeningToBroadcastMessage(std::string const & messageName, NetworkId const & broadcaster);

//-----------------------------------------------------------------------

public:

	void                          addSynchronizedUi              (const std::vector<NetworkId> & clients);
	void                          addSynchronizedUiClient        (ServerObject & client);
	bool                          areContentsVisibleToClient     (const Client &) const;
	void                          attachStartupScripts           ();
	void                          customize                      (const std::string & customName, int value);
	void                          serverObjectEndBaselines       (bool fromDatabase);
	unsigned long                 getAndIncrementMoveSequenceNumber ();
	uint32                        getAuthServerProcessId         () const;
	const int                     getCacheVersion                () const;
	Client *                      getClient                      () const;
	int                           getObserversCount              () const;
	std::set<Client *> const & getObservers                   () const;
	void                          addObserver                    (Client * client);
	void                          removeObserver                 (Client * client);
	void                          clearObservers                 ();
	void                          updateObserversCount           (int delta);
	void                          onObserversChanged             ();

	size_t                        getProxyCount                  () const;
	ProxyList const &             getExposedProxyList            () const;
	bool                          isProxiedOnServer              (uint32 pid) const;
	void                          addServerToProxyList           (uint32 proxyServerProcessId);
	void                          removeServerFromProxyList      (uint32 proxyServerProcessId);

	// objvar functions
	DynamicVariableList const &   getObjVars                     () const;
	void                          removeObjVarItem(std::string const &name);
	void                          eraseObjVarListEntries(std::string const &name);
	void                          copyObjVars(std::string const &name, ServerObject const &srcObject, std::string const &srcVarName);
	bool                          setObjVarItem(std::string const &name, int value);
	bool                          setObjVarItem(std::string const &name, std::vector<int> const &value);
	bool                          setObjVarItem(std::string const &name, float value);
	bool                          setObjVarItem(std::string const &name, std::vector<float> const &value);
	bool                          setObjVarItem(std::string const &name, std::string const &value);
	bool                          setObjVarItem(std::string const &name, Unicode::String const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<Unicode::String> const &value);
	bool                          setObjVarItem(std::string const &name, NetworkId const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<NetworkId> const &value);
	bool                          setObjVarItem(std::string const &name, DynamicVariableLocationData const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<DynamicVariableLocationData> const &value);
	bool                          setObjVarItem(std::string const &name, StringId const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<StringId> const &value);
	bool                          setObjVarItem(std::string const &name, Transform const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<Transform> const &value);
	bool                          setObjVarItem(std::string const &name, Vector const &value);
	bool                          setObjVarItem(std::string const &name, std::vector<Vector> const &value);
	bool                          setObjVarItem(std::string const &name, DynamicVariable const &value);

	std::string const             getPackedObjVars(std::string const &prefix) const;
	bool                          setPackedObjVars(std::string const &packedVarString);

	const std::string &           getSceneId                     () const;
	GameScriptObject *            getScriptObject                (); 
	const GameScriptObject *      getScriptObject                () const;
	ServerSynchronizedUi *        getSynchronizedUi              ();
	const ServerSynchronizedUi *  getSynchronizedUi              () const;
	bool                          getTransformChanged() const;
	void                          deliverMessageTo               (MessageToPayload & message);
	int                           cancelMessageTo                (std::string const & messageName);
	int                           cancelMessageToByMessageId     (NetworkId const & messageId);
	int                           timeUntilMessageTo             (std::string const & messageName) const;
	unsigned long                 processQueuedMessageTos        (unsigned long effectiveMessageToTime);
	std::string                   debugGetMessageToList          () const;
	bool                          handleTeleportFixup            (bool force);
	bool                          serverObjectInitializeFirstTimeObject(ServerObject *cell, Transform const &transform);
	bool                          getLocalFlag                   (int flag) const;
	void                          setLocalFlag                   (int flag, bool enabled);
	virtual bool                  isInitialized() const;
	bool                          isInWorldCell() const;
	virtual bool                  isVisibleOnClient              (const Client & client) const = 0;
	virtual void                  kill                           ();

	void                          performSocial                  (const NetworkId & target, unsigned long socialType, bool animationOk, bool textOk);
	void                          performSocial                  (const MessageQueueSocial & socialMsg);

	void                          performCombatSpam              (const MessageQueueCombatSpam & combatSpam, bool sendToSelf, bool sendToTarget, bool sendToBystanders);

	void                          releaseAuthority               (uint32 newAuthoritativeProcessId);
	void                          removeSynchronizedUiClient     (const NetworkId & clientId);

	void                          seeSocial                      (const MessageQueueSocial & socialMsg);
	void                          seeCombatSpam                  (const MessageQueueCombatSpam & spamMsg);

	bool                          getSendToClient                () const;
	void                          sendCreateAndBaselinesToClient (Client &client) const;
	void                          sendCreateAndBaselinesToDatabaseServer() const;
	void                          forwardServerCreateAndBaselines() const;
	void                          sendToClientsInUpdateRange     (const GameNetworkMessage & message, bool reliable, bool includeSelf=true) const;
	static void                   sendToSpecifiedClients         (const GameNetworkMessage & message, bool reliable, const std::vector<NetworkId> & clients);
	void                          appendMessage                  (int message, float value, uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	void                          appendMessage                  (int message, float value, MessageQueue::Data *data, uint32 flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	virtual void                  setAuthServerProcessId         (uint32 processId);
	void                          setClient                      (Client &newClient);
	void                          clearClient                    ();
	virtual void                  setParentCell                  (CellProperty *cellProperty);
	virtual void                  setAuthority                   ();
	void                          setObjectName                  (const Unicode::String &newName);
	void                          setObjectNameStringId          (const StringId & id);
	void                          setSceneId                     (const std::string & sceneId);
	virtual const NetworkId &     getOwnerId() const;
	virtual bool                  isOwner(NetworkId const &id, Client const *client) const;
	virtual void                  setOwnerId(const NetworkId &id);
	void                          setSceneIdOnThisAndContents    (const std::string & sceneId);
	void                          setPlayerControlled            (bool newValue);
	void                          speakText                      (NetworkId const &target, unsigned long chatType, unsigned long mood, unsigned long flags, Unicode::String const &speech, int language, Unicode::String const &oob);
	virtual void                  speakText                      (MessageQueueSpatialChat const &spatialChat);
	virtual void                  hearText                       (ServerObject const &source, MessageQueueSpatialChat const &spatialChat, int chatMessageIndex);
	void                          teleportObject                 (Vector const & position_w, NetworkId const &targetContainer, std::string const &targetCellName, Vector const &position_p, std::string const &scriptCallback, bool forceLoadScreen = false);
	virtual void                  changeTeleportDestination      (Vector & position, float & yaw) const;
	void                          transferAuthority              (uint32 newAuthoritativeGameServerProcessId, bool skipLoadScreen, bool handlingCrash, bool informPlanetServer);
	void                          transferAuthority              (uint32 newAuthoritativeGameServerProcessId, bool skipLoadScreen, bool handlingCrash, const NetworkId &goalCell, const Transform &goalTransform, bool informPlanetServer);
	void                          updatePositionOnPlanetServer   (bool forceUpdate = false);
	int                           getGameObjectType              () const;

	typedef std::vector<std::pair<std::string, Unicode::String> > AttributeVector;
	virtual void                  getAttributes                  (AttributeVector &data) const = 0;
	virtual void                  getAttributes                  (const NetworkId & playerId, AttributeVector &data) const;
	virtual void                  getAttributesForAuction        (AttributeVector &data) const;

	void                          retrieveStoredObjectAttributes (AttributeVector & av) const;
	void                          storeObjectAttributes          (const ServerObject & prototype);
	void                          clearStoredObjectAttributes    ();
	void                          replaceStoredObjectAttribute   (Unicode::String const & attribute, Unicode::String const & value);

	void                          clearDeltas() const;
	virtual bool                  wantSawAttackTriggers() const;
	void                          setTeleportScriptCallback(std::string const &scriptCallback);
	void                          setInteriorTeleportDestination(NetworkId const &destContainerId, std::string const &destCellName, Vector const &destPosition_p);

	void                          sendDirtyObjectMenuNotification ();
	void                          sendDirtyAttributesNotification ();

	virtual std::string           getItemLog() const;

	void                          checkAndLogInvalidTransform() const;

	void                          setLayer (TerrainGenerator::Layer* layer);
	TerrainGenerator::Layer*      getLayer () const;
	virtual void                  getAuthClients(std::set<Client const *> &authClients) const;
	virtual void                  setCacheVersion (const int cacheVersion);

	void                          setDefaultAlterTime(float time);
	float                         getDefaultAlterTime() const;

	bool hasAttributeCaching() const;
	int getAttributeRevision() const;
	void setAttributeRevisionDirty();

	// ai patrol pathing support
	void                                 setPatrolPathRoot         (const ServerObject & root);
	const std::set<CachedNetworkId> & getPatrolPathRoots        () const;
	void                                 addPatrolPathingObject    (const ServerObject & ai);
	void                                 removePatrolPathingObject (const ServerObject & ai);
	void                                 addPatrolPathObserver     ();
	void                                 removePatrolPathObserver  ();
	int                                  getPatrolPathObservers    () const;
	bool                                 isPatrolPathNode          () const;
	bool                                 isPatrolPathRoot          () const;

	void                          setDescriptionStringId(const StringId &);
	StringId const &              getDescriptionStringId();

protected:

	virtual void                  addObjectToConcludeList();
	void                          onAutoDeltaChanged();
	virtual void                  endBaselines();
	virtual void                  onLoadedFromDatabase();
	virtual void                  initializeFirstTimeObject();
	void                          sendDeltasForSelfAndContents(ProxyList const *proxyList = 0) const;
	void                          sendDeltas(ProxyList const *proxyList = 0) const;

	//Package functions, for adding shared data to be auto-synchronized.
	//_np is the package that gets ignored by the DB (i.e. not persisted)
	void                    addAuthClientServerVariable    (Archive::AutoDeltaVariableBase & Source);
	void                    addAuthClientServerVariable_np (Archive::AutoDeltaVariableBase & Source);
	void                    addFirstParentAuthClientServerVariable(Archive::AutoDeltaVariableBase & source);
	void                    addFirstParentAuthClientServerVariable_np(Archive::AutoDeltaVariableBase & source);
	void                    addServerVariable              (Archive::AutoDeltaVariableBase & Source);
	void                    addServerVariable_np           (Archive::AutoDeltaVariableBase & Source);
	void                    addSharedVariable              (Archive::AutoDeltaVariableBase & source);
	void                    addSharedVariable_np           (Archive::AutoDeltaVariableBase & source);
	
	virtual bool            handleContentsSetup();
	virtual void            forwardServerObjectSpecificBaselines() const;
	virtual void            sendObjectSpecificBaselinesToClient(Client const &client) const;
	virtual void            updatePlanetServerInternal     (bool forceUpdate) const;

	virtual void            handleCMessageTo               (MessageToPayload const & message);

public:
	void         createFarNetworkUpdateVolume(float const overrideVal = 0.0f);
	void         destroyFarNetworkUpdateVolume();
	bool         isNetworkUpdateFarTriggerVolume(TriggerVolume const &triggerVolume) const;
	TriggerVolume *getNetworkTriggerVolume();
	virtual float getFarNetworkUpdateRadius() const;
	virtual void getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

private:
	ServerObject(const ServerObject & other);
	ServerObject& operator=(const ServerObject & rhs);

	virtual ServerSynchronizedUi * createSynchronizedUi ();

	bool         handlePlayerInInteriorSetup(ContainedByProperty* containedBy);
	void         markChildrenPersisted();
	void         beginAuthTransfer(uint32 pid) const;
	void         transferAuthoritySceneChange(uint32 pid);
	void         transferAuthorityNoSceneChange(uint32 pid, bool skipLoadScreen, const NetworkId &goalCell, const Transform &goalTransform, bool informPlanetServer);
	void         addPendingSynchronizedUi(const ServerObject & uiObject);

	ProxyList const &getPrivateProxyList() const;
	void             copyProxyList(ServerObject const &source);

	void         handleMessageTo   (MessageToPayload const & message);
	void         pushMessageTo     (MessageToPayload const & newMessage);
	void         popMessageTo      ();
	void         removeMessageTo   (unsigned int position);

	void         clearObserversCount();

protected:
	void         setTransformChanged (bool);
	void         updateWorldSphere();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	void setInitialized(bool initialized);
	void setBeingDestroyed(bool beingDestroyed);
	void setPlacing(bool placing);
	void setUnloading(bool unloading);
	void setGoingToConclude(bool goingToConclude);
	void setInEndBaselines(bool inEndBaselines);
	void setNeedsPobFixup(bool needsPobFixup);

	float getLocationReservationRadius() const;

	virtual void observersCountChanged();

	Vector                        m_oldPosition;

private:

	struct AuthProcessIdCallback
	{
		void modified(ServerObject &target, uint32 oldValue, uint32 newValue, bool isLocal) const;
	};

	struct ObserversCountCallback
	{
		void modified(ServerObject &target, int oldValue, int newValue, bool isLocal) const;
	};
	friend struct ObserversCountCallback;

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
	const        SharedObjectTemplate * m_sharedTemplate;			// template shared between client and server

	static float ms_buildingUpdateRadiusMultiplier;
	static void setBuildingUpdateRadiusMultiplier(float m);

	/** If this object is being controlled by a client, this pointer will be set.  nullptr otherwise
	 */
	Client *              m_client;
	std::set<Client *>    m_observers;
	uint32                m_localFlags;
	GameScriptObject *    m_scriptObject;
	DynamicVariableList   m_objVars;
	
	Archive::AutoDeltaVariableCallback<int, ObserversCountCallback, ServerObject> m_observersCount; // the total number of clients observing this object and its proxies
	
	Archive::AutoDeltaVariable<int>   m_volume;			// volume this object takes up
	Archive::AutoDeltaVariable<float> m_complexity;		// difficulty crafting/manufacturing the object

	/** Which scene is this object in?  For development -- will be replaced
	 * by a spacial division node number or something similar.
	 */
	Archive::AutoDeltaVariable<PooledString> m_sceneId;

	/** A name for this object.  For characters, will be their character name.
	 * May be empty for other types of objects.
	 */
	Archive::AutoDeltaVariable<Unicode::String> m_objectName;

	/** set this flag to true if you want it included
	 * in buildout file exports
	 */
	Archive::AutoDeltaVariable<bool>            m_includeInBuildout;
	
	/** A name for this object.  It is overridden by the existance of a characterName datamember.  Otherwise
	 *  it is an id for the entry in the localized string table for the name of the object.
	 */
	Archive::AutoDeltaVariable<StringId> m_nameStringId;

	Archive::AutoDeltaVariable<StringId> m_descriptionStringId;

	Archive::AutoDeltaVariable<std::string> m_staticItemName;
	Archive::AutoDeltaVariable<int>         m_staticItemVersion;

	Archive::AutoDeltaVariable<int>         m_conversionId;

	/** Flag for whether this object is player-controlled (e.g. a player or a vehicle piloted by a player)
	 */
	Archive::AutoDeltaVariable<bool> m_playerControlled;

	/** SynchronizedUI package for objects that require real time updated UI.
	 */

	Watcher<ServerSynchronizedUi>                 m_synchronizedUi;
	// list of objects with sync ui that are waiting for this object to have a client
	// attached, due to an authority transfer
	std::vector<NetworkId> *            m_pendingSyncUi;

	/** Flag for whether the object is persisted
	 */
	Archive::AutoDeltaVariable<bool> m_persisted;

	Archive::AutoDeltaVariableCallback<uint32, AuthProcessIdCallback, ServerObject> m_authServerProcessId;
	Archive::AutoDeltaSet<uint32> m_proxyServerProcessIds;

	Archive::AutoDeltaVariable<unsigned long> m_transformSequence;

	Archive::AutoDeltaVariable<int> m_cacheVersion;
	Archive::AutoDeltaVariable<bool> m_loadContents;
	mutable Archive::AutoDeltaVariable<bool> m_contentsLoaded;
	mutable Archive::AutoDeltaVariable<bool> m_contentsRequested;
	NetworkId m_loadWith;
	
	// There are no setters for these deliberately!  Don't add setters.  Use the money transfer functions
	// instead.
	Archive::AutoDeltaVariable<int> m_cashBalance;
	Archive::AutoDeltaVariable<int> m_bankBalance;

	Archive::AutoDeltaVariable<float> m_defaultAlterTime;

	// broadcast system
	Archive::AutoDeltaSet<BroadcastMapItem>   m_broadcastListeners;    // who is currently listening to me
	Archive::AutoDeltaSet<BroadcastMapItem>   m_broadcastBroadcasters; // who I am currently listening to

protected:
	/** Synchronization packages.  _np indicates that it is ignored by the DB (ie not persisted)
	 *  authClient refers to the authoritative client for this object (if it has one)
	 *  server refers to proxies of this object on other servers
	 *  authClientServer is both of the above
	 *  shared is all of the above + proxies on other clients
   *  firstParentAuthClient refers to the auth client for the firstParentInWorld of this object (if any)
	 */
	Archive::AutoDeltaByteStream m_authClientServerPackage;
	Archive::AutoDeltaByteStream m_authClientServerPackage_np;
	Archive::AutoDeltaByteStream m_firstParentAuthClientServerPackage;
	Archive::AutoDeltaByteStream m_firstParentAuthClientServerPackage_np;
	Archive::AutoDeltaByteStream m_serverPackage;
	Archive::AutoDeltaByteStream m_serverPackage_np;
	Archive::AutoDeltaByteStream m_sharedPackage;
	Archive::AutoDeltaByteStream m_sharedPackage_np;


	TriggerVolume *                        m_networkUpdateFar;
	TriggerVolumeMap                       m_triggerVolumes;

	Archive::AutoDeltaVariable<BitArray>   m_attributesAttained;
	Archive::AutoDeltaVariable<BitArray>   m_attributesInterested;

private:
	
	Archive::AutoDeltaVector<TriggerVolumeInfo, ServerObject> m_triggerVolumeInfo;
	Archive::AutoDeltaMap<std::pair<std::pair<unsigned long, uint64>, MessageToId>, MessageToPayload> m_messageTos;

	Sphere                                 m_worldSphere;

	//members to be added
//	equipmentSlotType;

	Archive::OnDirtyCallback<ServerObject>       m_dirtyArchiveCallback;

	mutable int                                  m_unreliableControllerMessageSequenceNumber;
	int                                          m_gameObjectType;
	bool                                         m_calledTriggerDestroy;
	bool                                         m_calledTriggerRemovingFromWorld;
	bool                                         m_loadCTSBank;
	bool                                         m_loadCTSPackedHouses;
};

//-----------------------------------------------------------------------

inline Client * ServerObject::getClient(void) const
{
	return m_client;
}

//-----------------------------------------------------------------------

inline const SharedObjectTemplate * ServerObject::getSharedTemplate() const
{
	if (m_sharedTemplate != nullptr)
		return m_sharedTemplate;
	return getDefaultSharedTemplate();
}

//-----------------------------------------------------------------------

inline const StringId & ServerObject::getObjectNameStringId() const
{
	return m_nameStringId.get();
}

//-----------------------------------------------------------------------

inline DynamicVariableList const &ServerObject::getObjVars() const
{
	return m_objVars;
}

//-----------------------------------------------------------------------

inline const std::string & ServerObject::getSceneId() const
{
	return m_sceneId.get();
}

// ----------------------------------------------------------------------

inline uint32 ServerObject::getTemplateCrc() const
{
	return getObjectTemplate()->getCrcName().getCrc();
}

//-----------------------------------------------------------------------

inline bool ServerObject::isPlayerControlled() const
{
	return m_playerControlled.get();
}

// ----------------------------------------------------------------------

inline float ServerObject::getComplexity() const
{
	return m_complexity.get();
}

// ----------------------------------------------------------------------

inline void ServerObject::setComplexity(float complexity)
{
	m_complexity = complexity;
}

//-----------------------------------------------------------------------

inline ServerObject::TriggerVolumeMap & ServerObject::getTriggerVolumeMap()
{
	return m_triggerVolumes;
}

//-----------------------------------------------------------------------

inline const std::set<TriggerVolume *> * ServerObject::getTriggerVolumeEntered() const
{
	return nullptr;
}

//-----------------------------------------------------------------------

inline ServerObject::BroadcastMap const & ServerObject::getBroadcastListeners() const
{
	return m_broadcastListeners.get();
}

//-----------------------------------------------------------------------

inline ServerObject::BroadcastMap const & ServerObject::getBroadcastBroadcasters() const
{
	return m_broadcastBroadcasters.get();
}

//-----------------------------------------------------------------------

inline void ServerObject::setSceneId(const std::string & newSceneId)
{
	m_sceneId = newSceneId;
}

//-----------------------------------------------------------------------

inline ServerSynchronizedUi *        ServerObject::getSynchronizedUi            ()
{
	return m_synchronizedUi;
}

//----------------------------------------------------------------------

inline const ServerSynchronizedUi *  ServerObject::getSynchronizedUi            () const
{
	return m_synchronizedUi;
}

//-----------------------------------------------------------------------

inline GameScriptObject* ServerObject::getScriptObject()
{
	return m_scriptObject;
}

//-----------------------------------------------------------------------

inline const GameScriptObject* ServerObject::getScriptObject() const
{
	return m_scriptObject;
}

//-----------------------------------------------------------------------


inline int ServerObject::getTotalMoney() const
{
	return getCashBalance() + getBankBalance();
}

//-----------------------------------------------------------------

inline const Unicode::String & ServerObject::getAssignedObjectName () const
{
	return m_objectName.get ();
}

// ----------------------------------------------------------------------

inline bool ServerObject::isPersisted() const
{
	return m_persisted.get();
}

// ----------------------------------------------------------------------

inline bool ServerObject::getLoadContents() const
{
	return m_loadContents.get();
}

//------------------------------------------------------------------------------------------

inline const NetworkId& ServerObject::getLoadWith() const
{
	return m_loadWith;//.get();
}

// ----------------------------------------------------------------------


inline int ServerObject::getCashBalance() const
{
	return m_cashBalance.get();
}

// ----------------------------------------------------------------------

inline int ServerObject::getBankBalance() const
{
	return m_bankBalance.get();
}

// ----------------------------------------------------------------------

inline ProxyList const &ServerObject::getPrivateProxyList() const
{
	return m_proxyServerProcessIds.get();
}

//-----------------------------------------------------------------------

inline int ServerObject::getObserversCount() const
{
	return m_observersCount.get();
}

//-----------------------------------------------------------------------

inline std::set<Client *> const &ServerObject::getObservers() const
{
	return m_observers;
}

// ----------------------------------------------------------------------

inline bool ServerObject::getLocalFlag(int flag) const
{
	return (m_localFlags&(1u<<flag)) ? true : false;
}

// ----------------------------------------------------------------------

inline void ServerObject::setLocalFlag(int flag, bool enabled)
{
	if (enabled)
		m_localFlags |= (1u<<flag);
	else
		m_localFlags &= ~(1u<<flag);
}

// ----------------------------------------------------------------------

inline float ServerObject::getDefaultAlterTime() const
{
	return m_defaultAlterTime.get();
}

// ----------------------------------------------------------------------

inline const std::string & ServerObject::getStaticItemName() const
{
	return m_staticItemName.get();
}

// ----------------------------------------------------------------------

inline void ServerObject::setStaticItemName(std::string const & newStaticName)
{
	m_staticItemName = newStaticName;
}

// ----------------------------------------------------------------------

inline int ServerObject::getStaticItemVersion() const
{
	return m_staticItemVersion.get();
}

// ----------------------------------------------------------------------

inline void ServerObject::setStaticItemVersion(int newStaticVersion)
{
	m_staticItemVersion = newStaticVersion;
}

// ----------------------------------------------------------------------

inline int ServerObject::getConversionId() const
{
	return m_conversionId.get();
}

// ----------------------------------------------------------------------

inline void ServerObject::setConversionId(int newConversionId)
{
	m_conversionId = newConversionId;
}

//----------------------------------------------------------------------

inline int ServerObject::getGameObjectType() const
{
	return m_gameObjectType;
}

// ======================================================================

namespace Archive
{
	void put(Archive::ByteStream &target, TriggerVolumeInfo const &source);
	void get(Archive::ReadIterator &source, TriggerVolumeInfo &target);
}

// ======================================================================

#endif	// INCLUDED_ServerObject_H

