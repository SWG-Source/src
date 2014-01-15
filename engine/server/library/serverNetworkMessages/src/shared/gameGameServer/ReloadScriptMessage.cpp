// ======================================================================
//
// ReloadScriptMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReloadScriptMessage.h"

// ======================================================================

ReloadScriptMessage::ReloadScriptMessage(const std::string & script) :
	GameNetworkMessage("ReloadScriptMessage"),
	m_script(script)
{
	addVariable(m_script);
}

// ----------------------------------------------------------------------

ReloadScriptMessage::ReloadScriptMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ReloadScriptMessage"),
	m_script()
{
	addVariable(m_script);
	unpack(source);
}

// ----------------------------------------------------------------------

ReloadScriptMessage::~ReloadScriptMessage()
{
}

// ======================================================================

