// ======================================================================
//
// CreatureObjectBuffer.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CreatureObjectBuffer_H
#define INCLUDED_CreatureObjectBuffer_H

// ======================================================================

#include "SwgDatabaseServer/IndexedNetworkTableBuffer.h"
#include "SwgDatabaseServer/ObjectQueries.h"
#include "SwgDatabaseServer/Schema.h"
#include "swgSharedUtility/Attributes.def"

class UpdateObjectPositionMessage;

// ======================================================================

class CreatureObjectBuffer : public IndexedNetworkTableBuffer<DBSchema::CreatureObjectBufferRow, DBSchema::CreatureObjectRow, DBQuery::CreatureObjectQuery, DBQuery::CreatureObjectQuerySelect>
{
  public:
	CreatureObjectBuffer(DB::ModeQuery::Mode mode);

	void setAttribute(const NetworkId &objectId, Attributes::Enumerator type, Attributes::Value value);
	void getAttributesForObject (const NetworkId &objectId, std::vector<Attributes::Value> &values, int offset, int howMany) const;

	virtual bool handleUpdateObjectPosition(const UpdateObjectPositionMessage &message);
};

// ======================================================================

#endif
