// ======================================================================
//
// DisconnectPlayerMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_DisconnectPlayerMessage_H
#define	_DisconnectPlayerMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class DisconnectPlayerMessage : public GameNetworkMessage
{
public:

	DisconnectPlayerMessage();
	explicit DisconnectPlayerMessage(Archive::ReadIterator &source);

	~DisconnectPlayerMessage();

public: 

private: 
};

// ----------------------------------------------------------------------

#endif // _DisconnectPlayerMessage_H

