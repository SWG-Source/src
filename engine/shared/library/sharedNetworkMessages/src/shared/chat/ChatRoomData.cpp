//======================================================================
//
// ChatRoomData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatRoomData.h"

//-----------------------------------------------------------------------

//======================================================================

ChatRoomData::ChatRoomData () :
id          (0),
roomType    (0),
path        (),
owner       (),
creator     (),
title       (),
moderators  (),
moderated   (0)
{
}

//======================================================================
