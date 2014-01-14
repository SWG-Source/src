// ======================================================================
//
// UdpManagerHandlerMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpHandlerMT.h"
#include "UdpLibraryMT.h"
#include "UdpConnectionMT.h"
#include "Events.h"

// ======================================================================

class UdpManagerHandlerInternal: public UdpManagerHandler
{
public:
	UdpManagerHandlerInternal(UdpManagerHandlerMT *owner);

	virtual void OnConnectRequest(UdpConnection *con);
	virtual int OnUserSuppliedEncrypt(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedDecrypt(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedEncrypt2(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);
	virtual int OnUserSuppliedDecrypt2(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen);

	virtual ~UdpManagerHandlerInternal ();

private:
	UdpManagerHandlerMT *m_owner;
};

// ======================================================================

UdpManagerHandlerInternal::~UdpManagerHandlerInternal ()
{
	m_owner = 0;
}

//----------------------------------------------------------------------

UdpManagerHandlerInternal::UdpManagerHandlerInternal(UdpManagerHandlerMT *owner) :
	m_owner(owner)
{
}

// ----------------------------------------------------------------------

void UdpManagerHandlerInternal::OnConnectRequest(UdpConnection *con)
{
	m_owner->queueConnectRequest(con);
}

// ----------------------------------------------------------------------

int UdpManagerHandlerInternal::OnUserSuppliedEncrypt(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	return m_owner->OnUserSuppliedEncrypt(conMT, destData, sourceData, sourceLen);
}

// ----------------------------------------------------------------------

int UdpManagerHandlerInternal::OnUserSuppliedDecrypt(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	return m_owner->OnUserSuppliedDecrypt(conMT, destData, sourceData, sourceLen);
}

// ----------------------------------------------------------------------

int UdpManagerHandlerInternal::OnUserSuppliedEncrypt2(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	return m_owner->OnUserSuppliedEncrypt2(conMT, destData, sourceData, sourceLen);
}

// ----------------------------------------------------------------------

int UdpManagerHandlerInternal::OnUserSuppliedDecrypt2(UdpConnection *con, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	UdpConnectionMT *conMT = reinterpret_cast<UdpConnectionMT *>(con->GetPassThroughData());
	return m_owner->OnUserSuppliedDecrypt2(conMT, destData, sourceData, sourceLen);
}

// ======================================================================

UdpManagerHandlerMT::UdpManagerHandlerMT() :
	m_refCount(1),
	m_managerHandler(0)
{
	m_managerHandler = new UdpManagerHandlerInternal(this);
}

// ----------------------------------------------------------------------

UdpManagerHandlerMT::~UdpManagerHandlerMT()
{
	delete m_managerHandler;
	m_managerHandler = 0;
}

// ----------------------------------------------------------------------

void UdpManagerHandlerMT::AddRef()
{
	++m_refCount;
}

// ----------------------------------------------------------------------

void UdpManagerHandlerMT::Release()
{
	if (--m_refCount == 0)
		delete this;
}

// ----------------------------------------------------------------------

int UdpManagerHandlerMT::OnUserSuppliedEncrypt(UdpConnectionMT *, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	memcpy(destData, sourceData, sourceLen);
	return sourceLen;
}

// ----------------------------------------------------------------------
int UdpManagerHandlerMT::OnUserSuppliedDecrypt(UdpConnectionMT *, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	memcpy(destData, sourceData, sourceLen);
	return sourceLen;
}

// ----------------------------------------------------------------------

int UdpManagerHandlerMT::OnUserSuppliedEncrypt2(UdpConnectionMT *, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	memcpy(destData, sourceData, sourceLen);
	return sourceLen;
}

// ----------------------------------------------------------------------

int UdpManagerHandlerMT::OnUserSuppliedDecrypt2(UdpConnectionMT *, unsigned char *destData, unsigned char const *sourceData, int sourceLen)
{
	memcpy(destData, sourceData, sourceLen);
	return sourceLen;
}

// ----------------------------------------------------------------------

UdpManagerHandler *UdpManagerHandlerMT::getManagerHandler()
{
	return m_managerHandler;
}

// ----------------------------------------------------------------------

void UdpManagerHandlerMT::queueConnectRequest(UdpConnection *con)
{

	if (UdpLibraryMT::getUseNetworkThread())
		Events::pushIncomingEventConnectRequest(this, con);
	else
	{
		con->AddRef();
		UdpConnectionMT *udpConnectionMT = new UdpConnectionMT(con);
		OnConnectRequest(udpConnectionMT);
		udpConnectionMT->Release();
	}
}

// ======================================================================

