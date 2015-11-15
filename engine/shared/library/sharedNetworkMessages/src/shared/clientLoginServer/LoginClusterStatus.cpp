// ======================================================================
//
// LoginClusterStatus.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"

// ======================================================================

const char * const LoginClusterStatus::MessageType = "LoginClusterStatus";

LoginClusterStatus::LoginClusterStatus(const std::vector<ClusterData> &data) :
		GameNetworkMessage(LoginClusterStatus::MessageType),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

LoginClusterStatus::LoginClusterStatus(Archive::ReadIterator & source) :
		GameNetworkMessage(LoginClusterStatus::MessageType),
		m_data()
{
	addVariable(m_data);
	unpack(source);
}

//-----------------------------------------------------------------------

LoginClusterStatus::~LoginClusterStatus()
{
}

// ======================================================================
