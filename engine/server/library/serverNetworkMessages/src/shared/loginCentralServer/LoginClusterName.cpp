// LoginClusterName.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginClusterName.h"

//-----------------------------------------------------------------------

LoginClusterName::LoginClusterName(const std::string & clusterName, int timeZone) :
GameNetworkMessage("LoginClusterName"),
m_clusterName(clusterName),
m_timeZone(timeZone)
{
	addVariable(m_clusterName);
	addVariable(m_timeZone);
}

//-----------------------------------------------------------------------

LoginClusterName::LoginClusterName(Archive::ReadIterator & source) :
GameNetworkMessage("LoginClusterName"),
m_clusterName(),
m_timeZone()
{
	addVariable(m_clusterName);
	addVariable(m_timeZone);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

LoginClusterName::~LoginClusterName()
{
}

//-----------------------------------------------------------------------

