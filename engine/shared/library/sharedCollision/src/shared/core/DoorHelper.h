// ======================================================================
//
// DoorHelper.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DoorHelper_H
#define INCLUDED_DoorHelper_H

class DoorInfo;

// ----------------------------------------------------------------------

class DoorHelper
{
public:

	DoorHelper( DoorInfo const & info );

	// ----------

	void	update		( float time );
	void	alter		( float time );

	void	hit			( void );

	float	getPos		( void ) const;

	bool	isOpen		( void ) const;
	bool	isClosed	( void ) const;
	bool	isOpening	( void ) const;
	bool	isClosing	( void ) const;

	enum Event
	{
		DHE_openBegin = 0,
		DHE_openEnd = 1,
		DHE_closeBegin = 2,
		DHE_closeEnd = 3,
		DHE_invalid = -1,
	};

	Event   getEvent    ( void ) const;

protected:

	float m_pos;
	float m_openTime;
	float m_closeTime;
	int   m_hitFrame;
	bool  m_opening;
	bool  m_open;
	Event m_event;
};

// ======================================================================

#endif
