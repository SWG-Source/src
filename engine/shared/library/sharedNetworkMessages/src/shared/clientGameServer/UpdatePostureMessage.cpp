// UpdatePostureMessage.cpp
// copyright 2001 Verant Interactive
//
//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdatePostureMessage.h"

UpdatePostureMessage::UpdatePostureMessage(const NetworkId & sourceObject, uint8 posture) : 
GameNetworkMessage("UpdatePostureMessage"),
m_posture(posture),
m_target(sourceObject)
{
	AutoByteStream::addVariable(m_posture);
	AutoByteStream::addVariable(m_target);
}

//-----------------------------------------------------------------------

UpdatePostureMessage::UpdatePostureMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UpdatePostureMessage"),
m_posture(),
m_target(NetworkId::cms_invalid)
{
	AutoByteStream::addVariable(m_posture);
	AutoByteStream::addVariable(m_target);
	unpack(source);
}

//-----------------------------------------------------------------------

UpdatePostureMessage::~UpdatePostureMessage()
{
}

//-----------------------------------------------------------------------

void UpdatePostureMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//-----------------------------------------------------------------------
