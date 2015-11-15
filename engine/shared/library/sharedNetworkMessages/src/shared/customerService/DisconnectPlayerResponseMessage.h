// ======================================================================
//
// DisconnectPlayerResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_DisconnectPlayerResponseMessage_H
#define	_DisconnectPlayerResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class DisconnectPlayerResponseMessage : public GameNetworkMessage
{
public:
	DisconnectPlayerResponseMessage(int32 result);
	explicit DisconnectPlayerResponseMessage(Archive::ReadIterator &source);

	~DisconnectPlayerResponseMessage();

public: // methods

	int32                            getResult() const;

public: // types

private: 
	Archive::AutoVariable<int32>   m_result;  
};

// ----------------------------------------------------------------------

inline int32 DisconnectPlayerResponseMessage::getResult() const
{
	return m_result.get();
}


// ----------------------------------------------------------------------

#endif // _DisconnectPlayerResponseMessage_H

