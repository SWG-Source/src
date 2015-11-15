// ======================================================================
//
// UdpConnectionMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpLibraryMT.h"
#include "UdpConnectionHandlerMT.h"
#include "Events.h"
#include "sharedSynchronization/Guard.h"

// ======================================================================

class UdpConnectionHandlerInternal: public UdpConnectionHandler
{
public:
	UdpConnectionHandlerInternal();
	virtual ~UdpConnectionHandlerInternal();

	void setOwner(UdpConnectionHandlerMT *owner);
	void processReceive(UdpConnectionMT *conMT, unsigned char const *data, int dataLen);
	void processTerminated(UdpConnectionMT *conMT);
	void processConnectComplete(UdpConnectionMT *conMT);

	virtual void OnRoutePacket(UdpConnection *con, unsigned char const *data, int dataLen);
	virtual void OnConnectComplete(UdpConnection *con);
	virtual void OnTerminated(UdpConnection *con);
	virtual void OnPacketCorrupt(UdpConnection *con, const uchar *data, int dataLen, UdpCorruptionReason reason);
private:
	UdpConnectionHandlerInternal(UdpConnectionHandlerInternal const &);
	UdpConnectionHandlerInternal &operator=(UdpConnectionHandlerInternal const &);

private:
	UdpConnectionHandlerMT *m_owner;
};

// ----------------------------------------------------------------------

UdpConnectionHandlerInternal::UdpConnectionHandlerInternal() :
	m_owner(0)
{
}

UdpConnectionHandlerInternal::~UdpConnectionHandlerInternal(){}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::setOwner(UdpConnectionHandlerMT *owner)
{
	m_owner = owner;
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::processReceive(UdpConnectionMT *conMT, unsigned char const *data, int dataLen)
{
	if (m_owner)
		m_owner->OnRoutePacket(conMT, data, dataLen);
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::processTerminated(UdpConnectionMT *conMT)
{
	if (m_owner)
		m_owner->OnTerminated(conMT);
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::processConnectComplete(UdpConnectionMT *conMT)
{
	if (m_owner)
		m_owner->OnConnectComplete(conMT);
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::OnRoutePacket(UdpConnection *con, unsigned char const *data, int dataLen)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	NOT_NULL(conMT);
	if (UdpLibraryMT::getUseNetworkThread())
		Events::pushIncomingEventReceive(conMT, data, dataLen);
	else if (m_owner)
		m_owner->OnRoutePacket(conMT, data, dataLen);
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::OnConnectComplete(UdpConnection *con)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	NOT_NULL(conMT);
	if (UdpLibraryMT::getUseNetworkThread())
		Events::pushIncomingEventConnectComplete(conMT);
	else if (m_owner)
		m_owner->OnConnectComplete(conMT);
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerInternal::OnTerminated(UdpConnection *con)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	NOT_NULL(conMT);
	if (UdpLibraryMT::getUseNetworkThread())
		Events::pushIncomingEventTerminated(conMT);
	else if (m_owner)
		m_owner->OnTerminated(conMT);
}

//-----------------------------------------------------------------------

void UdpConnectionHandlerInternal::OnPacketCorrupt(UdpConnection *con, const uchar *data, int dataLen, UdpCorruptionReason reason)
{
	UdpConnectionMT * conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	m_owner->OnPacketCorrupt(conMT, data, dataLen, reason);
}

// ======================================================================

// Note: this is only called in situations where the UdpLibraryMT mutex is locked already, and we are taking over a reference already gotten from newing the connection
UdpConnectionMT::UdpConnectionMT(UdpConnection *udpConnection) :
	m_refCount(1),
	m_udpConnection(udpConnection),
	m_passThroughData(0),
	m_connectionHandlerInternal(new UdpConnectionHandlerInternal)
{
	udpConnection->SetPassThroughData(this);
	udpConnection->SetHandler(m_connectionHandlerInternal);
}

// ----------------------------------------------------------------------

UdpConnectionMT::~UdpConnectionMT()
{
	Guard lock(UdpLibraryMT::getMutex());
	m_udpConnection->SetHandler(0);
	m_udpConnection->SetPassThroughData(0);
	m_udpConnection->Release();
	m_udpConnection = 0;
	delete m_connectionHandlerInternal;
}

// ----------------------------------------------------------------------

void UdpConnectionMT::AddRef()
{
	Guard lock(UdpLibraryMT::getMutex());
	++m_refCount;
}

// ----------------------------------------------------------------------

void UdpConnectionMT::Release()
{
	Guard lock(UdpLibraryMT::getMutex());
	if (--m_refCount == 0)
		delete this;
}

// ----------------------------------------------------------------------

void UdpConnectionMT::processReceive(unsigned char const *data, int dataLen)
{
	m_connectionHandlerInternal->processReceive(this, data, dataLen);
}

// ----------------------------------------------------------------------

void UdpConnectionMT::processTerminated()
{
	m_connectionHandlerInternal->processTerminated(this);
}

// ----------------------------------------------------------------------

void UdpConnectionMT::processConnectComplete()
{
	m_connectionHandlerInternal->processConnectComplete(this);
}

// ----------------------------------------------------------------------

bool UdpConnectionMT::Send(UdpChannel channel, unsigned char const *data, int dataLen)
{
	if (UdpLibraryMT::getUseNetworkThread())
	{
		Events::pushOutgoingEventSendRaw(m_udpConnection, channel, data, dataLen);
		return true;
	}
	else
		return m_udpConnection->Send(channel, data, dataLen);
}

// ----------------------------------------------------------------------

bool UdpConnectionMT::Send(UdpChannel channel, LogicalPacket const *packet)
{
	if (UdpLibraryMT::getUseNetworkThread())
	{
		Events::pushOutgoingEventSendLogicalPacket(m_udpConnection, channel, packet);
		return true;
	}
	else
		return m_udpConnection->Send(channel, packet);
}

// ----------------------------------------------------------------------

void UdpConnectionMT::Disconnect(int flushTimeout)
{
	Guard lock(UdpLibraryMT::getMutex());
	m_udpConnection->Disconnect(flushTimeout);
}

// ----------------------------------------------------------------------

void *UdpConnectionMT::GetPassThroughData() const
{
	return m_passThroughData;
}

// ----------------------------------------------------------------------

UdpConnection::DisconnectReason UdpConnectionMT::GetDisconnectReason() const
{
	return m_udpConnection->GetDisconnectReason();
}

// ----------------------------------------------------------------------

UdpConnection::Status UdpConnectionMT::GetStatus() const
{
	// TODO: change this after status is cached
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->GetStatus();
}

// ----------------------------------------------------------------------

int UdpConnectionMT::TotalPendingBytes() const
{
	// TODO: change this after cached
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->TotalPendingBytes();
}

// ----------------------------------------------------------------------

unsigned short UdpConnectionMT::ServerSyncStampShort() const
{
	// TODO: is there a cleaner way to deal with this?
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->ServerSyncStampShort();
}

// ----------------------------------------------------------------------

unsigned long UdpConnectionMT::ServerSyncStampLong() const
{
	// TODO: is there a cleaner way to deal with this?
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->ServerSyncStampLong();
}

// ----------------------------------------------------------------------

UdpIpAddress UdpConnectionMT::GetDestinationIp() const
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->GetDestinationIp();
}

// ----------------------------------------------------------------------

int UdpConnectionMT::GetDestinationPort() const
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->GetDestinationPort();
}

// ----------------------------------------------------------------------

void UdpConnectionMT::GetChannelStatus(UdpChannel channel, UdpConnection::ChannelStatus *channelStatus) const
{
	Guard lock(UdpLibraryMT::getMutex());
	m_udpConnection->GetChannelStatus(channel, channelStatus);
}

// ----------------------------------------------------------------------

int UdpConnectionMT::LastReceive() const
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->LastReceive();
}

// ----------------------------------------------------------------------

int UdpConnectionMT::LastSend() const
{
	Guard lock(UdpLibraryMT::getMutex());
	return m_udpConnection->LastSend();
}

// ----------------------------------------------------------------------

void UdpConnectionMT::SetHandler(UdpConnectionHandlerMT *handler)
{
	m_connectionHandlerInternal->setOwner(handler);
}

// ----------------------------------------------------------------------

void UdpConnectionMT::SetPassThroughData(void *passThroughData)
{
	m_passThroughData = passThroughData;
}

// ----------------------------------------------------------------------

void UdpConnectionMT::SetNoDataTimeout(int noDataTimeout)
{
	Guard lock(UdpLibraryMT::getMutex());
	m_udpConnection->SetNoDataTimeout(noDataTimeout);
}

// ======================================================================

