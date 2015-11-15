// ======================================================================
//
// LocationBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationBuffer_H
#define INCLUDED_LocationBuffer_H

// ======================================================================

#include <map>
#include <vector>
#include "serverDatabase/TableBuffer.h"
#include "serverUtility/LocationData.h"
#include "sharedFoundation/NetworkId.h"

namespace DBSchema
{
	struct LocationRow;
}

// ======================================================================

/** 
 * LocationBuffer is a table buffer customized to the job of loading and
 * saving locations for OnArrivedAt() targets, etc.
 *
 * Locations are keyed by object id, a list number, and a sequence number.  (Thus
 * each object can have multiple lists of locations.)
 *
 * Unlike most other buffer classes, this class does not maintain a list of rows.
 * Instead, it has a map of vectors of actions to be applied in the database.
 */

class LocationBuffer : public AbstractTableBuffer
{
  public:
	explicit LocationBuffer(DB::ModeQuery::Mode mode);
	virtual ~LocationBuffer(void);
	
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool save(DB::Session *session);
	virtual void removeObject(const NetworkId &object);
		
	void set    (const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value);
	void insert (const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value);
	void erase  (const NetworkId &objectId, size_t listId, size_t sequenceNumber);
	void getLocationList(const NetworkId &objectId, size_t listId, std::vector<LocationData> &values) const;

  private:
	struct BufferEntry
	{
		enum Action {be_INSERT, be_ERASE, be_SET};
		size_t m_listId;
		Action m_action;
		size_t m_index;
		LocationData m_data;
		
		BufferEntry(size_t listId, Action action, size_t index, const LocationData &data);
	};

	struct IndexKey
	{
		NetworkId m_objectId;
		size_t    m_listId;
		size_t    m_index;

		IndexKey();
		IndexKey(const NetworkId &objectId, size_t listId, size_t index);
		IndexKey(const IndexKey &rhs);
		bool operator < (const IndexKey &rhs) const;
	};
	
	typedef std::map<NetworkId, std::vector<BufferEntry> > SaveDataType;
	SaveDataType m_saveData;

	typedef std::map<IndexKey, LocationData> LoadDataType;
	LoadDataType m_loadData;

  private:
	void addAction(BufferEntry::Action action, const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value);
	
  private:
	LocationBuffer(); //disable
	LocationBuffer(const LocationBuffer&); //disable
	LocationBuffer & operator=(const LocationBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline void LocationBuffer::set(const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value)
{
	addAction(BufferEntry::be_SET, objectId, listId,  sequenceNumber, value);
}

// ----------------------------------------------------------------------

inline void LocationBuffer::insert(const NetworkId &objectId, size_t listId, size_t sequenceNumber, const LocationData &value)
{
	addAction(BufferEntry::be_INSERT, objectId, listId,  sequenceNumber, value);
}

// ----------------------------------------------------------------------

inline void LocationBuffer::erase(const NetworkId &objectId, size_t listId, size_t sequenceNumber)
{
	addAction(BufferEntry::be_ERASE, objectId, listId,  sequenceNumber, LocationData());
}

// ======================================================================

#endif
