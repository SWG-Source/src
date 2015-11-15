//========================================================================
//
// SetObjectPositionMessage.cpp - tells where to put an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetObjectPositionMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
SetObjectPositionMessage::SetObjectPositionMessage(const NetworkId& id,
	const std::string &scene, bool added, bool authoritative) :
	GameNetworkMessage("SetObjectPositionMessage"),
	m_added(added),
	m_authoritative(authoritative),
	m_id(id),
	m_scene(scene)
{
	addVariable(m_added);
	addVariable(m_authoritative);
	addVariable(m_id);
	addVariable(m_scene);
}

//-----------------------------------------------------------------------

SetObjectPositionMessage::SetObjectPositionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("SetObjectPositionMessage"),
m_added(false),
m_authoritative(false),
m_id(),
m_scene()
{
	addVariable(m_added);
	addVariable(m_authoritative);
	addVariable(m_id);
	addVariable(m_scene);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
SetObjectPositionMessage::~SetObjectPositionMessage()
{
}

//-----------------------------------------------------------------------
