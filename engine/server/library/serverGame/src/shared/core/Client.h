// Client.h
// copyright 2000 Verant Interactive


#ifndef    _CLIENT_H
#define    _CLIENT_H

//-----------------------------------------------------------------------

#include "serverUtility/AdminAccountManager.h"
#include "serverGame/ConfigServerGame.h"

#include "Unicode.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "serverGame/ServerSynchronizedUi.h"

#include "sharedGame/CombatDataTable.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Object.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <unordered_set>
#include <map>

class ConnectionServerConnection;

class GameNetworkMessage;

class CreatureObject;

class ObjectList;

class PlayerObject;

class ServerObject;

class TangibleObject;

struct ClientDestroy;

//-----------------------------------------------------------------------


class Client : public MessageDispatch::Receiver, public MessageDispatch::Emitter {
    friend class ClientChannel;

public:
    typedef std::map<uint32, int> AccountFeatureIdList;

    Client(ConnectionServerConnection &connection, const NetworkId &characterObjectId, const std::string &accountName, const std::string &ipAddr, bool isSecure, bool isSkipLoadScreen, unsigned int stationId, std::set <NetworkId> const &observedObjects, uint32 gameFeatures, uint32 subscriptionFeatures, AccountFeatureIdList const &accountFeatureIds, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints, std::vector <std::pair<NetworkId, std::string>> const &consumedRewardEvents, std::vector <std::pair<NetworkId, std::string>> const &claimedRewardItems, bool usingAdminLogin, CombatDataTable::CombatSpamFilterType combatSpamFilter, int combatSpamRangeSquaredFilter, int furnitureRotationDegree, bool hasUnoccupiedJediSlot, bool isJediSlotCharacter, bool sendToStarport = false);

    virtual                  ~Client();

    static void install();

    void addControlledObject(ServerObject &object);

    void clearControlledObjects();

    ServerObject *findControlledObject(NetworkId const &id) const;

    std::vector<ServerObject *> const &getControlledObjects() const;

    const Unicode::String &getCharacterName() const; //@todo is this used?
    const std::string &getIpAddress() const;

    ServerObject *getCharacterObject() const;

    const NetworkId &getCharacterObjectId() const;

    const std::string &getAccountName() const;

    const unsigned int getStationId() const;

    uint32 getGameFeatures() const;

    uint32 getSubscriptionFeatures() const;

    AccountFeatureIdList const &getAccountFeatureIds() const;

    void setAccountFeatureIds(AccountFeatureIdList const &accountFeatureIds);

    void setAccountFeatureId(uint32 featureId, int count);

    unsigned int getEntitlementTotalTime() const;

    unsigned int getEntitlementEntitledTime() const;

    unsigned int getEntitlementTotalTimeSinceLastLogin() const;

    unsigned int getEntitlementEntitledTimeSinceLastLogin() const;

    bool isFreeTrialAccount() const;

    int getBuddyPoints() const;

    bool isSecure() const;

    std::vector <std::pair<NetworkId, std::string>> const &getConsumedRewardEvents() const;

    std::vector <std::pair<NetworkId, std::string>> const &getClaimedRewardItems() const;

    bool isUsingAdminLogin() const;

    void setUsingAdminLogin(bool value);

    CombatDataTable::CombatSpamFilterType getCombatSpamFilter() const;

    int getCombatSpamRangeSquaredFilter() const;

    bool
    shouldReceiveCombatSpam(NetworkId const &attacker, Vector const &attackerPosition_w, NetworkId const &defender, Vector const &defenderPosition_w) const;

    int getFurnitureRotationDegree() const;

    bool getHasUnoccupiedJediSlot() const;

    bool getIsJediSlotCharacter() const;

    bool getSendToStarport() const;

    ConnectionServerConnection *getConnection();

    // sync stamp for dealing with times synced to our connection server
    uint16 getServerSyncStampShort() const;

    uint32 getServerSyncStampLong() const;

    float computeDeltaTimeInSeconds(uint32 syncStampLong) const;

    WatchedByList &getWatchedByList() const;

    int getConnectionServerLag() const;

    int getGameServerLag() const;

    MessageDispatch::Transceiver<ClientDestroy &> &getDestroyNotifier();

    bool getIsReady() const;

    bool isControlled(NetworkId const &id) const;

    bool isGod() const;

    int getGodLevel() const;

    bool isGodValidated() const;

    void onLoadPlayer(CreatureObject &newCharacter);

    void assumeControl(CreatureObject &newCharacter);

    void addSynchronizedUi(ServerSynchronizedUi *sync);

    void removeSynchronizedUi(ServerSynchronizedUi const *sync);

    void receiveClientMessage(const GameNetworkMessage &message);

    void receiveMessage(const MessageDispatch::Emitter &source, const MessageDispatch::MessageBase &message);

    void removeControlledObject(ServerObject &object);

    void selfDestruct();

    void selfDestruct(const bool immediate);

    /** send() sends a message to the client via the connection server.
    */
    void send(const GameNetworkMessage &outgoingMessage, bool reliable) const;

    void sendToConnectionServer(const GameNetworkMessage &outgoingMessage);

    bool setGodMode(bool status);

    struct ServerObjectPointerHash {
        size_t operator()(ServerObject *const ptr) const {
            return (reinterpret_cast<const size_t>(ptr) >> 4);
        };
    };

    struct TangibleObjectPointerHash {
        size_t operator()(TangibleObject *const ptr) const {
            return (reinterpret_cast<const size_t>(ptr) >> 4);
        };
    };

    typedef std::unordered_set<ServerObject *, ServerObjectPointerHash> ObservingList;

    // objects being observed that are in this list needs
    // to have their pvp status updated when there are changes
    // that requires pvp status to be recalcuclated
    typedef std::unordered_set<TangibleObject *, TangibleObjectPointerHash> ObservingListPvpSync;

    ObservingList const &getObserving() const;

    ObservingListPvpSync const &getObservingPvpSync() const;

    void addObserving(ServerObject *o);

    void addObservingPvpSync(TangibleObject *to);

    void removeObserving(ServerObject *o);

    void removeObservingPvpSync(ServerObject *o);

    void removeAllObserving();

    std::set<ServerObject *> &getOpenedContainers();

    std::set<ServerObject *> const &getOpenedContainers() const;

    void resetIdleTimeAfterCharacterTransfer();

    void resetIdleTime();

    bool idledTooLong() const;

    void handleIdleDisconnect() const;

    void openContainer(ServerObject &obj, int sequence, std::string const &slotDesc);

    void closeContainer(ServerObject &obj);

    void observeContainer(ServerObject &obj, int sequence, std::string const &slotDesc);

    void launchWebBrowser(std::string const &url) const;

    class ClientMessage : public MessageDispatch::MessageBase {
    public:
        ClientMessage(const std::string &message);

        ~ClientMessage();

    private:
        ClientMessage();

        ClientMessage(const ClientMessage &source);

        ClientMessage &operator=(const ClientMessage &rhs);
    };

    static std::map <std::string, uint32> &getPacketBytesPerMinStats();

private:
    Client(const Client &);

    Client &operator=(const Client &);

private:
    std::string m_accountName;
    Unicode::String m_characterName;
    NetworkId m_characterObjectId;
    ConnectionServerConnection *m_connection;
    std::vector<ServerObject *> m_controlledObjects;
    int m_godLevel;
    bool m_godMode;
    bool m_godValidated;
    std::string m_ipAddress;
    bool m_isReady;
    bool m_isSecure;
    bool m_isSkipLoadScreen;
    CachedNetworkId m_primaryControlledObject; // This will be the creature object representing the player (or possibly a vehicle in the future)
    MessageDispatch::Transceiver<ClientDestroy &> destroyNotifier;

    ObservingList m_observing;

    // for optimization, we keep the list of observed objects
    // that needs to have their pvp status updated when there
    // are changes that requires pvp status to be recalcuclated;
    // this is so that we don't have to iterate over the
    // entire list of observed objects (which can get pretty
    // large) to find these particular observed objects
    ObservingListPvpSync m_observingPvpSync;

    std::set<ServerObject *> m_openedContainers;
    mutable WatchedByList m_watchedByList;
    unsigned long m_lastNonIdleTime;
    unsigned int m_stationId;
    int m_connectionServerLag;
    int m_gameServerLag;
    uint32 m_gameFeatures;
    uint32 m_subscriptionFeatures;
    AccountFeatureIdList m_accountFeatureIds;
    unsigned int m_entitlementTotalTime;
    unsigned int m_entitlementEntitledTime;
    unsigned int m_entitlementTotalTimeSinceLastLogin;
    unsigned int m_entitlementEntitledTimeSinceLastLogin;
    int m_buddyPoints;
    std::set <NetworkId> m_previousObservedObjects;
    std::vector <Watcher<ServerSynchronizedUi>> m_syncUIs;
    std::vector <std::pair<NetworkId, std::string>> m_consumedRewardEvents;
    std::vector <std::pair<NetworkId, std::string>> m_claimedRewardItems;
    bool m_usingAdminLogin;
    CombatDataTable::CombatSpamFilterType m_combatSpamFilter;
    int m_combatSpamRangeSquaredFilter;
    int m_furnitureRotationDegree;

    // these 2 come from the client (via the LoginServer) so should not be trusted;
    // we cache them here to do preliminary checks to avoid unnecessarily sending
    // commands to the LoginServer (assuming the values haven't been hacked on the
    // client); when we do send the commands to the LoginServer, we'll check again there
    bool m_hasUnoccupiedJediSlot;
    bool m_isJediSlotCharacter;

    bool m_sendToStarport;

    static std::map <std::string, uint32> sm_outgoingBytesMap_Working;  // working stats that will rotate after 1 minute
    static std::map <std::string, uint32> sm_outgoingBytesMap_Stats;    // computed stats from the last minute
    static uint32 sm_outgoingBytesMap_Worktime; // time we started filling in the working map
};

//-----------------------------------------------------------------------

struct ClientDestroy {
    explicit ClientDestroy(Client *c) : client(c) {
    };

    ~ClientDestroy() {};
    Client *client;
};

//-----------------------------------------------------------------------

inline bool Client::isGodValidated() const {
    if (isGod()) {
        return m_godValidated;
    }

    return false;
}

//-----------------------------------------------------------------------

inline const Unicode::String &Client::getCharacterName() const {
    return m_characterName;
}

//-----------------------------------------------------------------------

inline const NetworkId &Client::getCharacterObjectId() const {
    return m_characterObjectId;
}

//-----------------------------------------------------------------------

inline ServerObject *Client::getCharacterObject() const {
    return findControlledObject(getCharacterObjectId());
}

//-----------------------------------------------------------------------

inline ConnectionServerConnection *Client::getConnection() {
    return m_connection;
}

//-----------------------------------------------------------------------

inline MessageDispatch::Transceiver<ClientDestroy &> &Client::getDestroyNotifier() {
    return destroyNotifier;
}

//-----------------------------------------------------------------------

inline bool Client::getIsReady() const {
    return m_isReady;
}

//-----------------------------------------------------------------------

inline bool Client::isGod() const {
    // likewise if we are someone on a testbed we'll just return anyway if they set god to all
    if (m_godLevel > 0 || ConfigServerGame::getAdminGodToAll()) {
        return m_godMode;
    }

    return false;
}

//-----------------------------------------------------------------------

inline int Client::getGodLevel() const {
    if (isGod()) {
        return m_godLevel;
    } else {
        return -1; // because they may be aliased in the admin table, but don't want to be in gm right now
    }
}

//-----------------------------------------------------------------------

inline Client::ObservingList const &Client::getObserving() const {
    return m_observing;
}

//-----------------------------------------------------------------------

inline Client::ObservingListPvpSync const &Client::getObservingPvpSync() const {
    return m_observingPvpSync;
}

//-----------------------------------------------------------------------

inline void Client::removeObserving(ServerObject *o) {
    if (o) {
        IGNORE_RETURN(m_observing.erase(o));
        removeObservingPvpSync(o);
    }
}

//-----------------------------------------------------------------------

inline void Client::removeAllObserving() {
    m_observing.clear();
    m_observingPvpSync.clear();
}

//-----------------------------------------------------------------------

inline std::set<ServerObject *> &Client::getOpenedContainers() {
    return m_openedContainers;
}

//-----------------------------------------------------------------------

inline std::set<ServerObject *> const &Client::getOpenedContainers() const {
    return m_openedContainers;
}

//-----------------------------------------------------------------------

inline WatchedByList &Client::getWatchedByList() const {
    return m_watchedByList;
}

//-----------------------------------------------------------------------

inline uint32 Client::getGameFeatures() const {
    return m_gameFeatures;
}

//-----------------------------------------------------------------------

inline uint32 Client::getSubscriptionFeatures() const {
    return m_subscriptionFeatures;
}

//-----------------------------------------------------------------------

inline Client::AccountFeatureIdList const &Client::getAccountFeatureIds() const {
    return m_accountFeatureIds;
}

//-----------------------------------------------------------------------

inline void Client::setAccountFeatureIds(AccountFeatureIdList const &accountFeatureIds) {
    m_accountFeatureIds = accountFeatureIds;
}

//-----------------------------------------------------------------------

inline void Client::setAccountFeatureId(uint32 featureId, int count) {
    if (count > 0) {
        m_accountFeatureIds[featureId] = count;
    } else {
        IGNORE_RETURN(m_accountFeatureIds.erase(featureId));
    }
}

// ----------------------------------------------------------------------

inline unsigned int Client::getEntitlementTotalTime() const {
    return m_entitlementTotalTime;
}

// ----------------------------------------------------------------------

inline unsigned int Client::getEntitlementEntitledTime() const {
    return m_entitlementEntitledTime;
}

// ----------------------------------------------------------------------

inline unsigned int Client::getEntitlementTotalTimeSinceLastLogin() const {
    return m_entitlementTotalTimeSinceLastLogin;
}

// ----------------------------------------------------------------------

inline unsigned int Client::getEntitlementEntitledTimeSinceLastLogin() const {
    return m_entitlementEntitledTimeSinceLastLogin;
}

//-----------------------------------------------------------------------

inline CombatDataTable::CombatSpamFilterType Client::getCombatSpamFilter() const {
    return m_combatSpamFilter;
}

//-----------------------------------------------------------------------

inline int Client::getCombatSpamRangeSquaredFilter() const {
    return m_combatSpamRangeSquaredFilter;
}

//-----------------------------------------------------------------------

inline int Client::getFurnitureRotationDegree() const {
    return m_furnitureRotationDegree;
}

//-----------------------------------------------------------------------

inline bool Client::getHasUnoccupiedJediSlot() const {
    return m_hasUnoccupiedJediSlot;
}

//-----------------------------------------------------------------------

inline bool Client::getIsJediSlotCharacter() const {
    return m_isJediSlotCharacter;
}

//-----------------------------------------------------------------------

#endif	// _CLIENT_H
