// ======================================================================
//
// LogoutMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_LogoutMessage_H
#define	_INCLUDED_LogoutMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LogoutMessage : public GameNetworkMessage
{
public:
	LogoutMessage();
	LogoutMessage(Archive::ReadIterator &source);
	~LogoutMessage();

private:
	LogoutMessage(LogoutMessage const &);
	LogoutMessage &operator=(LogoutMessage const &);
};

// ======================================================================

#endif	// _INCLUDED_LogoutMessage_H

