//========================================================================
//
// ReloadAdminTableMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReloadAdminTableMessage_H
#define	_INCLUDED_ReloadAdminTableMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  Game Server
 * Action:  Tells a game server to reload the admin datatable
 */
class ReloadAdminTableMessage: public GameNetworkMessage
{
public:
	ReloadAdminTableMessage();
	ReloadAdminTableMessage(Archive::ReadIterator & source);
	~ReloadAdminTableMessage();

private:
	ReloadAdminTableMessage(const ReloadAdminTableMessage&);
	ReloadAdminTableMessage& operator= (const ReloadAdminTableMessage&);
};


// ======================================================================

#endif	// _INCLUDED_ReloadAdminTableMessage_H

