// ======================================================================
//
// SetupSwgSharedNetworkMessages.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupSwgSharedNetworkMessages_H
#define INCLUDED_SetupSwgSharedNetworkMessages_H

// ======================================================================

class SetupSwgSharedNetworkMessages
{
public:

	static void install ( void );
	static void remove();

private:

	SetupSwgSharedNetworkMessages();
	~SetupSwgSharedNetworkMessages();
	SetupSwgSharedNetworkMessages(const SetupSwgSharedNetworkMessages&);
	SetupSwgSharedNetworkMessages& operator= (const SetupSwgSharedNetworkMessages&);
};

// ======================================================================

#endif
