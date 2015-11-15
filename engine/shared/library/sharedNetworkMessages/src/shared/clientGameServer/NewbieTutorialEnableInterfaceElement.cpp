// ==================================================================
//
// NewbieTutorialEnableInterfaceElement.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialEnableInterfaceElement.h"

// ==================================================================

const char* const NewbieTutorialEnableInterfaceElement::cms_name = "NewbieTutorialEnableInterfaceElement";

// ==================================================================

NewbieTutorialEnableInterfaceElement::NewbieTutorialEnableInterfaceElement (const std::string& name, const bool enable) :
	GameNetworkMessage ("NewbieTutorialEnableInterfaceElement"),
	m_name (name),
	m_enable (enable)
{
	addVariable (m_name);
	addVariable (m_enable);
}

// ------------------------------------------------------------------

NewbieTutorialEnableInterfaceElement::NewbieTutorialEnableInterfaceElement (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialEnableInterfaceElement"),
	m_name (),
	m_enable ()
{
	addVariable (m_name);
	addVariable (m_enable);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialEnableInterfaceElement::~NewbieTutorialEnableInterfaceElement ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialEnableInterfaceElement::getName () const
{
	return m_name.get ();
}

// ------------------------------------------------------------------

bool NewbieTutorialEnableInterfaceElement::getEnable () const
{
	return m_enable.get ();
}

// ==================================================================
