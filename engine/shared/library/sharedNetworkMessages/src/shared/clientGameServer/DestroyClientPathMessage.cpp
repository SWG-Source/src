// ==================================================================
//
// DestroyClientPathMessage.h
// Copyright 2003, Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DestroyClientPathMessage.h"

// ==================================================================

char const * const DestroyClientPathMessage::cms_name = "DestroyClientPathMessage";

// ==================================================================

DestroyClientPathMessage::DestroyClientPathMessage () :
	GameNetworkMessage (cms_name)
{
}

// ------------------------------------------------------------------

DestroyClientPathMessage::DestroyClientPathMessage (Archive::ReadIterator& /*source*/) :
	GameNetworkMessage (cms_name)
{
}

// ------------------------------------------------------------------

DestroyClientPathMessage::~DestroyClientPathMessage ()
{
}

// ==================================================================
