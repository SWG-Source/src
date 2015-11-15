// ======================================================================
//
// CloseHolocronMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CloseHolocronMessage.h"

#include "Archive/ByteStream.h"
#include "Archive/AutoByteStream.h"

// ======================================================================

const char * const CloseHolocronMessage::MessageType = "CloseHolocronMessage";

// ======================================================================

CloseHolocronMessage::CloseHolocronMessage()
: GameNetworkMessage(MessageType)
{
}

//-----------------------------------------------------------------------

CloseHolocronMessage::CloseHolocronMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MessageType)
{
	unpack (source);
}

// ======================================================================

