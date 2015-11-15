//========================================================================
//
// CreateNewObjectMessage.cpp - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateNewObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object that was created
 */
CreateNewObjectMessage::CreateNewObjectMessage(const NetworkId& id, uint32 process,
	const std::string &scene, const Vector &pos) :
	GameNetworkMessage("CreateNewObjectMessage"),
	m_id(id),
	m_process(process),
	m_scene(scene),
	m_pos(pos)
{
	addVariable(m_id);
	addVariable(m_process);
	addVariable(m_scene);
	addVariable(m_pos);
}	// CreateNewObjectMessage::CreateNewObjectMessage

//-----------------------------------------------------------------------

CreateNewObjectMessage::CreateNewObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("CreateNewObjectMessage"),
m_id(),
m_process(),
m_scene(),
m_pos()
{
	addVariable(m_id);
	addVariable(m_process);
	addVariable(m_scene);
	addVariable(m_pos);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
CreateNewObjectMessage::~CreateNewObjectMessage()
{
}	// CreateNewObjectMessage::~CreateNewObjectMessage

//-----------------------------------------------------------------------
