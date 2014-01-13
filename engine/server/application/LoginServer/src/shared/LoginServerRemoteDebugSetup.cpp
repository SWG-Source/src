// ======================================================================
//
// LoginServerRemoteDebugSetup.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "LoginServerRemoteDebugSetup.h"
#include "LoginServerRemoteDebugConnection.h"
#include "sharedDebug/RemoteDebug_inner.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"

//static definitions
//Network::Address   *LoginServerRemoteDebugSetup::ms_remoteDebugService;
bool                LoginServerRemoteDebugSetup::ms_installed;
unsigned int        LoginServerRemoteDebugSetup::ms_remoteDebugToolChannelID;
Connection          *LoginServerRemoteDebugSetup::ms_connection;
Service             *LoginServerRemoteDebugSetup::ms_remoteDebugService;

void LoginServerRemoteDebugSetup::install(void)
{
	if (ms_installed)
	{
		//TODO bad
		return;
	}

	if (!ConfigSharedFoundation::getUseRemoteDebug())
		return;

	NetworkSetupData setup;
	setup.port = 4446;
	setup.maxConnections = 16;
	ms_remoteDebugService = new Service(ConnectionAllocator<LoginServerRemoteDebugConnection>(), setup);

	RemoteDebugServer::install(remove, open, close, send, 0);//, receive);
	ms_installed = true;
}

void LoginServerRemoteDebugSetup::remove(void)
{
	if (!ms_installed)
	{
		//TODO bad
		return;
	}
	delete ms_remoteDebugService;

	RemoteDebugServer::remove();
	ms_installed = false;
}

void LoginServerRemoteDebugSetup::open(const char *, uint16)
{
	DEBUG_WARNING(true, ("not implemented! Fix ME!"));
	/*
	//TODO config file this?
	//attempt to auto-connect to a localhost client app (if it's auto-listening)
	if (ms_remoteDebugService->getHostPort())
		Network::connect(ms_connection, "127.0.0.1", port);
	else
	{
		//TODO bad
	}
	*/
}

void LoginServerRemoteDebugSetup::close(void)
{
	if (!ms_installed)
	{
		//TODO bad
		return;
	}
}

void LoginServerRemoteDebugSetup::send(void *, uint32)
{
	DEBUG_WARNING(true, ("not implemented! Fix ME!"));
	/*
	if (!ms_installed)
	{
		//TODO bad
		return;
	}
	//always send on VNL channel 0, send the buffer, it's length, and reliable=1
	NOT_NULL(ms_connection);
	static Archive::ByteStream bs(reinterpret_cast<unsigned char *>(buffer), bufferLen);
	ms_connection->send(bs, true);
	*/
}//lint !e818 // parameter bufffer could be declared as pointint to const (but the function prototype beyond this class cannot)
