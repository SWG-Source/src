// ======================================================================
//
// EditAppearanceMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/EditAppearanceMessage.h"

// ======================================================================

const char * const EditAppearanceMessage::MessageType = "EditAppearanceMessage";

//----------------------------------------------------------------------

EditAppearanceMessage::EditAppearanceMessage(const NetworkId & target) :
GameNetworkMessage(MessageType),
m_target(target)
{
	addVariable(m_target);
}

//----------------------------------------------------------------------

EditAppearanceMessage::EditAppearanceMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_target()
{
	addVariable(m_target);
	unpack(source);
}

//----------------------------------------------------------------------
