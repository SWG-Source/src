// ======================================================================
//
// BountyHunterTargetListMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef BountyHunterTargetListMessage_h
#define BountyHunterTargetListMessage_h

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include <vector>

// ======================================================================

class BountyHunterTargetListMessage : public GameNetworkMessage
{
public:
	BountyHunterTargetListMessage(const std::vector< std::pair< NetworkId, NetworkId > > & targetList);

	BountyHunterTargetListMessage(Archive::ReadIterator & source);
	~BountyHunterTargetListMessage();

	const std::vector< std::pair< NetworkId, NetworkId > > & getTargetList() const;

protected:

private:
	Archive::AutoVariable<std::vector< std::pair< NetworkId, NetworkId > > > m_targetList;
};

inline const std::vector< std::pair< NetworkId, NetworkId > > & BountyHunterTargetListMessage::getTargetList() const 
{
	return m_targetList.get();
}


// ======================================================================
#endif
