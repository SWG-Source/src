// ======================================================================
//
// LoginEnumCluster.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginEnumCluster_H
#define INCLUDED_LoginEnumCluster_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

struct LoginEnumCluster_ClusterData
{
	uint32      m_clusterId;
	std::string m_clusterName;
	int         m_timeZone;
};

class LoginEnumCluster : public GameNetworkMessage
{
  public:
	typedef LoginEnumCluster_ClusterData ClusterData;

  public:
	const std::vector<ClusterData> &getData() const;
	int getMaxCharactersPerAccount() const;

  public:
	LoginEnumCluster(const std::vector<ClusterData> &data, int maxCharactersPerAccount);
	LoginEnumCluster(Archive::ReadIterator & source);
	~LoginEnumCluster();

  private:
	Archive::AutoArray<ClusterData> m_data;
	Archive::AutoVariable<int> m_maxCharactersPerAccount;

  private: //disable
	LoginEnumCluster();
	LoginEnumCluster(const LoginEnumCluster&);
	LoginEnumCluster& operator= (const LoginEnumCluster&);
};

// ======================================================================

namespace Archive
{
	inline void get(ReadIterator & source, LoginEnumCluster_ClusterData &c)
	{
		get(source, c.m_clusterId);
		get(source, c.m_clusterName);
		get(source, c.m_timeZone);
	}

	inline void put(ByteStream & target, const LoginEnumCluster_ClusterData &c)
	{
		put(target, c.m_clusterId);
		put(target, c.m_clusterName);
		put(target, c.m_timeZone);
	}
}

// ----------------------------------------------------------------------

inline const std::vector<LoginEnumCluster::ClusterData> &LoginEnumCluster::getData() const
{
	return m_data.get();
}

// ----------------------------------------------------------------------

inline int LoginEnumCluster::getMaxCharactersPerAccount() const
{
	return m_maxCharactersPerAccount.get();
}

// ======================================================================

#endif
