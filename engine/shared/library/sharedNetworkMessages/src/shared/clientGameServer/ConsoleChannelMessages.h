// ConsoleChannelMessages.h
// copyright 2000 Verant Interactive
// Author: Chris Mayer

#ifndef	_CONSOLE_CHANNEL_MESSAGES_H
#define	_CONSOLE_CHANNEL_MESSAGES_H

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;


//-----------------------------------------------------------------------
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**	Send a generic console string */

class ConGenericMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	explicit ConGenericMessage	(const std::string & consoleMsg, uint32 newMsgId = 0);
	explicit ConGenericMessage	(Archive::ReadIterator & source);
	virtual	~ConGenericMessage	();
	const std::string &         getMsg				(void) const;
	const uint32                getMsgId (void) const;

private:
	Archive::AutoVariable<std::string> msg;
	Archive::AutoVariable<uint32>      msgId;

	ConGenericMessage();
	ConGenericMessage(const ConGenericMessage&);
	ConGenericMessage& operator= (const ConGenericMessage&);
};

//-----------------------------------------------------------------------

inline const std::string & ConGenericMessage::getMsg(void) const
{
	return msg.get(); //lint !e1037 // ambiguity is irrelevent here
}

//-----------------------------------------------------------------------

inline const uint32 ConGenericMessage::getMsgId(void) const
{
	return msgId.get(); 
}

//-----------------------------------------------------------------------

class ExecuteConsoleCommand : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	explicit ExecuteConsoleCommand	(const std::string & consoleMsg);
	explicit ExecuteConsoleCommand	(Archive::ReadIterator & source);
	virtual	~ExecuteConsoleCommand	();
	const std::string &         getCommand(void) const;

private:
	Archive::AutoVariable<std::string> m_command;

	ExecuteConsoleCommand();
	ExecuteConsoleCommand(const ExecuteConsoleCommand&);
	ExecuteConsoleCommand& operator= (const ExecuteConsoleCommand&);
};

//-----------------------------------------------------------------------

inline const std::string & ExecuteConsoleCommand::getCommand(void) const
{
	return m_command.get(); //lint !e1037 // ambiguity is irrelevent here
}

//-----------------------------------------------------------------------

#endif //_CONSOLE_CHANNEL_MESSAGES_H

