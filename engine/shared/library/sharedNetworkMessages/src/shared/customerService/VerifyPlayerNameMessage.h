// ============================================================================
//
// VerifyPlayerNameMessage.h
//
// Copyright 2004 Sony Online Entertainment
//
// ============================================================================

#ifndef	INCLUDED_VerifyPlayerNameMessage_H
#define	INCLUDED_VerifyPlayerNameMessage_H

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
class VerifyPlayerNameMessage : public GameNetworkMessage
{
public:

	VerifyPlayerNameMessage(const Unicode::String &characterName, const NetworkId &sourceNetworkId);
	explicit VerifyPlayerNameMessage(Archive::ReadIterator &source);

	~VerifyPlayerNameMessage();

	const Unicode::String &getPlayerName() const;
	const NetworkId &      getSourceNetworkId() const;

private: 

	Archive::AutoVariable<Unicode::String> m_playerName;
	Archive::AutoVariable<NetworkId>       m_sourceNetworkId;
};

// ============================================================================

#endif // INCLUDED_VerifyPlayerNameMessage_H
