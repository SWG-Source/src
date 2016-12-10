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
	  static void handleStructuresAndVendorsForPurge(StationId stationId, std::vector<std::pair<NetworkId, NetworkId> > const & structures, std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & vendors, bool warnOnly);
};

// ======================================================================

#endif
