// LoginClusterName2.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginClusterName2.h"
#include "sharedFoundation/Branch.h"
#include "sharedFoundation/ApplicationVersion.h"

//-----------------------------------------------------------------------

LoginClusterName2::LoginClusterName2(const std::string & clusterName, int timeZone) :
GameNetworkMessage("LoginClusterName2"),
m_clusterName(clusterName),
m_timeZone(timeZone),
m_branch( Branch().getBranchName() ),
m_changelist( atoi( ApplicationVersion::getPublicVersion() ) ),
m_networkVersion( GameNetworkMessage::NetworkVersionId )
{
	addVariable(m_clusterName);
	addVariable(m_timeZone);
	addVariable(m_branch);
	addVariable(m_changelist);
	addVariable(m_networkVersion);
}

//-----------------------------------------------------------------------

LoginClusterName2::LoginClusterName2(Archive::ReadIterator & source) :
GameNetworkMessage("LoginClusterName2"),
m_clusterName(),
m_timeZone()
{
	addVariable(m_clusterName);
	addVariable(m_timeZone);
	addVariable(m_branch);
	addVariable(m_changelist);
	addVariable(m_networkVersion);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

LoginClusterName2::~LoginClusterName2()
{
}

//-----------------------------------------------------------------------

