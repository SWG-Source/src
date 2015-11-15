// ChatServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "ChatServerConnection.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

ChatServerConnection::ChatServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

ChatServerConnection::~ChatServerConnection()
{
}

//-----------------------------------------------------------------------

void ChatServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("ChatServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------
