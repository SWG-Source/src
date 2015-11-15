// ======================================================================
//
// GetTicketsMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetTicketsMessage_H
#define	_GetTicketsMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class GetTicketsMessage : public GameNetworkMessage
{
public:

	GetTicketsMessage(unsigned int start, unsigned int count);
	explicit GetTicketsMessage(Archive::ReadIterator &source);

	~GetTicketsMessage();

public: 

	unsigned int                          getStart() const;
	unsigned int                          getCount() const;
	unsigned int                          getStationId() const;
	void                                  setStationId(unsigned int id);

private: 
	Archive::AutoVariable<unsigned int>    m_start;  
	Archive::AutoVariable<unsigned int>    m_count;  
	Archive::AutoVariable<unsigned int>    m_stationId;  
};

// ----------------------------------------------------------------------

inline unsigned int GetTicketsMessage::getStart() const
{
	return m_start.get();
}

// ----------------------------------------------------------------------

inline unsigned int GetTicketsMessage::getCount() const
{
	return m_count.get();
}

// ----------------------------------------------------------------------

inline unsigned int GetTicketsMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void GetTicketsMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

#endif // _GetTicketsMessage_H

