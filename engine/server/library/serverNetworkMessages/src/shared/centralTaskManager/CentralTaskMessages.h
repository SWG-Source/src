// CentralTaskMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralTaskMessages_H
#define	_CentralTaskMessages_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

namespace CentralTaskMessageConstants
{
	const unsigned char   SpawnGameServer             = 1;
	const unsigned char   CentralGameServiceAddress = 2;
}

//-----------------------------------------------------------------------

class SpawnGameServer : public GameNetworkMessage
{
public:
	SpawnGameServer(const std::string & newVolumeName, const std::string & newClusterName);
	SpawnGameServer(Archive::ReadIterator & source);
	~SpawnGameServer();

	const std::string &  getClusterName(void) const;
	const std::string &   getVolumeName(void) const;

private:
	Archive::AutoVariable<std::string>          clusterName;
	Archive::AutoVariable<std::string>          volumeName;

	SpawnGameServer();
	SpawnGameServer(const SpawnGameServer&);
	SpawnGameServer& operator= (const SpawnGameServer&);
};

//-----------------------------------------------------------------------

inline const std::string & SpawnGameServer::getClusterName(void) const
{
	return clusterName.get();
}

//-----------------------------------------------------------------------

inline const std::string & SpawnGameServer::getVolumeName(void) const
{
	return volumeName.get();
}

//-----------------------------------------------------------------------

class CentralGameServiceAddress : public GameNetworkMessage
{
public:
	CentralGameServiceAddress(const std::string & newClientServiceAddress, const uint16 newClientServicePort);
	CentralGameServiceAddress(Archive::ReadIterator & source);
	~CentralGameServiceAddress();

	const std::string & getGameServiceAddress (void) const;
	const uint16          getGameServicePort    (void) const;
private:
	Archive::AutoVariable<std::string> clientServiceAddress;
	Archive::AutoVariable<uint16>      clientServicePort;

	CentralGameServiceAddress();
	CentralGameServiceAddress(const CentralGameServiceAddress&);
	CentralGameServiceAddress& operator= (const CentralGameServiceAddress&);
};

//-----------------------------------------------------------------------

inline const std::string & CentralGameServiceAddress::getGameServiceAddress(void) const
{
	return clientServiceAddress.get();
}

//-----------------------------------------------------------------------

inline const uint16 CentralGameServiceAddress::getGameServicePort(void) const
{
	return clientServicePort.get();
}

//-----------------------------------------------------------------------

#endif	// _CentralTaskMessages_H

