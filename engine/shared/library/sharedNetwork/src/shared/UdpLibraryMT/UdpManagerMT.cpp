// ======================================================================
//
// UdpManagerMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpManagerMT.h"
#include "UdpLibraryMT.h"
#include "sharedSynchronization/Guard.h"

// ======================================================================

UdpManagerMT::UdpManagerMT(UdpManager::Params *params) :
	m_refCount(1),
	m_udpManager(0)
{
	Guard lock(UdpLibraryMT::getMutex());
	m_udpManager = new UdpManager(params);
	UdpLibraryMT::registerUdpManager(m_udpManager);
}

// ----------------------------------------------------------------------

UdpManagerMT::~UdpManagerMT()
{
	Guard lock(UdpLibraryMT::getMutex());
	UdpLibraryMT::unregisterUdpManager(m_udpManager);
	m_udpManager->SetHandler(0);
	m_udpManager->Release();
}

// ----------------------------------------------------------------------

void UdpManagerMT::AddRef()
{
	Guard lock(UdpLibraryMT::getMutex());
	++m_refCount;
}

// ----------------------------------------------------------------------
void UdpManagerMT::Release()
{
	Guard lock(UdpLibraryMT::getMutex());
	if (--m_refCount == 0)
		delete this;
}

// ----------------------------------------------------------------------

void UdpManagerMT::GiveTime()
{
	if (!UdpLibraryMT::getUseNetworkThread())
		m_udpManager->GiveTime();
}

// ----------------------------------------------------------------------

UdpConnectionMT *UdpManagerMT::EstablishConnection(char const *serverAddress, int serverPort, int timeout)
{
	Guard lock(UdpLibraryMT::getMutex());
	UdpConnection *connection = m_udpManager->EstablishConnection(serverAddress, serverPort, timeout);
	if (connection)
		return new UdpConnectionMT(connection);
	return 0;
}

// ----------------------------------------------------------------------

LogicalPacket const *UdpManagerMT::CreatePacket(void const *data, int dataLen, void const *data2, int dataLen2)
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpManager->CreatePacket(data, dataLen, data2, dataLen2);
}

// ----------------------------------------------------------------------

int UdpManagerMT::GetLocalPort()
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpManager->GetLocalPort();
}

// ----------------------------------------------------------------------

void UdpManagerMT::ClearHandler()
{
	m_udpManager->SetHandler(0);
}

// ======================================================================

