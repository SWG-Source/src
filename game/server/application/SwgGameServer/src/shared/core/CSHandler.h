//CSHandler.h
// copyright 2005 Sony Online Entertainment
//-----------------------------------------------
// handler class for CS requests.

#ifndef _CSHANDLER_H
#define _CSHANDLER_H




#define DECLARE_CS_HANDLER( _name_ ) void handle_##_name_( CSHandlerRequest & request, GameServerCSResponseMessage & msg );

class GameServerCSRequestMessage;
class GameServerCSResponseMessage;

class CSHandlerRequest
{
public:
	CSHandlerRequest( const std::string & 	command,
			  const std::string & 	args,
			  const uint32		access,
			  const std::string & 	username );
			  
	std::string	m_command;
	std::string	m_args;
	uint32		m_access;
	std::string	m_name;
};

class CSHandler
{
  public:

  	static void remove();
  
  void handle( GameServerCSRequestMessage & request );
	static void install();
	static CSHandler& getInstance();
    
  // handlers
  	DECLARE_CS_HANDLER( get_pc_info );
	DECLARE_CS_HANDLER( set_bank_credits );
	DECLARE_CS_HANDLER( undelete_item );
	DECLARE_CS_HANDLER( list_objvars );
	DECLARE_CS_HANDLER( set_objvar );
	DECLARE_CS_HANDLER( remove_objvar );
	DECLARE_CS_HANDLER( dump_info );
	DECLARE_CS_HANDLER( rename_player );
	
	DECLARE_CS_HANDLER( freeze );
	DECLARE_CS_HANDLER( unfreeze );
	
	DECLARE_CS_HANDLER( get_player_id );
	DECLARE_CS_HANDLER( get_player_items );
	
	DECLARE_CS_HANDLER( move_object );
	DECLARE_CS_HANDLER( delete_object );
	
	DECLARE_CS_HANDLER( create_crafted_object );

	DECLARE_CS_HANDLER( adjust_lots );

	DECLARE_CS_HANDLER( warp_player );
	
  private:
  void registerCommands();
  
  static CSHandler * sm_instance;
  CSHandler();
  virtual ~CSHandler();

};


typedef void( CSHandler::*CSHandlerFunc )( CSHandlerRequest &, GameServerCSResponseMessage & );

class CSHandlerEntry
{
public:
	CSHandlerEntry();
	CSHandlerEntry( const std::string & name, uint32 access, CSHandlerFunc handler );
	std::string sCommand;
	uint32 access;
	CSHandlerFunc handlerFunc;
	
private:

};
#endif // _CSHANDLER_H

