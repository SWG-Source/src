// ======================================================================
//
// LoginClusterStatusEx.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/LoginClusterStatusEx.h"

// ======================================================================

const char * const LoginClusterStatusEx::MessageType = "LoginClusterStatusEx";

LoginClusterStatusEx::LoginClusterStatusEx(const std::vector<ClusterData> &data) :
		GameNetworkMessage(LoginClusterStatusEx::MessageType),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

LoginClusterStatusEx::LoginClusterStatusEx(Archive::ReadIterator & source) :
		GameNetworkMessage(LoginClusterStatusEx::MessageType),
		m_data()
{
	addVariable(m_data);
	unpack(source);
}

//-----------------------------------------------------------------------

LoginClusterStatusEx::~LoginClusterStatusEx()
{
}

// ======================================================================
