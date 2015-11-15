//
// GameControllerMessage.h
// asommers 2-24-99
//
// copyright 1999, bootprint entertainment
//
// data files should be including GameControllerMessage.def
// code files should be including GameControllerMessage.h

#ifndef GAMECONTROLLERMESSAGE_H
#define GAMECONTROLLERMESSAGE_H

namespace GameControllerMessageFlags
{
	const uint32    NONE                  = 0x00000000;
	const uint32    SEND                  = 0x00000001;  // send should always be combined with one of the dest flags below
	const uint32    RELIABLE              = 0x00000002;
	const uint32    SOURCE_REMOTE_SERVER  = 0x00000004;
	const uint32    DEST_AUTH_CLIENT      = 0x00000008;
	const uint32    DEST_PROXY_CLIENT     = 0x00000010;
	const uint32    DEST_AUTH_SERVER      = 0x00000020;
	const uint32    DEST_PROXY_SERVER     = 0x00000040;
	const uint32    DEST_SERVER           = 0x00000080;
	const uint32    SOURCE_REMOTE_CLIENT  = 0x00000100;

	const uint32    DEST_ALL_CLIENT   = DEST_AUTH_CLIENT | DEST_PROXY_CLIENT;
	const uint32    DEST_ALL_SERVER   = DEST_AUTH_SERVER | DEST_PROXY_SERVER;
	const uint32    DEST_ALL          = DEST_ALL_CLIENT | DEST_ALL_SERVER;

	const uint32    SOURCE_REMOTE     = SOURCE_REMOTE_SERVER | SOURCE_REMOTE_CLIENT;

  //@todo remove DESTINATIONS and DEST_SERVER
	const uint32    DESTINATIONS = DEST_AUTH_CLIENT | 
	                               DEST_PROXY_CLIENT |
	                               DEST_AUTH_SERVER |
	                               DEST_PROXY_SERVER |
	                               DEST_SERVER;
}

#include "sharedFoundation/GameControllerMessage.def"

#endif
