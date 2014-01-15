//========================================================================
//
// ReloadCommandTableMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReloadCommandTableMessage_H
#define	_INCLUDED_ReloadCommandTableMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  Game Server
 * Action:  Tells a game server to reload the command datatable
 */
class ReloadCommandTableMessage: public GameNetworkMessage
{
public:
	ReloadCommandTableMessage();
	ReloadCommandTableMessage(Archive::ReadIterator & source);
	~ReloadCommandTableMessage();

private:
	ReloadCommandTableMessage(const ReloadCommandTableMessage&);
	ReloadCommandTableMessage& operator= (const ReloadCommandTableMessage&);
};


// ======================================================================

#endif	// _INCLUDED_ReloadCommandTableMessage_H

