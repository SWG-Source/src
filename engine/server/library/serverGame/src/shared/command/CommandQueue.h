// ======================================================================
//
// CommandQueue.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandQueue_H
#define INCLUDED_CommandQueue_H

// ======================================================================

#include "Archive/AutoDeltaQueue.h"
#include "Archive/AutoDeltaVector.h"
#include "Archive/AutoDeltaMap.h"
#include "serverGame/CommandQueueEntry.h"
#include "sharedGame/Command.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedObject/Property.h"

#include <stack>

// ======================================================================

class Command;
class CreatureObject;
class ServerObject;

// ======================================================================

/**
 * @brief this class is the spinal cord of the game server
 */
class CommandQueue : public Property
{
public:

	static PropertyId getClassPropertyId();
	static CommandQueue * getCommandQueue(Object & object);

	/**
	 * @brief The states that the command queue can be in.
	 */
	enum State
	{
		/**
		 * The command queue is waiting for a command to start warming up.
		 * The command queue may be empty in this state, or the current
		 * command is waiting for a cooldown timer to end.
		 */
		State_Waiting,

		/**
		 * the current command is warming up
		 */
		State_Warmup,

		/**
		 * the current command is executing
		 */
		State_Execute,

		/**
		 * dummy value to indicate the number of states
		 */
		State_MAX,

		State_Invalid = -1
	};

	/**
	 * Each command has a different time value for each class.
	 * This enum defines the different time value classes.  Note
	 * that the time value classes may appear similar to but are
	 * not the same as the timer states.
	 */
	enum TimerClass
	{
		TimerClass_Warmup,
		TimerClass_Execute,
		TimerClass_Cooldown,
		TimerClass_Cooldown2,
		TimerClass_MAX
	};

	//
	//                               +----- this is the crc value of the cooldown timer name
	//                               |
	//                               |                 +----- this is the time when the cooldown started
	//                               |                 |
	//                               |                 |       +----- this is the time when the cooldown will end
	//                               v                 v       v
	typedef Archive::AutoDeltaMap< uint32, std::pair<double, double> > CooldownMapType;

	static void install();
	static void shutdown();

	CommandQueue(ServerObject & owner);

	/**
	 * @brief adds a command to the command queue.
	 */
	void enqueue(Command const &command, NetworkId const &targetId, const Unicode::String &params, uint32 sequenceId = 0, bool clearable = true, Command::Priority priority = Command::CP_Default);

	/**
	 * @brief used to remove a specific command from the command queue
	 */
	void    remove(uint32 sequenceId);

	/**
	 * @brief called from the owning object
	 *
	 * Used to update the command queue.  Should be called once
	 * for each queue at each server loop.
	 */
	void    update(float time);

	/**
	 * @brief used to tell if the command queue has a command with the given group
	 *
	 * It does not appear that this method is used anywhere
	 */
	bool    hasCommandFromGroup(uint32 groupHash) const;

	/**
	 * @brief removes commands of a specific group from the command queue
	 *
	 * Removes all of the commands that are currently in the command queue
	 * that match the specified group ID.  If the command at the head of
	 * the queue matches, it is cancelled even if the command is executing.
	 */
	void    clearCommandsFromGroup(uint32 groupHash, bool force = false);

	/**
	 * @brief autodelta packing
	 */
	void    addToPackage(Archive::AutoDeltaByteStream & bs);

	/**
	 * @brief changes the amount of time to use for a given
	 * timer class for the command at the head of the command queue
	 */
	bool    setCommandTimerValue(TimerClass timerClass, float newValue );

	/**
	 * @brief returns the current state of the command queue
	 *
	 * This returns the current state of the command queue.  Currently
	 * there is no way to tell why the command queue might be in
	 * a particular state (this is a future feature).
	 *
	 * @return the state of the queue.
	 *
	 * @see CommandQueue::State
	 */
	State   getCurrentCommandState() const;

	/**
	 * @brief cancels the current command
	 *
	 * Causes the command at the top of the command queue
	 * to stop executing, removes the command from the queue
	 * and if necessary tells the client that the command
	 * has been terminated.
	 */
	void    cancelCurrentCommand();

	/**
	 * @brief gets the crc value of the command at the head of the command queue
	 *
	 * @returns the crc value of the command at the head of the command queue.
	 * if the command queue is empty, this method returns 0.
	 */
	uint32  getCurrentCommand() const;

	/**
	 * @brief returns the amount of cooldown time left
	 * in the specified cooldown timer group
	 *
	 * this method is used by scripts
	 */
	float getCooldownTimeLeft( uint32 cooldownId ) const;

	/**
	 * @brief returns the amount of cooldown tim left
	 * int the specified cooldown timer group
	 *
	 * this method is used by scripts
	 */
	float getCooldownTimeLeft( const std::string &cooldownName ) const;

	/**
	 * @brief gets the list of cooldowns
	 *
	 * @returns the list of cooldowns
	 */
	CooldownMapType const & getCooldowns() const;

	/**
	* @brief setter for m_logCommandEnqueue
	*/
	void setLogCommandEnqueue(bool logCommandEnqueue);

	/**
	 * @brief depersist command cooldowns which have been persisted
	 *
	 * if a cool down value is large enough, we persist
	 * it across server restarts and character unloading
	 * and reloading from the DB; we depersist the persisted
	 * cooldowns when the character is reloaded from the DB
	 */
	void depersistCooldown();

	/*
	 * @brief resets all the cooldown timers to zero
	 */
	void resetCooldowns();

	// debug diagnostic
	void spew(std::string * output = nullptr);

private:
	CommandQueue(CommandQueue const &);
	CommandQueue &operator=(CommandQueue const &);

private:

	/**
	 * @brief does command cleanup work
	 *
	 * This should be called any time a command is removed from the queue.  This method
	 * keeps the combat command count syncronized and notifies the client if necessary
	 * that a command has been removed.
	 */
	void handleEntryRemoved(CommandQueueEntry const &entry, bool notifyClient = true);

	/**
	 * @brief tells the client that a command has been removed from the queue
	 */
	void notifyClientOfCommandRemoval(uint32 sequenceId, float waitTime, Command::ErrorCode status = Command::CEC_Success, int statusDetail = 0);

	/**
	 * @brief returns true if the command queue contains the maximum number
	 * of combat commands.
	 */
	bool isFull() const;

	/**
	 * @brief attempts to resolve the state of the command queue.
	 */
	void executeCommandQueue();

	/**
	 * @brief changes the state of the command queue.
	 *
	 * This determines what state the command queue should be in, and changes
	 * the state.
	 */
	void switchState();

	/**
	 * @brief fires the warmup trigger in the script engine.
	 *
	 * This method is called to run the warmup trigger in the script engine.  It
	 * is only called if the command has a warmup time.
	 */
	bool doWarmupTrigger(const CommandQueueEntry &entry );

	/**
	 * @brief starts the command handler in script for the action
	 *
	 * This method also performs the necessary initialization and
	 * handles command failures for the command handlers.
	 */
	void doExecute(const CommandQueueEntry &entry );

	/**
	 * @brief tells the client that a new command has started executing.
	 * @see updateClient
	 */
	void notifyClient();

	/**
	 * @brief sends the client a message indicating that timer values have changed.
	 * @see notifyClient
	 */
	void updateClient(TimerClass timerClass );

	/**
	 * @brief removes pending combat commands
	 *
	 * This method removes any pending combat commands from
	 * the command queue, including the combat command at
	 * the top of the stack if it's in a waiting state
	 * because it's unable to fire for some reason.
	 */
	void clearPendingCombatCommands();

	/**
	 * @brief persist a command cooldown
	 *
	 * if a cool down value is large enough, persist
	 * it across server restarts and character unloading
	 * and reloading from the DB
	 */
	void persistCooldown(uint32 cooldownGroupCrc, int cooldownTimeSeconds);

	/**
	 * @brief returns the current command queue state
	 * as a human-readable string.
	 */
	static const char *getStateAsString( State state );

	void executeCommand(Command const &command, NetworkId const &targetId, Unicode::String const &params, Command::ErrorCode &status, int &statusDetail, bool commandIsFromCommandQueue);

	ServerObject & getServerOwner();
	ServerObject const & getServerOwner() const;

	float getCooldownTime(Command const &command);

private:
#define AUTO(type) Archive::AutoDeltaVariable<type>

	typedef Archive::AutoDeltaQueue<CommandQueueEntry> EntryList;
	typedef Archive::AutoDeltaVector< double > CooldownArrayType;
	typedef Archive::AutoDeltaVector< float >  CommandTimeType;

	/**
	 * @brief temporary instance value used during command execution
	 */
	Command::ErrorCode    m_status;

	/**
	 * @brief temporary instance value used during command execution
	 */
	int                   m_statusDetail;

	/**
	 * @brief indicates the current state of the queue.
	 */
	AUTO(uint32)          m_state;

	/**
	 * @brief the time that the state of the queue should change.
	 *
	 * if the queue is empty, this value is undefined
	 *
	 * @see m_commandTimes
	 */
	AUTO(double)          m_nextEventTime;

	/**
	 * @brief the queue that stores the commands to be executed
	 */
	EntryList             m_queue;

	/**
	 * @brief an array of cooldown timer values.
	 */
	CooldownMapType       m_cooldowns;

	/**
	 * @brief modified command time values
	 *
	 * This is a small array of values that represent the
	 * current time values for the current command.  This array
	 * can be modified in script.  The values for this array
	 * are used to set m_nextEventTime when the queue state
	 * changes
	 *
	 * @see m_nextEventTime
	 */
	CommandTimeType       m_commandTimes;

	/**
	 * @brief keeps track of the number of combat commands
	 * in the queue.
	 *
	 * We use this value so that we do not need to iterate
	 * through the queue every time we want to see how many combat
	 * commands are in there, which happens every time the player
	 * adds a combat command to the queue.
	 */
	mutable AUTO(uint)    m_combatCount;

	/**
	 * @brief the time that the command queue last changed its state
	 */
	AUTO(double)          m_eventStartTime;

	/**
	 * @brief a stack of command pointers that are used during
	 * command execution.
	 */
	std::stack< const CommandQueueEntry * > m_commandStack;

	/**
	* @brief for debugging, log detailed information every time a
	* command is enqueued
	*/
	bool m_logCommandEnqueue;
};

// ----------------------------------------------------------------------

inline Command::Priority CommandQueueEntry::getPriority() const
{
	return m_priority;
}

// ----------------------------------------------------------------------

inline CommandQueue::CooldownMapType const & CommandQueue::getCooldowns() const
{
	return m_cooldowns;
}

// ----------------------------------------------------------------------

inline void CommandQueue::setLogCommandEnqueue(bool logCommandEnqueue)
{
	m_logCommandEnqueue = logCommandEnqueue;
}

// ======================================================================

#endif	// INCLUDED_CommandQueue_H
