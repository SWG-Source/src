// ======================================================================
//
// ReloadTemplateMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReloadTemplateMessage.h"

// ======================================================================

ReloadTemplateMessage::ReloadTemplateMessage(const std::string & templateName) :
	GameNetworkMessage("ReloadTemplateMessage"),
	m_template(templateName)
{
	addVariable(m_template);
}

// ----------------------------------------------------------------------

ReloadTemplateMessage::ReloadTemplateMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ReloadTemplateMessage"),
	m_template()
{
	addVariable(m_template);
	unpack(source);
}

// ----------------------------------------------------------------------

ReloadTemplateMessage::~ReloadTemplateMessage()
{
}

// ======================================================================

