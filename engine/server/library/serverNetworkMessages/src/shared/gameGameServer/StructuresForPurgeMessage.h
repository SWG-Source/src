// ======================================================================
//
// StructuresForPurgeMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StructuresForPurgeMessage_H
#define INCLUDED_StructuresForPurgeMessage_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"

// ======================================================================

/**
 * Sent from:  DB Process
 * Sent to:  Game Server
 * Action:  Identifies a list of structures that should be purged.  Send
 * messages to each structure to get them to purge themselves.  Report
 * back to the database process when all structures have been purged.
 */
class StructuresForPurgeMessage : public GameNetworkMessage
{
  public:
	StructuresForPurgeMessage  (StationId stationId, std::vector<std::pair<NetworkId, NetworkId> > const & structures, std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & vendors, bool warnOnly);
	StructuresForPurgeMessage  (Archive::ReadIterator & source);
	~StructuresForPurgeMessage ();

	StationId getStationId() const;
	std::vector<std::pair<NetworkId, NetworkId> > const & getStructures() const; // pair of (structure, owner)
	std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & getVendors() const;    // pair of (vendor, pair(owner, vendor name))
	bool getWarnOnly() const;
	
  private:
	Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoArray<std::pair<NetworkId, NetworkId> > m_structures;
	Archive::AutoArray<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > m_vendors;
	Archive::AutoVariable<bool> m_warnOnly;
	
  private:        
	StructuresForPurgeMessage(const StructuresForPurgeMessage&);
	StructuresForPurgeMessage& operator= (const StructuresForPurgeMessage&);
};

// ======================================================================

#endif
