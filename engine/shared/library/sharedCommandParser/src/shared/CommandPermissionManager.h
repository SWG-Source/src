// ======================================================================
//
// CommandPermissionManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandPermissionManager_H
#define INCLUDED_CommandPermissionManager_H

class NetworkId;

// ======================================================================
class CommandPermissionManager
{
public:

	virtual                     ~CommandPermissionManager () = 0;

	/**
	* the commandPath is a period-seperated path down through the command tree
	*/

	virtual bool                 isCommandAllowed         (const NetworkId & userId, const Unicode::String & commandPath) const = 0;

protected:
	                             CommandPermissionManager ();
	                             CommandPermissionManager (const CommandPermissionManager & rhs);
	CommandPermissionManager &   operator=                (const CommandPermissionManager & rhs);
};

// ======================================================================

#endif
