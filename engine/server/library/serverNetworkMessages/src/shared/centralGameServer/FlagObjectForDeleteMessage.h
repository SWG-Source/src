//========================================================================
//
// FlagObjectForDeleteMessage.h - tells CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_FlagObjectForDeleteMessage_H
#define	_INCLUDED_FlagObjectForDeleteMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class FlagObjectForDeleteMessage : public GameNetworkMessage
{
public:

	explicit FlagObjectForDeleteMessage  (const NetworkId& id, int reason, bool immediate, bool demandLoadedContainer, bool cascadeReason);
	FlagObjectForDeleteMessage           (Archive::ReadIterator & source);
	~FlagObjectForDeleteMessage ();

	const NetworkId&          getId() const;
	int                       getReason() const;
	bool                      getImmediate() const;
	bool                      getDemandLoadedContainer() const;
	bool                      getCascadeReason() const;

private:
	Archive::AutoVariable<NetworkId> m_id;
	Archive::AutoVariable<int>       m_reason;
	Archive::AutoVariable<bool>      m_immediate;
	Archive::AutoVariable<bool>      m_demandLoadedContainer;
	Archive::AutoVariable<bool>      m_cascadeReason;

	FlagObjectForDeleteMessage();
	FlagObjectForDeleteMessage(const FlagObjectForDeleteMessage&);
	FlagObjectForDeleteMessage& operator= (const FlagObjectForDeleteMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& FlagObjectForDeleteMessage::getId(void) const
{
	return m_id.get();
}	// FlagObjectForDeleteMessage::getId

// ----------------------------------------------------------------------

inline int FlagObjectForDeleteMessage::getReason(void) const
{
	return m_reason.get();
}	

// ----------------------------------------------------------------------

inline bool FlagObjectForDeleteMessage::getImmediate() const
{
	return m_immediate.get();
}

// ----------------------------------------------------------------------

inline bool FlagObjectForDeleteMessage::getDemandLoadedContainer() const
{
	return m_demandLoadedContainer.get();
}

// ----------------------------------------------------------------------

inline bool FlagObjectForDeleteMessage::getCascadeReason() const
{
	return m_cascadeReason.get();
}

// ======================================================================

#endif	// _INCLUDED_FlagObjectForDeleteMessage_H
