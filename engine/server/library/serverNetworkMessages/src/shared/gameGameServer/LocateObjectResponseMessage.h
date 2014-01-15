// ======================================================================
//
// LocateObjectResponseMessage.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_LocateObjectResponseMessage_H
#define	_INCLUDED_LocateObjectResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LocateObjectResponseMessage: public GameNetworkMessage
{
public:
	LocateObjectResponseMessage(NetworkId const &targetId, NetworkId const &responseId, uint32 responsePid, Vector const &position_w, std::string const &scene, std::string const &sharedTemplateName, uint32 targetPid, std::vector<NetworkId> const &containers, bool isAuthoritative, NetworkId const &residenceOf);
	LocateObjectResponseMessage(Archive::ReadIterator &source);
	~LocateObjectResponseMessage();

public:
	NetworkId const &getTargetId() const;
	NetworkId const &getResponseId() const;
	uint32 getResponsePid() const;
	Vector const &getPosition_w() const;
	std::string const &getScene() const;
	std::string const &getSharedTemplateName() const;
	uint32 getTargetPid() const;
	std::vector<NetworkId> const &getContainers() const;
	bool getIsAuthoritative() const;
	NetworkId const &getResidenceOf() const;
	
private:
	Archive::AutoVariable<NetworkId> m_targetId;
	Archive::AutoVariable<NetworkId> m_responseId;
	Archive::AutoVariable<uint32> m_responsePid;
	Archive::AutoVariable<Vector> m_position_w;
	Archive::AutoVariable<std::string> m_scene;
	Archive::AutoVariable<std::string> m_sharedTemplateName;
	Archive::AutoVariable<uint32> m_targetPid;
	Archive::AutoVariable<std::vector<NetworkId> > m_containers;
	Archive::AutoVariable<bool> m_isAthoritative;
	Archive::AutoVariable<NetworkId> m_residenceOf;

private:
	LocateObjectResponseMessage(LocateObjectResponseMessage const &);
	LocateObjectResponseMessage &operator=(LocateObjectResponseMessage const &);
};

// ======================================================================

#endif	// _INCLUDED_LocateObjectResponseMessage_H

