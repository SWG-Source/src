//========================================================================
//
// TaskProcessDiedMessage.h - tells Gameserver object baseline data has ended.
//
// copyright 2003 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_TaskProcessDiedMessage_H
#define	_INCLUDED_TaskProcessDiedMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <string>

//-----------------------------------------------------------------------

class TaskProcessDiedMessage : public GameNetworkMessage
{
public:
	explicit TaskProcessDiedMessage  (const int processId, const std::string & processName);
	TaskProcessDiedMessage(Archive::ReadIterator & source);
	~TaskProcessDiedMessage ();

	const int &          getProcessId(void) const;
	const std::string &  getProcessName(void) const;

private:
	Archive::AutoVariable<int>          m_processId;
	Archive::AutoVariable<std::string>  m_processName;

	TaskProcessDiedMessage();
	TaskProcessDiedMessage(const TaskProcessDiedMessage&);
	TaskProcessDiedMessage& operator= (const TaskProcessDiedMessage&);
};


//-----------------------------------------------------------------------

inline const int & TaskProcessDiedMessage::getProcessId() const
{
	return m_processId.get();
}	// TaskProcessDiedMessage::getId
//-----------------------------------------------------------------------

inline const std::string & TaskProcessDiedMessage::getProcessName() const
{
    return m_processName.get();
}

//-----------------------------------------------------------------------


#endif	// _INCLUDED_TaskProcessDiedMessage_H
