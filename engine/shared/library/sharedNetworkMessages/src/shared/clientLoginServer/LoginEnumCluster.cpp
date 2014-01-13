// ======================================================================
//
// LoginEnumCluster.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"

// ======================================================================

LoginEnumCluster::LoginEnumCluster(const std::vector<ClusterData> &data, int maxCharactersPerAccount) :
		GameNetworkMessage("LoginEnumCluster"),
		m_data(),
		m_maxCharactersPerAccount(maxCharactersPerAccount)
{
	m_data.set(data);
	addVariable(m_data);
	addVariable(m_maxCharactersPerAccount);
	// maxCharactersPerAccount is not directly related to the cluster list, but it's something we need
	// to tell the client about at the same time that we send the cluster list.
	// Including it here seems cheaper than making a separate message for it.
}

//-----------------------------------------------------------------------

LoginEnumCluster::LoginEnumCluster(Archive::ReadIterator & source) :
		GameNetworkMessage("LoginEnumCluster"),
		m_data()
{
	addVariable(m_data);
	addVariable(m_maxCharactersPerAccount);
	unpack(source);
}

//-----------------------------------------------------------------------

LoginEnumCluster::~LoginEnumCluster()
{
}

// ======================================================================
