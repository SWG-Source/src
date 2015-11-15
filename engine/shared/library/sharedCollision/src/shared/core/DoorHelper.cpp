// ======================================================================
//
// DoorHelper.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/DoorHelper.h"

#include "sharedCollision/DoorInfo.h"

#include "sharedObject/World.h"

// ======================================================================

DoorHelper::DoorHelper( DoorInfo const & info ) 
: m_pos			( 0.0f ),
  m_openTime	( info.m_openTime ),
  m_closeTime	( info.m_closeTime ),
  m_hitFrame	( -100 ),
  m_opening		( false ),
  m_open		( false ),
  m_event       ( DHE_invalid )
{
}

void DoorHelper::update ( float time )
{
	UNREF(time);

	// @todo change this to time-based hitting instead of frame-based hitting
	int const framesSinceHit = World::getFrameNumber() - m_hitFrame;
	if ((framesSinceHit >= - 1) && (framesSinceHit <= 1))
	{
		m_opening = true;
	}
	else
	{
		m_opening = false;
	}
}

void DoorHelper::alter ( float time )
{
	float startPos = m_pos;

	if(m_opening)
	{
		m_pos += time * (1.0f / m_openTime);
	}
	else
	{
		m_pos -= time * (1.0f / m_closeTime);
	}

	float endPos = m_pos;

	// ----------

	if((startPos == 0.0f) && (endPos > 0.0f))
	{
		m_event = DHE_openBegin;
	}
	else if((startPos < 1.0f) && (endPos >= 1.0f))
	{
		m_event = DHE_openEnd;
	}
	else if((startPos == 1.0f) && (endPos < 1.0f))
	{
		m_event = DHE_closeBegin;
	}
	else if((startPos > 0.0f) && (endPos <= 0.0f))
	{
		m_event = DHE_closeEnd;
	}
	else
	{
		m_event = DHE_invalid;
	}

	// ----------

	if(m_pos > 1.0f) 
	{
		m_pos = 1.0f;
	}

	if(m_pos < 0.0f) 
	{
		m_open = false;
		m_pos = 0.0f;
	}
}

// ----------

void DoorHelper::hit ( void )
{
	m_open = true;
	m_hitFrame = World::getFrameNumber();
}

// ----------

float DoorHelper::getPos ( void ) const
{
	return m_pos;
}

bool DoorHelper::isOpen ( void ) const
{
	return m_open;
}

bool DoorHelper::isClosed ( void ) const
{
	return !m_open;
}

bool DoorHelper::isOpening ( void ) const
{
	return m_open && m_opening && (m_pos < 1.0f);
}

bool DoorHelper::isClosing ( void ) const
{
	return m_open && (!m_opening) && (m_pos > 0.0f);
}

DoorHelper::Event DoorHelper::getEvent ( void ) const
{
	return m_event;
}

