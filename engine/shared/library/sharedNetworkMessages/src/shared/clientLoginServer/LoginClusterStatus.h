// ======================================================================
//
// LoginClusterStatus.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginClusterStatus_H
#define INCLUDED_LoginClusterStatus_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

struct LoginClusterStatus_ClusterData
{
	enum Status {S_down, S_loading, S_up, S_locked, S_restricted, S_full};

	// enum value must be ordered in ascending population density
	enum PopulationStatus {PS_LOWEST = 0, PS_very_light = PS_LOWEST, PS_light, PS_medium, PS_heavy, PS_very_heavy, PS_extremely_heavy, PS_full, PS_HIGHEST = PS_full};

	uint32      m_clusterId;
	std::string m_connectionServerAddress;
	uint16      m_connectionServerPort;
	uint16      m_connectionServerPingPort;
	int         m_populationOnline; // must be signed, -1 is a legitimate value meaning not available (for security reason)
	PopulationStatus m_populationOnlineStatus;
	int         m_maxCharactersPerAccount;
	int         m_timeZone;
	Status      m_status;
	bool        m_dontRecommend;
	uint32      m_onlinePlayerLimit;
	uint32      m_onlineFreeTrialLimit;
};

/**
 * Sent From:  LoginServer
 * Sent To:    Client
 * Action:     Lists all the clusters that are currently available for login.
*/
class LoginClusterStatus : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef LoginClusterStatus_ClusterData ClusterData;
	
public:
	const std::vector<ClusterData> &getData() const;
	//	void addCluster(uint32 clusterId, const std::string &m_clusterName);
	
public:
	LoginClusterStatus(const std::vector<ClusterData> &data);
	LoginClusterStatus(Archive::ReadIterator & source);
	~LoginClusterStatus();
	
private:
	Archive::AutoArray<ClusterData> m_data;
	
private: //disable
	LoginClusterStatus();
	LoginClusterStatus(const LoginClusterStatus&);
	LoginClusterStatus& operator= (const LoginClusterStatus&);
};

// ======================================================================

namespace Archive
{
	inline void get(ReadIterator & source, LoginClusterStatus_ClusterData &c)
	{
		get(source,c.m_clusterId);
		get(source,c.m_connectionServerAddress);
		get(source,c.m_connectionServerPort);
		get(source,c.m_connectionServerPingPort);
		get(source,c.m_populationOnline);
		int tempStatus;
		get(source,tempStatus);
		c.m_populationOnlineStatus=static_cast<LoginClusterStatus_ClusterData::PopulationStatus>(tempStatus);
		get(source,c.m_maxCharactersPerAccount);
		get(source,c.m_timeZone);
		get(source,tempStatus);
		c.m_status=static_cast<LoginClusterStatus_ClusterData::Status>(tempStatus);
		get(source,c.m_dontRecommend);
		get(source,c.m_onlinePlayerLimit);
		get(source,c.m_onlineFreeTrialLimit);
	}

	inline void put(ByteStream & target, const LoginClusterStatus_ClusterData &c)
	{
		put(target,c.m_clusterId);
		put(target,c.m_connectionServerAddress);
		put(target,c.m_connectionServerPort);
		put(target,c.m_connectionServerPingPort);
		put(target,c.m_populationOnline);
		put(target,static_cast<int>(c.m_populationOnlineStatus));
		put(target,c.m_maxCharactersPerAccount);
		put(target,c.m_timeZone);
		put(target,static_cast<int>(c.m_status));
		put(target,c.m_dontRecommend);
		put(target,c.m_onlinePlayerLimit);
		put(target,c.m_onlineFreeTrialLimit);
	}
}

// ======================================================================

inline const std::vector<LoginClusterStatus::ClusterData> &LoginClusterStatus::getData() const
{
	return m_data.get();
}

// ======================================================================

#endif
