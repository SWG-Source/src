// ======================================================================
//
// BountyHunterTargetBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BountyHunterTargetBuffer_H
#define INCLUDED_BountyHunterTargetBuffer_H

// ======================================================================

#include "serverDatabase/AbstractTableBuffer.h"
#include "serverNetworkMessages/BountyHunterTargetListMessage.h"
#include "serverNetworkMessages/BountyHunterTargetMessage.h"

// ======================================================================

namespace DB
{
	class Session;
}

namespace DBSchema
{
	struct BountyHunterTargetRow;
}

class GameServerConnection;

// ======================================================================

/**
 * A buffer to hold bounty hunter target data.
 */
class BountyHunterTargetBuffer : public AbstractTableBuffer
{
  public:
	void setBountyHunterTarget         (const NetworkId &objectId, const NetworkId &targetId);
	void sendBountyHunterTargetMessage (GameServerConnection & conn) const;

	virtual bool save(DB::Session *session);
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual void removeObject(const NetworkId &object);
	DBSchema::BountyHunterTargetRow * findRowByIndex   (const NetworkId &objectId);
	
  public:
	BountyHunterTargetBuffer();
	virtual ~BountyHunterTargetBuffer();
		
  private:
	void addRowToIndex (const NetworkId &objectId, DBSchema::BountyHunterTargetRow *row);
	typedef std::map<NetworkId, DBSchema::BountyHunterTargetRow*> DataType;
	DataType m_data;
};

// ======================================================================

#endif
