// ======================================================================
//
// ResourceTypeBuffer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ResourceTypeBuffer.h"

#include "SwgDatabaseServer/ResourceTypeQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

ResourceTypeBuffer::ResourceTypeBuffer() :
		AbstractTableBuffer(),
		m_data()
{
}

// ----------------------------------------------------------------------

ResourceTypeBuffer::~ResourceTypeBuffer()
{
	for (DataType::iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

void ResourceTypeBuffer::handleAddResourceTypeMessage(AddResourceTypeMessage const & message)
{
	for (std::vector<AddResourceTypeMessageNamespace::ResourceTypeData>::const_iterator typeData = message.getData().begin(); typeData != message.getData().end(); ++typeData)
	{
		DBSchema::ResourceTypeRow * row = nullptr;
		DataType::iterator rowIter = m_data.find(typeData->m_networkId);
		if (rowIter == m_data.end())
		{
			row = new DBSchema::ResourceTypeRow;
			row->resource_id = typeData->m_networkId;
			m_data.insert(std::make_pair(typeData->m_networkId, row));
		}
		else
			row=rowIter->second;

		row->resource_name = typeData->m_name;
		row->resource_class = typeData->m_parentClass;
		row->depleted_timestamp = typeData->m_depletedTimestamp;

		// Pack attributes:
		{
			std::string packedAttributes;
			for (std::vector<std::pair<std::string, int> >::const_iterator i=typeData->m_attributes.begin(); i!=typeData->m_attributes.end(); ++i)
			{
				char buffer[256];
				snprintf(buffer, 256, "%s %i:", i->first.c_str(), i->second);
				buffer[255]='\0';
				packedAttributes+=buffer;
			}
			if (packedAttributes.size()==0)
				packedAttributes=" ";
			row->attributes = packedAttributes;
		}

		// Pack fractal seeds:
		{
			std::string packedSeeds;
			for (std::vector<std::pair<NetworkId, int> >::const_iterator i=typeData->m_fractalSeeds.begin(); i!=typeData->m_fractalSeeds.end(); ++i)
			{
				char buffer[256];
				snprintf(buffer, 256, "%s %i:", i->first.getValueString().c_str(), i->second);
				buffer[255]='\0';
				packedSeeds+=buffer;
			}
			if (packedSeeds.size()==0)
				packedSeeds=" ";
			row->fractal_seeds = packedSeeds;
		}
	}
}

// ----------------------------------------------------------------------

void ResourceTypeBuffer::sendResourceTypeObjects(GameServerConnection & conn) const
{
	if (m_data.empty())
		return;
	
	std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> resourceTypeDataVector;	
	for (DataType::const_iterator row = m_data.begin(); row != m_data.end(); ++row)
	{
		AddResourceTypeMessageNamespace::ResourceTypeData typeData;

		NOT_NULL(row->second);

		row->second->resource_id.getValue(typeData.m_networkId);
		row->second->resource_name.getValue(typeData.m_name);
		row->second->resource_class.getValue(typeData.m_parentClass);
		row->second->depleted_timestamp.getValue(typeData.m_depletedTimestamp);

		// unpack attributes:
		{
			std::string packedAttributes;
			row->second->attributes.getValue(packedAttributes);
			static const int BUFSIZE = 256;
			char buffer[BUFSIZE];
			char *bufpos = buffer;
			std::string currentAttributeName;
			for (std::string::iterator i = packedAttributes.begin(); i != packedAttributes.end() && bufpos < buffer + BUFSIZE; ++i)
			{
				if ((*i) == ' ')
				{
					*bufpos = '\0';
					currentAttributeName = buffer;

					// remove double quotes around some double quoted resource name
					if (currentAttributeName.find_first_of('"') == 0)
					{
						std::string::size_type const closingQuote = currentAttributeName.find_last_of('"');
						if ((closingQuote >= 2) && (closingQuote == (currentAttributeName.size() - 1)))
							currentAttributeName = currentAttributeName.substr(1, (closingQuote - 1));
					}

					bufpos = buffer;
				}
				else if ((*i) == ':')
				{
					*bufpos = '\0';
					typeData.m_attributes.push_back(std::make_pair(currentAttributeName, atoi(buffer)));
					bufpos = buffer;				
				}
				else
				{
					*(bufpos++) = *i;
				}
			}
		}

		// unpack fractal seeds:
		{
			std::string packedSeeds;
			row->second->fractal_seeds.getValue(packedSeeds);
			static const int BUFSIZE = 256;
			char buffer[BUFSIZE];
			char *bufpos = buffer;
			NetworkId currentPlanetId;
			for (std::string::iterator i = packedSeeds.begin(); i != packedSeeds.end() && bufpos < buffer + BUFSIZE; ++i)
			{
				if ((*i) == ' ')
				{
					*bufpos = '\0';
					currentPlanetId = NetworkId(std::string(buffer));
					bufpos = buffer;
				}
				else if ((*i) == ':')
				{
					*bufpos = '\0';
					typeData.m_fractalSeeds.push_back(std::make_pair(currentPlanetId, atoi(buffer)));
					bufpos = buffer;				
				}
				else
				{
					*(bufpos++) = *i;
				}
			}
		}

		resourceTypeDataVector.push_back(typeData);
	}

	AddResourceTypeMessage msg(resourceTypeDataVector);
	conn.send(msg,true);
}

// ----------------------------------------------------------------------

bool ResourceTypeBuffer::save(DB::Session *session)
{
	if (m_data.empty())
		return true;
	
	int actualSaves=0;
	DBQuery::ResourceTypeQuery qry;
	if (!qry.setupData(session))
		return false;

	for (DataType::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		++actualSaves;
		NOT_NULL(i->second);
		if (!qry.addData(*(i->second)))
			return false;

		if (qry.getNumItems() == ConfigServerDatabase::getDefaultMessageBulkBindSize())
		{	
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();

	LOG("SaveCounts",("Resource types:  %i saved in db",actualSaves));
	return true;
}

// ----------------------------------------------------------------------

bool ResourceTypeBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	int rowsFetched = 0;
	// Resource data is associated with PlanetObjects, so load it if the planets are being loaded
	if (tags.find(ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag)!=tags.end())
	{
		DEBUG_REPORT_LOG(true,("Loading resource types\n"));
		DBQuery::ResourceTypeQuerySelect qry(schema);

		if (! (session->exec(&qry)))
			return false;

		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t const numRows = qry.getNumRowsFetched();
			size_t count = 0;
			std::vector<DBSchema::ResourceTypeRow> const & data = qry.getData();

			for (std::vector<DBSchema::ResourceTypeRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;

				DBSchema::ResourceTypeRow *row=new DBSchema::ResourceTypeRow(*i);
				NOT_NULL(row);
				NetworkId temp;
				row->resource_id.getValue(temp);
				m_data.insert(std::make_pair(row->resource_id.getValue(), row));
			}
		}

		qry.done();
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void ResourceTypeBuffer::removeObject(const NetworkId &object)
{
}

// ======================================================================
