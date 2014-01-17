// ======================================================================
//
// BattlefieldParticipantBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BattlefieldParticipantBuffer_H
#define INCLUDED_BattlefieldParticipantBuffer_H

// ======================================================================

#include <map>
#include <string>
#include "serverDatabase/TableBuffer.h"
#include "SwgDatabaseServer/BattlefieldParticipantQuery.h"
#include "SwgDatabaseServer/Schema.h"

// ======================================================================

class BattlefieldParticipantBuffer : public AbstractTableBuffer
{
public:
	explicit                  BattlefieldParticipantBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~BattlefieldParticipantBuffer      ();
	
	virtual bool              load                               (DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save                               (DB::Session *session);
	virtual void              removeObject                       (const NetworkId &object);
		
	DBSchema::BattlefieldParticipantRow * findRowByIndex         (const NetworkId &regionObjectId, const NetworkId &characterObjectId);

	void                      setParticipantForRegion            (const NetworkId &regionObjectId, const NetworkId &characterObjectId, uint32 factionId);
	void                      removeParticipantForRegion         (const NetworkId &regionObjectId, const NetworkId &characterObjectId);
	bool                      getParticipantsForRegion           (const NetworkId &regionObjectId, std::vector<std::pair<NetworkId, uint32> > &values) const;
	
private:
	void                      addRowToIndex                      (const NetworkId &regionObjectId, const NetworkId &characterObjectId, DBSchema::BattlefieldParticipantRow *row);

private:
	struct IndexKey
	{
		NetworkId   m_regionObjectId;
		NetworkId   m_characterObjectId;

		IndexKey(const NetworkId &regionObjectId, const NetworkId &characterObjectId);
		bool operator==(const IndexKey &rhs) const;
	};
	struct IndexSorter
	{
		bool operator() (const IndexKey &left, const IndexKey &right) const;
	};

	typedef std::map<IndexKey,DBSchema::BattlefieldParticipantRow*,IndexSorter> IndexType;
	IndexType m_rows;
	DB::ModeQuery::Mode m_mode;
	
 private:
	BattlefieldParticipantBuffer(); //disable
	BattlefieldParticipantBuffer(const BattlefieldParticipantBuffer&); //disable
	BattlefieldParticipantBuffer & operator=(const BattlefieldParticipantBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline BattlefieldParticipantBuffer::IndexKey::IndexKey(const NetworkId &regionObjectId, const NetworkId &characterObjectId) :
	m_regionObjectId(regionObjectId),
	m_characterObjectId(characterObjectId)
{
}

// ----------------------------------------------------------------------

inline bool BattlefieldParticipantBuffer::IndexKey::operator==(const IndexKey &rhs) const
{
	return ((m_regionObjectId == rhs.m_regionObjectId) && (m_characterObjectId == rhs.m_characterObjectId));
}

// ----------------------------------------------------------------------

inline bool BattlefieldParticipantBuffer::IndexSorter::operator() (const IndexKey &left, const IndexKey &right) const
{
	if (left.m_regionObjectId == right.m_regionObjectId)
		return (left.m_characterObjectId < right.m_characterObjectId);
	else
		return (left.m_regionObjectId < right.m_regionObjectId);
}

// ----------------------------------------------------------------------

/**
 * Removing a battlefield participant is the same as setting it to 0. (The database package
 * will delete the row if faction_id == 0.)
 */
inline void BattlefieldParticipantBuffer::removeParticipantForRegion(const NetworkId &regionObjectId, const NetworkId &characterObjectId)
{
	setParticipantForRegion(regionObjectId,characterObjectId,0);
}

// ======================================================================

#endif
