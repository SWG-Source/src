// ======================================================================
//
// PurgeManager.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PurgeManager_H
#define INCLUDED_PurgeManager_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "Unicode.h"

// ======================================================================

/**
 * Singleton for organizing structures, characters, vendors, etc. to be
 * purged.
 */
class PurgeManager
{
  public:
	  static void handleStructuresAndVendorsForPurge(StationId stationId, stdvector<std::pair<NetworkId, NetworkId> >::fwd const & structures, stdvector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > >::fwd const & vendors, bool warnOnly);
};

// ======================================================================

#endif
