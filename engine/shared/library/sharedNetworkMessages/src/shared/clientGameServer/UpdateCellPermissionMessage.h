// ======================================================================
//
// UpdateCellPermissionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_UpdateCellPermissionMessage_H
#define	_UpdateCellPermissionMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class UpdateCellPermissionMessage : public GameNetworkMessage
{
public: //ctor/dtor
	UpdateCellPermissionMessage(NetworkId const &sourceObject, bool allowed);
	explicit UpdateCellPermissionMessage(Archive::ReadIterator &source);

	~UpdateCellPermissionMessage();

public: // methods

	bool             getAllowed() const;
	NetworkId const &getTarget() const;

public: // types

private: 
	Archive::AutoVariable<bool>       m_allowed;
	Archive::AutoVariable<NetworkId>  m_target;
};

// ----------------------------------------------------------------------

inline bool UpdateCellPermissionMessage::getAllowed() const
{
	return m_allowed.get();
}

// ----------------------------------------------------------------------

inline const NetworkId& UpdateCellPermissionMessage::getTarget() const
{
	return m_target.get();
}

// ----------------------------------------------------------------------

#endif // _UpdateCellPermissionMessage_H

