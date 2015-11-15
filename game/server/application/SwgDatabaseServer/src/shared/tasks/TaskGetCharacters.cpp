// TaskGetCharacters.cpp
// copyright 2001 Verant Interactive
// Author: Calan Thurow

//-----------------------------------------------------------------------

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskGetCharacters.h"

#include "Unicode.h"
#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/CharacterListMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

TaskGetCharacters::TaskGetCharacters(uint32 stationId) :
	TaskRequest(),
	m_stationId(stationId)
{											
}
	
//-----------------------------------------------------------------------

TaskGetCharacters::~TaskGetCharacters()
{
	for (CharactersType::iterator i=characters.begin(); i!=characters.end(); ++i)
	{
		delete *i;
	}
}

//-----------------------------------------------------------------------

bool TaskGetCharacters::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetCharacters qry;
	
	qry.setStationId(m_stationId); //lint !e713 //loss of precision ok //@todo check this

	if (! (session->exec(&qry)))
		return false;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		CharacterRec *cr=new CharacterRec;

		cr->characterName = qry.getData().character_name.getValue();
		cr->characterId = NetworkId(qry.getData().object_id.getValue());
		cr->location = Unicode::wideToNarrow(qry.getData().scene_id.getValue());
		cr->objectTemplate = Unicode::wideToNarrow(qry.getData().object_template.getValue());
		cr->containerId = qry.getData().container_id.getValue();
		cr->x = static_cast<float>(qry.getData().x.getValue());
		cr->y = static_cast<float>(qry.getData().y.getValue());
		cr->z = static_cast<float>(qry.getData().z.getValue());
		characters.push_back(cr);
	}
	
	qry.done();
	return (rowsFetched >= 0);
}

//-----------------------------------------------------------------------

void TaskGetCharacters::onComplete()
{
	std::vector<CharacterListMessageData> charlist;
	for(CharactersType::iterator i = characters.begin(); i != characters.end(); ++i)
	{
		charlist.push_back(CharacterListMessageData((*i)->characterName, (*i)->objectTemplate, (*i)->characterId, (*i)->containerId, (*i)->location, Vector((*i)->x,(*i)->y,(*i)->z)));
	}
	CharacterListMessage msg(m_stationId, charlist);
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
}

//-----------------------------------------------------------------------
