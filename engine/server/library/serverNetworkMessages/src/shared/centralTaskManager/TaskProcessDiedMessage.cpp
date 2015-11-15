//========================================================================
//
// TaskProcessDiedMessage.cpp - tells Gameserver object baseline data has ended.
//
// copyright 2003 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TaskProcessDiedMessage.h"

#include "serverNetworkMessages/GameGameServerMessages.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object we are updating
 */
TaskProcessDiedMessage::TaskProcessDiedMessage(const int id, const std::string & processName) :
	GameNetworkMessage("TaskProcessDiedMessage"),
	m_processId(id),
	m_processName(processName)
{
	addVariable(m_processId);
	addVariable(m_processName);

}	// TaskProcessDiedMessage::TaskProcessDiedMessage

//-----------------------------------------------------------------------

TaskProcessDiedMessage::TaskProcessDiedMessage(Archive::ReadIterator & source) :
GameNetworkMessage("TaskProcessDiedMessage"),
m_processId(),
m_processName()
{
	addVariable(m_processId);
	addVariable(m_processName);
	unpack(source);
}	// TaskProcessDiedMessage::TaskProcessDiedMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
TaskProcessDiedMessage::~TaskProcessDiedMessage()
{
}	// TaskProcessDiedMessage::~TaskProcessDiedMessage

//-----------------------------------------------------------------------

