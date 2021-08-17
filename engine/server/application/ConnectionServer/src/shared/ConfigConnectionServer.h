// ConfigConnectionServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef    _ConfigConnectionServer_H
#define    _ConfigConnectionServer_H

//-----------------------------------------------------------------------

class ConfigConnectionServer {
public:
    struct Data {
        const char *sessionURL;
        const char *centralServerAddress;
        int centralServerPort;
        int clientServicePortPublic;
        int clientServicePortPrivate;
        int clientOverflowLimit;
        int gameServicePort;
        const char *clusterName;
        bool disableWorldSnapshot;
        int maxClients;
        int pingPort;
        bool spamLimitEnabled;
        int spamLimitResetTimeMs;
        int spamLimitResetScaleFactor;
        int spamLimitBytesPerSec;
        int spamLimitPacketsPerSec;
        bool startPublicServer;
        const char *chatServiceBindInterface;
        const char *clientServiceBindInterface;
        const char *customerServiceBindInterface;
        const char *gameServiceBindInterface;
        bool compressClientNetworkTraffic;
        int crashRecoveryTimeout;
        bool shouldSleep;
        int clientMaxOutstandingPackets;
        int clientMaxRawPacketSize;
        int clientMaxConnections;
        int clientFragmentSize;
        int clientMaxDataHoldTime;
        int clientHashTableSize;
        int lagReportThreshold;

        bool validateStationKey;
        const char *sessionServers;
        int sessionType;
        bool disableSessionLogout;
        bool sessionRecordPlayTime;
        bool disconnectOnInactive;
        bool disconnectFreeTrialOnInactive;
        const char *adminAccountDataTable;

        float timeBetweenSessionUpdates;

        int defaultGameFeatures;
        int defaultSubscriptionFeatures;
        int requiredSubscriptionBits;
        int requiredGameBits;
        bool setJtlRetailIfBetaIsSet;
        bool setEpisode3RetailIfBetaIsSet;
        bool setTrialsOfObiwanRetailIfBetaIsSet;

        int disabledFeatureBits;

        bool validateClientVersion;

        int connectionServerNumber;
        int fakeBuddyPoints;

        bool useSecureLoginForGodAccess;

	bool useOldSuidGenerator;

        const char *altPublicBindAddress;
    };


    static const char *getSessionURL();

    static const char *getCentralServerAddress();

    static const uint16 getCentralServerPort();

    static const int getClientOverflowLimit();

    static const char *getClientServiceBindInterface();

    static const uint16 getClientServicePortPrivate();

    static const uint16 getClientServicePortPublic();

    static const char *getClusterName();

    static bool getDisableWorldSnapshot();

    static const uint16 getGameServicePort();

    static const int getMaxClients();

    static const uint16 getPingPort();

    static const bool getSpamLimitEnabled();

    static const unsigned int getSpamLimitResetTimeMs();

    static const unsigned int getSpamLimitResetScaleFactor();

    static const unsigned int getSpamLimitBytesPerSec();

    static const unsigned int getSpamLimitPacketsPerSec();

    static const bool getStartPublicServer();

    static const char *getChatServiceBindInterface();

    static const char *getCustomerServiceBindInterface();

    static const char *getGameServiceBindInterface();

    static const bool getCompressClientNetworkTraffic();

    static void install();

    static void remove();

    static const uint getCrashRecoveryTimeout();

    static bool getShouldSleep();

    static const int getClientMaxOutstandingPackets();

    static const int getClientMaxRawPacketSize();

    static const int getClientMaxConnections();

    static const int getClientFragmentSize();

    static const int getClientMaxDataHoldTime();

    static const int getClientHashTableSize();

    static const int getLagReportThreshold();

    static bool getValidateStationKey();

    static const char *getSessionServers();

    static const int getSessionType();

    static bool getDisableSessionLogout();

    static bool getSessionRecordPlayTime();

    static bool getDisconnectOnInactive();

    static bool getDisconnectFreeTrialOnInactive();

    static const char *getAdminAccountDataTable(void);

    static int getNumberOfSessionServers();

    static char const *getSessionServer(int index);

    static float getTimeBetweenSessionUpdates();

    static const uint32 getDefaultGameFeatures();

    static const uint32 getDefaultSubscriptionFeatures();

    static uint32 getRequiredSubscriptionBits();

    static uint32 getRequiredGameBits();

    static bool getSetJtlRetailIfBetaIsSet();

    static bool getSetEpisode3RetailIfBetaIsSet();

    static bool getSetTrialsOfObiwanRetailIfBetaIsSet();

    static int getDisabledFeatureBits();

    static bool getValidateClientVersion();

    static int getConnectionServerNumber();

    static int getFakeBuddyPoints();

    static const char *getPublicBindAddress();

    static bool getUseOldSuidGenerator();

    static bool getUseSecureLoginForGodAccess();

private:
    static Data *data;
};


inline const char *ConfigConnectionServer::getSessionURL() {
    return data->sessionURL;
}

//-----------------------------------------------------------------------

inline bool ConfigConnectionServer::getShouldSleep() {
    return data->shouldSleep;
}

//-----------------------------------------------------------------------

inline const bool ConfigConnectionServer::getCompressClientNetworkTraffic() {
    return data->compressClientNetworkTraffic;
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getCentralServerAddress() {
    return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigConnectionServer::getCentralServerPort() {
    return static_cast<const uint16>(data->centralServerPort);
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getClientServiceBindInterface() {
    return data->clientServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientOverflowLimit() {
    return data->clientOverflowLimit;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigConnectionServer::getClientServicePortPrivate() {
    return static_cast<const uint16>(data->clientServicePortPrivate);
}

//-----------------------------------------------------------------------

inline const uint16 ConfigConnectionServer::getClientServicePortPublic() {
    return static_cast<const uint16>(data->clientServicePortPublic);
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getClusterName() {
    return data->clusterName;
}

//-----------------------------------------------------------------------

inline bool ConfigConnectionServer::getDisableWorldSnapshot() {
    return data->disableWorldSnapshot;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigConnectionServer::getGameServicePort() {
    return static_cast<const uint16>(data->gameServicePort);
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getMaxClients() {
    return data->maxClients;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigConnectionServer::getPingPort() {
    return static_cast<const uint16>(data->pingPort);
}

//-----------------------------------------------------------------------

inline const bool ConfigConnectionServer::getSpamLimitEnabled() {
    return data->spamLimitEnabled;
}

//-----------------------------------------------------------------------

inline const unsigned int ConfigConnectionServer::getSpamLimitResetTimeMs() {
    return static_cast<const unsigned int>(data->spamLimitResetTimeMs);
}

//-----------------------------------------------------------------------

inline const unsigned int ConfigConnectionServer::getSpamLimitResetScaleFactor() {
    return static_cast<const unsigned int>(data->spamLimitResetScaleFactor);
}

//-----------------------------------------------------------------------

inline const unsigned int ConfigConnectionServer::getSpamLimitBytesPerSec() {
    return static_cast<const unsigned int>(data->spamLimitBytesPerSec);
}

//-----------------------------------------------------------------------

inline const unsigned int ConfigConnectionServer::getSpamLimitPacketsPerSec() {
    return static_cast<const unsigned int>(data->spamLimitPacketsPerSec);
}

//-----------------------------------------------------------------------

inline const bool ConfigConnectionServer::getStartPublicServer() {
    return data->startPublicServer;
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getChatServiceBindInterface() {
    return data->chatServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getCustomerServiceBindInterface() {
    return data->customerServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getGameServiceBindInterface() {
    return data->gameServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const uint ConfigConnectionServer::getCrashRecoveryTimeout() {
    return static_cast<uint>(data->crashRecoveryTimeout);
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientMaxOutstandingPackets() {
    return data->clientMaxOutstandingPackets;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientMaxRawPacketSize() {
    return data->clientMaxRawPacketSize;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientMaxConnections() {
    return data->clientMaxConnections;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientFragmentSize() {
    return data->clientFragmentSize;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientMaxDataHoldTime() {
    return data->clientMaxDataHoldTime;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getClientHashTableSize() {
    return data->clientHashTableSize;
}

//-----------------------------------------------------------------------

inline const int ConfigConnectionServer::getLagReportThreshold() {
    return data->lagReportThreshold;
}

// ----------------------------------------------------------------------

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getValidateStationKey() {
    return data->validateStationKey;
}

//------------------------------------------------------------------------------------------

inline const char *ConfigConnectionServer::getSessionServers() {
    return data->sessionServers;
}

//------------------------------------------------------------------------------------------

inline const int ConfigConnectionServer::getSessionType() {
    return data->sessionType;
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getDisableSessionLogout() {
    return data->disableSessionLogout;
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getSessionRecordPlayTime() {
    return data->sessionRecordPlayTime;
}

// ----------------------------------------------------------------------

inline const char *ConfigConnectionServer::getAdminAccountDataTable(void) {
    return data->adminAccountDataTable;
}

//-----------------------------------------------------------------------

inline float ConfigConnectionServer::getTimeBetweenSessionUpdates() {
    return data->timeBetweenSessionUpdates;
}

//-----------------------------------------------------------------------

inline uint32 ConfigConnectionServer::getRequiredSubscriptionBits() {
    return static_cast<uint32>(data->requiredSubscriptionBits);
}
//-----------------------------------------------------------------------


inline uint32 ConfigConnectionServer::getRequiredGameBits() {
    return static_cast<uint32>(data->requiredGameBits);
}

//-----------------------------------------------------------------------


inline const uint32 ConfigConnectionServer::getDefaultGameFeatures() {
    return static_cast<uint32>(data->defaultGameFeatures);
}

// ----------------------------------------------------------------------

inline const uint32 ConfigConnectionServer::getDefaultSubscriptionFeatures() {
    return static_cast<uint32>(data->defaultSubscriptionFeatures);
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getSetJtlRetailIfBetaIsSet() {
    return data->setJtlRetailIfBetaIsSet;
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getSetEpisode3RetailIfBetaIsSet() {
    return data->setEpisode3RetailIfBetaIsSet;
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getSetTrialsOfObiwanRetailIfBetaIsSet() {
    return data->setTrialsOfObiwanRetailIfBetaIsSet;
}

//------------------------------------------------------------------------------------------

inline bool ConfigConnectionServer::getValidateClientVersion() {
    return data->validateClientVersion;
}

// ----------------------------------------------------------------------

inline int ConfigConnectionServer::getConnectionServerNumber() {
    return data->connectionServerNumber;
}


inline bool ConfigConnectionServer::getUseOldSuidGenerator() {
    return data->useOldSuidGenerator;
}

inline bool ConfigConnectionServer::getUseSecureLoginForGodAccess()
{
    return data->useSecureLoginForGodAccess;
}

#endif	// _ConfigConnectionServer_H
