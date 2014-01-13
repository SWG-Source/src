// ======================================================================
//
// SetupSharedNetworkMessages.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupSharedNetworkMessages_H
#define INCLUDED_SetupSharedNetworkMessages_H

// ======================================================================

class SetupSharedNetworkMessages
{
public:

	static void install ( void );
	static void remove();

private:

	SetupSharedNetworkMessages();
	~SetupSharedNetworkMessages();
	SetupSharedNetworkMessages(const SetupSharedNetworkMessages&);
	SetupSharedNetworkMessages& operator= (const SetupSharedNetworkMessages&);
};

// ======================================================================

#endif
