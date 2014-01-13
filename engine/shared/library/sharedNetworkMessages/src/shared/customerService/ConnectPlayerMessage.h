// ======================================================================
//
// ConnectPlayerMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_ConnectPlayerMessage_H
#define	_ConnectPlayerMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class ConnectPlayerMessage : public GameNetworkMessage
{
public:

	ConnectPlayerMessage();
	explicit ConnectPlayerMessage(Archive::ReadIterator &source);

	~ConnectPlayerMessage();

public: 

	unsigned int                          getStationId() const;
	void                                  setStationId(unsigned int id);

private: 
	Archive::AutoVariable<unsigned int>   m_stationId;  
};

// ----------------------------------------------------------------------

inline unsigned int ConnectPlayerMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void ConnectPlayerMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

#endif // _ConnectPlayerMessage_H

