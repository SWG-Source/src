// LoginServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_LoginServer_H
#define	_LoginServer_H

//-----------------------------------------------------------------------

#include "Singleton/Singleton.h"
#include "serverNetworkMessages/AvatarList.h"
#include "sharedFoundation/StationId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetwork/Service.h"
#include <unordered_map>
#include <map>
#include <string>

#include "serverKeyShare/KeyShare.h"

class CentralServerConnection;
class ClientConnection;
class CMonitorAPI;
class DeleteCharacterMessage;
class GameNetworkMessage;
class KeyServer;
class ServerConnection;
class Service;
class SessionApiClient;
class TransferAccountData;
class TransferCharacterData;
class TransferRequestMoveValidation;

//-----------------------------------------------------------------------

class LoginServer : public Singleton<LoginServer>, public MessageDispatch::Receiver
{
public:

	~LoginServer ();
	int                      addClient           (ClientConnection & client);
	ClientConnection*        getValidatedClient  (const StationId& suid);
	ClientConnection*        getUnvalidatedClient (int clientId);
	void                     removeClient        (int clientId);
		
	const std::unordered_map<std::string, const CentralServerConnection *> & getCentralServerMap_hide() const;

	bool                     deleteCharacter     (uint32 clusterId, NetworkId const & characterId, StationId suid);

	const KeyShare::Key &    getCurrentKey       (void) const;
	SessionApiClient*        getSessionApiClient ();
	KeyShare::Token          makeToken           (const unsigned char * data, const uint32 dataLen) const;
	void                     pushAllKeys         (CentralServerConnection * targetCentralServer) const;
	void                     pushKeyToAllServers (void);
	void                     receiveMessage      (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	static void              run                 (void);
	void                     onValidateClient    (StationId id, const std::string & username, ClientConnection*,bool, const char*, uint32 gameBits, uint32 subscriptionBits);

	void                     sendAvatarList      (const StationId& stationId, int stationIdNumberJediSlot, const AvatarList &avatars, TransferCharacterData * const);

	void                     updateClusterData   (uint32 clusterId, const std::string &clusterName, const std::string &address, const uint16 port, bool secret, bool locked, bool notRecommended, int maxCharactersPerAccount, int onlinePlayerLimit, int onlineFreeTrialLimit, bool freeTrialCanCreateChar, int onlineTutorialLimit);
	void                     onClusterRegistered (uint32 clusterId, const std::string &clusterName);
	void                     validateAccount     (const StationId& stationId, uint32 clusterId, uint32 subscriptionBits, bool canCreateRegular, bool canCreateJedi, bool canSkipTutorial, unsigned int track, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems);
	void                     validateAccountForTransfer(const TransferRequestMoveValidation & request, uint32 clusterId, uint32 sourceCharacterTemplateId, bool canCreateRegular, bool canCreateJedi);
	void                     sendToCluster       (uint32 clusterId, const GameNetworkMessage &message);
	void                     sendToAllClusters   (GameNetworkMessage const & message, Connection const * excludeCentralConnection = nullptr, uint32 excludeClusterId = 0, char const * excludeClusterName = nullptr);
	bool                     areAllClustersUp    () const;
	void                     getClusterIds       (std::vector<uint32> result);
	void                     performAccountTransfer (const AvatarList &avatars, TransferAccountData * transferAccountData);
	void                     setDone             (const bool isDone);
	
	void                     setClusterInfoByName( const std::string &name, const std::string &branch, int changelist, const std::string &networkVersion );
	void                     sendExtendedClusterInfo( ClientConnection &client ) const;
	
	void getAllClusterNamesAndIDs( std::map< std::string, uint32 > &results ) const;
	uint32 getClusterIDByName( const std::string &sName );
	const std::string & getClusterNameById( uint32 clusterId );
	
	struct ConnectionServerEntry
    {
        std::string clientServiceAddress;
        uint16      clientServicePortPrivate;
        uint16      clientServicePortPublic;
		int         id;
		uint16      pingPort;
		int         numClients;

		bool operator==(const ConnectionServerEntry &rhs) const;
    };

	
protected: 
	friend class Singleton<LoginServer>;
	LoginServer  ();

private:
	
	/**
	 * Data structure to track a cluster.
	 * This may be only partly filled in if the cluster exists
	 * but isn't connected.
	 */
	struct ClusterListEntry
	{
		uint32                              m_clusterId;
		std::string                         m_clusterName;
		CentralServerConnection *           m_centralServerConnection;
		std::vector<ConnectionServerEntry>  m_connectionServers;
		int                                 m_numPlayers;
		int                                 m_numFreeTrialPlayers;
		int                                 m_numTutorialPlayers;
		int                                 m_maxCharacters;
		int                                 m_maxCharactersPerAccount;
		int                                 m_onlinePlayerLimit;
		int                                 m_onlineFreeTrialLimit;
		int                                 m_onlineTutorialLimit;
		bool                                m_freeTrialCanCreateChar;
		int                                 m_timeZone;
		bool                                m_connected;
		std::string                         m_address;
		uint16                              m_port;
		bool                                m_allowReconnect;
		bool                                m_secret;
		bool                                m_readyForPlayers;
		bool                                m_locked;
		bool                                m_notRecommendedDatabase;
		bool                                m_notRecommendedCentral;
		std::string                         m_branch;
		int                                 m_changelist;
		std::string                         m_networkVersion;
		
		ClusterListEntry             ();
		ClusterListEntry             (const ClusterListEntry &rhs);
		ClusterListEntry & operator= (const ClusterListEntry &rhs);
	};

	typedef std::vector<ClusterListEntry*> ClusterListType;
	typedef std::map<int, ClientConnection *> ActiveClientsType;
	
private:
	void                           installSessionValidation();
	ClusterListEntry *             findClusterById             (uint32 clusterId);     
	ClusterListEntry *             findClusterByName           (const std::string & clusterName);
	ClusterListEntry *             findClusterByConnection     (const CentralServerConnection *connection);
	void                           sendClusterStatus           (ClientConnection &conn) const;
	void                           sendClusterStatusToAll      () const;
	void                           refreshConnections          ();
	ClusterListEntry *             addCluster                  (const std::string &clusterName);
	void                           disconnectCluster           (ClusterListEntry &cle, bool forceDisconnect, bool reconnect);
	    
private:

	
	bool                    done;
	Service *               m_centralService;
	Service *               clientService;
	Service *               pingService;
	Service *		m_CSService;
	KeyServer *             keyServer;
	ActiveClientsType       m_clientMap;
	ClusterListType         m_clusterList;
	SessionApiClient*       m_sessionApiClient;
	std::map<StationId, ClientConnection*> m_validatedClientMap;
	bool                    m_clusterStatusChanged;  // true if any cluster has changed status and the list should be resent to all clients
	CMonitorAPI *           m_soeMonitor;
	
	
private:							
	LoginServer (const LoginServer & source);
	LoginServer & operator=(const LoginServer & rhs);
};

// ======================================================================

inline bool LoginServer::ConnectionServerEntry::operator==(const ConnectionServerEntry &rhs) const
{
	return (id == rhs.id);
}


// ======================================================================

#endif	// _LoginServer_H
