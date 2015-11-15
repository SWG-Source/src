// ======================================================================
//
// PropertyListBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyListBuffer_H
#define INCLUDED_PropertyListBuffer_H

// ======================================================================

#include "SwgDatabaseServer/PropertyListQuery.h"
#include "serverDatabase/TableBuffer.h"
#include "sharedFoundation/NetworkId.h"

#include <map>
#include <set>

// ======================================================================

namespace PropertyListBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace PropertyListBufferNamespace;

// ======================================================================

/** 
 * PropertyListBuffer is a buffer customized to the job of loading and
 * saving lists of sets.
 *
 * This buffer does not combine changes.  All operations given to it are
 * sent to the database.
 */

class PropertyListBuffer : public TableBuffer<DBSchema::PropertyListQueryData, DBQuery::PropertyListQuery>
{
  public:
	// WARNING:  Do not renumber anything in this list.  You will break objects that are already persisted.
	// If you remove something from the list, leave the numbers of other items unchanged
	enum ListIds
	{
		LI_Commands=0,
		LI_DraftSchematics=1,
		// removed: LI_PvpEnemies=2,
		LI_Allowed=3,
		LI_Banned=4,
		LI_GuildNames=5,
		LI_GuildAbbrevs=6,
		LI_GuildMembers=7,
		LI_GuildEnemies=8,
		LI_GuildLeaders=10,
		LI_Skills=11,
		LI_Cities=12,
		LI_Citizens=13,
		LI_CityStructures=14
	};
	
public:
	explicit PropertyListBuffer(DB::ModeQuery::Mode mode);
	virtual ~PropertyListBuffer(void);
	
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool save(DB::Session *session);
	virtual void removeObject(const NetworkId &object);
		
	bool getPropertyListForObject(const NetworkId &objectId, int listId, std::set<std::string> &values) const;

	void insertProperty (const NetworkId &objectId, int listId, const std::string &value);
	void deleteProperty (const NetworkId &objectId, int listId, const std::string &value);
	void clearProperties(const NetworkId &objectId, int listId);
	
  private:
	struct PropertyListEntry
	{
		NetworkId m_objectId;
		int m_listId;
		std::string m_value;

		PropertyListEntry(const NetworkId &objectId, int listId, const std::string &value);
		bool operator<(const PropertyListEntry &rhs) const;
	
		PropertyListEntry();
		PropertyListEntry(const PropertyListEntry & rhs);
		PropertyListEntry & operator=(const PropertyListEntry & rhs);
	};

	typedef std::set<PropertyListEntry> PropertyListSet;
	PropertyListSet m_loadData;

	PropertyListSet m_adds;
	PropertyListSet m_deletes;
	PropertyListSet m_clears;

	typedef std::set<NetworkId> OIDList;
	OIDList m_cancelledObjects;
	
  private:
	PropertyListBuffer(); //disable
	PropertyListBuffer(const PropertyListBuffer&); //disable
	PropertyListBuffer & operator=(const PropertyListBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline PropertyListBuffer::PropertyListEntry::PropertyListEntry(const NetworkId &objectId, int listId, const std::string &value) :
		m_objectId(objectId),
		m_listId(listId),
		m_value(value)
{
}

// ----------------------------------------------------------------------

inline bool PropertyListBuffer::PropertyListEntry::operator<(const PropertyListEntry &right) const
{
	if (m_objectId < right.m_objectId)
		return true;
	if (m_objectId == right.m_objectId)
	{
		if (m_listId < right.m_listId)
			return true;
		if (m_listId == right.m_listId)
			return m_value < right.m_value;
	}
	return false;
}

// ======================================================================

#endif

