//======================================================================
//
// ChatPersistentMessageToClientData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"

//======================================================================

ChatPersistentMessageToClientData::ChatPersistentMessageToClientData () :
fromCharacterName (),
fromGameCode      (),
fromServerCode    (),
id                (0),
isHeader          (true),
message           (),
subject           (),
outOfBand         (),
status            (0),
timeStamp         (0)
{

}

//======================================================================
