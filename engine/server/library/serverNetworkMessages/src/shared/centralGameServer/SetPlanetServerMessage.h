// ======================================================================
//
// SetPlanetServerMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetPlanetServerMessage_H
#define INCLUDED_SetPlanetServerMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class SetPlanetServerMessage : public GameNetworkMessage
{
public:
	SetPlanetServerMessage  (const std::string &address, uint16 port);
	SetPlanetServerMessage  (Archive::ReadIterator & source);
	virtual ~SetPlanetServerMessage ();

	const std::string &getAddress() const;
	uint16 getPort() const;
	
private:
	Archive::AutoVariable<std::string> m_address;
	Archive::AutoVariable<uint16> m_port;

	SetPlanetServerMessage();
	SetPlanetServerMessage(const SetPlanetServerMessage&);
	SetPlanetServerMessage& operator= (const SetPlanetServerMessage&);
};

//-----------------------------------------------------------------------

inline const std::string& SetPlanetServerMessage::getAddress(void) const
{
	return m_address.get();
}

// ----------------------------------------------------------------------

inline uint16 SetPlanetServerMessage::getPort(void) const
{
	return m_port.get();
}

// ----------------------------------------------------------------------

#endif	
