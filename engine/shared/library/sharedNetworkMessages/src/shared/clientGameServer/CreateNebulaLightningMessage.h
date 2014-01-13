//======================================================================
//
// CreateNebulaLightningMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CreateNebulaLightningMessage_H
#define INCLUDED_CreateNebulaLightningMessage_H

//======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/NebulaLightningData.h"

//----------------------------------------------------------------------

class CreateNebulaLightningMessage : public GameNetworkMessage
{
public:

	static char const * const MessageType;

	explicit CreateNebulaLightningMessage(NebulaLightningData const & nebulaLightningData);
	explicit CreateNebulaLightningMessage(Archive::ReadIterator &source);

	NebulaLightningData const & getNebulaLightningData() const;

private:
	Archive::AutoVariable<NebulaLightningData> m_nebulaLightningData;
};

//======================================================================

#endif
