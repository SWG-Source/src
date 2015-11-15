// ======================================================================
//
// CommandQueueEntry.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandQueueEntry_H
#define INCLUDED_CommandQueueEntry_H


// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/Command.h"

// ======================================================================

class CommandQueueEntry
{
public:
	CommandQueueEntry();
	
	CommandQueueEntry( 
		Command const &command,
		NetworkId const &targetId,
		Unicode::String const &params,
		uint32 sequenceId,
		bool clearable,
		Command::Priority,
		bool autoAttack );
		
	CommandQueueEntry(CommandQueueEntry const &entry);
	CommandQueueEntry& operator=(CommandQueueEntry const &);
	
	Command::Priority getPriority() const;

public:
	Command const *   m_command;
	NetworkId         m_targetId;
	Unicode::String   m_params;
	uint32            m_sequenceId;
	bool              m_clearable;
	Command::Priority m_priority;

};

	
// ======================================================================

namespace Archive
{
	class ByteStream;
	class ReadIterator;
	
	void put(Archive::ByteStream &target, CommandQueueEntry const &source);
	void get(Archive::ReadIterator &source, CommandQueueEntry &target);
}


// ======================================================================

#endif // INCLUDED_CommandQueueEntry_H
