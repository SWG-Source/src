// CentralTaskMessages.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CentralTaskMessages.h"

//-----------------------------------------------------------------------

SpawnGameServer::SpawnGameServer(const std::string & newVolumeName, const std::string & newClusterName) :
GameNetworkMessage("SpawnGameServer"),
clusterName(newClusterName),
volumeName(newVolumeName)
{
	addVariable(clusterName);
	addVariable(volumeName);
}

//-----------------------------------------------------------------------

SpawnGameServer::SpawnGameServer(Archive::ReadIterator & source) :
GameNetworkMessage("SpawnGameServer"),
clusterName(),
volumeName()
{
	addVariable(clusterName);
	addVariable(volumeName);
	unpack(source);
}

//-----------------------------------------------------------------------

SpawnGameServer::~SpawnGameServer()
{
}

//-----------------------------------------------------------------------

CentralGameServiceAddress::CentralGameServiceAddress(const std::string & newClientServiceAddress, const uint16 newClientServicePort) :
GameNetworkMessage("CentralGameServiceAddress"),
clientServiceAddress(newClientServiceAddress),
clientServicePort(newClientServicePort)
{
	addVariable(clientServiceAddress);
	addVariable(clientServicePort);
}

//-----------------------------------------------------------------------

CentralGameServiceAddress::CentralGameServiceAddress(Archive::ReadIterator & source) :
GameNetworkMessage("CentralGameServiceAddress"),
clientServiceAddress(),
clientServicePort()
{
	addVariable(clientServiceAddress);
	addVariable(clientServicePort);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralGameServiceAddress::~CentralGameServiceAddress()
{
}

//-----------------------------------------------------------------------

