// ======================================================================
//
// ManufactureSchematicAttributeBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ManufactureSchematicAttributeBuffer.h"

#include "SwgDatabaseServer/ManufactureSchematicAttributeQuery.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

#include<vector>

// ======================================================================

ManufactureSchematicAttributeBuffer::ManufactureSchematicAttributeBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_mode(mode),
		m_rows()
{
}

// ----------------------------------------------------------------------

ManufactureSchematicAttributeBuffer::~ManufactureSchematicAttributeBuffer(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

void ManufactureSchematicAttributeBuffer::setManufactureSchematicAttribute(const NetworkId &objectId, std::string attributeType, float value)
{
	DBSchema::ManufactureSchematicAttributeRow *row=findRowByIndex(objectId,attributeType);
	if (!row)
	{
		row=new DBSchema::ManufactureSchematicAttributeRow;
		addRowToIndex(objectId,attributeType,row);
	}

	row->action = 1; //TODO:  enum somewhere
	row->object_id = objectId;
	row->attribute_type=attributeType;
	row->value = value;
}

// ----------------------------------------------------------------------

void ManufactureSchematicAttributeBuffer::removeManufactureSchematicAttribute(const NetworkId &objectId, std::string attributeType)
{
	DBSchema::ManufactureSchematicAttributeRow *row=findRowByIndex(objectId,attributeType);
	if (!row)
	{
		row=new DBSchema::ManufactureSchematicAttributeRow;
		addRowToIndex(objectId,attributeType,row);
	}

	row->action = 0; //TODO:  enum somewhere
}

// ----------------------------------------------------------------------

bool ManufactureSchematicAttributeBuffer::getDataForObject(const NetworkId &objectId, std::vector<std::pair<std::string, float> > &values) const 
{
	DEBUG_FATAL(values.size()!=0,("Values vector should be empty on call to getExperienceForObject()\n"));
	IndexType::const_iterator i ( m_rows.lower_bound(IndexKey(objectId,"")) );
	if (i==m_rows.end())
		return false;
	for (; (i!=m_rows.end()) && (((*i).second)->object_id.getValue()==objectId); ++i)
	{
		std::string temp;
		(*i).second->attribute_type.getValue(temp);
		values.push_back(std::pair<std::string, float>(temp, static_cast<float>((*i).second->value.getValue())));
	}
	return true;
}

// ----------------------------------------------------------------------

DBSchema::ManufactureSchematicAttributeRow * ManufactureSchematicAttributeBuffer::findRowByIndex(NetworkId objectId, std::string experienceType)
{
	IndexType::iterator i=m_rows.find(IndexKey(objectId,experienceType));
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void ManufactureSchematicAttributeBuffer::addRowToIndex(NetworkId objectId, std::string attributeType, DBSchema::ManufactureSchematicAttributeRow *row)
{
	m_rows[IndexKey(objectId,attributeType)]=row;
}

// ----------------------------------------------------------------------

/**
 * Load the attributes for all the remembered objectId's.
 */
bool ManufactureSchematicAttributeBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (tags.find(static_cast<Tag>(ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag)) != tags.end())
	{
		DBQuery::GetAllManufactureSchematicAttributes qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<DBSchema::ManufactureSchematicAttributeRow> &data = qry.getData();

			for (std::vector<DBSchema::ManufactureSchematicAttributeRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				DBSchema::ManufactureSchematicAttributeRow *row=new DBSchema::ManufactureSchematicAttributeRow(*i);
				NOT_NULL(row);
				std::string temp;
				row->attribute_type.getValue(temp);
				addRowToIndex(row->object_id.getValue(),temp,row);
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool ManufactureSchematicAttributeBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("ManufactureSchematicAttributeBuffer:  %i saved to db",m_rows.size()));
	
	DBQuery::ManufactureSchematicAttributeQuery qry;
	
	if (m_mode==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();
	
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		qry.setData(*(i->second));
		if (! (session->exec(&qry)))
			return false;
	}
	qry.done();
	return true;
}


// ----------------------------------------------------------------------

void ManufactureSchematicAttributeBuffer::removeObject(const NetworkId &object)
{
	IndexType::iterator i=m_rows.lower_bound(IndexKey(object,""));
	while (i!=m_rows.end() && i->first.m_objectId==object)
	{
		IndexType::iterator next=i;
		++next;
		delete i->second;
		i->second=nullptr;
		m_rows.erase(i);
		i=next;
	}
}

// ======================================================================
