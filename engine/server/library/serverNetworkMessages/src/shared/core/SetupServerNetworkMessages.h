// ======================================================================
//
// SetupServerNetworkMessages.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupServerNetworkMessages_H
#define INCLUDED_SetupServerNetworkMessages_H

#include "serverNetworkMessages/SetupServerNetworkMessages.h"


// ======================================================================

class SetupServerNetworkMessages
{
public:

	static void install ( void );
	static void remove();

protected:
	         SetupServerNetworkMessages();
	virtual ~SetupServerNetworkMessages();

	virtual void internalInstall();
	virtual void internalRemove();

private:

	SetupServerNetworkMessages(const SetupServerNetworkMessages&);
	SetupServerNetworkMessages& operator= (const SetupServerNetworkMessages&);
};

// ======================================================================

#endif
