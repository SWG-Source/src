
// ======================================================================
//
// CloseHolocronMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CloseHolocronMessage_H
#define INCLUDED_CloseHolocronMessage_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

/** The network message to close the client's holocron window
 */
class CloseHolocronMessage : public GameNetworkMessage
{
public:
	CloseHolocronMessage();
	explicit CloseHolocronMessage (Archive::ReadIterator& source);

	static const char * const MessageType;
};

//-----------------------------------------------------------------------

#endif
