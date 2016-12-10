// ======================================================================
//
// BiographyManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BiographyManager_H
#define INCLUDED_BiographyManager_H

// ======================================================================

#include "Singleton/Singleton.h"

class CreatureObject;
class CachedNetworkId;
class NetworkId;

// ======================================================================

/** 
 * A singleton to handle (character) biographies.  Caches biographies,
 * and forwards updates and load requests to the database.
 * @todo Expire entries in the cache?
 */
class BiographyManager
{
public:
	static void install();
	static void remove();

	static void setBiography(const NetworkId &owner, const Unicode::String &bio);
	static void deleteBiography(const NetworkId &owner);
	static void requestBiography(const NetworkId &owner, CreatureObject * requestedBy);
	static void onBiographyRetrieved(const NetworkId &owner, const Unicode::String &bio);

private:
	BiographyManager();
	~BiographyManager();

	static bool m_installed;

	/**
	 * Map of (object bio) -> (set of objects that want the bio when it's loaded)
	 */
	static std::map<NetworkId, std::vector<CachedNetworkId> > *m_retrievalQueue;
};

// ======================================================================

#endif
