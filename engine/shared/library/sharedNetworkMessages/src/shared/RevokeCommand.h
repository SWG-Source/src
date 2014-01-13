// RevokeCommand.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_RevokeCommand_H
#define	_INCLUDED_RevokeCommand_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RevokeCommand : public GameNetworkMessage
{
public:
	RevokeCommand(const std::string & commandName);
	RevokeCommand(Archive::ReadIterator & source);
	~RevokeCommand();

	const std::string & getCommandName() const;

private:
	RevokeCommand & operator = (const RevokeCommand & rhs);
	RevokeCommand(const RevokeCommand & source);

private:
	Archive::AutoVariable<std::string> commandName;

};

//-----------------------------------------------------------------------

inline const std::string & RevokeCommand::getCommandName() const
{
	return commandName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_RevokeCommand_H
