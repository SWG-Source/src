// ======================================================================
//
// ProfilerOperationMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_ProfilerOperationMessage_H
#define	_INCLUDED_ProfilerOperationMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class ProfilerOperationMessage : public GameNetworkMessage
{
public:
	ProfilerOperationMessage(uint32 processId, std::string const &operation);
	ProfilerOperationMessage(Archive::ReadIterator & source);
	~ProfilerOperationMessage();

public:
	uint32 getProcessId() const;
	std::string const &getOperation() const;
	
private:
	Archive::AutoVariable<uint32> m_processId;
	Archive::AutoVariable<std::string> m_operation;

private:
	ProfilerOperationMessage(ProfilerOperationMessage const &);
	ProfilerOperationMessage &operator=(ProfilerOperationMessage const &);
};

// ======================================================================

inline uint32 ProfilerOperationMessage::getProcessId() const
{
	return m_processId.get();
}

// ----------------------------------------------------------------------

inline std::string const &ProfilerOperationMessage::getOperation() const
{
	return m_operation.get();
}

// ======================================================================

#endif	// _INCLUDED_ProfilerOperationMessage_H
