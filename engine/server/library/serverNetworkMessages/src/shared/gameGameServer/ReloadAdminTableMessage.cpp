// ======================================================================
//
// ReloadAdminTableMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReloadAdminTableMessage.h"

// ======================================================================

ReloadAdminTableMessage::ReloadAdminTableMessage() :
	GameNetworkMessage("ReloadAdminTableMessage")
{
}

// ----------------------------------------------------------------------

ReloadAdminTableMessage::ReloadAdminTableMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ReloadAdminTableMessage")
{
	unpack(source);
}

// ----------------------------------------------------------------------

ReloadAdminTableMessage::~ReloadAdminTableMessage()
{
}

// ======================================================================

