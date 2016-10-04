// ======================================================================
//
// SharedRemoteDebugServer.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedRemoteDebugServer/FirstSharedRemoteDebugServer.h"
#include "Archive/ByteStream.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServer.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServerConnection.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedDebug/RemoteDebug_inner.h"

//-------------------------------------------------------------------

//static definitions
Service *                      SharedRemoteDebugServer::ms_serviceHandle;
SharedRemoteDebugServerConnection *  SharedRemoteDebugServer::ms_connection;
bool                           SharedRemoteDebugServer::ms_installed;
unsigned int                   SharedRemoteDebugServer::ms_remoteDebugToolChannelID;

//-------------------------------------------------------------------

void SharedRemoteDebugServer::install()
{
	ms_serviceHandle = nullptr;
	ms_connection = nullptr;

	if (!ConfigSharedFoundation::getUseRemoteDebug())
		return;

	DEBUG_FATAL(ms_installed, ("sharedRemoteDebugServer already installed"));

	NetworkSetupData setup;
	setup.port = static_cast<unsigned short>(ConfigSharedFoundation::getDefaultRemoteDebugPort());
	setup.maxConnections = 5;
	ms_serviceHandle = new Service(ConnectionAllocator<SharedRemoteDebugServerConnection>(), setup);

	//even though this is the game client, this is a remoteDebug *server*, since it sends data to a Qt app for viewing
	RemoteDebugServer::install(nullptr, open, close, send, nullptr);

	//this value needs to be true before the call to RemoteDebugServer::open
	ms_installed = true;

	//try to connect to a localhost instance of the remotedebug app
	RemoteDebugServer::open("127.0.0.1", 4444);
}

//-------------------------------------------------------------------

void SharedRemoteDebugServer::remove(void)
{
	if(!ms_installed)
		return;

	close();

	NOT_NULL(ms_serviceHandle);
	delete ms_serviceHandle;
	delete ms_connection;

	RemoteDebugServer::remove();
	ms_installed = false;
}

//-------------------------------------------------------------------

void SharedRemoteDebugServer::open(const char *server, uint16 port)
{
	DEBUG_FATAL(!ms_installed, ("sharedRemoteDebugServer not installed"));

	//attempt to auto-connect to a localhost client app (if it's auto-listening)

	ms_connection = new SharedRemoteDebugServerConnection(server, port);
}

//-------------------------------------------------------------------

void SharedRemoteDebugServer::close(void)
{
	DEBUG_FATAL(!ms_installed, ("sharedRemoteDebugServer not installed"));
	delete ms_connection;
	ms_connection = nullptr;
}

//-------------------------------------------------------------------

void SharedRemoteDebugServer::send(void *buffer, uint32 bufferLen)
{
	DEBUG_FATAL(!ms_installed, ("sharedRemoteDebugServer not installed"));

	NOT_NULL(ms_connection);
	ms_connection->send(Archive::ByteStream(reinterpret_cast<const unsigned char *>(buffer), bufferLen), false);
	//always send on VNL channel 0, send the buffer, it's length, and unreliable
}

//-------------------------------------------------------------------
