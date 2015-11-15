// LoginConnectionServerAddress.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "LoginConnectionServerAddress.h"

//-----------------------------------------------------------------------

LoginConnectionServerAddress::LoginConnectionServerAddress (int newId, const std::string & newServiceAddress, const uint16 newClientPortPrivate, const uint16 newClientPortPublic, int newNumClients, const uint16 _pingPort) :
GameNetworkMessage        ("LoginConnectionServerAddress"),
clientServiceAddress      (newServiceAddress),
clientServicePortPrivate  (newClientPortPrivate),
clientServicePortPublic   (newClientPortPublic),
id(newId),
numClients(newNumClients),
pingPort                  (_pingPort)
{
	addVariable(clientServiceAddress);
	addVariable(clientServicePortPrivate);
	addVariable(clientServicePortPublic);
	addVariable(id);
	addVariable(numClients);
	addVariable(pingPort);
}

//-----------------------------------------------------------------------

LoginConnectionServerAddress::LoginConnectionServerAddress (Archive::ReadIterator & source) :
GameNetworkMessage        ("LoginConnectionServerAddress"),
clientServiceAddress      (),
clientServicePortPrivate  (0),
clientServicePortPublic   (0),
id(0),
numClients(0),
pingPort                  (0)
{
	addVariable(clientServiceAddress);
	addVariable(clientServicePortPrivate);
	addVariable(clientServicePortPublic);
	addVariable(id);
	addVariable(numClients);
	addVariable(pingPort);

	AutoByteStream::unpack(source);
}
//-----------------------------------------------------------------------

LoginConnectionServerAddress::~LoginConnectionServerAddress ()
{
}

//-----------------------------------------------------------------------

