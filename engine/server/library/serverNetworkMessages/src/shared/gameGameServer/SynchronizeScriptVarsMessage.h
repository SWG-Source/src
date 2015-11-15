// ======================================================================
//
// SynchronizeScriptVarsMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_SynchronizeScriptVarsMessage_H
#define	_INCLUDED_SynchronizeScriptVarsMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <vector>

// ======================================================================

class SynchronizeScriptVarsMessage: public GameNetworkMessage
{
public:
	SynchronizeScriptVarsMessage(NetworkId const &networkId, std::vector<int8> const &data);
	SynchronizeScriptVarsMessage(Archive::ReadIterator &source);

	NetworkId const &getNetworkId() const;
	std::vector<int8> const &getData() const;
	
private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<std::vector<int8> > m_data;

private:
	SynchronizeScriptVarsMessage(SynchronizeScriptVarsMessage const &);
	SynchronizeScriptVarsMessage &operator=(SynchronizeScriptVarsMessage const &);
};

// ======================================================================

inline NetworkId const &SynchronizeScriptVarsMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline std::vector<int8> const &SynchronizeScriptVarsMessage::getData() const
{
	return m_data.get();
}

// ======================================================================

#endif	// _INCLUDED_SynchronizeScriptVarsMessage_H

