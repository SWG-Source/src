#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CleanupInvalidItemRetrievalMessage.h"

CleanupInvalidItemRetrievalMessage::CleanupInvalidItemRetrievalMessage(Archive::ReadIterator & source) :
GameNetworkMessage("CleanupInvalidItemRetrievalMessage"),
m_responseId(),
m_trackId(),
m_itemId()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	unpack(source);
}

CleanupInvalidItemRetrievalMessage::CleanupInvalidItemRetrievalMessage( int responseId, const NetworkId & itemId) :
GameNetworkMessage("CleanupInvalidItemRetrievalMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
}

CleanupInvalidItemRetrievalMessage::~CleanupInvalidItemRetrievalMessage()
{
}

