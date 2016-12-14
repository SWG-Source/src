// ======================================================================
//
// CommandQueue.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommandQueue.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/WeaponObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"

// --------------------------------------------------------------------------

namespace CommandQueueNamespace
{
	/**
	 * @brief whether or not to produce debug logging
	 */
#ifdef _DEBUG
	const bool         cs_debug                   = false;
#endif

	/**
	 * @brief the maximum number of combat commands that are allowed in
	 * the queue at any given time.
	 */
	const uint32       cs_maxQueuedCombatCommands = 2;
	
	/**
	 * @brief the number of seconds in a real-time day.
	 *
	 * This value is used for sanity checking
	 * @see setCommandTimerValue
	 */
	const float        cs_secondsPerDay           = 86400.f;
	
	/**
	 * @brief the current time is cached in this class variable.
	 *
	 * this is used to reduce the number of calls to Clock::getCurrentTime().
	 * note it may not be appropriate to use this value in every
	 * place in the code since there's guarantee that it's accurate from
	 * every entry point in script.  
	 *
	 * @see Clock::getCurrentTime
	 */
	double             s_currentTime             = 0.f;
	
}
using namespace CommandQueueNamespace;

// --------------------------------------------------------------------------

CommandQueue::CommandQueue(ServerObject & owner) :
	Property(getClassPropertyId(), owner),
	m_status              (Command::CEC_Success),
	m_statusDetail        (0),
	m_state               ( State_Waiting ),
	m_nextEventTime       ( 0.f ),
	m_queue               ( ),
	m_cooldowns           ( ),
	m_commandTimes        ( TimerClass_MAX ),
	m_combatCount         ( 0 ),
	m_eventStartTime      ( 0.0 ),
	m_logCommandEnqueue   (false)
{
}

// ----------------------------------------------------------------------
PropertyId CommandQueue::getClassPropertyId()
{
	return PROPERTY_HASH(CommandQueue, 889825674);
}

// ----------------------------------------------------------------------

bool CommandQueue::isFull() const
{
	// is it empty?
	if ( m_combatCount.get() == 0 )
	{
		return false;
	}
	
	// is it at the max?
	if ( m_combatCount.get() >= cs_maxQueuedCombatCommands )
	{
		return true;
	}
	
	// test to see if the current command is waiting on a cooldown.  we're considered
	// full if we're waiting on a cooldown timer
	
	if ( m_state.get() == State_Waiting && !m_queue.empty() )
	{
		const CommandQueueEntry &entry = *(m_queue.begin());
	
		if ( (getCooldownTimeLeft( entry.m_command->m_coolGroup ) > 0.f) ||
			 (getCooldownTimeLeft( entry.m_command->m_coolGroup2) > 0.f) )
		{
			return true;
		}
	}
	
	return false;
}

// ----------------------------------------------------------------------

void CommandQueue::enqueue(Command const &command, NetworkId const &targetId, Unicode::String const &params, uint32 sequenceId, bool clearable, Command::Priority priority)
{
	DEBUG_REPORT_LOG( cs_debug, ( "%f: CommandQueue::enqueue(%s, %s, %s, %d, %d, %d)\n",
		Clock::getCurrentTime(),
		command.m_commandName.c_str(),
		targetId.getValueString().c_str(),
		Unicode::wideToNarrow(params).c_str(),
		static_cast<int>(sequenceId),
		clearable ? 1 : 0,
		static_cast<int>(priority) ));

	// make sure the cooldown timer value is initialized if it's not in the map
	
    if(static_cast<int>(command.m_coolGroup) != -1)
	{
		if ( m_cooldowns.find( command.m_coolGroup ) == m_cooldowns.end() )
		{
			m_cooldowns.set( command.m_coolGroup, std::make_pair( 0.f, 0.f ) );
		}
	}
	if(static_cast<int>(command.m_coolGroup2) != -1)
	{
		if ( m_cooldowns.find( command.m_coolGroup2 ) == m_cooldowns.end() )
		{
			m_cooldowns.set( command.m_coolGroup2, std::make_pair( 0.f, 0.f ) );
		}
	}

	if ( priority == Command::CP_Default )
	{
		priority = command.m_defaultPriority;
	}
  	
  	//
  	// execute immediate commands imemdiately
  	//
  	if ( priority == Command::CP_Immediate )
  	{
  		Command::ErrorCode status       = Command::CEC_Success;
  		int                statusDetail = 0;

  		executeCommand(command, targetId, params, status, statusDetail, false);
		notifyClientOfCommandRemoval(sequenceId, 0.0f, status, statusDetail);
  	}
  	else
  	{
	  	if ( command.m_addToCombatQueue && isFull() )
		{
			// we do special things if the command queue is full or the current command is waiting on a cooldown
			// timer of combat actions and the player tries to add another
			// combat action!
			 
			// look in the queue for a command to replace this with
	
			for ( EntryList::iterator it = m_queue.begin(); it != m_queue.end(); )
			{
				EntryList::iterator j = it++;
				
				// if there's a command warming up or executing then we can skip the first command in the queue
				if ( m_state.get() != State_Waiting && j == m_queue.begin() )
				{
					continue;
				}
				
				const CommandQueueEntry &entry = *j;
				
				// test to see of this command is a combat command
				if ( entry.m_command->m_addToCombatQueue )
				{					
					// remove command from queue
					m_queue.erase( j );
					
					// we want to end this loop
					it = m_queue.end();
				}
			}
		}
		
		// create the command queue entry object
		const CommandQueueEntry entry( command, targetId, params, sequenceId, clearable, priority, false );
	   			
		m_queue.push( entry );
		
		if ( command.m_addToCombatQueue )
		{
			m_combatCount = m_combatCount.get() + 1;
		}
		
		// debug - /spewCommandQueue available from console
		if (m_logCommandEnqueue)
			spew();
		
		 // if there's only one thing in the command queue ,
		// then its okay to start execution immediately.
		if ( ( m_queue.size() == 1 ) )
		{
			executeCommandQueue();
		}
	}
}


// ----------------------------------------------------------------------

void CommandQueue::remove(uint32 sequenceId)
{
	DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::remove(0x%08x)\n", (int)sequenceId ) );
	
	if ( sequenceId == 0 )
	{
		// this would occur as a command from the client to cancel all pending combat actions
		// as a result of the player hitting ESC
		clearPendingCombatCommands();
	}
	else
	{
		// sometimes the game server wants to remove a specific command from the queue
		
		// iterate over each item in the queue
		for ( EntryList::iterator it = m_queue.begin(); it != m_queue.end(); /* no action */ )
		{
			EntryList::iterator j = it++;

						
			if ( ( j != m_queue.begin() ) || ( m_state.get() != State_Execute ) ) // <- don't cancel an executing command
			{			
				const CommandQueueEntry &entry = *j;
				
				if ( entry.m_sequenceId == sequenceId )
				{
					if ( j == m_queue.begin() )
					{
						m_nextEventTime = 0;
						m_state = State_Waiting;
					}
					
					handleEntryRemoved(entry );
					
					m_queue.erase( j );
					
					// force this loop to terminate, since sequenceIds must be unique
					// there's no way that if we removed a matching sequenceId that
					// there are any more of them in the command queue
					it = m_queue.end(); 
				}
			}
		}
	}

}	

// ----------------------------------------------------------------------

void CommandQueue::update(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("CommandQueue::update");
	executeCommandQueue();
}

void CommandQueue::executeCommandQueue()
{
	// cache the current time, so that we don't have to hit the getCurrentTime() method every time
	s_currentTime = Clock::getCurrentTime();
	
	//
	// iterate through each command in the queue
	//
	while ( !m_queue.empty() )
	{
		CommandQueueEntry &entry = *(m_queue.begin());

		// try to recover from having a nullptr command
		// maybe this should result in a fatal error?
		if ( entry.m_command == 0 )
		{
			WARNING( true, ( "executeCommandQueue: entry.m_command was nullptr! WTF?\n" ) );
			m_queue.pop();
			m_state = State_Waiting;
			m_nextEventTime = 0.f;
			continue;
		}

		// check the cooldown timer for this command's group

		const unsigned int cooldownGroup = entry.m_command->m_coolGroup;
		const unsigned int cooldownGroup2 = entry.m_command->m_coolGroup2;

		// if this group is still cooling down, we need to break
		// out of this loop and go do something else

		if(static_cast<int>(cooldownGroup) != -1)
		{		
			float coolTimeLeft = getCooldownTimeLeft( cooldownGroup );
			
			if ( coolTimeLeft > FLT_EPSILON )
			{
				DEBUG_REPORT_LOG( cs_debug, ( "%f: CommandQueue::executeCommandQueue(): waiting on cooldown group 0x%u: %f seconds left!\n", s_currentTime, cooldownGroup, coolTimeLeft ) );
				break;
			}
		}
		if(static_cast<int>(cooldownGroup2) != -1)
		{		
			float coolTimeLeft2 = getCooldownTimeLeft( cooldownGroup2 );
			
			if ( coolTimeLeft2 > FLT_EPSILON )
			{
				DEBUG_REPORT_LOG( cs_debug, ( "%f: CommandQueue::executeCommandQueue(): waiting on cooldown group 2 0x%u: %f seconds left!\n", s_currentTime, cooldownGroup2, coolTimeLeft2 ) );
				break;
			}
		}

		// if we need to wait more time before switching states, we should
		// just update the timer and leave now.
		if ( m_nextEventTime.get() > s_currentTime )
		{
			DEBUG_REPORT_LOG( cs_debug, ( "%f: CommandQueue::executeCommandQueue(): waiting on state %s: %f seconds left!\n",
				s_currentTime,
				getStateAsString((State) m_state.get() ), 
				m_nextEventTime.get() - s_currentTime ) );
			break;
		}

		switchState();

		switch ( m_state.get() )
		{
		case State_Waiting:
			{
				// don't call cancelCurrentCommand() here, because that method jacks with the command timer
				// so we do the work manually
				
				// NOTE RHanz:
				// this is the one place in this class that we're not calling
				// handleEntryRemoved when we're getting rid of an entry.
				// It was explicitly removed... if there seems to be a problem
				// with entries not being removed, this might be the culprit.
				// Leaving for the time being as it was explicitly removed.

				// NOTE SWyckoff:
				// Turns out that there were problems with not calling
				// handleEntryRemoved here that would cause the client to
				// trigger a cool down even if the command failed. Putting it
				// back in now.

				// This pop removes commands that were executed last frame and
				// need to be taken off the queue

				// switch state might have removed elements from the queue and invalidated entry
				// so we can't assume that we're still safe
				if(!m_queue.empty() && (&(*(m_queue.begin())) == &entry) && m_queue.begin()->m_command != nullptr)
				{
					handleEntryRemoved(*(m_queue.begin()), true);
					m_queue.pop();
				}

				break;
			}
		case State_Warmup:
			{
				// do something in script
				bool result = doWarmupTrigger(entry );
				
				if ( result == false )
				{
					cancelCurrentCommand();
				}
				else
				{
					notifyClient();
				}
			}
			break;
		case State_Execute:
			{
				// The matching m_queue.pop() for this doExecute is in the next frame's State_Waiting
				doExecute(entry );
			}
			break;
		default:
			FATAL( true, ( "command queue is in an invalid state. value=%d", (int)m_state.get() ) );
			break;
		}
	}
}

// ----------------------------------------------------------------------

void CommandQueue::doExecute(const CommandQueueEntry &entry )
{
	// execute command here
	m_status            = Command::CEC_Success;
	m_statusDetail      = 0;
	
	// push the command on the stack
	m_commandStack.push( &entry );

	executeCommand( *entry.m_command, entry.m_targetId, entry.m_params, m_status, m_statusDetail, true );

	FATAL( m_commandStack.empty(), ( "the command stack was empty!  something is horribly wrong with the command queue!" ) );
	
	// pop
	m_commandStack.pop();

	// check to see if the command was not removed from the queue
	// or if the command failed

	if ( !m_queue.empty() && (*(m_queue.begin())).m_sequenceId == entry.m_sequenceId && m_status == Command::CEC_Success )
	{
		// after executing the command, send execute and cooldown times to the client
		notifyClient();
	}
	
}

// ----------------------------------------------------------------------

void CommandQueue::updateClient(TimerClass timerClass )
{
	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();

	if (   (creatureOwner == nullptr)
	    || !creatureOwner->getClient())
	{
		return;
	}

	//
	// grab the command that's currently working
	//

	if ( m_queue.empty() )
		return;

	const CommandQueueEntry &entry = *(m_queue.begin());

	if ( entry.m_command == 0 ) // woah that's bad news!
	{
		WARNING( true, ( "CommandQueue::updateClient(): command was nullptr!\n" ) );
		return;
	}

	// create a message
	
	MessageQueueCommandTimer *msg = new MessageQueueCommandTimer(
		entry.m_sequenceId,
		entry.m_command->m_coolGroup,
		entry.m_command->m_coolGroup2,
		Crc::normalizeAndCalculate(entry.m_command->m_commandName.c_str()) );

	float currentTimer = 0.f;
	
	// figure out what to put in the message based on our current state
	
	if ( ( timerClass == TimerClass_Warmup && m_state.get() == State_Warmup ) || ( timerClass == TimerClass_Execute && m_state.get() == State_Execute ) )
	{
		currentTimer = static_cast< float > ( s_currentTime - m_eventStartTime.get() );
	}
	
	switch ( timerClass )
	{
		case TimerClass_Warmup:
			msg->setCurrentTime( MessageQueueCommandTimer::F_warmup, currentTimer );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_warmup, m_commandTimes[ TimerClass_Warmup ] );
			break;
		case TimerClass_Execute:
			msg->setCurrentTime( MessageQueueCommandTimer::F_execute, currentTimer );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_execute, m_commandTimes[ TimerClass_Execute ] );
			break;
		case TimerClass_Cooldown:
			msg->setCurrentTime( MessageQueueCommandTimer::F_cooldown, currentTimer );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_cooldown, m_commandTimes[ TimerClass_Cooldown ] );
			if(static_cast<int>(entry.m_command->m_coolGroup2) != -1)
			{			
				msg->setCurrentTime( MessageQueueCommandTimer::F_cooldown2, currentTimer );
				msg->setMaxTime    ( MessageQueueCommandTimer::F_cooldown2, m_commandTimes[ TimerClass_Cooldown2] );
			}
			break;
		case TimerClass_Cooldown2:
			DEBUG_WARNING(true, ("MessageQueueCommandTimer: updateClient; don't update cooldown 2 timer directly, please"));
			break;
		default:
			break;
	}
	
	// send message

	creatureOwner->getController()->appendMessage(
		static_cast< int >( CM_commandTimer ),
		0.0f,
		msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT );
}
	
// ----------------------------------------------------------------------

void CommandQueue::notifyClient()
{
	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();

	if (   (creatureOwner == nullptr)
	    || !creatureOwner->getClient())
	{
		return;
	}

	if ( m_queue.empty() )
	{
		WARNING( true, ( "CommandQueue::notifyClient(): queue is empty!\n" ) );
		return;
	}

	const CommandQueueEntry &entry = *(m_queue.begin());

	if ( entry.m_command == 0 )
	{
		WARNING( true, ( "CommandQueue::notifyClient(): command was nullptr!\n" ) );
		return;
	}

	MessageQueueCommandTimer *msg = new MessageQueueCommandTimer( entry.m_sequenceId, entry.m_command->m_coolGroup,
		entry.m_command->m_coolGroup2,
		Crc::normalizeAndCalculate(entry.m_command->m_commandName.c_str()) );

	DEBUG_REPORT_LOG( cs_debug && creatureOwner->getClient(), ( "%f: CommandQueue::notifyClient() [%f] warmup=%f execute=%f cooldown=%f cooldown2=%f\n",
		s_currentTime,
		s_currentTime - m_eventStartTime.get(),
		m_commandTimes[ TimerClass_Warmup ],
		m_commandTimes[ TimerClass_Execute ],
		m_commandTimes[ TimerClass_Cooldown ],
		m_commandTimes[ TimerClass_Cooldown2 ]) );
		
	switch ( m_state.get() )
	{
		case State_Warmup:
			msg->setCurrentTime( MessageQueueCommandTimer::F_warmup, static_cast< float >( s_currentTime - m_eventStartTime.get() ) );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_warmup, m_commandTimes[ TimerClass_Warmup ] );
			break;
		case State_Execute:
			msg->setCurrentTime( MessageQueueCommandTimer::F_execute, static_cast< float >( s_currentTime - m_eventStartTime.get() ) );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_execute, m_commandTimes[ TimerClass_Execute ] );
			msg->setCurrentTime( MessageQueueCommandTimer::F_cooldown, 0.f );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_cooldown, m_commandTimes[ TimerClass_Cooldown ] );
			msg->setCurrentTime( MessageQueueCommandTimer::F_cooldown2, 0.f );
			msg->setMaxTime    ( MessageQueueCommandTimer::F_cooldown2, m_commandTimes[ TimerClass_Cooldown2 ] );
			break;
		default:
			break;
	}

	creatureOwner->getController()->appendMessage(
		static_cast< int >( CM_commandTimer ),
		0.0f,
		msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT );

}

// ----------------------------------------------------------------------

bool CommandQueue::doWarmupTrigger(const CommandQueueEntry &entry )
{
	bool result = false;

	// make sure we could actually execute this command
	
	m_status       = Command::CEC_Success;
	m_statusDetail = 0;

	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();
	
	if (creatureOwner != nullptr)
	{
		creatureOwner->doWarmupChecks( *entry.m_command, entry.m_targetId, entry.m_params, m_status, m_statusDetail );
	
		// if everything is okay, start the warmup trigger
			
		if ( m_status == Command::CEC_Success )
		{
			FATAL( entry.m_command == 0, ( "entry had a nullptr command\n" ) );
		
			std::vector<float> timeValues;
			timeValues.push_back( entry.m_command->m_warmTime );
			timeValues.push_back( entry.m_command->m_execTime );
			timeValues.push_back( getCooldownTime(*entry.m_command) );
			timeValues.push_back( entry.m_command->m_coolTime2 );
		
			ScriptParams params;
		
			std::string entryParams( Unicode::wideToNarrow( entry.m_params ) );
			
			params.addParam( entry.m_command->m_commandName.c_str() );
			params.addParam( (int)entry.m_command->m_coolGroup );
			params.addParam( (int)entry.m_command->m_coolGroup2 );
			params.addParam( timeValues );
			params.addParam( entryParams.c_str() );
			params.addParam( entry.m_targetId );
			
			result = SCRIPT_OVERRIDE != creatureOwner->getScriptObject()->trigAllScripts( Scripting::TRIG_BEGIN_WARMUP, params );
		}
	}

	return result;
}

// ----------------------------------------------------------------------

uint32 CommandQueue::getCurrentCommand() const
{
	if ( m_queue.empty() )
	{
		return 0;
	}
	
	const CommandQueueEntry &entry = *(m_queue.begin());
	
	if ( entry.m_command == 0 )
	{
		WARNING( true, ( "CommandQueue::getCurrentCommand(): command was nullptr!\n" ) );
		return 0;
	}

	return Crc::calculate( entry.m_command->m_commandName.c_str() );
}

// ----------------------------------------------------------------------

void CommandQueue::switchState()
{
	if ( m_queue.empty() )
	{
		WARNING( true, ( "CommandQueue::switchState(): queue is empty!\n" ) );
		return;
	}

	const CommandQueueEntry &entry = *(m_queue.begin());

	if ( entry.m_command == 0 )
	{
		WARNING( true, ( "CommandQueue::switchState(): command was nullptr!\n" ) );
		return;
	}

#if defined(COMMAND_QUEUE_ENTRY_LOGGING)
	CommandQueueEntry const entryCopy = entry;
#endif

	m_eventStartTime = s_currentTime;
	
	const unsigned int savedQueueSize = m_queue.size();
	switch ( m_state.get() )
	{
		case State_Waiting:
			// set timing info for execute and cooldown timers
			m_commandTimes.set( TimerClass_Execute, entry.m_command->m_execTime );
			m_commandTimes.set( TimerClass_Cooldown, getCooldownTime(*entry.m_command) );
			m_commandTimes.set( TimerClass_Cooldown2, entry.m_command->m_coolTime2 );

			DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): cooldown timer setting id=%lu time=%f id2=%lu time2=%f\n", entry.m_command->m_coolGroup, getCooldownTime(*entry.m_command),
				entry.m_command->m_coolGroup2, entry.m_command->m_coolTime2) );
			
			if ( entry.m_command->m_warmTime < FLT_EPSILON )
			{
				if( entry.m_command->m_execTime < FLT_EPSILON )
				{
					// skipping both warmup and execute if both the timers are 0					
					m_nextEventTime = s_currentTime;

					doExecute(entry );

					bool entryIsTopOfQueue = true;
					
					if (!m_queue.empty())
					{
						entryIsTopOfQueue = &(*(m_queue.begin())) == &entry;

#if defined(COMMAND_QUEUE_ENTRY_LOGGING)
						if (!entryIsTopOfQueue && m_queue.size() == savedQueueSize)
						{
							REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_INVALID: Entry does not match top of queue!\n"));

							Object const * const target = NetworkIdManager::getObjectById(entryCopy.m_targetId);
							bool const targetIsAuthoritative = target && target->isAuthoritative();

							REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_COMMAND: commandName=[%s] commandHash=[%lu] owner=[%s][%s][%s] target=[%s][%s][%s] sequenceId=[%lu] params=[%s] clearable=[%s] priority=[%d]\n",
								entryCopy.m_command ? entryCopy.m_command->m_commandName.c_str() : "nullptr",
								entryCopy.m_command ? entryCopy.m_command->m_commandHash : 0,
								NameManager::getInstance().getPlayerName(getOwner().getNetworkId()).c_str(),
								getOwner().getNetworkId().getValueString().c_str(),
								getOwner().isAuthoritative() ? "auth" : "proxy",
								NameManager::getInstance().getPlayerName(entryCopy.m_targetId).c_str(),
								entryCopy.m_targetId.getValueString().c_str(),
								targetIsAuthoritative ? "auth" : "proxy",
								entryCopy.m_sequenceId,
								Unicode::wideToNarrow(entryCopy.m_params).c_str(),
								entryCopy.m_clearable ? "true" : "false",
								entryCopy.m_priority)
								);

							REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_QUEUE: state=State_Waiting queueSize=%d\n", m_queue.size()));

							EntryList::const_iterator itEntry = m_queue.begin();

							for (; itEntry != m_queue.end(); ++itEntry)
							{
								REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_QUEUE: commandName=[%s]\n", itEntry->m_command ? itEntry->m_command->m_commandName.c_str() : "nullptr"));
							}
						}
#endif
					}
					else
						entryIsTopOfQueue = false;

					//Check the size of the queue.  If it has shrunk then it means the entry was popped off the queue by cancelCurrentCommand,
					//most likely because the command failed.  If this is the case, do not perform the following actions.
					if (m_queue.size() == savedQueueSize && entryIsTopOfQueue)
					{
						m_state = State_Waiting;

						// set the cooldown timer for this command group			
						m_cooldowns.set( entry.m_command->m_coolGroup, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown ] ) );
						m_cooldowns.set( entry.m_command->m_coolGroup2, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown2 ] ) );

						// if the cool down value is large enough, persist it across
						// server restarts and character unloading and reloading from the DB
						if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
						{
							persistCooldown( entry.m_command->m_coolGroup, static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) );
						}

						if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
						{
							persistCooldown( entry.m_command->m_coolGroup2, static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) );
						}

						DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Waiting->Waiting all the way around : m_nextEventTime=%f m_commandTime=%f m_execTime=%f\n",
							m_nextEventTime.get(),
							m_commandTimes[ TimerClass_Execute ],
							entry.m_command->m_execTime ) );
						handleEntryRemoved(entry, true);
						m_queue.pop();
					}
				}
				else
				{
				
					// here we skip the warmup state if the default warmup timer is 0
					m_state = State_Execute;
					
					m_nextEventTime = s_currentTime + entry.m_command->m_execTime;
					
					m_commandTimes.set( TimerClass_Warmup, 0.f );
					
					DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Waiting->Execute\n" ) );
				}
			}
			else
			{
				// otherwise we set up the warmup time
				m_state = State_Warmup;

				// set the command time state
				// m_commandTimes[ TimerClass_Warmup ] = entry.m_command->m_warmTime;
				m_commandTimes.set( TimerClass_Warmup, entry.m_command->m_warmTime );
				
				// update the next event time value
				
				m_nextEventTime = s_currentTime + entry.m_command->m_warmTime;
					
				DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Waiting->Warmup\n" ) );
			}
			
			break;

		case State_Warmup:
		
			if(entry.m_command->m_execTime < FLT_EPSILON)
			{
				m_nextEventTime = s_currentTime;

				doExecute(entry );

				bool entryIsTopOfQueue = true;

				if (!m_queue.empty())
				{
					entryIsTopOfQueue = &(*(m_queue.begin())) == &entry;

#if defined(COMMAND_QUEUE_ENTRY_LOGGING)
					if (!entryIsTopOfQueue && m_queue.size() == savedQueueSize)
					{
						REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_INVALID: Entry does not match top of queue!\n"));

						Object const * const target = NetworkIdManager::getObjectById(entryCopy.m_targetId);
						bool const targetIsAuthoritative = target && target->isAuthoritative();

						REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_COMMAND: commandName=[%s] commandHash=[%lu] owner=[%s][%s][%s] target=[%s][%s][%s] sequenceId=[%lu] params=[%s] clearable=[%s] priority=[%d]\n",
							entryCopy.m_command ? entryCopy.m_command->m_commandName.c_str() : "nullptr",
							entryCopy.m_command ? entryCopy.m_command->m_commandHash : 0,
							NameManager::getInstance().getPlayerName(getOwner().getNetworkId()).c_str(),
							getOwner().getNetworkId().getValueString().c_str(),
							getOwner().isAuthoritative() ? "auth" : "proxy",
							NameManager::getInstance().getPlayerName(entryCopy.m_targetId).c_str(),
							entryCopy.m_targetId.getValueString().c_str(),
							targetIsAuthoritative ? "auth" : "proxy",
							entryCopy.m_sequenceId,
							Unicode::wideToNarrow(entryCopy.m_params).c_str(),
							entryCopy.m_clearable ? "true" : "false",
							entryCopy.m_priority)
							);

						REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_QUEUE: state=State_Warmup queueSize=%d\n", m_queue.size()));

						EntryList::const_iterator itEntry = m_queue.begin();

						for (; itEntry != m_queue.end(); ++itEntry)
						{
							REPORT_LOG(true, ("CommandQueue::switchState():QUEUE_ENTRY_QUEUE: commandName=[%s]\n", itEntry->m_command ? itEntry->m_command->m_commandName.c_str() : "nullptr"));
						}
					}
#endif
				}
				else
					entryIsTopOfQueue = false;

				//Check the size of the queue.  If it has shrunk then it means the entry was popped off the queue by cancelCurrentCommand,
				//most likely because the command failed.  If this is the case, do not perform the following actions.
				if (m_queue.size() == savedQueueSize && entryIsTopOfQueue)
				{
					m_state = State_Waiting;

					// set the cooldown timer for this command group			
					m_cooldowns.set( entry.m_command->m_coolGroup, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown ] ) );
					m_cooldowns.set( entry.m_command->m_coolGroup2, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown2 ] ) );
					
					// if the cool down value is large enough, persist it across
					// server restarts and character unloading and reloading from the DB
					if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
					{
						persistCooldown( entry.m_command->m_coolGroup, static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) );
					}

					if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
					{
						persistCooldown( entry.m_command->m_coolGroup2, static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) );
					}

					DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Warmup->Waiting m_nextEventTime=%f m_commandTime=%f m_execTime=%f\n",
						m_nextEventTime.get(),
						m_commandTimes[ TimerClass_Execute ],
						entry.m_command->m_execTime ) );
				}
			}
			else
			{			
				m_state = State_Execute;
				
				m_nextEventTime = s_currentTime + entry.m_command->m_execTime;
				
				DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Warmup->Execute m_nextEventTime=%f m_commandTime=%f m_execTime=%f\n",
					m_nextEventTime.get(),
					m_commandTimes[ TimerClass_Execute ],
					entry.m_command->m_execTime ) );
					
			}
			break;

	case State_Execute:

			m_state = State_Waiting;
			
			// set the cooldown timer for this command group			
			m_cooldowns.set( entry.m_command->m_coolGroup, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown ] ) );
			m_cooldowns.set( entry.m_command->m_coolGroup2, std::make_pair( s_currentTime, s_currentTime + m_commandTimes[ TimerClass_Cooldown2 ] ) );
			
			DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::switchState(): Execute->%s\n", getStateAsString( (State)m_state.get() )  ) );

			// if the cool down value is large enough, persist it across
			// server restarts and character unloading and reloading from the DB
			if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
			{
				persistCooldown( entry.m_command->m_coolGroup, static_cast<int>( m_commandTimes[ TimerClass_Cooldown ] ) );
			}

			if ( static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) >= ConfigServerGame::getCoolDownPersistThresholdSeconds() )
			{
				persistCooldown( entry.m_command->m_coolGroup2, static_cast<int>( m_commandTimes[ TimerClass_Cooldown2 ] ) );
			}

			break;
			
		default:
			WARNING( true, ( "CommandQueue::switchState(): queue is in an invalid state!\n" ) );
			break;
	}

}

// ----------------------------------------------------------------------

void CommandQueue::clearPendingCombatCommands()
{
	DEBUG_REPORT_LOG( cs_debug, ( "%s: attempting to cancel pending commands (queueSize=%d)\n", Unicode::wideToNarrow( getServerOwner().getObjectName() ).c_str(), m_queue.size() ) );
	
	for ( EntryList::iterator it = m_queue.begin(); it != m_queue.end(); )
	{
		EntryList::iterator j = it++;
		
		const CommandQueueEntry &entry = *j;

		// if ( command is combat command )
		// {
		//    if ( ( command is at the top and is not executing ) or ( command is not at the top )
		//    {
		//       remove command		
		//    }
		// }

		if ( entry.m_command->m_addToCombatQueue && ( j != m_queue.begin() || m_state.get() != State_Execute ) )
		{

			handleEntryRemoved(entry );
			
			if ( j == m_queue.begin() )
			{
				cancelCurrentCommand();
			}
			else
			{
				m_queue.erase( j );
			}

		}

	}
	
}

// ----------------------------------------------------------------------

void CommandQueue::persistCooldown(uint32 cooldownGroupCrc, int cooldownTimeSeconds)
{
	if (cooldownTimeSeconds <= 0)
		return;

	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();
	if (!creatureOwner)
		return;

	int const currentGameTime = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());

	char buffer[256];

	snprintf(buffer, sizeof(buffer)-1, "commandCooldown.beginTime.%lu", cooldownGroupCrc);
	buffer[sizeof(buffer)-1] = '\0';
	std::string const coolDownBeginTime = buffer;

	snprintf(buffer, sizeof(buffer)-1, "commandCooldown.endTime.%lu", cooldownGroupCrc);
	buffer[sizeof(buffer)-1] = '\0';
	std::string const coolDownEndTime = buffer;

	IGNORE_RETURN(creatureOwner->setObjVarItem(coolDownBeginTime, currentGameTime));
	IGNORE_RETURN(creatureOwner->setObjVarItem(coolDownEndTime, (currentGameTime + cooldownTimeSeconds)));
}

// ----------------------------------------------------------------------

void CommandQueue::depersistCooldown()
{
	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();
	if (!creatureOwner)
		return;

	DynamicVariableList const & objvars = creatureOwner->getObjVars();
	std::map<uint32, std::pair<int, int> > commandCooldownTimers;
	for (DynamicVariableList::MapType::const_iterator iterObjVar = objvars.begin(); iterObjVar != objvars.end(); ++iterObjVar)
	{
		if (iterObjVar->first.find("commandCooldown.beginTime.") == 0)
		{
			uint32 cooldownGroupCrc = 0;
			if (1 == sscanf(iterObjVar->first.c_str(), "commandCooldown.beginTime.%lu", &cooldownGroupCrc))
			{
				int beginTime = 0;
				if (iterObjVar->second.get(beginTime))
				{
					std::map<uint32, std::pair<int, int> >::iterator iterFind = commandCooldownTimers.find(cooldownGroupCrc);
					if (iterFind == commandCooldownTimers.end())
					{
						commandCooldownTimers[cooldownGroupCrc] = std::make_pair(beginTime, 0);
					}
					else
					{
						iterFind->second.first = beginTime;
					}
				}
			}
		}
		else if (iterObjVar->first.find("commandCooldown.endTime.") == 0)
		{
			uint32 cooldownGroupCrc = 0;
			if (1 == sscanf(iterObjVar->first.c_str(), "commandCooldown.endTime.%lu", &cooldownGroupCrc))
			{
				int endTime = 0;
				if (iterObjVar->second.get(endTime))
				{
					std::map<uint32, std::pair<int, int> >::iterator iterFind = commandCooldownTimers.find(cooldownGroupCrc);
					if (iterFind == commandCooldownTimers.end())
					{
						commandCooldownTimers[cooldownGroupCrc] = std::make_pair(0, endTime);
					}
					else
					{
						iterFind->second.second = endTime;
					}
				}
			}
		}
	}

	char buffer[256];
	int const currentGameTime = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());
	double const currentTime = Clock::getCurrentTime();
	for (std::map<uint32, std::pair<int, int> >::const_iterator iterCooldown = commandCooldownTimers.begin(); iterCooldown != commandCooldownTimers.end(); ++iterCooldown)
	{
		// update the command queue with any unexpired cooldowns
		if ((iterCooldown->second.first > 0) && (iterCooldown->second.second > 0) && (iterCooldown->second.first <= currentGameTime) && (currentGameTime < iterCooldown->second.second))
		{
			m_cooldowns.set(iterCooldown->first, std::make_pair(currentTime - static_cast<double>(currentGameTime - iterCooldown->second.first), currentTime + static_cast<double>(iterCooldown->second.second - currentGameTime)));
		}
		else
		{
			// cleanup the objvars for any expired/invalid cooldowns
			snprintf(buffer, sizeof(buffer)-1, "commandCooldown.beginTime.%lu", iterCooldown->first);
			buffer[sizeof(buffer)-1] = '\0';
			creatureOwner->removeObjVarItem(buffer);

			snprintf(buffer, sizeof(buffer)-1, "commandCooldown.endTime.%lu", iterCooldown->first);
			buffer[sizeof(buffer)-1] = '\0';
			creatureOwner->removeObjVarItem(buffer);
		}
	}
}

// ----------------------------------------------------------------------

bool CommandQueue::hasCommandFromGroup(uint32 groupHash) const
{
	for (EntryList::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i)
		if ((*i).m_command->m_commandGroup == groupHash)
			return true;
	return false;
}


// ----------------------------------------------------------------------

void CommandQueue::clearCommandsFromGroup(uint32 groupHash, bool force)
{
	
	// iterate over each command in the queue
	for ( EntryList::iterator it = m_queue.begin(); it != m_queue.end(); /* nil */ )
	{
		EntryList::iterator j = it++;
		
		const CommandQueueEntry &entry = *j;
		
		if ( entry.m_command->m_commandGroup == groupHash && ( force || entry.m_clearable ) )
		{
			// decrement the combat entry count if this is a combat command
			// and update client
			handleEntryRemoved(entry );

			// if this is the top of the queue, we need to put the queue into a waiting state
			if ( j == m_queue.begin() )
			{
				cancelCurrentCommand();
			}
			else
			{
				// remove item from queue
				m_queue.erase( j );
			}

		}
	}
			
}

// ----------------------------------------------------------------------

void CommandQueue::handleEntryRemoved(CommandQueueEntry const &entry, bool notifyClient)
{
	DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::handleEntryRemoved(%s, %d)\n", entry.m_command->m_commandName.c_str(), notifyClient ? 1 : 0 ) );

	ServerObject const & serverOwner = getServerOwner();
	
	if ( entry.m_command->m_addToCombatQueue && ( m_combatCount.get() > 0 ) )
	{
		m_combatCount = m_combatCount.get() - 1;
	}


	// If we have a sequenceId, then the owner's client is interested in
	// hearing about the removal of that command.
	if (serverOwner.isAuthoritative() && notifyClient)
	{
		notifyClientOfCommandRemoval(entry.m_sequenceId, 0.0f, m_status, m_statusDetail);
	}
	
}

// ----------------------------------------------------------------------

void CommandQueue::notifyClientOfCommandRemoval(uint32 sequenceId, float waitTime, Command::ErrorCode status, int statusDetail)
{
	DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::notifyClientOfCommandRemoval(%d, %f, %d, %d)\n", static_cast<int>(sequenceId), waitTime, status, statusDetail ) );

	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();

	if (   (creatureOwner != nullptr)
	    && creatureOwner->getClient()
	    && (sequenceId != 0))
	{
		WARNING(status < 0 || status >= Command::CEC_Max, ("CommandQueue::notifyClientOfCommandRemoval received invalid status %d", static_cast<int>(status)));

		creatureOwner->getController()->appendMessage(
			static_cast<int>(CM_commandQueueRemove),
			0.0f,
			new MessageQueueCommandQueueRemove(sequenceId, waitTime, static_cast<int>(status), statusDetail),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

// ----------------------------------------------------------------------

const char *CommandQueue::getStateAsString( State state )
{
	static const char * states[] =
	{
		"waiting",
		"warmup",
		"execute",
		"?unknown?"
	};

	if ( state > State_MAX )
	{
		state = State_MAX;
	}

	return states[ state ];
}

// ----------------------------------------------------------------------

bool CommandQueue::setCommandTimerValue(TimerClass timerClass, float newValue )
{
	FATAL( timerClass >= (int)m_commandTimes.size(), ( "CommandQueue::setCommandTimerValue(): invalid timer class %d\n", timerClass ) );
	
	FATAL( fabsf( newValue ) > cs_secondsPerDay, ( "tried to set a command timer outside allowed range. (value=%f)\n", newValue ) );
	
	if ( m_state.get() == State_Waiting )
	{
		return false;
	}
	
	if ( ( timerClass == TimerClass_Warmup && m_state.get() == State_Warmup ) || ( timerClass == TimerClass_Execute && m_state.get() == State_Execute ) )
	{
		m_nextEventTime = m_eventStartTime.get() + newValue;
	}

	m_commandTimes.set( timerClass, newValue );
	
	DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::setCommandTimerValue() timerClass=%d m_nextEventTime=%f newValue=%f\n", timerClass, m_nextEventTime.get(), newValue ) );
	
	updateClient(timerClass );
	
	return true;
}

// ----------------------------------------------------------------------

void CommandQueue::cancelCurrentCommand()
{
	DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::cancelCurrentCommand()\n" ) );

	WARNING( m_queue.empty(), ( "queue was empty.\n " ) );

	if ( !m_queue.empty() )
	{
		const CommandQueueEntry &entry = *(m_queue.begin());
		m_nextEventTime = 0.f;
		m_status = Command::CEC_Cancelled;
		m_state = State_Waiting;
		handleEntryRemoved(entry, true);
		m_queue.pop();
	}
}

// ----------------------------------------------------------------------

CommandQueue::State CommandQueue::getCurrentCommandState() const
{
	return (State)m_state.get();
}

// ----------------------------------------------------------------------

void CommandQueue::addToPackage(Archive::AutoDeltaByteStream &bs)
{
	bs.addVariable(m_queue);
	bs.addVariable(m_state);
	bs.addVariable(m_nextEventTime);
	bs.addVariable(m_combatCount);
	bs.addVariable(m_eventStartTime);
	bs.addVariable(m_cooldowns);
	bs.addVariable(m_commandTimes);
}

// ----------------------------------------------------------------------

float CommandQueue::getCooldownTimeLeft( uint32 cooldownId ) const
{
	CooldownMapType::const_iterator it = m_cooldowns.find( cooldownId );

	float result = 0.f;
		
	if ( it != m_cooldowns.end() )
	{
		result = static_cast< float > ( (*it).second.second - Clock::getCurrentTime() );
	}
	
	return result;
}

// ----------------------------------------------------------------------

float CommandQueue::getCooldownTimeLeft( const std::string &name ) const
{
	return getCooldownTimeLeft( Crc::normalizeAndCalculate( name.c_str() ) );
}

//-----------------------------------------------------------------------

void CommandQueue::executeCommand(Command const &command, NetworkId const &targetId, Unicode::String const &params, Command::ErrorCode &status, int &statusDetail, bool commandIsFromCommandQueue)
{
	DEBUG_REPORT_LOG( cs_debug, ( "%f: CommandQueue::executeCommand(%s, %s, %s, %d, %d)\n",
		Clock::getCurrentTime(),
		command.m_commandName.c_str(),
		targetId.getValueString().c_str(),
		Unicode::wideToNarrow(params).c_str(),
		status,
		statusDetail));

	status = Command::CEC_Success;

	static bool s_tradeAllowedCommandsInit = false;
	typedef std::vector<uint32> Uint32Vector;
	static Uint32Vector s_tradeAllowedCommands;

	if (!s_tradeAllowedCommandsInit)
	{
		s_tradeAllowedCommandsInit = true;

		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("addFriend"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("addIgnore"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("closeContainer"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("combatSpam"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("find"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("gc"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("getAttributes"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("getAttributesBatch"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("getFriendList"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("getIgnoreList"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("groupChat"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("groupChat"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("gsay"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("gtell"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("guildsay"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("kneel"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("maskscent"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("meditate"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("newbiehelper"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("openContainer"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("prose"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("removeFriend"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("removeIgnore"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("requestBiography"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("requestCharacterMatch"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("requestDraftSlots"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("requestManfSchematicSlots"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("requestResourceWeights"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("setMatchMakingCharacterId"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("setMatchMakingPersonalId"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("setMood"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("setMoodInternal"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("setSpokenLanguage"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("sitServer"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("social"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("socialInternal"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("spatialChat"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("spatialChatInternal"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("stand"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("synchronizedUiListen"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("synchronizedUiStopListening"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("target"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("tip"));
		s_tradeAllowedCommands.push_back (Crc::normalizeAndCalculate("waypoint"));

		s_tradeAllowedCommands.push_back (CommandTable::getNullCommand().m_commandHash);

		std::sort (s_tradeAllowedCommands.begin (), s_tradeAllowedCommands.end ());
	}

	CreatureObject * const creatureOwner = getServerOwner().asCreatureObject();

	if (creatureOwner != nullptr)
	{
		CreatureController * const creatureController = creatureOwner->getCreatureController();
		if (creatureController && creatureController->getSecureTrade())
		{
			if (!std::binary_search (s_tradeAllowedCommands.begin (), s_tradeAllowedCommands.end (), command.m_commandHash))
			{
				LOG ("Command", ("CreatureObject [%s] trade canceled by [%s] command", creatureOwner->getNetworkId ().getValueString ().c_str (), command.m_commandName.c_str ()));

				creatureController->getSecureTrade()->cancelTrade(*creatureOwner);
			}
		}

		// posture and ability verification - don't execute the command
		// if we're in a posture for which the command is not valid, or
		// we do not have the ability required for the command (if any)
		creatureOwner->doWarmupChecks( command, targetId, params, status, statusDetail );
		
		if (status == Command::CEC_Success && command.m_godLevel > 0)
		{
			Client const *client = creatureOwner->getClient();
			if (!client)
			{
				status = Command::CEC_GodLevel;
				LOGU("CustomerService", ("Avatar:%s tried to execute command %s >%s with no client", PlayerObject::getAccountDescription(creatureOwner).c_str(), command.m_commandName.c_str(), targetId.getValueString().c_str()), params);
			}
			else if (client->getGodLevel() < command.m_godLevel)
			{
				status = Command::CEC_GodLevel;
				LOGU("CustomerService", ("Avatar:%s doesn't have adequate level for command %s >%s", PlayerObject::getAccountDescription(creatureOwner).c_str(), command.m_commandName.c_str(), targetId.getValueString().c_str()), params);
			}
			else
			{
				LOGU("CustomerService", ("Avatar:%s has executed command %s >%s", PlayerObject::getAccountDescription(creatureOwner).c_str(), command.m_commandName.c_str(), targetId.getValueString().c_str()), params);
			}
		}
	}

	TangibleObject * const tangibleOwner = getServerOwner().asTangibleObject();

	if (tangibleOwner != nullptr)
	{
		// Really execute the command - swap targets if necessary, and call
		// forceExecuteCommand (which will handle messaging if not authoritative)
		if (status == Command::CEC_Success)
		{
			if (!command.m_callOnTarget)
			{
				DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::executeCommand(): tangibleOwner->forceExecuteCommand(%s, %s, %s, %d)\n",
					command.m_commandName.c_str(),
					targetId.getValueString().c_str(),
					Unicode::wideToNarrow(params).c_str(),
					status));

				tangibleOwner->forceExecuteCommand(command, targetId, params, status, commandIsFromCommandQueue);
			}

			else
			{
				TangibleObject * const tangibleTarget = TangibleObject::getTangibleObject(targetId);

				if (tangibleTarget)
				{
					DEBUG_REPORT_LOG( cs_debug, ( "CommandQueue::executeCommand(): tangibleTarget->forceExecuteCommand(%s, %s, %s, %d)\n",
						command.m_commandName.c_str(),
						tangibleOwner->getNetworkId().getValueString().c_str(),
						Unicode::wideToNarrow(params).c_str(),
						status));

					tangibleTarget->forceExecuteCommand(command, tangibleOwner->getNetworkId(), params, status, commandIsFromCommandQueue);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
CommandQueue * CommandQueue::getCommandQueue(Object & object)
{
	Property * const property = object.getProperty(getClassPropertyId());

	return (property != nullptr) ? (static_cast<CommandQueue *>(property)) : nullptr;
}

// ----------------------------------------------------------------------
ServerObject & CommandQueue::getServerOwner()
{
	return *getOwner().asServerObject();
}

// ----------------------------------------------------------------------

ServerObject const & CommandQueue::getServerOwner() const
{
	return *getOwner().asServerObject();
}

// ----------------------------------------------------------------------

float CommandQueue::getCooldownTime(Command const &command)
{
	if(command.isPrimaryCommand())
	{
		CreatureObject * const creature = getServerOwner().asCreatureObject();
		if(creature)
		{
			WeaponObject *weapon = creature->getCurrentWeapon();
			if(weapon)
				return weapon->getAttackTime();
		}
	}
	return command.m_coolTime;
}

// ----------------------------------------------------------------------

void CommandQueue::resetCooldowns()
{
	m_commandTimes.set( TimerClass_Cooldown, 0.0f );
	updateClient( TimerClass_Cooldown );
	for (CooldownMapType::const_iterator iterCooldown = m_cooldowns.begin(); iterCooldown != m_cooldowns.end(); ++iterCooldown)
	{
		//has not expired
		if((*iterCooldown).second.second > (Clock::getCurrentTime() + FLT_EPSILON))
		{
			//Reset the end of the cooldown to be right now
			m_cooldowns.set( (*iterCooldown).first, std::make_pair( (*iterCooldown).second.first, Clock::getCurrentTime() ) );

			//notify client
			MessageQueueCommandTimer *msg = new MessageQueueCommandTimer(
				0,
				(*iterCooldown).first,
				-1,
				0 );
			
			msg->setCurrentTime(MessageQueueCommandTimer::F_cooldown, 1.0f);
			msg->setMaxTime    (MessageQueueCommandTimer::F_cooldown, 1.0f);

			CreatureObject * const creature = getServerOwner().asCreatureObject();
			if(creature)
			{			
				creature->getController()->appendMessage(
					static_cast< int >( CM_commandTimer ),
					0.0f,
					msg,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}

		}
	}
}

// ----------------------------------------------------------------------

void CommandQueue::spew(std::string * output)
{
	CreatureObject * const creature = getServerOwner().asCreatureObject();
	if(creature)
	{
		FormattedString<1024> fsOutput;
		char const * fsSprintfOutput;
		double const currentTime = Clock::getCurrentTime();

		fsSprintfOutput = fsOutput.sprintf("%s current time [%.10f], m_queue size [%d], m_cooldowns size [%d], m_combatCount [%u]", creature->getNetworkId().getValueString().c_str(), currentTime, m_queue.size(), m_cooldowns.size(), m_combatCount.get());
		if (output)
		{
			*output += fsSprintfOutput;
			*output += "\n";
		}
		else
		{
			LOG("CommandQueueSpew", ("%s", fsSprintfOutput));
		}

		for ( EntryList::iterator it = m_queue.begin(); it != m_queue.end(); )
		{
			EntryList::iterator j = it++;
			
			const CommandQueueEntry &entry = *j;

			fsSprintfOutput = fsOutput.sprintf("%s commandName [%s] hash[%lu] cd1[%lu] cd2[%lu]",
				creature->getNetworkId().getValueString().c_str(),
				entry.m_command->m_commandName.c_str(),
				entry.m_command->m_commandHash,
				entry.m_command->m_coolGroup,
				entry.m_command->m_coolGroup2);
			if (output)
			{
				*output += fsSprintfOutput;
				*output += "\n";
			}
			else
			{
				LOG("CommandQueueSpew", ("%s", fsSprintfOutput));
			}
		}

		for (CooldownMapType::const_iterator iterCooldown = m_cooldowns.begin(); iterCooldown != m_cooldowns.end(); ++iterCooldown)
		{
			fsSprintfOutput = fsOutput.sprintf("%s cooldown key [%lu] value1[%.10f] value2[%.10f] duration[%.10f] remaining[%.10f]",
				creature->getNetworkId().getValueString().c_str(),
				iterCooldown->first,
				iterCooldown->second.first,
				iterCooldown->second.second,
				iterCooldown->second.second - iterCooldown->second.first,
				iterCooldown->second.second - currentTime);
			if (output)
			{
				*output += fsSprintfOutput;
				*output += "\n";
			}
			else
			{
				LOG("CommandQueueSpew", ("%s", fsSprintfOutput));
			}
		}

		fsSprintfOutput = fsOutput.sprintf("%s m_commandTimes Warmup[%.10f] Execute[%.10f] Cooldown[%.10f] Cooldown2[%.10f]",
			creature->getNetworkId().getValueString().c_str(),
			m_commandTimes[TimerClass_Warmup],
			m_commandTimes[TimerClass_Execute],
			m_commandTimes[TimerClass_Cooldown],
			m_commandTimes[TimerClass_Cooldown2]);
		if (output)
		{
			*output += fsSprintfOutput;
			*output += "\n";
		}
		else
		{
			LOG("CommandQueueSpew", ("%s", fsSprintfOutput));
		}
	}
}

// ======================================================================
