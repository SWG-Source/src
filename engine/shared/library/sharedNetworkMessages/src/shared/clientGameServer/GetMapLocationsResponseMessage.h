// ======================================================================
//
// GetMapLocationsResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetMapLocationsResponseMessage_H
#define	_GetMapLocationsResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"

//-----------------------------------------------------------------------

class GetMapLocationsResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef std::vector<MapLocation> MapLocationVector;

	GetMapLocationsResponseMessage          (const std::string &planetName, const MapLocationVector & locStatic, const MapLocationVector & locDynamic, const MapLocationVector & locPersist, int versionStatic, int versionDynamic, int versionPersist);
	explicit GetMapLocationsResponseMessage (Archive::ReadIterator &source);

	~GetMapLocationsResponseMessage();

public: // methods

	const std::string &          getPlanetName          ()  const;
	const MapLocationVector &    getMapLocationsStatic  ()  const;
	const MapLocationVector &    getMapLocationsDynamic ()  const;
	const MapLocationVector &    getMapLocationsPersist ()  const;

	int                          getVersionStatic       ()  const;
	int                          getVersionDynamic      ()  const;
	int                          getVersionPersist      ()  const;

public: // types

private: 
	Archive::AutoVariable<std::string>           m_planetName; 
	Archive::AutoArray<MapLocation>              m_mapLocationsStatic; 
	Archive::AutoArray<MapLocation>              m_mapLocationsDynamic; 
	Archive::AutoArray<MapLocation>              m_mapLocationsPersist; 
	Archive::AutoVariable<int>                   m_versionStatic;
	Archive::AutoVariable<int>                   m_versionDynamic;
	Archive::AutoVariable<int>                   m_versionPersist;
};

// ----------------------------------------------------------------------

inline const std::string &GetMapLocationsResponseMessage::getPlanetName() const
{
	return m_planetName.get();
}

// ----------------------------------------------------------------------

inline const GetMapLocationsResponseMessage::MapLocationVector &GetMapLocationsResponseMessage::getMapLocationsStatic () const
{
	return m_mapLocationsStatic.get();
}

// ----------------------------------------------------------------------

inline const GetMapLocationsResponseMessage::MapLocationVector &GetMapLocationsResponseMessage::getMapLocationsDynamic () const
{
	return m_mapLocationsDynamic.get();
}

// ----------------------------------------------------------------------

inline const GetMapLocationsResponseMessage::MapLocationVector &GetMapLocationsResponseMessage::getMapLocationsPersist () const
{
	return m_mapLocationsPersist.get();
}

//----------------------------------------------------------------------

inline int GetMapLocationsResponseMessage::getVersionStatic       ()  const
{
	return m_versionStatic.get ();
}

//----------------------------------------------------------------------

inline int GetMapLocationsResponseMessage::getVersionDynamic      ()  const
{
	return m_versionDynamic.get ();
}

//----------------------------------------------------------------------

inline int GetMapLocationsResponseMessage::getVersionPersist      ()  const
{
	return m_versionPersist.get ();
}

// ----------------------------------------------------------------------

#endif // _GetMapLocationsResponseMessage_H

