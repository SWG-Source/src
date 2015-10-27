// TaskGetStructures.cpp
// copyright 2001 Verant Interactive
// Author: Calan Thurow

//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "TaskGetStructures.h"

#include "Unicode.h"
#include "serverDatabase/StructureQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/StructureListMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

TaskGetStructures::TaskGetStructures(NetworkId & characterId,
				     uint32 toolId,
				     uint32 loginServerId) :
	TaskRequest(),
	m_characterId(characterId),
	m_toolId(toolId),
	m_loginServerId(loginServerId)
{											
}
	
//-----------------------------------------------------------------------

TaskGetStructures::~TaskGetStructures()
{
	for (StructuresType::iterator i=structures.begin(); i!=structures.end(); ++i)
	{
		delete *i;
	}
}

//-----------------------------------------------------------------------

bool TaskGetStructures::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetStructures qry;
	
	qry.setCharacterId(m_characterId); //lint !e713 //loss of precision ok //@todo check this

	if (! (session->exec(&qry)))
		return false;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		StructureRec *cr=new StructureRec;

		cr->structureId = NetworkId(qry.getData().object_id.getValue());
		cr->location = Unicode::wideToNarrow(qry.getData().scene_id.getValue());
		cr->objectTemplate = Unicode::wideToNarrow(qry.getData().object_template.getValue());
		cr->x = static_cast<float>(qry.getData().x.getValue());
		cr->y = static_cast<float>(qry.getData().y.getValue());
		cr->z = static_cast<float>(qry.getData().z.getValue());
		cr->deleted = qry.getData().deleted.getValue();
		structures.push_back(cr);
	}
	
	qry.done();
	return (rowsFetched >= 0);
}

//-----------------------------------------------------------------------

void TaskGetStructures::onComplete()
{
	std::vector<StructureListMessageData> structurelist;
	for(StructuresType::iterator i = structures.begin(); i != structures.end(); ++i)
	{
		structurelist.push_back(StructureListMessageData((*i)->objectTemplate, (*i)->structureId, (*i)->location, Vector((*i)->x,(*i)->y,(*i)->z),(*i)->deleted));
	}
	StructureListMessage msg(m_toolId, m_loginServerId, m_characterId, structurelist);
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
}

//-----------------------------------------------------------------------
