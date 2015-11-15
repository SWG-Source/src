
// ======================================================================
//
// ClientMfdStatusUpdateMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientMfdStatusUpdateMessage_H
#define INCLUDED_ClientMfdStatusUpdateMessage_H

//-----------------------------------------------------------------------

#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Timer.h"
#include <vector>
//-----------------------------------------------------------------------

class ClientMfdStatusUpdateMessage : public GameNetworkMessage
{
public:

	static const char * const MESSAGE_TYPE;

	ClientMfdStatusUpdateMessage (const NetworkId & sourceId, const std::string & sceneName, const Vector & worldCoordinates, float memberUpdateTimer, float memberUpdateDistance);
	explicit   ClientMfdStatusUpdateMessage (Archive::ReadIterator & source);
	virtual   ~ClientMfdStatusUpdateMessage ();

	bool                 getHasChanged        () const;
	std::vector<NetworkId> &  getMembersToUpdate();
	const std::string &  getSceneName         () const;
	const NetworkId &    getSourceId          () const;
	const Vector &       getWorldCoordinates  () const;
	
	void                 addMemberToUpdate    (const NetworkId & memberId);
	void                 setSceneName         (const std::string & sceneName);
	void                 setWorldCoordinates  (const Vector & worldCoordinates);
	void                 setHasChanged        (bool hasChanged);
	bool                 updateMemberUpdateTimer(float elapsedTimeSeconds);
	
private:
	Archive::AutoVariable<std::string>  m_sceneName;
	Archive::AutoVariable<NetworkId>    m_sourceId;
	Archive::AutoVariable<Vector>       m_worldCoordinates;
	bool                                m_hasChanged;
	std::vector<NetworkId>              m_membersToUpdate;
	Timer                               m_memberUpdateTimer;
	float                               m_memberUpdateDistance;
	
private:
	ClientMfdStatusUpdateMessage(const ClientMfdStatusUpdateMessage&);
	ClientMfdStatusUpdateMessage& operator= (const ClientMfdStatusUpdateMessage&);
};

//-----------------------------------------------------------------

#endif
