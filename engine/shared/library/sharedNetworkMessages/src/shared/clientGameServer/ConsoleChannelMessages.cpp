// ConsoleChannelMessages.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"


//-----------------------------------------------------------------------
//	ConsoleGenericMessage : ConsoleChannelMessage

//-----------------------------------------------------------------------

const char * const ConGenericMessage::MessageType = "ConGenericMessage";

//----------------------------------------------------------------------

ConGenericMessage::ConGenericMessage(const std::string & consoleMsg, uint32 newMsgId) :
GameNetworkMessage (ConGenericMessage::MessageType),
msg                (consoleMsg),
msgId              (newMsgId)
{
	if (consoleMsg.empty())
	{
		WARNING(true, ("ConGenericMessage constructed with empty message"));
	}

	addVariable(msg);
	addVariable(msgId);
}

//-----------------------------------------------------------------------

ConGenericMessage::ConGenericMessage(Archive::ReadIterator & source) :
GameNetworkMessage (ConGenericMessage::MessageType),
msg                (),
msgId              (0)
{										
	addVariable(msg);
	addVariable(msgId);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

ConGenericMessage::~ConGenericMessage()
{
}

//-----------------------------------------------------------------------

const char * const ExecuteConsoleCommand::MessageType = "ExecuteConsoleCommand";

//----------------------------------------------------------------------

ExecuteConsoleCommand::ExecuteConsoleCommand(const std::string & consoleMsg) :
GameNetworkMessage (ExecuteConsoleCommand::MessageType),
m_command          (consoleMsg)
{
	addVariable(m_command);
}

//-----------------------------------------------------------------------

ExecuteConsoleCommand::ExecuteConsoleCommand(Archive::ReadIterator & source) :
GameNetworkMessage (ExecuteConsoleCommand::MessageType),
m_command          ()
{										
	addVariable(m_command);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

ExecuteConsoleCommand::~ExecuteConsoleCommand()
{
}

//-----------------------------------------------------------------------

