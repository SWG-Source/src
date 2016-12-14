// ======================================================================
//
// ManufactureSchematicAttributeBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ManufactureSchematicAttributeBuffer_H
#define INCLUDED_ManufactureSchematicAttributeBuffer_H

// ======================================================================

#include <map>
#include <string>
#include "serverDatabase/AbstractTableBuffer.h"
#include "SwgDatabaseServer/ManufactureSchematicAttributeQuery.h"

namespace DBSchema
{
	struct ManufactureSchematicAttributeRow;
}

// ======================================================================

class ManufactureSchematicAttributeBuffer : public AbstractTableBuffer // <DBSchema::ManufactureSchematicAttributeRow, DBQuery::ManufactureSchematicAttributeQuery>
{
public:
	explicit                  ManufactureSchematicAttributeBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~ManufactureSchematicAttributeBuffer      ();
	
	virtual bool              load                                       (DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save                                       (DB::Session *session);
	virtual void              removeObject                               (const NetworkId &object);
	
		
	DBSchema::ManufactureSchematicAttributeRow * findRowByIndex          (NetworkId objectId, std::string attributeType);

	void                      setManufactureSchematicAttribute           (const NetworkId &objectId, std::string attributeType, float value);
	void                      removeManufactureSchematicAttribute        (const NetworkId &objectId, std::string attributeType);
	bool                      getDataForObject                           (const NetworkId &objectId, std::vector<std::pair<std::string, float> > &values) const;
	
  private:
	void                      addRowToIndex                              (NetworkId objectId, std::string attributeType, DBSchema::ManufactureSchematicAttributeRow *row);

  private:
	struct IndexKey
	{
		NetworkId   m_objectId;
		std::string m_attributeType;

		IndexKey(NetworkId objectId, std::string attributeType);
		bool operator==(const IndexKey &rhs) const;
	};
	struct IndexSorter
	{
		bool operator() (const IndexKey &left, const IndexKey &right) const;
	};

	typedef std::map<IndexKey,DBSchema::ManufactureSchematicAttributeRow*,IndexSorter> IndexType;

	DB::ModeQuery::Mode m_mode;
	IndexType m_rows;

 private:
	ManufactureSchematicAttributeBuffer(); //disable
	ManufactureSchematicAttributeBuffer(const ManufactureSchematicAttributeBuffer&); //disable
	ManufactureSchematicAttributeBuffer & operator=(const ManufactureSchematicAttributeBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

inline ManufactureSchematicAttributeBuffer::IndexKey::IndexKey(NetworkId objectId, std::string attributeType) :
	m_objectId(objectId),
	m_attributeType(attributeType)
{
}

// ----------------------------------------------------------------------

inline bool ManufactureSchematicAttributeBuffer::IndexKey::operator==(const IndexKey &rhs) const
{
	return ((m_objectId == rhs.m_objectId) && (m_attributeType == rhs.m_attributeType));
}

// ----------------------------------------------------------------------

inline bool ManufactureSchematicAttributeBuffer::IndexSorter::operator() (const IndexKey &left, const IndexKey &right) const
{
	if (left.m_objectId == right.m_objectId)
		return (left.m_attributeType < right.m_attributeType);
	else
		return (left.m_objectId < right.m_objectId);
}

// ======================================================================

#endif
