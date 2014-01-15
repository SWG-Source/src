// ======================================================================
//
// GameServerMessageArchive.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#ifndef INCLUDED_GameServerMessageArchive_H
#define INCLUDED_GameServerMessageArchive_H

#include "serverNetworkMessages/GameServerMessageInterface.h"


//-----------------------------------------------------------------------

// This class is used as an interface to pack/unpack messages that rely on game 
// server specific code, so the serverGame library doesn't have to be linked
// with every server.
class GameServerMessageArchive : public GameServerMessageInterface
{
public:

	static void install();
	static void remove();

	virtual void pack(Archive::ByteStream & target, AiCreatureStateMessage const & msg);
	virtual void unpack(Archive::ReadIterator & source, AiCreatureStateMessage & msg);

	virtual void pack(Archive::ByteStream & target, AiMovementMessage const & msg);
	virtual void unpack(Archive::ReadIterator & source, AiMovementMessage & msg);

protected:
	
	         GameServerMessageArchive();
	virtual ~GameServerMessageArchive();
};


#endif	// INCLUDED_GameServerMessageArchive_H
