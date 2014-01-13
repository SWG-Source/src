// ============================================================================
//
// VerifyPlayerNameResponseMessage.h
//
// Copyright 2004 Sony Online Entertainment
//
// ============================================================================

#ifndef	INCLUDED_VerifyPlayerNameResponseMessage_H
#define	INCLUDED_VerifyPlayerNameResponseMessage_H

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
class VerifyPlayerNameResponseMessage : public GameNetworkMessage
{
public:

	VerifyPlayerNameResponseMessage(bool const valid, Unicode::String const &playerName);
	explicit VerifyPlayerNameResponseMessage(Archive::ReadIterator &source);

	~VerifyPlayerNameResponseMessage();

	bool                   isValid() const;
	Unicode::String const &getPlayerName() const;

private: 

	Archive::AutoVariable<bool>            m_valid;  
	Archive::AutoVariable<Unicode::String> m_playerName;  
};

// ============================================================================

#endif // INCLUDED_VerifyPlayerNameResponseMessage_H
