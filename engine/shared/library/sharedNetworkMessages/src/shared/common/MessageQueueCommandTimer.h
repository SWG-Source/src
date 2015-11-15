// MessageQueueCommandTimer.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.

#ifndef	_INCLUDED_MessageQueueCommandTimer_H
#define	_INCLUDED_MessageQueueCommandTimer_H

#include "sharedFoundation/MessageQueue.h"

class MessageQueueCommandTimer : public MessageQueue::Data
{
public:
	enum Flags
	{
		F_warmup,
		F_execute,
		F_cooldown,
		F_failed,
		F_failedRetry,
		F_cooldown2,
		F_MAX
	};

	MessageQueueCommandTimer( uint32 sequenceId, int cooldownGroup, int cooldownGroup2, uint32 commandNameCrc );

	/**
	 * @brief sets the current time for the given state
	 */
	void              setCurrentTime( Flags flag, float time );

	/**
	 * @brief gets the current time for the requested state
	 */
	float             getCurrentTime( Flags flag ) const;

	/**
	 * @brief gets the maximum time for the given state
	 */
	void              setMaxTime( Flags flag, float time );

	/**
	 * @brief sets the maximum time for the given state
	 */
	float             getMaxTime( Flags flag ) const;

	/**
	 * @brief sets the cooldown group for this command
	 */
	void              setCooldownGroup( int group );

	/**
	 * @brief gets the cooldown group for this command, if it has one.
	 * this method returns -1 if this command is not part of a cooldown group.
	 * NOTE: the -1 value is reserved for future use; all commands are part of a cooldown group.
	 */
	int               getCooldownGroup() const;

	/**
	 * @brief returns true if this command is part of a cooldown group.  as per
	 * the current design, this method should always return true.
	 */
	bool              hasCooldownGroup() const;

	/**
	 * @brief sets the cooldown group for this command
	 */
	void              setCooldownGroup2( int group );

	/**
	 * @brief gets the cooldown group 2 for this command, if it has one.
	 * this method returns -1 if this command is not part of a cooldown group 2.
	 * NOTE: unlike cooldown group 1, this value can certainly be -1.
	 */
	int               getCooldownGroup2() const;

	/**
	 * @brief returns true if this command is part of a cooldown group.
	 */
	bool              hasCooldownGroup2() const;

	/**
	 * @brief indicates that this message means the corresponding command failed
	 */
	void              setFailed();

	/**
	 * @brief indicates that this message means the corresponding command failed
	 */
	bool              getFailed() const;

	/**
	 * @brief indicates that this message means the corresponding command failed
	 */
	void              setFailedRetry();

	/**
	 * @brief indicates that this message means the corresponding command failed
	 */
	bool              getFailedRetry() const;

	/**
	 * @brief returns the flag bit field
	 */
	uint32            getFlags() const;

	/**
	 * @brief returns true if this packet contains time information
	 * for the requested state.
	 */
	bool              hasTime( Flags flag ) const;

	/**
	 * @brief sets the sequence id for this command
	 */
	void              setSequenceId( uint32 id );

	/**
	 * @brief gets the sequence id for this command
	 */
	uint32            getSequenceId() const;

	/**
	 * @brief sets the command name crc for this command
	 */
	void              setCommandNameCrc( uint32 commandNameCrc );

	/**
	 * @brief gets the command name crc for this command
	 */
	uint32            getCommandNameCrc() const;

	/**
	 * @brief returns the given flag as a bit value
	 */
	static uint32     toBitValue( Flags flag );

private:
	uint32                    m_commandNameCrc;
	uint32                    m_flags;
	uint32                    m_sequenceId;
	int                       m_cooldownGroup;
	int                       m_cooldownGroup2;
	std::pair< float, float > m_time[ F_MAX ];
};

#define NULL_COOLDOWN_GROUP -1

inline MessageQueueCommandTimer::MessageQueueCommandTimer( uint32 sequenceId, int cooldownGroup, int cooldownGroup2, uint32 commandNameCrc ) :
m_commandNameCrc( commandNameCrc ),
m_flags( 0 ),
m_sequenceId( sequenceId ),
m_cooldownGroup( cooldownGroup ),
m_cooldownGroup2( cooldownGroup2 )
{
	for ( int i = 0 ; i < F_MAX; ++i )
	{
		m_time[ i ] = std::pair<float,float>( 0.f, 0.f );
	}
}

inline bool MessageQueueCommandTimer::hasTime( Flags flag ) const
{
	return ( m_flags & toBitValue( flag ) ) != 0;
}

inline void MessageQueueCommandTimer::setCurrentTime( Flags flag, float time )
{
	FATAL( flag >= F_MAX, ( "invalid command timer id\n" ) );
	m_time[ (int)flag ].first = time;
	m_flags |= toBitValue( flag );
}

inline float MessageQueueCommandTimer::getCurrentTime( Flags flag ) const
{
	FATAL( flag >= F_MAX, ( "invalid command timer id\n" ) );
	return m_time[ (int)flag ].first;
}

inline void MessageQueueCommandTimer::setMaxTime( Flags flag, float time )
{
	FATAL( flag >= F_MAX, ( "invalid command timer id\n" ) );
	m_time[ (int)flag ].second = time;
	m_flags |= toBitValue( flag );
}

inline float MessageQueueCommandTimer::getMaxTime( Flags flag ) const
{
	FATAL( flag >= F_MAX, ( "invalid command timer id\n" ) );
	return m_time[ (int)flag ].second;
}

inline void MessageQueueCommandTimer::setCooldownGroup( int group )
{
	m_cooldownGroup = group;
}

inline bool MessageQueueCommandTimer::hasCooldownGroup() const
{
	return m_cooldownGroup != NULL_COOLDOWN_GROUP;
}

inline int MessageQueueCommandTimer::getCooldownGroup() const
{
	return m_cooldownGroup;
}

inline void MessageQueueCommandTimer::setCooldownGroup2( int group  )
{
	m_cooldownGroup2 = group;
}

inline int MessageQueueCommandTimer::getCooldownGroup2() const
{
	return m_cooldownGroup2;
}

inline bool MessageQueueCommandTimer::hasCooldownGroup2() const
{
	return m_cooldownGroup2 != NULL_COOLDOWN_GROUP;
}

inline uint32 MessageQueueCommandTimer::getFlags() const
{
	return m_flags;
}

inline void MessageQueueCommandTimer::setSequenceId( uint32 sequenceId )
{
	m_sequenceId = sequenceId;
}

inline uint32 MessageQueueCommandTimer::getSequenceId() const
{
	return m_sequenceId;
}

inline void MessageQueueCommandTimer::setCommandNameCrc( uint32 commandNameCrc )
{
	m_commandNameCrc = commandNameCrc;
}

inline uint32 MessageQueueCommandTimer::getCommandNameCrc() const
{
	return m_commandNameCrc;
}

inline uint32 MessageQueueCommandTimer::toBitValue( Flags flag )
{
	return 1 << (int)flag;
}

inline void MessageQueueCommandTimer::setFailed()
{
	m_flags |= toBitValue( F_failed );
}

inline bool MessageQueueCommandTimer::getFailed() const
{
	return ( m_flags & toBitValue( F_failed ) ) != 0;
}

inline void MessageQueueCommandTimer::setFailedRetry()
{
	m_flags |= toBitValue( F_failedRetry );
}

inline bool MessageQueueCommandTimer::getFailedRetry() const
{
	return ( m_flags & toBitValue( F_failedRetry ) ) != 0;
}

#endif // _INCLUDED_MessageQueueCommandTimer_H
