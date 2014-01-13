// ==================================================================
//
// NewbieTutorialEnableHudElement.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialEnableHudElement.h"

// ==================================================================

const char* const NewbieTutorialEnableHudElement::cms_name = "NewbieTutorialEnableHudElement";

// ==================================================================

NewbieTutorialEnableHudElement::NewbieTutorialEnableHudElement (const std::string& name, const bool enable, const float blinkTime) :
	GameNetworkMessage ("NewbieTutorialEnableHudElement"),
	m_name (name),
	m_enable (enable),
	m_blinkTime (blinkTime)
{
	addVariable (m_name);
	addVariable (m_enable);
	addVariable (m_blinkTime);
}

// ------------------------------------------------------------------

NewbieTutorialEnableHudElement::NewbieTutorialEnableHudElement (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialEnableHudElement"),
	m_name (),
	m_enable (),
	m_blinkTime ()
{
	addVariable (m_name);
	addVariable (m_enable);
	addVariable (m_blinkTime);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialEnableHudElement::~NewbieTutorialEnableHudElement ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialEnableHudElement::getName () const
{
	return m_name.get ();
}

// ------------------------------------------------------------------

bool NewbieTutorialEnableHudElement::getEnable () const
{
	return m_enable.get ();
}

// ------------------------------------------------------------------

float NewbieTutorialEnableHudElement::getBlinkTime () const
{
	return m_blinkTime.get ();
}

// ==================================================================
