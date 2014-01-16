// ======================================================================
//
// SwgServerUniverse.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgServerUniverse_H
#define INCLUDED_SwgServerUniverse_H

#include "serverGame/ServerUniverse.h"

class JediManagerObject;


// ======================================================================

/**
* Singleton that manages Universe objects and functions that are global
* to the entire game universe.
*/
class SwgServerUniverse : public ServerUniverse
{
public:
	static void install();
	
public:
	JediManagerObject *  getJediManager     () const;
	void                 registerJediManager(JediManagerObject & jediManager);

protected:
	virtual void updateAndValidateData();

private:
	JediManagerObject * m_jediManager;

private:
	         SwgServerUniverse();
	virtual ~SwgServerUniverse();

	SwgServerUniverse(SwgServerUniverse const &);
	SwgServerUniverse & operator=(SwgServerUniverse const &);
};

// ======================================================================

inline JediManagerObject * SwgServerUniverse::getJediManager() const
{
	return m_jediManager;
}

// ----------------------------------------------------------------------

inline void SwgServerUniverse::registerJediManager(JediManagerObject & jediManager)
{
	m_jediManager = &jediManager;
}

// ======================================================================


#endif
