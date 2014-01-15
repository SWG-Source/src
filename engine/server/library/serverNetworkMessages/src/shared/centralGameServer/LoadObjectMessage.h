// ======================================================================
//
// LoadObjectMessage.h - tells Gameserver another Gameserver wants to load a
// proxy from an authoritative object.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_LoadObjectMessage_H
#define	_INCLUDED_LoadObjectMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LoadObjectMessage: public GameNetworkMessage
{
public:
	LoadObjectMessage(NetworkId const &id, uint32 process);
	LoadObjectMessage(Archive::ReadIterator &source);
	~LoadObjectMessage();

	NetworkId const &   getId() const;
	uint32              getProcess() const;

private:
	Archive::AutoVariable<NetworkId>   m_id;       // id of object we want to load
	Archive::AutoVariable<uint32>      m_process;  // process id of Gameserver that wants to load the object

	LoadObjectMessage();
	LoadObjectMessage(LoadObjectMessage const &);
	LoadObjectMessage &operator=(LoadObjectMessage const &);
};

// ----------------------------------------------------------------------

inline NetworkId const &LoadObjectMessage::getId() const
{
	return m_id.get();
}

// ----------------------------------------------------------------------

inline uint32 LoadObjectMessage::getProcess() const
{
	return m_process.get();
}

// ======================================================================

#endif	// _INCLUDED_LoadObjectMessage_H

