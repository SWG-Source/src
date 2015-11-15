// ======================================================================
//
// UpdatePvpStatusMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_UpdatePvpStatusMessage_H
#define	_UpdatePvpStatusMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class UpdatePvpStatusMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

public: //ctor/dtor
	UpdatePvpStatusMessage(NetworkId const &sourceObject, uint32 flags, uint32 factionId);
	explicit UpdatePvpStatusMessage(Archive::ReadIterator &source);

	~UpdatePvpStatusMessage();

public: // methods

	uint32           getFlags() const;
	uint32           getFactionId() const;
	NetworkId const &getTarget() const;

public: // types

private: 
	Archive::AutoVariable<uint32>     m_flags;
	Archive::AutoVariable<uint32>     m_factionId;
	Archive::AutoVariable<NetworkId>  m_target;
};

// ----------------------------------------------------------------------

inline uint32 UpdatePvpStatusMessage::getFlags() const
{
	return m_flags.get();
}

// ----------------------------------------------------------------------

inline uint32 UpdatePvpStatusMessage::getFactionId() const
{
	return m_factionId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId& UpdatePvpStatusMessage::getTarget() const
{
	return m_target.get();
}

// ----------------------------------------------------------------------

#endif // _UpdatePvpStatusMessage_H

