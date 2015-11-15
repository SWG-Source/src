// ======================================================================
//
// SetupSwgServerNetworkMessages.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupSwgServerNetworkMessages_H
#define INCLUDED_SetupSwgServerNetworkMessages_H

#include "serverNetworkMessages/SetupServerNetworkMessages.h"


// ======================================================================

class SetupSwgServerNetworkMessages : public SetupServerNetworkMessages
{
public:

	static void install ( void );
	static void remove();

protected:
	         SetupSwgServerNetworkMessages();
	virtual ~SetupSwgServerNetworkMessages();

	virtual void internalInstall();
	virtual void internalRemove();

private:

	SetupSwgServerNetworkMessages(const SetupSwgServerNetworkMessages&);
	SetupSwgServerNetworkMessages& operator= (const SetupSwgServerNetworkMessages&);
};

// ======================================================================

#endif
