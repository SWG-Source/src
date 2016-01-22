// ======================================================================
//
// ScriptBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ScriptBuffer.h"

#include "serverDatabase/ScriptQueries.h"
#include "sharedDatabaseInterface/DbSession.h"
#include <algorithm>
#include <map>
#include <set>

// ======================================================================

ScriptBuffer::ScriptBuffer() :
		m_loadedData(new LoadedDataType)
{
}

// ----------------------------------------------------------------------

ScriptBuffer::~ScriptBuffer()
{
	delete m_loadedData;
	m_loadedData=0;
}
	
// ----------------------------------------------------------------------

void ScriptBuffer::clearScripts(const NetworkId &objectId)
{
	m_objectsToClear.insert(objectId);
}

// ----------------------------------------------------------------------

bool ScriptBuffer::save(DB::Session *session)
{
	DBQuery::ScriptClearQuery clearQry;
	
	for (std::set<NetworkId>::const_iterator i=m_objectsToClear.begin(); i!=m_objectsToClear.end(); ++i)
	{
		clearQry.setNetworkId(*i);
		if (! (session->exec(&clearQry)))
			return false;
	}

	clearQry.done();
	return true;
}

// ----------------------------------------------------------------------

bool ScriptBuffer::load(DB::Session *session,const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched;
	UNREF(tags); // any object can have scripts
	
	DBQuery::GetScripts qry(schema);

	if (! (session->exec(&qry)))
		return false;

	std::string scriptName;
	while ((rowsFetched = qry.fetch()) > 0)
	{
		size_t numRows = qry.getNumRowsFetched();
		size_t count = 0;
		const std::vector<DBQuery::GetScripts::ScriptRow> &data = qry.getData();

		for (std::vector<DBQuery::GetScripts::ScriptRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
		{
			if (++count > numRows)
				break;
			
			i->script_name.getValue(scriptName);
			(*m_loadedData)[IndexKey(i->object_id.getValue(),i->index.getValue())]=scriptName;
		}
	}
		
	qry.done();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void ScriptBuffer::getScriptsForObject(const NetworkId &objectId, std::vector<std::string> &scripts) const
{
	scripts.clear();

#ifdef _DEBUG
	int expectedIndex = 0;
#endif
	
	for (LoadedDataType::const_iterator i=m_loadedData->find(IndexKey(objectId,0)); (i!=m_loadedData->end()) && (i->first.m_objectId==objectId); ++i)
	{
#ifdef _DEBUG
		DEBUG_WARNING((i->first.m_index != expectedIndex++), ("Programmer bug:  Script buffer was not sorted, or there were gaps in the sequence.  ObjectId %s, sequence number %i\n",objectId.getValueString().c_str(),i->first.m_index));
#endif
		scripts.push_back(i->second);
	}
}

// ----------------------------------------------------------------------

void ScriptBuffer::removeObject(const NetworkId &object)
{
	m_objectsToClear.erase(object);
}

// ======================================================================

ScriptBuffer::IndexKey::IndexKey(const NetworkId &objectId, int index) :
		m_objectId(objectId),
		m_index(index)
{
}

// ----------------------------------------------------------------------

bool ScriptBuffer::IndexKey::operator<(const IndexKey &rhs) const
{
	if (m_objectId == rhs.m_objectId)
		return (m_index < rhs.m_index);
	else
		return (m_objectId < rhs.m_objectId);
}

// ======================================================================
