// ======================================================================
//
// ExperienceBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ExperienceBuffer_H
#define INCLUDED_ExperienceBuffer_H

// ======================================================================

#include "SwgDatabaseServer/ExperienceQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/AbstractTableBuffer.h"
#include <map>
#include <string>

// ======================================================================

class ExperienceBuffer : public AbstractTableBuffer
{
public:
	explicit                  ExperienceBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~ExperienceBuffer      ();
	
	virtual bool              load                   (DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save                   (DB::Session *session);
	virtual void              removeObject           (const NetworkId &object);
		
	DBSchema::ExperiencePointRow * findRowByIndex    (const NetworkId &objectId, const std::string &experienceType);

	void                      setExperience          (const NetworkId &objectId, const std::string &experienceType, int value);
	void                      removeExperience       (const NetworkId &objectId, std::string experienceType);
	bool                      getExperienceForObject (const NetworkId &objectId, std::vector<std::pair<std::string, int> > &values) const;
	
  private:
	void                      addRowToIndex          (const NetworkId &objectId, const std::string &experienceType, DBSchema::ExperiencePointRow *row);

  private:
	struct IndexKey
	{
		NetworkId   m_objectId;
		std::string m_experienceType;

		IndexKey(NetworkId objectId, std::string experienceType);
		bool operator==(const IndexKey &rhs) const;
	};
	struct IndexSorter
	{
		bool operator() (const IndexKey &left, const IndexKey &right) const;
	};
	typedef std::map<IndexKey,DBSchema::ExperiencePointRow*,IndexSorter> IndexType;

	DB::ModeQuery::Mode m_mode;
	IndexType m_rows;

 private:
	ExperienceBuffer(); //disable
	ExperienceBuffer(const ExperienceBuffer&); //disable
	ExperienceBuffer & operator=(const ExperienceBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline ExperienceBuffer::IndexKey::IndexKey(NetworkId objectId, std::string experienceType) :
	m_objectId(objectId),
	m_experienceType(experienceType)
{
}

// ----------------------------------------------------------------------

inline bool ExperienceBuffer::IndexKey::operator==(const IndexKey &rhs) const
{
	return ((m_objectId == rhs.m_objectId) && (m_experienceType == rhs.m_experienceType));
}

// ----------------------------------------------------------------------

inline bool ExperienceBuffer::IndexSorter::operator() (const IndexKey &left, const IndexKey &right) const
{
	if (left.m_objectId == right.m_objectId)
		return (left.m_experienceType < right.m_experienceType);
	else
		return (left.m_objectId < right.m_objectId);
}

// ----------------------------------------------------------------------

/**
 * Removing experience is the same as setting it to 0.  (The database package
 * will delete the row if points == 0.)
 */
inline void ExperienceBuffer::removeExperience(const NetworkId &objectId, std::string experienceType)
{
	setExperience(objectId,experienceType,0);
}

// ======================================================================

#endif
