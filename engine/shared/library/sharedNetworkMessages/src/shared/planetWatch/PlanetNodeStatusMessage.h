// ======================================================================
//
// PlanetNodeStatusMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetNodeStatusMessage_H
#define INCLUDED_PlanetNodeStatusMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class PlanetNodeStatusMessageData
{
  public:
	int                 m_x;
	int                 m_z;
	bool                m_loaded;
	std::vector<uint32> m_servers;
	std::vector<int>    m_subscriptionCounts;
	
  public:
	PlanetNodeStatusMessageData(int x, int z, bool loaded, const std::vector<uint32> &servers, const std::vector<int> &subscriptionCounts);
	PlanetNodeStatusMessageData();
};

// ======================================================================

/**
 * Sent from:  PlanetServer
 * Sent to:    PlanetWatcher
 * Action:     Tells the PlanetWatcher the status of nodes in the spatial subdivision.
 */
class PlanetNodeStatusMessage : public GameNetworkMessage
{
  public:
	PlanetNodeStatusMessage(const std::vector<PlanetNodeStatusMessageData> &data);
	explicit PlanetNodeStatusMessage(Archive::ReadIterator & source);
	virtual ~PlanetNodeStatusMessage();

  public:
	void getData(std::vector<PlanetNodeStatusMessageData> &data);
	
  private:
	Archive::AutoArray<PlanetNodeStatusMessageData> m_data;

  private:
	PlanetNodeStatusMessage();
	PlanetNodeStatusMessage(const PlanetNodeStatusMessage&);
	PlanetNodeStatusMessage& operator= (const PlanetNodeStatusMessage&);
};

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PlanetNodeStatusMessageData &d);
	void put(ByteStream & target, const PlanetNodeStatusMessageData &d);
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
