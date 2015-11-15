// ======================================================================
//
// ResourceTypeBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceTypeBuffer_H
#define INCLUDED_ResourceTypeBuffer_H

// ======================================================================

#include "serverDatabase/AbstractTableBuffer.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"

// ======================================================================

namespace DB
{
	class Session;
}

namespace DBSchema
{
	struct ResourceTypeRow;
}

class GameServerConnection;

// ======================================================================

/**
 * A buffer to hold resource type data.
 */
class ResourceTypeBuffer : public AbstractTableBuffer
{
  public:
	void handleAddResourceTypeMessage     (AddResourceTypeMessage const & message);
	void sendResourceTypeObjects          (GameServerConnection & conn) const;

	virtual bool save(DB::Session *session);
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual void removeObject(const NetworkId &object);
	
  public:
	ResourceTypeBuffer();
	virtual ~ResourceTypeBuffer();
		
  private:
	typedef std::map<NetworkId, DBSchema::ResourceTypeRow*> DataType;
	
  private:
	DataType m_data;
};

// ======================================================================

#endif
