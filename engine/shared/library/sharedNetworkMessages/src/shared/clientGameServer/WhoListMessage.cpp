// ======================================================================
//
// WhoListMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/WhoListMessage.h"

//-----------------------------------------------------------------------

const char * const WhoListMessage::MESSAGE_TYPE = "WhoListMessage::MESSAGE_TYPE";

//-----------------------------------------------------------------

WhoListMessage::WhoListMessage() :
GameNetworkMessage (MESSAGE_TYPE),
m_data ()
{
	addVariable (m_data);
}

//-----------------------------------------------------------------------

WhoListMessage::WhoListMessage(Archive::ReadIterator & source) :
GameNetworkMessage (MESSAGE_TYPE),
m_data ()
{
	addVariable (m_data);
	unpack (source);
}

//-----------------------------------------------------------------------

void WhoListMessage::setData (const std::vector<Unicode::String> & data)
{
	m_data.set (data);
}

//----------------------------------------------------------------------

WhoListMessage::~WhoListMessage()
{
}

//-----------------------------------------------------------------------
