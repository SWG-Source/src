// SetTransformMessage.cpp
// copyright 2002 Verant Interactive

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SetTransformMessage.h"

//-----------------------------------------------------------------------

SetTransformMessage::SetTransformMessage(const NetworkId & id, const Transform& transform, const NetworkId& cellId) :
GameNetworkMessage("SetTransformMessage"),
m_id(id),
m_transform(transform),
m_cellId(cellId)
{
	addVariable(m_id);
	addVariable(m_transform);
	addVariable(m_cellId);
}

//-----------------------------------------------------------------------

SetTransformMessage::SetTransformMessage(Archive::ReadIterator & source) :
GameNetworkMessage("SetTransformMessage"),
m_id(NetworkId::cms_invalid),
m_transform(),
m_cellId(NetworkId::cms_invalid)
{
	addVariable(m_id);
	addVariable(m_transform);
	addVariable(m_cellId);
	unpack(source);
}

//-----------------------------------------------------------------------

SetTransformMessage::~SetTransformMessage  ()
{
}

//-----------------------------------------------------------------------

void SetTransformMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//-----------------------------------------------------------------------
