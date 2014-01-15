// ======================================================================
//
// BountyHunterTargetMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef BountyHunterTargetMessage_h
#define BountyHunterTargetMessage_h

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class BountyHunterTargetMessage : public GameNetworkMessage
{
public:
	BountyHunterTargetMessage(
		const NetworkId & objectId,
		const NetworkId & targetId);


	BountyHunterTargetMessage(Archive::ReadIterator & source);
	~BountyHunterTargetMessage();

	const NetworkId &          GetObjectId() const {return m_objectId.get();}
	const NetworkId &          GetTargetId() const {return m_targetId.get();}

protected:

private:
	Archive::AutoVariable<NetworkId>        m_objectId;
	Archive::AutoVariable<NetworkId>        m_targetId;
};

// ======================================================================
#endif
