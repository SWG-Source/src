// ======================================================================
//
// ConsentRequestMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/ConsentRequestMessage.h"

// ======================================================================

const char * const ConsentRequestMessage::cms_name = "ConsentRequestMessage";

// ======================================================================

ConsentRequestMessage::ConsentRequestMessage(const ProsePackage& question, int id)
: GameNetworkMessage(cms_name),
  m_question(question),
  m_id(id)
{
	addVariable(m_question);
	addVariable(m_id);
}

//-----------------------------------------------------------------------

ConsentRequestMessage::ConsentRequestMessage(Archive::ReadIterator & source)
: GameNetworkMessage(cms_name),
  m_question(),
  m_id()
{
	addVariable(m_question);
	addVariable(m_id);
	unpack(source);
}

// ----------------------------------------------------------------------

ConsentRequestMessage::~ConsentRequestMessage()
{
}

// ======================================================================
