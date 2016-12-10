// ======================================================================
//
// WaypointBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WaypointBuffer_H
#define INCLUDED_WaypointBuffer_H

// ======================================================================

#include "SwgDatabaseServer/PersistableWaypoint.h"
#include "SwgDatabaseServer/Schema.h"
#include "SwgDatabaseServer/WaypointQuery.h"
#include "serverDatabase/AbstractTableBuffer.h"

#include <map>
#include <string>

// ======================================================================

class WaypointBuffer : public AbstractTableBuffer
{
public:
	explicit                  WaypointBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~WaypointBuffer      ();
	
	virtual bool              load                   (DB::Session *session,const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save                   (DB::Session *session);
	virtual void              removeObject           (const NetworkId &object);
		
	void                      setWaypoint          (const NetworkId &objectId, const NetworkId &waypointId, const PersistableWaypoint &value);
	void                      removeWaypoint       (const NetworkId &objectId, const NetworkId &waypointId);
	bool                      getWaypointsForObject(const NetworkId &objectId, std::vector<PersistableWaypoint> &values) const;
	
  private:
	struct IndexKey
	{
		NetworkId   m_objectId;
		NetworkId   m_waypointId;

		IndexKey(const NetworkId &objectId, const NetworkId &waypointId);
		bool operator==(const IndexKey &rhs) const;
		bool operator<(const IndexKey &rhs) const;
	};
	typedef std::map<IndexKey,PersistableWaypoint> IndexType;

	DB::ModeQuery::Mode m_mode;
	IndexType m_data;

 private:
	WaypointBuffer(); //disable
	WaypointBuffer(const WaypointBuffer&); //disable
	WaypointBuffer & operator=(const WaypointBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline WaypointBuffer::IndexKey::IndexKey(const NetworkId &objectId, const NetworkId &waypointId) :
	m_objectId(objectId),
	m_waypointId(waypointId)
{
}

// ----------------------------------------------------------------------

inline bool WaypointBuffer::IndexKey::operator==(const IndexKey &rhs) const
{
	return ((m_objectId == rhs.m_objectId) && (m_waypointId == rhs.m_waypointId));
}

// ----------------------------------------------------------------------

inline bool WaypointBuffer::IndexKey::operator< (const IndexKey &rhs) const
{
	if (m_objectId == rhs.m_objectId)
		return (m_waypointId < rhs.m_waypointId);
	else
		return (m_objectId < rhs.m_objectId);
}

// ----------------------------------------------------------------------

/**
 *
 */
inline void WaypointBuffer::removeWaypoint(const NetworkId &objectId, const NetworkId &waypointId)
{
	PersistableWaypoint temp;
	temp.m_networkId=waypointId;
	temp.m_detached=true;
	setWaypoint(objectId,waypointId,temp);
}

// ======================================================================

#endif
