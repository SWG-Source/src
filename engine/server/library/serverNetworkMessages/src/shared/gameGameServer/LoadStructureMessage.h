//========================================================================
//
// LoadStructureMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_LoadStructureMessage_H
#define	_INCLUDED_LoadStructureMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent From:  GameServer
 * Sent To:    DatabaseProcess
 * Action:     Load the contents of the specified structure and send them
 *             to the GameServer that sent this message.
 *
 * Sent From:  DatabaseProcess
 * Sent To:    GameServer
 * Action:     Inform the GameServer that the requested load has been completed.
 */
class LoadStructureMessage : public GameNetworkMessage
{
public:
	LoadStructureMessage            (const NetworkId & structureId, const std::string &whoRequested);
	LoadStructureMessage            (Archive::ReadIterator & source);
	~LoadStructureMessage           ();
	const NetworkId& getStructureId       () const {return m_structureId.get();}
	const std::string &getWhoRequested    () const {return m_whoRequested.get();}
        

private:
	Archive::AutoVariable<NetworkId>      m_structureId;
	Archive::AutoVariable<std::string>    m_whoRequested;

	//disabled functions:
	LoadStructureMessage            ();
	LoadStructureMessage            (const LoadStructureMessage&);
	LoadStructureMessage& operator= (const LoadStructureMessage&);

};

// ----------------------------------------------------------------------

#endif

