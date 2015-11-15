// ======================================================================
//
// ConnectPlayerResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_ConnectPlayerResponseMessage_H
#define	_ConnectPlayerResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ConnectPlayerResponseMessage : public GameNetworkMessage
{
public:
	ConnectPlayerResponseMessage(int32 result);
	explicit ConnectPlayerResponseMessage(Archive::ReadIterator &source);

	~ConnectPlayerResponseMessage();

public: // methods

	int32                            getResult() const;

public: // types

private: 
	Archive::AutoVariable<int32>   m_result;  
};

// ----------------------------------------------------------------------

inline int32 ConnectPlayerResponseMessage::getResult() const
{
	return m_result.get();
}


// ----------------------------------------------------------------------

#endif // _ConnectPlayerResponseMessage_H

