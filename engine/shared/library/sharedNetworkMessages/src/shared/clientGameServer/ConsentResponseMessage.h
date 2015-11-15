// ======================================================================
//
// ConsentResponseMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsentResponseMessage_H
#define INCLUDED_ConsentResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Connection Server
 * Sent to:    Client
 * Action:     Tells the client about it's Consents
 *             connected to.
 */
class ConsentResponseMessage : public GameNetworkMessage
{
public:
	ConsentResponseMessage(const NetworkId& networkId, int id, bool response);
	explicit ConsentResponseMessage(Archive::ReadIterator & source);
	virtual ~ConsentResponseMessage();

public:
	static const char * const cms_name;

public:
	const NetworkId& getNetworkId() const;
	int  getId() const;
	bool getResponse() const;

private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<int>       m_id;
	Archive::AutoVariable<bool>      m_response;

	ConsentResponseMessage();
	ConsentResponseMessage(const ConsentResponseMessage&);
	ConsentResponseMessage& operator= (const ConsentResponseMessage&);
};

// ----------------------------------------------------------------------

inline const NetworkId& ConsentResponseMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline int ConsentResponseMessage::getId() const
{
	return m_id.get();
}

// ----------------------------------------------------------------------

inline bool ConsentResponseMessage::getResponse() const
{
	return m_response.get();
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
