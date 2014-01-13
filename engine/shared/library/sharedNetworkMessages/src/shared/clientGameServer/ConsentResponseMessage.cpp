// ======================================================================
//
// ConsentResponseMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ConsentResponseMessage.h"


// ======================================================================

const char * const ConsentResponseMessage::cms_name = "ConsentResponseMessage";

// ======================================================================

ConsentResponseMessage::ConsentResponseMessage(const NetworkId& networkId, int id, bool response)
: GameNetworkMessage (cms_name),
  m_networkId        (networkId),
  m_id               (id),
  m_response         (response)
{
	addVariable(m_networkId);
	addVariable(m_id);
	addVariable(m_response);
}

//-----------------------------------------------------------------------

ConsentResponseMessage::ConsentResponseMessage(Archive::ReadIterator & source)
: GameNetworkMessage (cms_name),
  m_networkId        (),
  m_id               (),
  m_response         ()
{
	addVariable(m_networkId);
	addVariable(m_id);
	addVariable(m_response);
	unpack(source);
}

// ----------------------------------------------------------------------

ConsentResponseMessage::~ConsentResponseMessage()
{
}

// ======================================================================
