// ==================================================================
//
// DestroyClientPathMessage.h
// Copyright 2003, Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_DestroyClientPathMessage_H
#define	INCLUDED_DestroyClientPathMessage_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class DestroyClientPathMessage : public GameNetworkMessage
{
public: 

	DestroyClientPathMessage ();
	explicit DestroyClientPathMessage (Archive::ReadIterator &source);
	virtual ~DestroyClientPathMessage();

public:

	static char const * const cms_name;
};

// ==================================================================

#endif 
