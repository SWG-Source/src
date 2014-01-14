// Location.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_Location_H
#define	_INCLUDED_Location_H

//-----------------------------------------------------------------------

#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/LocationArchive.h"

//-----------------------------------------------------------------------

// WARNING:  If you edit this class, you must also edit the persistence functions for it (SwgSnapshot.cpp)
class Location
{
public:
	Location();
	Location(const Vector & coordinates, const NetworkId & cell, unsigned int sceneIdCrc);
	Location & operator = (const Location & rhs);
	Location(const Location & source);
	~Location();

	bool operator != (const Location & rhs) const;
	bool operator < (const Location & rhs) const;

	const Vector &     getCoordinates  () const;
	const NetworkId &  getCell         () const;
	unsigned int       getSceneIdCrc   () const;
	const char * const getSceneId      () const;

	static const char * const getSceneNameByCrc  (unsigned int crc);
	static unsigned int       getCrcBySceneName  (const char * const name);
	static unsigned int       getCrcBySceneName  (const std::string & name);

	void               setCoordinates  (const Vector & coordinates);
	void               setCell         (const NetworkId & cell);
	void               setSceneIdCrc   (unsigned int crc);
	void               setSceneId      (const char * const sceneId);

private:
	friend void Archive::get(Archive::ReadIterator &, Location &);
	Vector          m_coordinates;
	NetworkId       m_cell;
	unsigned int    m_sceneIdCrc;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Location_H
