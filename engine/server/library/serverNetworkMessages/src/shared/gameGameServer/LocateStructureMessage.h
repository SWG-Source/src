//========================================================================
//
// LocateStructureMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_LocateStructureMessage_H
#define	_INCLUDED_LocateStructureMessage_H

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
class LocateStructureMessage : public GameNetworkMessage
{
public:
	LocateStructureMessage            (const NetworkId & structureId, const int x, const int z, const std::string &sceneId, const std::string &whoRequested);
	LocateStructureMessage            (Archive::ReadIterator & source);
	~LocateStructureMessage           ();

	const NetworkId& getStructureId    () const {return m_structureId.get();}
	const int getX			   () const {return m_x.get();}
	const int getZ			   () const {return m_z.get();}
	const std::string &getSceneId	   () const {return m_sceneId.get();}
	const std::string &getWhoRequested () const {return m_whoRequested.get();}
        
private:
	Archive::AutoVariable<NetworkId>   m_structureId;
	Archive::AutoVariable<int>	   m_x;
	Archive::AutoVariable<int>	   m_z;
	Archive::AutoVariable<std::string> m_sceneId;
	Archive::AutoVariable<std::string> m_whoRequested;

	//disabled functions:
	LocateStructureMessage             ();
	LocateStructureMessage             (const LocateStructureMessage&);
	LocateStructureMessage& operator=  (const LocateStructureMessage&);
};

//-----------------------------------------------------------------------

#endif

