// ======================================================================
//
// ReloadCommandTableMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReloadCommandTableMessage.h"

// ======================================================================

ReloadCommandTableMessage::ReloadCommandTableMessage() :
	GameNetworkMessage("ReloadCommandTableMessage")
{
}

// ----------------------------------------------------------------------

ReloadCommandTableMessage::ReloadCommandTableMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ReloadCommandTableMessage")
{
	unpack(source);
}

// ----------------------------------------------------------------------

ReloadCommandTableMessage::~ReloadCommandTableMessage()
{
}

// ======================================================================

