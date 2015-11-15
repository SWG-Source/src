// ConsoleConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "Archive/ByteStream.h"
#include "ConsoleConnection.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "TaskManager.h"

//-----------------------------------------------------------------------

ConsoleConnection::ConsoleConnection(UdpConnectionMT * udpConnection, TcpClient * t) :
ServerConnection(udpConnection, t)
{
}

//-----------------------------------------------------------------------

ConsoleConnection::~ConsoleConnection()
{
}

//-----------------------------------------------------------------------

void ConsoleConnection::onReceive(const Archive::ByteStream & message)
{
	static Archive::ReadIterator ri;
	ri = message.begin();

	GameNetworkMessage msg(ri);
	if(msg.isType("ConGenericMessage"))
	{
		ri = message.begin();
		ConGenericMessage taskConsoleCommand(ri);

		static std::string command;
		command = taskConsoleCommand.getMsg();
		while(command.rfind(' ') != command.npos || command.rfind('\n') != command.npos || command.rfind('\r') != command.npos)
			command = command.substr(0, command.length() - 1);

		ConGenericMessage response(TaskManager::executeCommand(command), taskConsoleCommand.getMsgId());
		send(response, true);

		GameNetworkMessage dis("RequestDisconnect");
		send(dis, true);

		REPORT_LOG(true, ("Received remote console command from %s : [%s]\n", getRemoteAddress().c_str(), command.c_str()));
	}
	else
	{
		// someone is sending the wrong data!
		disconnect();
	}
}

//-----------------------------------------------------------------------

