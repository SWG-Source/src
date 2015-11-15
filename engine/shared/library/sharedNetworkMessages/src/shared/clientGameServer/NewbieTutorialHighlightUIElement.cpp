// ==================================================================
//
// NewbieTutorialHighlightUIElement.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialHighlightUIElement.h"

// ==================================================================

const char* const NewbieTutorialHighlightUIElement::cms_name = "NewbieTutorialHighlightUIElement";

// ==================================================================

NewbieTutorialHighlightUIElement::NewbieTutorialHighlightUIElement (const std::string & widgetPath, float time) :
	GameNetworkMessage ("NewbieTutorialHighlightUIElement"),
	m_time (time),
	m_widgetPath (widgetPath)
{
	addVariable (m_time);
	addVariable (m_widgetPath);
}

// ------------------------------------------------------------------

NewbieTutorialHighlightUIElement::NewbieTutorialHighlightUIElement (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialHighlightUIElement"),
	m_time (),
	m_widgetPath ()
{
	addVariable(m_time);
	addVariable (m_widgetPath);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialHighlightUIElement::~NewbieTutorialHighlightUIElement ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialHighlightUIElement::getWidgetPath () const
{
	return m_widgetPath.get ();
}

// ------------------------------------------------------------------

float NewbieTutorialHighlightUIElement::getTime () const
{
	return m_time.get ();
}

// ==================================================================
