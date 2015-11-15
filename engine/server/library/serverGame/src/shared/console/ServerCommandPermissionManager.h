// ======================================================================
//
// ServerCommandPermissionManager.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerCommandPermissionManager_H
#define INCLUDED_ServerCommandPermissionManager_H


#include "sharedCommandParser/CommandPermissionManager.h"

class DataTable;

class ServerCommandPermissionManager : public CommandPermissionManager
{

public:
	ServerCommandPermissionManager ();
	virtual ~ServerCommandPermissionManager ();

	/**
	* the commandPath is a period-seperated path down through the command tree
	*/

	virtual bool                 isCommandAllowed         (const NetworkId & userId, const Unicode::String & commandPath) const;

protected:
	ServerCommandPermissionManager (const ServerCommandPermissionManager & rhs);
	ServerCommandPermissionManager &   operator=(const ServerCommandPermissionManager & rhs);

private:

	DataTable*                  m_permissionTable;
};


#endif
