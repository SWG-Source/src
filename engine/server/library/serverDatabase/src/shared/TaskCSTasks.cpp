// ======================================================================
//
// TaskVerifyCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "TaskCSTasks.h"

#include "Unicode.h"
#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/CharacterListMessage.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/ValidateCharacterForLoginReplyMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedLog/Log.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

TaskGetCharactersForAccount::TaskGetCharactersForAccount( uint32 accountId, uint32 loginServerId, uint32 toolId) :
		TaskRequest(),
		m_loginServerId( loginServerId ),
		m_toolId( toolId ),
		m_accountId( accountId )
{
	LOG("TRACE_LOGIN", ("Create DB TaskGetCharactersForAccount"));
}
	
//-----------------------------------------------------------------------

TaskGetCharactersForAccount::~TaskGetCharactersForAccount()
{
}

//-----------------------------------------------------------------------

bool TaskGetCharactersForAccount::process(DB::Session *session)
{
	LOG("TRACE_LOGIN", ("TaskVerifyCharacter -- starting process"));
	
	int rowsFetched = 0;
	{
		// add query info here.
		CSGetCharactersQuery qry;
		DEBUG_REPORT_LOG( true, ( "Sending query for sid %lu\n", m_accountId ) );
		qry.station_id = m_accountId; // debug, my station id.  Should have a char on here, hopefully!
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<CSGetCharactersQuery::CharacterRow> &data = qry.getData();

			for (std::vector<CSGetCharactersQuery::CharacterRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;
				// store
				CharacterRecord* rec = new CharacterRecord();
				rec->m_characterId = i->characterId.getValue();
				rec->m_characterName = i->characterName.getValue();
				m_characters.push_back( rec );			
			}
		}
		qry.done();
	}

	LOG("TRACE_LOGIN", ("TaskVerifyCharacter -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskGetCharactersForAccount::onComplete()
{
//	REPORT_LOG(true, ("TaskGetCharactersForAccount::onComplete()\n"));
	std::string response = "";
	for (std::vector<CharacterRecord*>::iterator i=m_characters.begin(); i!=m_characters.end(); ++i)
	{

		response = response + (*i)->m_characterId.getValueString() + ":" + Unicode::wideToNarrow( (*i)->m_characterName ) + "\r\n";
		
		// store variable amounts of data...
		delete *i;
	}
	m_characters.clear();
	GetCharactersForAccountCSReplyMsg msg( 17619, 0, 17619, m_toolId, m_loginServerId, response );
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
	
//	dynamic_cast<SwgLoader&>(SwgLoader::getInstance()).verifyCharacterFinished(this);
}

// ======================================================================

void TaskGetCharactersForAccount::CSGetCharactersQuery::getSQL(std::string &sql)
{
	// Note:  station id and character id are input parameters.  The rest are pass-by-reference output parameters.
	sql=std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier()+"custserv_procs.get_characters_for_account (:station_id ); end;";
}

// ----------------------------------------------------------------------

bool TaskGetCharactersForAccount::CSGetCharactersQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskGetCharactersForAccount::CSGetCharactersQuery::bindColumns()
{
	if( m_data.size() < 2 )
		m_data.resize( 2 );
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());
	if (!bindCol(m_data[0].characterId)) return false;
	if (!bindCol(m_data[0].characterName)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetCharactersForAccount::CSGetCharactersQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

TaskGetCharactersForAccount::CSGetCharactersQuery::CSGetCharactersQuery() :
		Query(),
		station_id(),
		character_id(),
		character_name()
{
}

// ----------------------------------------------------------------------
//



TaskGetDeletedItems::TaskGetDeletedItems(const NetworkId &characterId, uint32 loginServerId, uint32 toolId, uint32 pageNumber) :
		TaskRequest(),
		m_loginServerId( loginServerId ),
		m_toolId( toolId ),
		m_characterId( characterId ),
		m_pageNumber( pageNumber )
{
	LOG("TRACE_LOGIN", ("Create DB TaskGetCharactersForAccount"));
}
	
//-----------------------------------------------------------------------

TaskGetDeletedItems::~TaskGetDeletedItems()
{
}

//-----------------------------------------------------------------------

bool TaskGetDeletedItems::process(DB::Session *session)
{
	LOG("TRACE_LOGIN", ("TaskVerifyCharacter -- starting process"));
	
	int rowsFetched = 0;
	{
		// add query info here.
		CSGetDeletedItemsQuery qry;
		DEBUG_REPORT_LOG( true, ( "Sending query for sid %s\n", m_characterId.getValueString().c_str() ) );
		qry.character_id = m_characterId;
		qry.page_number = m_pageNumber;
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<CSGetDeletedItemsQuery::DeletedItemRow> &data = qry.getData();

			for (std::vector<CSGetDeletedItemsQuery::DeletedItemRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;
				// store
				DeletedItemRecord* rec = new DeletedItemRecord();
				rec->m_itemId = i->itemId.getValue();
				rec->m_itemTable = i->itemTable.getValue();
				rec->m_itemName= i->itemName.getValue();
				rec->m_displayName = i->displayName.getValue();
				m_items.push_back( rec );			
			}
		}
		qry.done();
	}

	LOG("TRACE_CS_DB", ("TaskGetDeletedItems -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskGetDeletedItems::onComplete()
{
	LOG("TRACE_CS_DB", ("TaskGetDeletedItems::onComplete() for account %s", m_characterId.getValueString().c_str()));
	std::string response = "";
	for (std::vector<DeletedItemRecord*>::iterator i=m_items.begin(); i!=m_items.end(); ++i)
	{
		LOG("TRACE_CS_DB", ("TaskGetDeletedItems::onComplete() reading record"));
		
		response = response + (*i)->m_itemId.getValueString() + ":" + Unicode::wideToNarrow( (*i)->m_itemTable ) + ":" + Unicode::wideToNarrow( (*i)->m_itemName ) +
			"(" + Unicode::wideToNarrow((*i)->m_displayName) + ")\r\n";
		
		// store variable amounts of data...
		delete *i;
	}
	m_items.clear();
	
	GetDeletedItemsReplyMessage msg( 0, 0, m_characterId, m_toolId, m_loginServerId, response );
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
	LOG("TRACE_CS_DB", ("TaskGetDeletedItems::onComplete() finished"));
//	dynamic_cast<SwgLoader&>(SwgLoader::getInstance()).verifyCharacterFinished(this);
}

// ======================================================================

void TaskGetDeletedItems::CSGetDeletedItemsQuery::getSQL(std::string &sql)
{
	// Note:  station id and character id are input parameters.  The rest are pass-by-reference output parameters.
	sql=std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier()+"custserv_procs.get_deleted_items(:character_id, :page_number ); end;";
}

// ----------------------------------------------------------------------

bool TaskGetDeletedItems::CSGetDeletedItemsQuery::bindParameters()
{
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(page_number)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskGetDeletedItems::CSGetDeletedItemsQuery::bindColumns()
{
	if( m_data.size() < 2 )
		m_data.resize( 2 );
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());
	if (!bindCol(m_data[0].itemId)) return false;
	if (!bindCol( m_data[ 0 ].itemTable )) return false;
	if (!bindCol(m_data[0].itemName)) return false;
	if (!bindCol(m_data[0].displayName)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetDeletedItems::CSGetDeletedItemsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

TaskGetDeletedItems::CSGetDeletedItemsQuery::CSGetDeletedItemsQuery() :
		Query(),
		character_id()
{
}

//----------------------------------------------------------------------




TaskGetCharacterId::TaskGetCharacterId(const std::string &characterName, uint32 loginServerId, uint32 toolId) :
		TaskRequest(),
		m_loginServerId( loginServerId ),
		m_toolId( toolId ),
		m_characterName( characterName )
{
	LOG("TRACE_LOGIN", ("Create DB TaskGetCharacterId"));
}
	
//-----------------------------------------------------------------------

TaskGetCharacterId::~TaskGetCharacterId()
{
}

//-----------------------------------------------------------------------

bool TaskGetCharacterId::process(DB::Session *session)
{
	LOG("TRACE_LOGIN", ("TaskGetCharacterId -- starting process"));
	
	int rowsFetched = 0;
	{
		// add query info here.
		CSGetCharacterIdQuery qry;
		DEBUG_REPORT_LOG( true, ( "Sending query for sid %s\n", m_characterName.c_str() ) );
		std::string temporaryName = m_characterName;
		if (temporaryName.length() > TaskGetCharacterId::MAX_NAME_LENGTH - 1)
		{
			temporaryName = temporaryName.substr(0, TaskGetCharacterId::MAX_NAME_LENGTH - 1);
		}
		qry.character_name = temporaryName;
		if (! (session->exec(&qry)))
			return false;
		while ((rowsFetched = qry.fetch()) > 0)
		{
			size_t numRows = qry.getNumRowsFetched();
			size_t count = 0;
			const std::vector<CSGetCharacterIdQuery::CharacterIdRow> &data = qry.getData();

			for (std::vector<CSGetCharacterIdQuery::CharacterIdRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
			{
				if (++count > numRows)
					break;
				// store
				CharacterIdRecord* rec = new CharacterIdRecord();
				rec->m_characterId = i->characterId.getValue();
				rec->m_stationId = i->stationId.getValue();
				m_characterIds.push_back( rec );			
			}
		}
		qry.done();
	}

	LOG("TRACE_LOGIN", ("TaskGetCharacterId -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskGetCharacterId::onComplete()
{
//	REPORT_LOG(true, ("TaskGetCharactersForAccount::onComplete()\n"));
	NetworkId characterId;
	uint32 stationId = 0;
	if(m_characterIds.size() > 1 )
	{
		DEBUG_REPORT_LOG(true,("Got more than one result looking up a character named %s\n", m_characterName.c_str()));
	}
	for (std::vector<CharacterIdRecord*>::iterator i=m_characterIds.begin(); i!=m_characterIds.end(); ++i)
	{
		characterId = (*i)->m_characterId;
		stationId = (*i)->m_stationId;
		delete *i;
	}
	m_characterIds.clear();
	
	GetCharacterIdReplyMessage msg( stationId, characterId, m_toolId, m_loginServerId, m_characterName );
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
	DEBUG_REPORT_LOG( true, ( "Finished deleted item query\n" ) );
//	dynamic_cast<SwgLoader&>(SwgLoader::getInstance()).verifyCharacterFinished(this);
}

// ======================================================================

void TaskGetCharacterId::CSGetCharacterIdQuery::getSQL(std::string &sql)
{
	// Note:  station id and character id are input parameters.  The rest are pass-by-reference output parameters.
	sql=std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier()+"custserv_procs.get_player_id(:character_name); end;";
}

// ----------------------------------------------------------------------

bool TaskGetCharacterId::CSGetCharacterIdQuery::bindParameters()
{
	if (!bindParameter(character_name)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskGetCharacterId::CSGetCharacterIdQuery::bindColumns()
{
	if( m_data.size() < 2 )
		m_data.resize( 2 );
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());
	if (!bindCol(m_data[0].characterId)) return false;
	if (!bindCol( m_data[ 0 ].stationId )) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetCharacterId::CSGetCharacterIdQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

TaskGetCharacterId::CSGetCharacterIdQuery::CSGetCharacterIdQuery() :
		Query(),
		character_name()
{
}

TaskMovePlayer::TaskMovePlayer( const NetworkId & id, const std::string & scene, double x, double y, double z ) :
	TaskRequest(),
	m_characterId(id),
	m_scene(scene),
	m_x(x),
	m_y(y),
	m_z(z)
{
}

//-----------------------------------------------------------------------

TaskMovePlayer::~TaskMovePlayer()
{
}

//-----------------------------------------------------------------------

bool TaskMovePlayer::process(DB::Session *session)
{
	
	{
		// add query info here.
		std::string temporaryString = m_scene;
		if (m_scene.length() > TaskMovePlayer::MAX_SCENE_NAME_LENGTH - 1)
		{
			temporaryString = temporaryString.substr(0, TaskMovePlayer::MAX_SCENE_NAME_LENGTH - 1);
		}
		CSMovePlayerQuery qry(m_characterId, temporaryString, static_cast<float>(m_x), static_cast<float>(m_y), static_cast<float>(m_z));
		if (! (session->exec(&qry)))
		{
			return false;
		}
		qry.done();
	}

	LOG("TRACE_LOGIN", ("TaskMovePlayer -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskMovePlayer::onComplete()
{
}
//----------------------------------------------------------------------

TaskMovePlayer::CSMovePlayerQuery::CSMovePlayerQuery(const NetworkId & characterIdIn,
				     const std:: string & scene,
				     float xIn,
				     float yIn,
				     float zIn) :
		characterId(characterIdIn),
		newScene(scene),
		x(xIn),
		y(yIn),
		z(zIn)
{	
}

// ======================================================================

void TaskMovePlayer::CSMovePlayerQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "custserv_procs.move_player (:character_id, :x, :y, :z, :scene_id); end;";
}

// ======================================================================

bool TaskMovePlayer::CSMovePlayerQuery::bindParameters()
{
	if (!bindParameter(characterId)) return false;
	if (!bindParameter(x)) return false;
	if (!bindParameter(y)) return false;
	if (!bindParameter(z)) return false;
	if (!bindParameter(newScene)) return false;
	return true;
}

// ======================================================================

bool TaskMovePlayer::CSMovePlayerQuery::bindColumns()
{
	return true;
}

// ======================================================================

// ----------------------------------------------------------------------


TaskCsUndeleteItem::TaskCsUndeleteItem(const NetworkId & characterId, const NetworkId & itemId, bool move) :
	TaskRequest(),
	m_characterId(characterId),
	m_itemId(itemId),
	m_move(move)
{
}

//-----------------------------------------------------------------------

TaskCsUndeleteItem::~TaskCsUndeleteItem()
{
}

//-----------------------------------------------------------------------

bool TaskCsUndeleteItem::process(DB::Session *session)
{
	
	{
		// add query info here.
		CsUndeleteItemQuery qry(m_characterId, m_itemId, m_move);
		if (! (session->exec(&qry)))
		{
			return false;
		}
		qry.done();
	}

	LOG("TRACE_LOGIN", ("CsUndeleteItemQuery -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskCsUndeleteItem::onComplete()
{
}
//----------------------------------------------------------------------

TaskCsUndeleteItem::CsUndeleteItemQuery::CsUndeleteItemQuery(const NetworkId & characterIdIn,
				     const NetworkId & itemIdIn,
				     bool moveIn):
		characterId(characterIdIn),
		itemId(itemIdIn),
		move(0)
{
	if(moveIn)
		move.setValue(1);
	DEBUG_REPORT_LOG(true,("Undeleltion query: %s %s %s\n", characterIdIn.getValueString().c_str(), itemIdIn.getValueString().c_str(), moveIn ? "Moving" : "Not moving"));
}

// ======================================================================

void TaskCsUndeleteItem::CsUndeleteItemQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "custserv_procs.undelete_item (:character_id, :item_id, :move); end;";
}

// ======================================================================

bool TaskCsUndeleteItem::CsUndeleteItemQuery::bindParameters()
{
	if (!bindParameter(characterId)) return false;
	if (!bindParameter(itemId)) return false;
	if (!bindParameter(move)) return false;
	return true;
}

// ======================================================================

bool TaskCsUndeleteItem::CsUndeleteItemQuery::bindColumns()
{
	return true;
}
//----------------------------------------------------------------------

