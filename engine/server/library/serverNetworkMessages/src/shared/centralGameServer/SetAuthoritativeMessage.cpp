// ======================================================================
//
// SetAuthoritativeMessage.cpp - tells a Gameserver which server is authoritative
// over an object..
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetAuthoritativeMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "sharedMathArchive/TransformArchive.h"

// ======================================================================

SetAuthoritativeMessage::SetAuthoritativeMessage(NetworkId const &id, uint32 process, bool sceneChange, bool handlingCrash, NetworkId const &goalCell, Transform const &goalTransform, bool goalIsValid) :
	GameNetworkMessage("SetAuthoritativeMessage"),
	m_id(id),
	m_process(process),
	m_sceneChange(sceneChange),
	m_handlingCrash(handlingCrash),
	m_goalCell(goalCell),
	m_goalTransform(goalTransform),
	m_goalIsValid(goalIsValid)
{
	addVariable(m_id);
	addVariable(m_process);
	addVariable(m_sceneChange);
	addVariable(m_handlingCrash);
	addVariable(m_goalCell);
	addVariable(m_goalTransform);
	addVariable(m_goalIsValid);
}

// ----------------------------------------------------------------------

SetAuthoritativeMessage::SetAuthoritativeMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("SetAuthoritativeMessage"),
	m_id(),
	m_process(),
	m_sceneChange(),
	m_handlingCrash(),
	m_goalCell(),
	m_goalTransform(),
	m_goalIsValid()
{
	addVariable(m_id);
	addVariable(m_process);
	addVariable(m_sceneChange);
	addVariable(m_handlingCrash);
	addVariable(m_goalCell);
	addVariable(m_goalTransform);
	addVariable(m_goalIsValid);
	unpack(source);
}

// ----------------------------------------------------------------------

SetAuthoritativeMessage::~SetAuthoritativeMessage()
{
}

// ======================================================================

