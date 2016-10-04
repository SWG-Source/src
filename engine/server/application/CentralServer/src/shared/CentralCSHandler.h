// ======================================================================
//
// CentralCSHandler.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================
// routing of CS commands on the Central Server.
// ======================================================================
#ifndef CENTRALCSHANDLER_H
#define CENTRALCSHANDLER_H

#include "serverNetworkMessages/GameServerCSRequestMessage.h"

class CSToolRequest;
class StructureListMessage;

#define DECLARE_CS_CMD( _name ) void handle_##_name( GameServerCSRequestMessage & message );

class CentralCSHandler
{
public:
	static void install();
	static void remove();
	static CentralCSHandler & getInstance();
	void handle(const CSToolRequest& msg, uint32 loginServerId);
	~CentralCSHandler();

	void handleFindObjectResponse(int iIndex, bool bFound);
	void handleStructureListResponse(StructureListMessage& msg);

	// only need to DECLARE_CS_CMD for commands handled at the CentralServer.

	DECLARE_CS_CMD(list_structures);
	DECLARE_CS_CMD(login_character);
	DECLARE_CS_CMD(warp_player);

protected:
	static CentralCSHandler * smp_instance;
	typedef void(CentralCSHandler::*CentralCSHandlerFunc)(GameServerCSRequestMessage &);

	class HandlerEntry
	{
	public:
		enum EntryType
		{
			TYPE_DB = 0,
			TYPE_GAME_SERVER,
			TYPE_CENTRAL,
			TYPE_CONDITIONAL,               // use the game server if character is online.  Use the DB server otherwise.
			TYPE_CONDITIONAL_CENTRAL,	// use the game server if character is online.  Use the central handler otherwise.
			TYPE_ARBITRARY_GAME_SERVER	// send to a game server, we don't care which one.  This is used
							// if all game servers have what we're looking for, so we don't care who gets it.
		};

		HandlerEntry(const std::string & in_name, CentralCSHandlerFunc in_func, EntryType in_type) :
			name(in_name),
			type(in_type),
			func(in_func)
		{
		}

		HandlerEntry(const std::string & in_name, EntryType in_type) :
			name(in_name),
			type(in_type),
			func(nullptr)
		{
		}

		std::string name;
		EntryType type;
		CentralCSHandlerFunc func; // will be nullptr unless it's of TYPE_CENTRAL.
	};

	class CSCharacterFindInfo
	{
	public:
		CSCharacterFindInfo(NetworkId & id, int numServers, GameServerCSRequestMessage &req, bool bHandleAtCentral) :
			commandLine(req.getCommandString()),
			command(req.getCommandName()),
			iAccessLevel(req.getAccessLevel()),
			user(req.getUserName()),
			iToolId(req.getToolId()),
			iAccount(req.getAccountId()),
			responsesWaiting(numServers),
			iLoginServerId(req.getLoginServerID()),
			bCentral(bHandleAtCentral)

		{
		}

		std::string commandLine;
		std::string command;

		uint32 iAccessLevel;
		std::string user;

		uint32 iToolId;
		int iAccount;

		int responsesWaiting;
		int iLoginServerId;
		bool bCentral;	// if offline, should we handle this at the Central Server or the DB?
	protected:
	};

	typedef std::map< int, CSCharacterFindInfo * > CentralCharFindMap;

	CentralCharFindMap m_findMap;

	typedef std::map< std::string, HandlerEntry * > CentralCSHandlerMap;

	CentralCSHandlerMap m_entries;

private:
	CentralCSHandler() :
		m_findMap(),
		m_entries()
	{
	};
};

#endif // CENTRALCSHANDLER_H
