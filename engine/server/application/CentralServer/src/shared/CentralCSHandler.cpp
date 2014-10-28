// CentralCSHandler.cpp

#include "FirstCentralServer.h"
#include "CentralCSHandler.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/CSToolRequest.h"
#include "serverNetworkMessages/CSToolResponse.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "serverNetworkMessages/StructureListMessage.h"

// defines

#define REGISTER_CS_CMD(_name_, _type_) \
smp_instance->m_entries[ #_name_ ] = new HandlerEntry( #_name_, CentralCSHandler::HandlerEntry::_type_ );

#define REGISTER_HANDLED_CMD(_name_, _type_ ) \
smp_instance->m_entries[ #_name_ ] = new HandlerEntry( #_name_, &CentralCSHandler::handle_##_name_, CentralCSHandler::HandlerEntry::_type_ );


#define CS_CMD(_name_) void CentralCSHandler::handle_##_name_( GameServerCSRequestMessage& request )
// statics

CentralCSHandler * CentralCSHandler::smp_instance = 0;

void CentralCSHandler::install()
{

	smp_instance = new CentralCSHandler();
	REGISTER_CS_CMD( get_characters, TYPE_DB );
	REGISTER_CS_CMD( get_pc_info, TYPE_GAME_SERVER);
	REGISTER_CS_CMD( undelete_item, TYPE_ARBITRARY_GAME_SERVER );
	REGISTER_CS_CMD( get_containment_info, TYPE_DB );
	REGISTER_CS_CMD( get_deleted_items, TYPE_DB );

	REGISTER_CS_CMD( test, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( set_bank_credits, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( check_args, TYPE_GAME_SERVER );

	REGISTER_CS_CMD( rename_player, TYPE_GAME_SERVER );

	REGISTER_CS_CMD( list_objvars, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( set_objvar, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( add_objvar, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( remove_objvar, TYPE_GAME_SERVER );

	REGISTER_CS_CMD( dump_info, TYPE_GAME_SERVER );

	REGISTER_CS_CMD( get_player_id, TYPE_DB );

	REGISTER_CS_CMD( move_object, TYPE_GAME_SERVER );
	REGISTER_CS_CMD( create_crafted_object, TYPE_GAME_SERVER );

	REGISTER_CS_CMD( list_structures, TYPE_DB );

	REGISTER_HANDLED_CMD( login_character, TYPE_CONDITIONAL_CENTRAL );
	REGISTER_CS_CMD( warp_player, TYPE_CONDITIONAL );

}

void CentralCSHandler::remove()
{
	NOT_NULL(smp_instance);
	delete smp_instance;
}

void CentralCSHandler::handleStructureListResponse( StructureListMessage& msg )
{
	std::string response;
	response = "structurelist\r\n";
	response += "character_id:" + msg.getCharacterId().getValueString() + "\r\n";

	std::vector<StructureListMessageData> data = msg.getData();

	char buf[512];
	for( unsigned i = 0; i < data.size(); ++i )
	{
		StructureListMessageData &row = data[ i ];
		snprintf(buf,512,"structure-%u:(%s)%s %s(%.02f %.02f %.02f)%ld\r\n", i,
							       row.m_structureId.getValueString().c_str(),
							       row.m_objectTemplate.c_str(),
							       row.m_location.c_str(),
							       row.m_coordinates.x,
							       row.m_coordinates.y,
							       row.m_coordinates.z,
							       row.m_deleted );

		response += buf;
	}

	// build the response message.

	CSToolResponse rmsg(0, response, msg.getToolId());
	CentralServer::getInstance().sendToLoginServer(msg.getLoginServerId(), rmsg);
}


void CentralCSHandler::handleFindObjectResponse( int iIndex, bool bFound )
{
	// try to find the entry.
	CentralCharFindMap::iterator it = m_findMap.find( iIndex );

	// if it's there...
	if( it != m_findMap.end() )
	{
		CSCharacterFindInfo &info = *( it->second );
		// if we found the character...
		if( bFound )
		{
			// send the request to the appropriate game server and delete the entry
			// make a new request
			GameServerCSRequestMessage gsreq( info.iAccount,
					info.commandLine,
					info.iToolId,
					info.iAccessLevel,
					info.iLoginServerId,
					info.user,
					info.command );

			// spam it, in case the object has moved servers in between.
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess( gsreq, true );

			delete it->second;

			m_findMap.erase( it );

			return;
		}

		// otherwise, decrement the counter.

		info.responsesWaiting--;

		// if the counter is <= 0,
		if( info.responsesWaiting <= 0 )
		{
			// send the request to the DB or the appropriate central handler.
			// make a new request
			if( info.bCentral )
			{
				// find the handler.
				CentralCSHandlerMap::iterator it = m_entries.find( info.command );

				// if we're here, we should have an entry, and it should be of
				// TYPE_CONDITIONAL_CENTRAL.  Still, better safe than sorry.
				if( it != m_entries.end() )
				{
					HandlerEntry & entry = *( it->second );

					if( entry.type == HandlerEntry::TYPE_CONDITIONAL_CENTRAL )
					{
						// make a CSToolRequest
						GameServerCSRequestMessage gsreq( info.iAccount,
								info.commandLine,
								info.iToolId,
								info.iAccessLevel,
								info.iLoginServerId,
								info.user,
								info.command );

						CentralCSHandlerFunc func = entry.func;
						( this->*func )( gsreq );
					}
				}
				// execute.
			}
			else
			{
				// fire to the DB.
				DBCSRequestMessage dbreq( info.iAccount,
							  info.iAccessLevel,
							  info.iToolId,
							  info.iLoginServerId,
							  info.command,
							  info.commandLine );
				CentralServer::getInstance().sendToDBProcess(dbreq, true);
			}

			delete it->second;

			m_findMap.erase( it );
		}
	}
}

void CentralCSHandler::handle( const CSToolRequest & req, uint32 loginServerId )
{
	CentralCSHandlerMap::iterator it = m_entries.find( req.getCommandName() );
	int type = -1;
	if( it != m_entries.end() )
	{
		HandlerEntry & entry = *( it->second );
		type = entry.type;
	}


	switch( type )
	{
	case HandlerEntry::TYPE_DB:
		{
			// send to DB
			DBCSRequestMessage dbreq( req.getAccountId(),
						  req.getAccessLevel(),
						  req.getToolId(),
						  loginServerId,
						  req.getCommandName(),
						  req.getCommandString() );
			CentralServer::getInstance().sendToDBProcess( dbreq, true );
		}
		break;



	case HandlerEntry::TYPE_CENTRAL:
		{
			// handle it here.
			if( it->second->func != 0 )
			{
				GameServerCSRequestMessage gsreq( req.getAccountId(),
					req.getCommandString(),
					req.getToolId(),
					req.getAccessLevel(),
					loginServerId,
					req.getUserName(),
					req.getCommandName() );
				CentralCSHandlerFunc func = it->second->func;
				( this->*func )( gsreq );
			}
		}
		break;
	case  HandlerEntry::TYPE_CONDITIONAL:
	case  HandlerEntry::TYPE_CONDITIONAL_CENTRAL:
		{
			static int sequence = 0;
			// don't check twice.  If we've already got one, that's fine.
			// get the network id out of our command string.  It should be the second word.
			std::string::size_type pos;
			std::string::size_type lastpos;
			pos = req.getCommandString().find(" ");
			if(pos == std::string::npos)
				return;
			lastpos = req.getCommandString().find(" ", pos + 1);

			std::string idstring;
			if(lastpos == std::string::npos)
				idstring = req.getCommandString().substr(pos);
			else
				idstring = req.getCommandString().substr(pos, lastpos - pos);

			NetworkId id( idstring );
			if( m_findMap.find( sequence ) != m_findMap.end() )
			{
				return;
			}

			GameServerCSRequestMessage gsreq( req.getAccountId(),
					req.getCommandString(),
					req.getToolId(),
					req.getAccessLevel(),
					loginServerId,
					req.getUserName(),
					req.getCommandName() );

			// queue up some messages to find the characters.
			GenericValueTypeMessage<std::pair<NetworkId, unsigned int> > const msg("CSFindAuthObject", std::make_pair(id, sequence));
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess( msg, true );

			CentralCSHandler::CSCharacterFindInfo * p_info = new CSCharacterFindInfo( id, CentralServer::getInstance().getNumGameServers(), gsreq, ( type == HandlerEntry::TYPE_CONDITIONAL_CENTRAL ) );

			m_findMap[ sequence ] = p_info;
			sequence++;
		}
	break;
	case HandlerEntry::TYPE_ARBITRARY_GAME_SERVER:
		{
			GameServerCSRequestMessage gsreq( req.getAccountId(),
							  req.getCommandString(),
							  req.getToolId(),
							  req.getAccessLevel(),
							  loginServerId,
							  req.getUserName(),
							  req.getCommandName() );
			CentralServer::getInstance().sendToRandomGameServer( gsreq );
		}
	break;
	case HandlerEntry::TYPE_GAME_SERVER:
	default:
		{
			// spam to game servers (except DB)
			GameServerCSRequestMessage gsreq( req.getAccountId(),
							  req.getCommandString(),
							  req.getToolId(),
							  req.getAccessLevel(),
							  loginServerId,
							  req.getUserName(),
							  req.getCommandName() );
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess( gsreq, true );
		}
		break;
	}
}

CentralCSHandler & CentralCSHandler::getInstance()
{
	NOT_NULL(smp_instance);
	return *smp_instance;
}

CentralCSHandler::~CentralCSHandler()
{
	for( CentralCSHandlerMap::iterator it = m_entries.begin();
	     it != m_entries.end();
	     ++it )
	{
		delete it->second;
	}
}

CS_CMD( login_character )
{
	const unsigned &pos = request.getCommandString().find(" ");
	std::string id;
	std::string account;

	if(pos == std::string::npos)
		return;
	const unsigned &pos2 = request.getCommandString().find(" ", pos + 1 );
	id = request.getCommandString().substr(pos, pos2 - pos);
	account = request.getCommandString().substr(pos2 + 1);

	// TODO:  Find a way to tell if the player is logged into a different
	// character on the same account.

	TransferCharacterData data(TransferRequestMoveValidation::TRS_transfer_server);
	data.setCharacterId(NetworkId(id));
	data.setSourceStationId(atoi(account.c_str()));
	data.setCSToolId(request.getToolId());
	const GenericValueTypeMessage<TransferCharacterData> loginCharacter("TransferGetLoginLocationData", data);
	CentralServer::getInstance().sendToDBProcess( loginCharacter, true );
}
