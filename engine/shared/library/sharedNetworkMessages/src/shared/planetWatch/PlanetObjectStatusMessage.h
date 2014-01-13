// ======================================================================
//
// PlanetObjectStatusMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetObjectStatusMessage_H
#define INCLUDED_PlanetObjectStatusMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------

/**
 * Not nested to allow forward declaration
 */
class PlanetObjectStatusMessageData
{
  public:
	NetworkId m_objectId;
	int       m_x;
	int       m_z;
	uint32    m_authoritativeServer;
	int       m_interestRadius;
	int       m_deleteObject;
	int       m_objectTypeTag;
	int       m_level;
	bool      m_hibernating;
	uint32    m_templateCrc;
	int       m_aiActivity;
	int       m_creationType;

  public:
	PlanetObjectStatusMessageData(const NetworkId &objectId, int x, int z, uint32 authoritativeServer, int interestRadius, int deleteObject, int objectTypeTag, int m_level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);
	PlanetObjectStatusMessageData();
};

// ======================================================================

/**
 * Sent from:  PlanetServer
 * Sent to:    PlanetWatcher
 * Action:     Tells the PlanetWatcher about objects on the PlanetServer
 */
class PlanetObjectStatusMessage : public GameNetworkMessage
{	
  public:
	PlanetObjectStatusMessage(const std::vector<PlanetObjectStatusMessageData> &data);
	explicit PlanetObjectStatusMessage(Archive::ReadIterator & source);
	virtual ~PlanetObjectStatusMessage();

	void getData(std::vector<PlanetObjectStatusMessageData> &data);

  private:
	Archive::AutoArray<PlanetObjectStatusMessageData>   m_data;

	PlanetObjectStatusMessage();
	PlanetObjectStatusMessage(const PlanetObjectStatusMessage&);
	PlanetObjectStatusMessage& operator= (const PlanetObjectStatusMessage&);
};

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PlanetObjectStatusMessageData &d);
	void put(ByteStream & target, const PlanetObjectStatusMessageData &d);
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
