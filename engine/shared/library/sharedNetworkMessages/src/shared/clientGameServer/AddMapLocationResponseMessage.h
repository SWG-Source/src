// ======================================================================
//
// AddMapLocationResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AddMapLocationResponseMessage_H
#define	_AddMapLocationResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class AddMapLocationResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	AddMapLocationResponseMessage(const NetworkId &locationId);
	explicit AddMapLocationResponseMessage(Archive::ReadIterator &source);

	~AddMapLocationResponseMessage();

public: // methods

	const NetworkId          & getLocationId() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_locationId;
};

// ----------------------------------------------------------------------

inline const NetworkId & AddMapLocationResponseMessage::getLocationId() const
{
	return m_locationId.get();
}

// ----------------------------------------------------------------------

#endif // _AddMapLocationResponseMessage_H

