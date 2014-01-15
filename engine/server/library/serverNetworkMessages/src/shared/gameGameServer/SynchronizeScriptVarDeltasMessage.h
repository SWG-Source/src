// ======================================================================
//
// SynchronizeScriptVarDeltasMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_SynchronizeScriptVarDeltasMessage_H
#define	_INCLUDED_SynchronizeScriptVarDeltasMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <vector>

// ======================================================================

class SynchronizeScriptVarDeltasMessage: public GameNetworkMessage
{
public:
	SynchronizeScriptVarDeltasMessage(NetworkId const &networkId, std::vector<int8> const &data);
	SynchronizeScriptVarDeltasMessage(Archive::ReadIterator &source);

	NetworkId const &getNetworkId() const;
	std::vector<int8> const &getData() const;
	
private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<std::vector<int8> > m_data;

private:
	SynchronizeScriptVarDeltasMessage(SynchronizeScriptVarDeltasMessage const &);
	SynchronizeScriptVarDeltasMessage &operator=(SynchronizeScriptVarDeltasMessage const &);
};

// ======================================================================

inline NetworkId const &SynchronizeScriptVarDeltasMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline std::vector<int8> const &SynchronizeScriptVarDeltasMessage::getData() const
{
	return m_data.get();
}

// ======================================================================

#endif	// _INCLUDED_SynchronizeScriptVarDeltasMessage_H

