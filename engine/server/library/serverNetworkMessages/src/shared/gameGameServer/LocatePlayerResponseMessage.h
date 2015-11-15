// ======================================================================
//
// LocateObjectResponseMessage.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_LocatePlayerResponseMessage_H
#define	_INCLUDED_LocatePlayerResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LocatePlayerResponseMessage: public GameNetworkMessage
{
public:
	LocatePlayerResponseMessage(NetworkId const &targetId, NetworkId const &responseId, uint32 responsePid, std::string const &scene, Vector const &position_w, uint32 targetPid);
	LocatePlayerResponseMessage(Archive::ReadIterator &source);
	~LocatePlayerResponseMessage();

public:
	NetworkId const &getTargetId() const;
	NetworkId const &getResponseId() const;
	uint32 getResponsePid() const;
	Vector const &getPosition_w() const;
	std::string const &getScene() const;
	uint32 getTargetPid() const;
	
private:
	Archive::AutoVariable<NetworkId>   m_targetId;
	Archive::AutoVariable<NetworkId>   m_responseId;
	Archive::AutoVariable<uint32>      m_responsePid;
	Archive::AutoVariable<std::string> m_scene;
	Archive::AutoVariable<Vector>      m_position_w;
	Archive::AutoVariable<uint32>      m_targetPid;

private:
	LocatePlayerResponseMessage(LocatePlayerResponseMessage const &);
	LocatePlayerResponseMessage &operator=(LocatePlayerResponseMessage const &);
};

// ======================================================================

#endif	// _INCLUDED_LocatePlayerResponseMessage_H

