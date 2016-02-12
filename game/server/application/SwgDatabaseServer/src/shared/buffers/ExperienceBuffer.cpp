// ======================================================================
//
// ExperienceBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ExperienceBuffer.h"

#include "SwgDatabaseServer/ExperienceQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include <vector>

// ======================================================================

ExperienceBuffer::ExperienceBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_mode(mode),
		m_rows()
{
}

// ----------------------------------------------------------------------

ExperienceBuffer::~ExperienceBuffer(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

void ExperienceBuffer::setExperience(const NetworkId &objectId, const std::string &experienceType, int value)
{
	DBSchema::ExperiencePointRow *row=findRowByIndex(objectId,experienceType);
	if (!row)
	{
		row=new DBSchema::ExperiencePointRow;
		addRowToIndex(objectId,experienceType,row);
	}
	row->object_id = objectId;
	row->experience_type=experienceType;
	row->points = value;
}

// ----------------------------------------------------------------------

bool ExperienceBuffer::getExperienceForObject(const NetworkId &objectId, std::vector<std::pair<std::string, int> > &values) const 
{
	DEBUG_FATAL(values.size()!=0,("Values vector should be empty on call to getExperienceForObject()\n"));
	IndexType::const_iterator i ( m_rows.lower_bound(IndexKey(objectId,"")) );
	if (i==m_rows.end())
		return false;
	for (; (i!=m_rows.end()) && (((*i).second)->object_id.getValue()==objectId); ++i)
	{
		std::string temp;
		(*i).second->experience_type.getValue(temp);
		values.push_back(std::pair<std::string, int>(temp, (*i).second->points.getValue()));
	}
	return true;
}

// ----------------------------------------------------------------------

DBSchema::ExperiencePointRow * ExperienceBuffer::findRowByIndex(const NetworkId &objectId, const std::string &experienceType)
{
	IndexType::iterator i=m_rows.find(IndexKey(objectId,experienceType));
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void ExperienceBuffer::addRowToIndex (const NetworkId &objectId, const std::string &experienceType, DBSchema::ExperiencePointRow *row)
{
	m_rows[IndexKey(objectId,experienceType)]=row;
}

// ----------------------------------------------------------------------

/**
 * Load the attributes for all the remembered objectId's.
 */
bool ExperienceBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched = 0;
	if (tags.find(static_cast<Tag>(ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag)) != tags.end())
	{
		DBQuery::GetAllExperience qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<DBSchema::ExperiencePointRow> &data = qry.getData();

			for (std::vector<DBSchema::ExperiencePointRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				DBSchema::ExperiencePointRow *row=new DBSchema::ExperiencePointRow(*i);
				NOT_NULL(row);
				std::string temp;
				row->experience_type.getValue(temp);
				addRowToIndex(row->object_id.getValue(),temp,row);
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

bool ExperienceBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("Experience:  %i saved to db",m_rows.size()));
	
	DBQuery::ExperienceQuery qry;
	
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

void ExperienceBuffer::removeObject(const NetworkId &object)
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
