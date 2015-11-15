// ==================================================================
//
// NewbieTutorialSetToolbarElement.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialSetToolbarElement.h"

// ==================================================================

const char* const NewbieTutorialSetToolbarElement::cms_name = "NewbieTutorialSetToolbarElement";

// ==================================================================

NewbieTutorialSetToolbarElement::NewbieTutorialSetToolbarElement (int slot, const std::string& commandName) :
	GameNetworkMessage ("NewbieTutorialSetToolbarElement"),
	m_slot (slot),
	m_commandName (commandName),
	m_object (-1)
{
	addVariable (m_slot);
	addVariable (m_commandName);
	addVariable (m_object);
}
// ==================================================================

NewbieTutorialSetToolbarElement::NewbieTutorialSetToolbarElement (int slot, int64 object) :
	GameNetworkMessage ("NewbieTutorialSetToolbarElement"),
	m_slot (slot),
	m_commandName (),
	m_object (object)
{
	addVariable (m_slot);
	addVariable (m_commandName);
	addVariable (m_object);
}

// ------------------------------------------------------------------

NewbieTutorialSetToolbarElement::NewbieTutorialSetToolbarElement (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialSetToolbarElement"),
	m_slot (),
	m_commandName (),
	m_object()
{
	addVariable(m_slot);
	addVariable (m_commandName);
	addVariable(m_object);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialSetToolbarElement::~NewbieTutorialSetToolbarElement ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialSetToolbarElement::getCommandName () const
{
	return m_commandName.get ();
}

// ------------------------------------------------------------------

int NewbieTutorialSetToolbarElement::getSlot () const
{
	return m_slot.get ();
}

// ------------------------------------------------------------------

int64 NewbieTutorialSetToolbarElement::getObject () const
{
	return m_object.get ();
}

// ==================================================================
