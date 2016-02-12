// CSHandler.cpp
//--------------------------
// Copyright 2005 Sony Online Entertainment

#include "FirstSwgGameServer.h"
#include "CSHandler.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerCreationManagerServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "SwgGameServer.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

#define REGISTER_CS_CMD(_name_,_access_) \
	CSHandlerNamespace::cmdMap[ #_name_ ] = CSHandlerEntry( #_name_, _access_, &CSHandler::handle_##_name_ );

#define CS_CMD(_name_) void CSHandler::handle_##_name_( CSHandlerRequest & request, GameServerCSResponseMessage & msg )

namespace CSHandlerNamespace
{
	const int smallBufferSize = 64;
	const int largeBufferSize = 256;

	typedef std::map< std::string, std::string > CSLogData;
	typedef std::vector< std::string > CSArgs;
	std::map< std::string, CSHandlerEntry > cmdMap;

	// from CommandCppFuncs.cpp
	ShipObject const *getFirstPackedShipForCreature(CreatureObject const &creature)
	{
		ServerObject const * const datapad = creature.getDatapad();
		if (datapad)
		{
			Container const * const container = ContainerInterface::getContainer(*datapad);
			if (container)
			{
				for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
				{
					ServerObject const * const content = safe_cast<ServerObject const *>((*i).getObject());
					if (   content
						&& content->getGameObjectType() == SharedObjectTemplate::GOT_data_ship_control_device
						&& !content->isBeingDestroyed())
					{
						Container const * const scdContainer = ContainerInterface::getContainer(*content);
						if (scdContainer)
						{
							for (ContainerConstIterator j = scdContainer->begin(); j != scdContainer->end(); ++j)
							{
								ServerObject const * const scdContent = safe_cast<ServerObject const *>((*j).getObject());
								if (scdContent && !scdContent->isBeingDestroyed() && scdContent->asShipObject())
									return scdContent->asShipObject();
							}
						}
					}
				}
			}
		}
		return 0;
	}

	std::string getOneArg( std::string input, int position)
	{
		unsigned pos; // where we stop looking
		unsigned lastpos;  // the last character in our argument.
		// bounds checking.
		if( position < 0 || position >= (int) input.length() )
		{
			return "";
		}
		// skip whitespace
		while( input[ position ] == ' ' && position < (int) input.length() )
		{
			++position;
		}

		if( position == (int) input.length() )
			return "";
		// see if the first character is a quote.
		if( input[ position ] == '"' )
		{
			// if it is...
			// skip over the quote.
			pos = ++position;
			// look for the next quote.
			while( 1 )
			{
				pos = input.find( '"', pos );
				// if we don't find one, assume the end of the line.
				if( pos == std::string::npos )
				{
					pos = input.length();
					lastpos = pos - 1;
					break;
				}
				else
				{
					// and verify that it's not preceded by a backslash.
					if( input[ pos - 1 ] != '\\' )
					{
						lastpos = pos;
						break;
					}
					else
					{
						pos++;
					}
				}
			}
		}
		else
		{
			// otherwise, just look for a space.
			pos = input.find( ' ', position );
			// if we don't find a space, assume the end of the line.
			if( pos == std::string::npos )
			{
				pos = input.length();
			}
			lastpos = pos;
		}


		// build the string.
		std::string output = input.substr( position, lastpos - position );

		// modify the position.  Assume that the caller will do bounds checking.
		position = pos + 1;
		return output;

	}

	void getArgs( std::string input, std::vector< std::string > & output )
	{
		unsigned pos = 0;
		while ( pos < input.length() )
		{
			std::string temp = getOneArg( input, pos );
			output.push_back( temp );
		}
	}
	void log( CSLogData & data )
	{
		std::string logline = "";
		bool b_first = true;
		for( CSLogData::iterator it = data.begin(); it != data.end(); ++it )
		{
			if( b_first )
			{
				b_first = false;
			}
			else
			{
				logline += ";";
			}
			logline = logline + it->first + "=" + it->second;
		}
		LOG( "CustomerService", ( "CSTool: %s", logline.c_str() ) );
	}

	void log( CSHandlerRequest &req, CSLogData &data )
	{
		// add the additional fields and pass to log.
		data[ "user" ] = req.m_name;
		log( data );
	}

	void addObjVarToString( std::string & response,
		const std::string & prefix,
		DynamicVariableList::NestedList::const_iterator &objVar )
	{
		char buffer[ smallBufferSize ];

		// print the data
		switch (objVar.getType())
		{
		case DynamicVariable::INT:
			{
				int value=0;
				objVar.getValue(value);
				IGNORE_RETURN(_itoa(value, buffer, 10));
				// print the name
				response += prefix + objVar.getName();
				response += ":";
				response += buffer;
				response += "\r\n";
			}
			break;
		case DynamicVariable::INT_ARRAY:
			{
				std::vector<int> value;
				objVar.getValue(value);
				std::string text = "[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					IGNORE_RETURN(_itoa(value[j], buffer, 10));
					text += buffer;
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\r\n";
				// print the name
				response += prefix + objVar.getName();
				response += ":";
				response += text;
			}
			break;
		case DynamicVariable::REAL:
			{
				real value = 0;
				objVar.getValue(value);
				snprintf(buffer, sizeof( buffer ) -1 , "%f\r\n", value);
				buffer[ sizeof( buffer ) -1 ] = 0;
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				response += buffer;
			}
			break;
		case DynamicVariable::REAL_ARRAY:
			{
				std::vector<real> value;
				objVar.getValue(value);
				std::string text = "[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					snprintf(buffer, sizeof( buffer ) - 1, "%f", value[j]);
					buffer[ sizeof( buffer ) - 1 ] = 0;
					text += buffer;
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\n";
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				response += text;
			}
			break;
		case DynamicVariable::STRING:
			{
				Unicode::String value;
				objVar.getValue(value);
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				response += Unicode::wideToNarrow( value );
				response += "\r\n";
			}
			break;
		case DynamicVariable::STRING_ARRAY:
			{
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				std::vector<Unicode::String> value;
				objVar.getValue(value);
				response += "[[\r\n";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					response += Unicode::wideToNarrow( value[j] );
					response += "\r\n";
				}
				response += "]]\r\n";
			}
			break;
		case DynamicVariable::NETWORK_ID:
			{
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				NetworkId value;
				objVar.getValue(value);
				response += "(NetworkId)";
				response += value.getValueString();
				response += "\r\n";
			}
			break;
		case DynamicVariable::NETWORK_ID_ARRAY:
			{
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				std::vector<NetworkId> value;
				objVar.getValue(value);
				std::string text = "(NetworkId)[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					text += value[j].getValueString();
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\r\n";
				response += text;
			}
			break;
		case DynamicVariable::LOCATION:
			{
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				DynamicVariableLocationData value;
				objVar.getValue(value);
				response += "(x=";
				snprintf(buffer, sizeof( buffer ) -1 , "%f", value.pos.x);
				buffer[ sizeof( buffer ) - 1 ] = 0;
				response += buffer;
				response += ", y=";
				snprintf(buffer, sizeof( buffer ) - 1, "%f", value.pos.y);
				buffer[ sizeof( buffer ) - 1 ] = 0;
				response += buffer;
				response += ", z=";
				snprintf(buffer, sizeof( buffer ) - 1, "%f", value.pos.z);
				buffer[ sizeof( buffer ) - 1 ] = 0;
				response += buffer;
				response += ", scene=";
				response += value.scene;
				response += ", cell=";
				response += value.cell.getValueString();
				response += ")\r\n";
			}
			break;
		case DynamicVariable::LOCATION_ARRAY:
			{
				// print the name
				response += prefix + objVar.getName();
				response += ":";

				std::vector<DynamicVariableLocationData> value;
				objVar.getValue(value);
				int k;
				int nameLength = objVar.getName().size();
				response += "[\r\n";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					for (k = 0; k < nameLength; ++k)
						response += " ";
					response += "(x=";
					snprintf(buffer, sizeof( buffer ) - 1, "%f", value[ j ].pos.x);
					buffer[ sizeof( buffer ) - 1 ] = 0;
					response += buffer;
					response += ", y=";
					snprintf(buffer, sizeof( buffer ) - 1, "%f", value[ j ].pos.y);
					buffer[ sizeof( buffer ) - 1 ] = 0;
					response += buffer;
					response += ", z=";
					snprintf(buffer, sizeof( buffer ) - 1, "%f", value[ j ].pos.z);
					buffer[ sizeof( buffer ) - 1 ] = 0;
					response += buffer;
					response += ", scene=";
					response += value[ j ].scene;
					response += ", cell=";
					response += value[ j ].cell.getValueString();
					//					response += ")\r\n";					}
					for (k = 0; k < nameLength; ++k)
						response += " ";
					response += "]\r\n";
				}
			}
			break;
		case DynamicVariable::LIST:
			{
				// call recursively
				std::string newPrefix = prefix + objVar.getName() + ".";
				const DynamicVariableList::NestedList &list = objVar.getNestedList();
				for(DynamicVariableList::NestedList::const_iterator newObjvar = list.begin();
					newObjvar != list.end();
					++newObjvar)
				{
					addObjVarToString( response, newPrefix, newObjvar );
				}
			}
			break;
		case DynamicVariable::STRING_ID:
			{
				StringId value;
				objVar.getValue(value);
				response += "(StringId)";
				response += value.getDebugString();
				response += "\r\n";
			}
			break;
		case DynamicVariable::STRING_ID_ARRAY:
			{
				std::vector<StringId> value;
				objVar.getValue(value);
				std::string text = "(StringId)[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					text += value[j].getDebugString();
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\r\n";
				response += text;
			}
			break;
		case DynamicVariable::TRANSFORM:
			{
				Transform value;
				objVar.getValue(value);
				response += "(Transform)";
				//response += getDebugString(value);
				response += "\r\n";
			}
			break;
		case DynamicVariable::TRANSFORM_ARRAY:
			{
				std::vector<Transform> value;
				objVar.getValue(value);
				std::string text = "(Transform)[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					//						text += getDebugString(value[j]);
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\r\n";
				response += text;
			}
			break;
		case DynamicVariable::VECTOR:
			{
				Vector value;
				objVar.getValue(value);
				response += "(Vector)";
				//					response += getDebugString(value);
				response += "\r\n";
			}
			break;
		case DynamicVariable::VECTOR_ARRAY:
			{
				std::vector<Vector> value;
				objVar.getValue(value);
				std::string text = "(Vector)[";
				size_t count = value.size();
				for (size_t j = 0; j < count; ++j)
				{
					//						text += getDebugString(value[j]);
					if (j + 1 < count)
						text += ", ";
				}
				text += "]\n";
				response += text;
			}
			break;
		}

	}

	void addItemInfo(const VolumeContainer * container, int & num, std::map<std::string, std::string> & stats)
	{
		if(!container)
			return;
		char stringbuffer[CSHandlerNamespace::largeBufferSize];
		for (ContainerConstIterator iter(container->begin());
			iter != container->end(); ++iter)
		{
			char catbuffer[ CSHandlerNamespace::smallBufferSize ];
			const CachedNetworkId & objId = (*iter);
			const TangibleObject * obj = safe_cast<const TangibleObject *>(objId.getObject());
			std::string category = "Item-";
			snprintf( catbuffer, sizeof( catbuffer ) - 1, "%s%d", category.c_str(), num++ );
			catbuffer[ sizeof( catbuffer ) -1 ] = 0;
			snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%s(%s)", Unicode::wideToNarrow( obj->getObjectName() ).c_str(),
				obj->getNetworkId().getValueString().c_str() );
			stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
			stats[ catbuffer ] = stringbuffer;

			// recurse if necessary.
			const VolumeContainer * obj_as_container = ContainerInterface::getVolumeContainer(*obj);
			if( obj_as_container )
			{
				addItemInfo( obj_as_container, num, stats );
			}
		}
	}
}

CSHandler* CSHandler::sm_instance = 0;

CSHandler::CSHandler()
{
	registerCommands();
}

CSHandler::~ CSHandler()
{
}

void CSHandler::install()
{
	if( !sm_instance )
		sm_instance = new CSHandler();

}

void CSHandler::remove()
{
	delete sm_instance;
}

CSHandler & CSHandler::getInstance()
{
	NOT_NULL( sm_instance );
	return *sm_instance;
}

void CSHandler::handle( GameServerCSRequestMessage & request )
{
	DEBUG_REPORT_LOG( true, ( "Handling command" ) );
	const std::string &original = request.getCommandString();
	std::string command;
	std::string args;

	unsigned pos = original.find( " " );
	if( pos == std::string::npos )
	{
		command = original;
	}
	else
	{
		command = original.substr( 0, pos );
		args = original.substr( pos + 1, original.length() - ( pos + 1 )  );
	}

	DEBUG_REPORT_LOG( true, ( "Command is %s", command.c_str() ) );

	// find the command
	std::map< std::string, CSHandlerEntry >::iterator it = CSHandlerNamespace::cmdMap.find( command );
	if( it != CSHandlerNamespace::cmdMap.end() )
	{
		// build the proto-response
		GameServerCSResponseMessage msg( request );

		std::string username = request.getUserName();

		// build the cs request
		CSHandlerRequest req( command, args, request.getAccessLevel(), username );

		// check the access
		if( req.m_access >= it->second.access )
		{
			// and fire it off.
			CSHandlerFunc func = it->second.handlerFunc;
			( this->*func )( req, msg );
		}

	}
	else
	{
		// try passing it off to the console parser.
		//ConsoleMgr::processStringForCSHandler( original );
		//		JavaLibrary::instance()->runScripts(m_owner->getNetworkId(), funcName, "Ouf", params);
	}
}

void CSHandler::registerCommands()
{
	REGISTER_CS_CMD( get_pc_info, 4 );
	REGISTER_CS_CMD( set_bank_credits, 4 );
	REGISTER_CS_CMD( undelete_item, 4 );
	REGISTER_CS_CMD( list_objvars, 4 );
	REGISTER_CS_CMD( set_objvar, 4 );
	REGISTER_CS_CMD( remove_objvar, 4 );
	REGISTER_CS_CMD( dump_info, 4 );
	REGISTER_CS_CMD( rename_player, 4 );

	REGISTER_CS_CMD( freeze, 4 );
	REGISTER_CS_CMD( unfreeze, 4 );

	REGISTER_CS_CMD( get_player_id, 4 );
	REGISTER_CS_CMD( get_player_items, 4 );

	REGISTER_CS_CMD( move_object, 4 );

	REGISTER_CS_CMD( create_crafted_object, 4 );

	REGISTER_CS_CMD( delete_object, 4 );

	REGISTER_CS_CMD( adjust_lots, 4 );

	REGISTER_CS_CMD( warp_player, 4 );
}

void addIntData( std::map< std::string, std::string > & data, const std::string & stat, int amount )
{
	char buffer[ CSHandlerNamespace::smallBufferSize ];
	snprintf( buffer, sizeof( buffer ) - 1, "%d", amount );
	buffer[ sizeof( buffer ) - 1 ] = 0;
	data[ stat ] = buffer;
}



enum ArgType
{
	STRING_ARGUMENT = 0,
	REAL_ARGUMENT,
	INT_ARGUMENT
};

int getArgumentType(const std::string &arg)
{
	bool foundDecimal = false;

	size_t count = arg.length();
	for (size_t i = 0; i < count; ++i)
	{
		if (!isdigit(arg[i]))
		{
			if (arg[i] != '.' || foundDecimal)
				return STRING_ARGUMENT;
			foundDecimal = true;
		}
	}
	if (foundDecimal)
		return REAL_ARGUMENT;
	return INT_ARGUMENT;
}

CS_CMD( warp_player )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 5 )
		return;

	// format:  target planet x y z
	NetworkId target_id( args[ 0 ] );
	CreatureObject *target = CreatureObject::getCreatureObject( target_id );
	if( !( target && target->isAuthoritative() ) )
		return;

	std::string planet = args[ 1 ];
	float x=0.0f, y=0.0f, z=0.0f;
	sscanf( args[2].c_str(), "%f", &x );
	sscanf( args[3].c_str(), "%f", &y );
	sscanf( args[4].c_str(), "%f", &z );

	Vector newPosition_w(x, y, z);
	Vector newPosition_p(0.0f, 0.0f, 0.0f);
	NetworkId newContainer;
	TangibleObject *targetTangible = target;

	if (targetTangible)
	{
		if (!ServerWorld::isSpaceScene() && !strncmp(planet.c_str(), "space_", 5) && targetTangible->asCreatureObject())
		{
			// don't allow CS Tool to warp to space from ground.
			return;

			// NOTE RHanz: This code is commented out due to lack of an obvious unified interface
			// for setting up launches (ie, having to manually set objvars)
			// to space in this usage case.  The code is exactly the same (except var names)
			// as the code for planetwarp (in CommandCppFuncs.cpp).  If CS actually does need to
			// move people from ground to space for admin purposes, you should be able to simply uncomment
			// the below section of code.

			//// going to space from the ground, so require a ship and set launch information
			//ShipObject const * const ship = CSHandlerNamespace::getFirstPackedShipForCreature(*targetTangible->asCreatureObject());
			//if (!ship)
			//{
			//	return;
			//}

			//DynamicVariableLocationData const loc(targetTangible->getPosition_w(), ServerWorld::getSceneId(), NetworkId::cms_invalid);
			//targetTangible->setObjVarItem("space.launch.worldLoc", loc);
			//targetTangible->setObjVarItem("space.launch.ship", ship->getNetworkId());
			//targetTangible->setObjVarItem("space.launch.startIndex", static_cast<int>(0));
		}
		if(!target->getClient())
		{
			// character is in the save queue.  This is currently not supported.  Planetwarping
			// appears to break the PseudoClientConnection system.
			msg.setResponse( "Unable to warp player:\r\n" );
			SwgGameServer::getInstance().sendToCentralServer(msg);
			return;
		}
		CSHandlerNamespace::CSLogData data;
		data[ "command" ] = "warp_player";
		data[ "target_id" ] = args[ 0 ];
		data[ "planet" ] = args[ 1 ];
		char buf[ CSHandlerNamespace::largeBufferSize ];
		snprintf( buf, CSHandlerNamespace::largeBufferSize,"%.02f %.02f %.02f", x, y, z );
		data[ "location" ] = buf;
		GameServer::getInstance().requestSceneWarp(CachedNetworkId(*targetTangible), planet, newPosition_w, newContainer, newPosition_p);
	}
}

CS_CMD( adjust_lots )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 2 )
		return; // not enough arguments.

	NetworkId target_id( args[ 0 ] );
	CreatureObject *target = CreatureObject::getCreatureObject( target_id );
	if( !( target && target->isAuthoritative() ) )
		return;
	PlayerObject * player = PlayerCreatureController::getPlayerObject( target );
	if( !player )
		return; // not a player, can't adjust lots.

	int count = atoi( args[ 1 ].c_str() );

	CSHandlerNamespace::CSLogData data;
	data[ "command" ] = "adjust_lots";
	data[ "target_id" ] = args[ 0 ];
	data[ "amount" ] = args[ 1 ];
	CSHandlerNamespace::log( request, data );

	player->adjustLotCount( count );

}

CS_CMD( create_crafted_object )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	// args should be:
	// destination schematic quality

	if( args.size() < 2 )
		return;

	NetworkId target_id( args[ 0 ] );
	CreatureObject *target = CreatureObject::getCreatureObject( target_id );
	if( !( target && target->isAuthoritative() ) )
		return;
	ServerObject *inventory = target->getInventory();
	if( inventory == nullptr )
		return;
	DEBUG_REPORT_LOG( true, ( "Trying to make %s\n", args[ 1 ].c_str()));
	GameScriptObject * script = target->getScriptObject();
	if( script )
	{
		ScriptParams params;
		params.addParam(target_id);
		params.addParam(args[ 1 ].c_str());
		script->trigAllScripts(Scripting::TRIG_CS_CREATE_STATIC_ITEM, params);

	}
	CSHandlerNamespace::CSLogData data;
	data[ "command" ] = "create_crafted_object";
	data[ "target_id" ] = args[ 0 ];
	data[ "schematic" ] = args[ 1 ];
	CSHandlerNamespace::log( request, data );
}

// informational command.  no logs needed.
CS_CMD( get_player_items )
{
	std::transform( request.m_args.begin(), request.m_args.end(), request.m_args.begin(), tolower );

	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() < 1 )
		return; // no player

	// get the player ID


	if( args.size() > 0 )
	{
		NetworkId player_id = NameManager::getInstance().getPlayerId( args[ 0 ] );

		// get the object
		const CreatureObject* const creatureActor = CreatureObject::getCreatureObject(player_id);

		if( !creatureActor )
			return;


		// are we authoritative?
		if( creatureActor->isAuthoritative() )
		{
			// if so, built a list of items
			std::string response;
			const ServerObject * inventory = creatureActor->getInventory();
			if( inventory )
			{

				std::map< std::string, std::string > stats;
				const VolumeContainer * inventoryContainer = ContainerInterface::getVolumeContainer(*inventory);
				if (inventoryContainer )
				{

					int item_num = 0;
					for (ContainerConstIterator iter(inventoryContainer->begin());
						iter != inventoryContainer->end(); ++iter)
					{
						char catbuffer[ CSHandlerNamespace::smallBufferSize ];
						char stringbuffer[ CSHandlerNamespace::largeBufferSize ];
						const CachedNetworkId & objId = (*iter);
						const TangibleObject * obj = safe_cast<const TangibleObject *>(objId.getObject());
						std::string category = "Item-";
						snprintf( catbuffer, sizeof( catbuffer ) - 1, "%s%d", category.c_str(), item_num++ );
						snprintf( stringbuffer, sizeof( stringbuffer ) - 1,"%s(%s)", Unicode::wideToNarrow( obj->getObjectName() ).c_str(),
							obj->getNetworkId().getValueString().c_str() );
						catbuffer[ sizeof( catbuffer ) - 1 ] = 0;
						stringbuffer[ sizeof( stringbuffer ) -1 ] = 0;
						stats[ catbuffer ] = stringbuffer;
					}
				}
				// and send it back.
				std::map< std::string, std::string >::iterator it = stats.begin();
				response = "item list\r\n";
				while( it != stats.end() )
				{
					response = response + it->first + ":" + it->second + "\r\n";
					it++;
				}
				msg.setResponse( response );
				SwgGameServer::getInstance().sendToCentralServer( msg );
			}
		}
	}
}

CS_CMD( move_object )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() < 2 )
		return;

	NetworkId oid( args[ 1 ] );
	NetworkId player( args[ 0 ] );

	if( !( oid.isValid() && player.isValid() ) )
		return; // bad args or this wasn't confirmed as an offline char.
	if (oid < ConfigServerGame::getMaxGoldNetworkId())
		return;
	else
	{
		ServerObject *object = ServerObject::getServerObject( oid );
		if (!( object && object->isAuthoritative() ) )
			return;
		else
		{
			if (!NameManager::getInstance().isPlayer(player))
				return;
			else
			{
				msg.setResponse( "Moving object:\r\n" );
				SwgGameServer::getInstance().sendToCentralServer( msg );
				CSHandlerNamespace::CSLogData data;
				data[ "command" ] = "move_object";
				object->moveToPlayerAndUnload(player);
				data[ "object" ] = args[ 1 ];
				data[ "target_char" ] = args[ 0 ];
				CSHandlerNamespace::log( request, data );

			}
		}

	}
}

CS_CMD( freeze )
{
	DEBUG_REPORT_LOG( true, ( "Trying to freeze\n" ) );
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 1 )
		return;

	NetworkId id( args[ 0 ].c_str() );
	if( id.isValid() )
	{
		ServerObject * const object = ServerWorld::findObjectByNetworkId(id);

		if( !object )
			return;
		if( object->isAuthoritative() )
		{
			JavaLibrary::freezePlayer( id );
			CSHandlerNamespace::CSLogData data;
			data[ "target" ] = args[ 0 ];
			data[ "command" ] = "freeze";
			CSHandlerNamespace::log( request, data );
			DEBUG_REPORT_LOG( true, ( "really doing freeze %s\n", id.getValueString().c_str() ) );
		}
	}

}

CS_CMD( unfreeze )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 1 )
		return;

	NetworkId id( args[ 0 ].c_str() );
	if( id.isValid() )
	{
		ServerObject * const object = ServerWorld::findObjectByNetworkId(id);

		if( !object )
			return;
		if( object->isAuthoritative() )
		{
			CSHandlerNamespace::CSLogData data;
			data[ "target" ] = args[ 0 ];
			data[ "command" ] = "unfreeze";
			CSHandlerNamespace::log( request, data );
			JavaLibrary::unFreezePlayer( id );
		}
	}

}

CS_CMD( dump_info )
{
	//	LocalRefPtr local_ref;
	DEBUG_REPORT_LOG( true, ( "dump_info" ) );
	NetworkId id( request.m_args );
	if( id.isValid() )
	{
		ServerObject * const object = ServerWorld::findObjectByNetworkId(id);
		if (object && object->isAuthoritative())
		{
			DEBUG_REPORT_LOG( true, ( "Valid ID\n" ) );
			std::string  returned_value;

			returned_value = GameScriptObject::callDumpTargetInfo( id );
			std::string response;
			response = "Character info dump:\r\n";
			response = response + returned_value;
			msg.setResponse( response );
			SwgGameServer::getInstance().sendToCentralServer( msg );
		}


	}
	//	JavaStringPtr javaResult= callStaticStringMethod(, getVarNameMid);
	//	JavaLibrary::convert(*javaCustomVarName, nativeCustomVarName);
}


CS_CMD( remove_objvar )
{
	std::string objid;
	std::string objvar;

	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() < 2 )
		return;

	objid = args[ 0 ];
	objvar = args[ 1 ];

	NetworkId specifiedNetworkId( objid );
	ServerObject * const object = ServerWorld::findObjectByNetworkId(specifiedNetworkId);

	if( !( object && object->isAuthoritative() ) )
		return;

	object->removeObjVarItem(objvar);
	CSHandlerNamespace::CSLogData data;
	data[ "target" ] = args[ 0 ];
	data[ "command" ] = "remove_objvar";
	data[ "objvar" ] = args[ 1 ];
	CSHandlerNamespace::log( request, data );
}

CS_CMD( set_objvar )
{
	// set_objvar [itemid] [objvar] [value]
	std::string objid;
	std::string objvar;
	std::string objvarvalue;

	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 3 )
	{
		return;
	}

	objid = args[ 0 ];
	objvar = args[ 1 ];
	objvarvalue = args[ 2 ];
	NetworkId specifiedNetworkId( objid );
	ServerObject * const object = ServerWorld::findObjectByNetworkId(specifiedNetworkId);
	if( !( object && object->isAuthoritative() ) )
	{
		return;
	}


	switch (getArgumentType(objvarvalue))
	{
	case INT_ARGUMENT:
		object->setObjVarItem(objvar, atoi(objvarvalue.c_str()));
		break;
	case REAL_ARGUMENT:
		object->setObjVarItem(objvar, static_cast<real>(atof(objvarvalue.c_str())));
		break;
	case STRING_ARGUMENT:
		object->setObjVarItem(objvar, objvarvalue);
		break;
	default:
		break;
	}
	CSHandlerNamespace::CSLogData data;
	data[ "target" ] = args[ 0 ];
	data[ "command" ] = "set_objvar";
	data[ "objvar" ] = args[ 1 ];
	data[ "value" ] = args[ 2 ];
	CSHandlerNamespace::log( request, data );
}

// this is kind of sloppy.  Code is copied from consolecommandparserobjvar.cpp, and modified to work.
// realistically, it seems like objvars should internally be able to print their own value as a string without
// having to rely on code to do it each time.
CS_CMD( list_objvars )
{
	DEBUG_REPORT_LOG( true, ( "Got list_objvars command\n" ) );
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 1 )
	{
		DEBUG_REPORT_LOG( true, ( "not enough args\n" ) );
		return;
	}


	std::string response;

	response = "Objvar list for ";
	response = response + args[ 0 ] + "\r\n";
	// get the obj


	NetworkId specifiedNetworkId = NetworkId(args[ 0 ]);

	ServerObject * const specifiedServerObject = ServerWorld::findObjectByNetworkId(specifiedNetworkId);
	// iterate objvars

	if( !( specifiedServerObject && specifiedServerObject->isAuthoritative() ) )
	{
		DEBUG_REPORT_LOG( true, ( "Item not found\n" ) );
		return;
	}

	DynamicVariableList::NestedList objvarList = specifiedServerObject->getObjVars();

	//	char buffer[ CSHandlerNamespace::smallBufferSize ];
	// add to response string
	for (DynamicVariableList::NestedList::const_iterator objVar(objvarList.begin()); objVar!=objvarList.end(); ++objVar)
	{
		CSHandlerNamespace::addObjVarToString( response, "", objVar );
	}
	// send response.
	msg.setResponse( response );
	SwgGameServer::getInstance().sendToCentralServer( msg );

}


CS_CMD( delete_object )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );

	if( args.size() < 1 )
		return;


	const NetworkId oid (args[0]);
	ServerObject *object = ServerObject::getServerObject( oid );

	if (object == nullptr)
	{
		return;
	}
	else if (object->getClient())
	{
		return;
	}
	else if (!object->isAuthoritative())
	{
		return;
	}
	else
	{
		CSHandlerNamespace::CSLogData data;
		data[ "target" ] = args[ 0 ];
		data[ "command" ] = "delete_object";
		CSHandlerNamespace::log( request, data );
		object->permanentlyDestroy(DeleteReasons::God);
	}
	return;
}

CS_CMD( rename_player )
{
	CSHandlerNamespace::CSArgs args;
	DEBUG_REPORT_LOG( true, ( "rename_player" ) );
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() < 3 )
	{
		return;
	}
	std::transform( args[ 0 ].begin(), args[ 0 ].end(), args[ 0 ].begin(), tolower );


	NetworkId player_id = NameManager::getInstance().getPlayerId( args[ 0 ] );

	// only execute on the authoritative object.  exit early for all others
	// to avoid spam.
	ServerObject *object = ServerObject::getServerObject( player_id );
	if( !( object && object->isAuthoritative() ) )
	{
		return;
	}

	std::string lowertarget = args[1];
	std::transform( lowertarget.begin(), lowertarget.end(), lowertarget.begin(), tolower );

	NetworkId target_check = NameManager::getInstance().getPlayerId( lowertarget );
	if( target_check.isValid() )
	{
		// invalid name, already used by someone.
		msg.setResponse( "Could not rename player, name in use:\r\n" );
		SwgGameServer::getInstance().sendToCentralServer( msg );
		return;
	}
	if( player_id.isValid() )
	{
		// nullptr id to pass to the playercreationmanager.
		NetworkId source( "0" );

		DEBUG_REPORT_LOG( true, ( "Attempting to rename %s.", args[ 0 ].c_str() ) );
		PlayerCreationManagerServer::renamePlayer( static_cast<int8>(RenameCharacterMessageEx::RCMS_cs_tool), NameManager::getInstance().getPlayerStationId(player_id), player_id, Unicode::narrowToWide( args[ 1 ] ), object->getAssignedObjectName(), source );
		CSHandlerNamespace::CSLogData data;
		msg.setResponse( "Rename player complete:\r\n" );
		SwgGameServer::getInstance().sendToCentralServer( msg );
		data[ "target" ] = args[ 0 ];
		data[ "command" ] = "rename_player";
		data[ "new_name" ] = args[ 1 ];
		CSHandlerNamespace::log( request, data );
	}
	else
	{
		DEBUG_REPORT_LOG( true, ( "Could not rename player.  %s not found", args[ 0 ].c_str() ) );
	}

}

CS_CMD( set_bank_credits )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() < 2 )
	{
		return;
	}

	std::transform( args[ 0 ].begin(), args[ 0 ].end(), args[ 0 ].begin(), tolower );
	int amount = atoi( args[ 1 ].c_str() );

	DEBUG_REPORT_LOG( true, ( "set_bank_credits: %d\n", amount ) );
	NetworkId player_id = NameManager::getInstance().getPlayerId( args[ 0 ] );
	if( player_id.isValid() )
	{
		CreatureObject* creatureActor = CreatureObject::getCreatureObject(player_id);
		PlayerObject const * const player = PlayerCreatureController::getPlayerObject(creatureActor);

		if( ( player != nullptr ) && ( player->isAuthoritative() ) )
		{
			amount -= creatureActor->getBankBalance();
			DEBUG_REPORT_LOG( true, ( "Amount to modify by: %d (%d current balance)\n", amount, creatureActor->getBankBalance() ) );
			if( amount < 0 )
			{
				DEBUG_REPORT_LOG( true, ( "Transferring to\n" ) );
				creatureActor->transferBankCreditsTo( "cs_" "stub", amount * -1 );
			}
			else
			{
				DEBUG_REPORT_LOG( true, ( "Transferring from\n" ) );
				creatureActor->transferBankCreditsFrom( "cs_" "stub", amount );

			}
			msg.setResponse( "Successfully modified bank amount" );
			CSHandlerNamespace::CSLogData data;
			data[ "target" ] = args[ 0 ];
			data[ "command" ] = "set_bank_credits";
			data[ "amount" ] = args[ 1 ];
			CSHandlerNamespace::log( request, data );
			SwgGameServer::getInstance().sendToCentralServer( msg );
		}
	}

}

CS_CMD( get_pc_info )
{
	// see if we have the character.
	std::transform( request.m_args.begin(), request.m_args.end(), request.m_args.begin(), tolower );
	DEBUG_REPORT_LOG( true, ( "Got get_pc_info for %s\n", request.m_args.c_str() ) );
	NetworkId player_id = NameManager::getInstance().getPlayerId( request.m_args );
	if( player_id.isValid() )
	{
		DEBUG_REPORT_LOG( true, ( "Found player.\n" ) );
		// get the info on the player.
		const CreatureObject* const creatureActor = CreatureObject::getCreatureObject(player_id);
		if (!( creatureActor && creatureActor->isAuthoritative() ) )
		{
			return;
		}

		char stringbuffer[ CSHandlerNamespace::smallBufferSize ]; // for formatted output.
		std::map< std::string, std::string > stats;

		//get the bind location
		Vector bindLoc;
		std::string bindPlanet;
		NetworkId bindId;
		if (creatureActor->getObjVars().hasItem("bind.facility"))
		{
			creatureActor->getObjVars().getItem("bind.facility", bindId);
		}
		if(bindId != NetworkId::cms_invalid)
		{
			const ServerObject* const bindObject = ServerObject::getServerObject(bindId);
			if (bindObject != nullptr)
			{
				bindLoc    = bindObject->getPosition_w();
				snprintf( stringbuffer, sizeof( stringbuffer ) -1, "%02f %02f %02f", bindLoc.x, bindLoc.y, bindLoc.z );
				stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
				stats[ "Bind Location" ] = stringbuffer;
				bindPlanet = bindObject->getSceneId();
				stats[ "Bind Planet" ] = bindPlanet;
			}
		}

		//get the bankId
		//		Vector bankLoc(0, 0, 0);
		std::string bankPlanet;
		if (creatureActor->getObjVars().hasItem("banking_bankid"))
		{
			creatureActor->getObjVars().getItem("banking_bankid", bankPlanet);
			stats[ "Bank Planet" ] = bankPlanet;
		}


		const ServerObject * inventory = creatureActor->getInventory();
		if( inventory )
		{

			const VolumeContainer * inventoryContainer = ContainerInterface::getVolumeContainer(*inventory);
			if (inventoryContainer )
			{
				int item_num = 0;
				CSHandlerNamespace::addItemInfo(inventoryContainer,item_num,stats);
			}
		}


		int item_num = 0;
		const SlottedContainer * p_equipment =  creatureActor->getSlottedContainerProperty();
		for( ContainerConstIterator it = p_equipment->begin();
			it != p_equipment->end();
			it++ )
		{
			Object * const object = (*it).getObject();
			char catbuffer[ CSHandlerNamespace::smallBufferSize ];

			if( object && object->asServerObject() )
			{
				snprintf( catbuffer, sizeof( catbuffer ) - 1, "Equipment-%d", item_num++ );
				catbuffer[ sizeof( catbuffer ) - 1 ] = 0;
				snprintf( stringbuffer, sizeof( stringbuffer ) -1, "%s(%s)", Unicode::wideToNarrow( object->asServerObject()->getObjectName() ).c_str(), object->asServerObject()->getNetworkId().getValueString().c_str() );
				stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
				stats[ catbuffer ] = stringbuffer;
			}

		}

		//get the residence location
		Vector resLoc;
		std::string resPlanet;

		/*		creatureActor->getAllBuffs();
		creatureActor->getAllShipsInDatapad();*/

		// attributes
		addIntData( stats, "Action", creatureActor->getAttribute( Attributes::Action ) );
		addIntData( stats, "Constitution", creatureActor->getAttribute( Attributes::Constitution ) );
		addIntData( stats, "Health", creatureActor->getAttribute( Attributes::Health ) );
		addIntData( stats, "Mind", creatureActor->getAttribute( Attributes::Mind ) );
		addIntData( stats, "Stamina", creatureActor->getAttribute( Attributes::Stamina ) );
		addIntData( stats, "Willpower", creatureActor->getAttribute( Attributes::Willpower ) );

		stats[ "Character ID" ] = player_id.getValueString();

		// skills

		const CreatureObject::SkillList & skills = creatureActor->getSkillList();
		int temp_int = 0;
		for( CreatureObject::SkillList::const_iterator slit = skills.begin();
			slit != skills.end();
			++slit )
		{
			snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", temp_int );
			stringbuffer[ sizeof( stringbuffer ) -1  ] = 0;
			std::string category = "Skill-";
			category = category + stringbuffer;
			stats[ category ] = ( *slit )->getSkillName();
			temp_int++;
		}

		// add experience into the data
		const std::map< std::string, int > & exp = creatureActor->getExperiencePoints();

		for( std::map< std::string, int >::const_iterator expit = exp.begin();
			expit != exp.end();
			++expit )
		{
			std::string category = "Experience-";
			category += expit->first;
			addIntData( stats, category, expit->second );
		}

		// money
		snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", creatureActor->getCashBalance() );
		stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
		stats[ "Credits" ] = stringbuffer;

		snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", creatureActor->getBankBalance() );
		stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
		stats[ "Bank Credits" ] = stringbuffer;

		// residence info
		PlayerObject const * const player = PlayerCreatureController::getPlayerObject(creatureActor);
		if (player != nullptr)
		{
			NetworkId houseNetworkId = creatureActor->getHouse();
			const ServerObject* const resObject = ServerObject::getServerObject(houseNetworkId);
			if (resObject != nullptr)
			{
				resLoc    = resObject->getPosition_w();
				snprintf( stringbuffer, sizeof( stringbuffer ) - 1 , "%02f %02f %02f", resLoc.x, resLoc.y, resLoc.z );
				stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
				stats[ "Residence Location" ] = stringbuffer;
				resPlanet = resObject->getSceneId();
				stats[ "Residence Planet" ] = resPlanet;
			}
			addIntData( stats, "Account ID", player->getStationId() );

		}

		//get the spouse's name
		Unicode::String spouseName = Unicode::emptyString;
		if (creatureActor->getObjVars().hasItem("marriage.spouseName"))
		{
			creatureActor->getObjVars().getItem("marriage.spouseName", spouseName);
			stats[ "Spouse Name" ] = Unicode::wideToNarrow( spouseName );
		}

		//get the number of used lots
		int lots = creatureActor->getMaxNumberOfLots();

		if(player)
		{
			int lotsUsed = player->getAccountNumLots();
			lots -= lotsUsed;
			snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", lots );
			stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
			stats[ "Lots Available" ] = stringbuffer;
		}

		//get the faction standing
		int factionAlignment   = creatureActor->getPvpFaction();
		snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", factionAlignment );
		stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
		stats[ "Faction" ] = stringbuffer;
		int factionDeclareType = creatureActor->getPvpType();
		snprintf( stringbuffer, sizeof( stringbuffer ) - 1, "%d", factionDeclareType );
		stringbuffer[ sizeof( stringbuffer ) - 1 ] = 0;
		stats[ "PvP Type" ] = stringbuffer;

		addIntData( stats, "Guild ID", creatureActor->getGuildId() );

		// TODO:  Move this into its own function.

		// build the response based on our map.
		std:: string response = "Stats for player ";
		response += request.m_args + "\r\n";
		std::map< std::string, std::string >::iterator statit = stats.begin();
		while( statit != stats.end() )
		{
			response += statit->first + ": " + statit->second + "\r\n";
			++statit;
		}
		msg.setResponse( response );
		SwgGameServer::getInstance().sendToCentralServer( msg );
	}
}

CS_CMD( undelete_item )
{
	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if(args.size() < 3)
		return;
	NetworkId characterId(args[0]);
	NetworkId itemId(args[ 1 ]);
	bool bMove = false;
	if (args[2] == "move")
	{
		bMove = true;
	}
	GenericValueTypeMessage<std::pair<std::pair<std::pair<NetworkId, NetworkId>, std::string>, bool > > dbmsg("UndeleteItemForCsMessage", std::make_pair(std::make_pair(std::make_pair(characterId, itemId), request.m_args), bMove));
	GameServer::getInstance().sendToDatabaseServer(dbmsg);
	CSHandlerNamespace::CSLogData data;
	data["character_id"]=args[0];
	data[ "item_id"] = args[ 1 ];
	data[ "command" ] = "undelete_item";
	CSHandlerNamespace::log( request, data );
	msg.setResponse( "Undelete command sent.\r\n" );

}

CS_CMD( get_player_id )
{
	std::transform( request.m_args.begin(), request.m_args.end(), request.m_args.begin(), tolower );

	CSHandlerNamespace::CSArgs args;
	CSHandlerNamespace::getArgs( request.m_args, args );
	if( args.size() > 0 )
	{
		NetworkId player_id = NameManager::getInstance().getPlayerId( args[ 0 ] );
		// build the response
		std::string response;
		response = "Player Id found\r\ncharactername:" + args[ 0 ] + "\r\ncharacter id:" + player_id.getValueString() + "\r\n";

		msg.setResponse( response );
		SwgGameServer::getInstance().sendToCentralServer( msg );
	}
}


CSHandlerEntry::CSHandlerEntry() :
access( 0 ),
handlerFunc( 0 )
{

}

CSHandlerEntry::CSHandlerEntry( const std::string & name, uint32 accessLevel, CSHandlerFunc handler ) :
sCommand( name ),
access( accessLevel ),
handlerFunc( handler )
{
}

CSHandlerRequest::CSHandlerRequest( const std::string & command,
								   const std::string & args,
								   const uint32 access,
								   const std::string & username ) :
m_command( command ),
m_args( args ),
m_access( access ),
m_name( username )
{
}
