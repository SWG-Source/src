// ======================================================================
//
// CommandQueueEntry.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommandQueueEntry.h"
#include "serverGame/CreatureObject.h"

#include "Archive/ByteStream.h"
#include "sharedGame/CommandTable.h"

// ======================================================================

CommandQueueEntry::CommandQueueEntry() :
	m_command(&CommandTable::getNullCommand()),
	m_targetId(NetworkId::cms_invalid),
	m_params(),
	m_sequenceId(0),
	m_clearable(true),
	m_priority()
{
}

// ----------------------------------------------------------------------

CommandQueueEntry::CommandQueueEntry(
	Command const &command,
	NetworkId const &targetId,
	Unicode::String const &params,
	uint32 sequenceId,
	bool clearable,
	Command::Priority priority,
	bool autoAttack) :

	m_command(&command),
	m_targetId(targetId),
	m_params(params),
	m_sequenceId(sequenceId),
	m_clearable(clearable),
	m_priority(priority)
{
}

// ----------------------------------------------------------------------

CommandQueueEntry::CommandQueueEntry(CommandQueueEntry const &entry) :
	m_command(entry.m_command), //lint !e1554
	m_targetId(entry.m_targetId),
	m_params(entry.m_params),
	m_sequenceId(entry.m_sequenceId),
	m_clearable(entry.m_clearable),
	m_priority(entry.m_priority)
{
}

// ----------------------------------------------------------------------

CommandQueueEntry &CommandQueueEntry::operator=(CommandQueueEntry const &rhs)
{
	if (&rhs != this)
	{
		m_command = rhs.m_command; //lint !e1555
		m_targetId = rhs.m_targetId;
		m_params = rhs.m_params;
		m_sequenceId = rhs.m_sequenceId;
		m_clearable = rhs.m_clearable;
		m_priority = rhs.m_priority;
	}
	return *this;
}

// ======================================================================

namespace Archive
{
	// ----------------------------------------------------------------------

	void get(ReadIterator &source, CommandQueueEntry &target)
	{
		uint32 commandHash;

		Archive::get(source, commandHash);
		Archive::get(source, target.m_targetId);
		Archive::get(source, target.m_params);
		Archive::get(source, target.m_sequenceId);
		Archive::get(source, target.m_clearable);
		target.m_command = &CommandTable::getCommand(commandHash);
	}

	// ----------------------------------------------------------------------

	void put(ByteStream &target, CommandQueueEntry const &source)
	{
		Archive::put(target, source.m_command->m_commandHash);
		Archive::put(target, source.m_targetId);
		Archive::put(target, source.m_params);
		Archive::put(target, source.m_sequenceId);
		Archive::put(target, source.m_clearable);
	}

	// ----------------------------------------------------------------------
} // namespace Archive