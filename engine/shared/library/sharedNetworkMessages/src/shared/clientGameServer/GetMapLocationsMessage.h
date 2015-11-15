// ======================================================================
//
// GetMapLocationsMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetMapLocationsMessage_H
#define	_GetMapLocationsMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"

//-----------------------------------------------------------------------

class GetMapLocationsMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	GetMapLocationsMessage(const std::string &planetName, int cacheVersionStatic, int cacheVersionDynamic, int cacheVersionPersist);
	explicit GetMapLocationsMessage(Archive::ReadIterator &source);

	~GetMapLocationsMessage();

public: // methods

	const std::string              & getPlanetName          () const;
	int                              getCacheVersionStatic  () const;
	int                              getCacheVersionDynamic () const;
	int                              getCacheVersionPersist () const;

public: // types

private:
	Archive::AutoVariable<std::string>           m_planetName;
	Archive::AutoVariable<int>                   m_cacheVersionStatic;
	Archive::AutoVariable<int>                   m_cacheVersionDynamic;
	Archive::AutoVariable<int>                   m_cacheVersionPersist;
};

// ----------------------------------------------------------------------

inline const std::string &GetMapLocationsMessage::getPlanetName() const
{
	return m_planetName.get();
}

//----------------------------------------------------------------------

inline int GetMapLocationsMessage::getCacheVersionStatic  () const
{
	return m_cacheVersionStatic.get ();
}

//----------------------------------------------------------------------

inline int GetMapLocationsMessage::getCacheVersionDynamic () const
{
	return m_cacheVersionDynamic.get ();
}

//----------------------------------------------------------------------

inline int GetMapLocationsMessage::getCacheVersionPersist () const
{
	return m_cacheVersionPersist.get ();
}

// ----------------------------------------------------------------------

#endif // _GetMapLocationsMessage_H

