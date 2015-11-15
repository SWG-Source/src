// ======================================================================
//
// RestartServerMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RestartServerMessage.h"

// ======================================================================

RestartServerMessage::RestartServerMessage(std::string const & scene, int x, int z) :
	GameNetworkMessage("RestartServerMessage"),
	m_scene(scene),
	m_x(x),
	m_z(z)
{
	addVariable(m_scene);
	addVariable(m_x);
	addVariable(m_z);
}

//-----------------------------------------------------------------------

RestartServerMessage::RestartServerMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("RestartServerMessage"),
	m_scene(),
	m_x(),
	m_z()
{
	addVariable(m_scene);
	addVariable(m_x);
	addVariable(m_z);
	unpack(source); //lint !e1506 // virtual in constructor
}

//-----------------------------------------------------------------------

RestartServerMessage::~RestartServerMessage()
{
}

// ----------------------------------------------------------------------

std::string const & RestartServerMessage::getScene() const
{
	return m_scene.get();
}

// ----------------------------------------------------------------------

int RestartServerMessage::getX() const
{
	return m_x.get();
}

// ----------------------------------------------------------------------

int RestartServerMessage::getZ() const
{
	return m_z.get();
}

// ======================================================================
