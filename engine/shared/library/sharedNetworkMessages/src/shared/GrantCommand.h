// GrantCommand.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GrantCommand_H
#define	_INCLUDED_GrantCommand_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GrantCommand : public GameNetworkMessage
{
public:
	GrantCommand(const std::string & commandName);
	GrantCommand(Archive::ReadIterator & source);
	~GrantCommand();

	const std::string & getCommandName() const;
private:
	GrantCommand & operator = (const GrantCommand & rhs);
	GrantCommand(const GrantCommand & source);

private:
	Archive::AutoVariable<std::string>  commandName;
};

//-----------------------------------------------------------------------

inline const std::string & GrantCommand::getCommandName() const
{
	return commandName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GrantCommand_H
