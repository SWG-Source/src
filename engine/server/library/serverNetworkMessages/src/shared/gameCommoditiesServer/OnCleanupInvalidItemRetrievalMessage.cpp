#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnCleanupInvalidItemRetrievalMessage.h"

OnCleanupInvalidItemRetrievalMessage::OnCleanupInvalidItemRetrievalMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnCleanupInvalidItemRetrievalMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_playerId(),
m_creatorId(),
m_reimburseAmt()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_creatorId);
	AutoByteStream::addVariable(m_reimburseAmt);
	unpack(source);
}

OnCleanupInvalidItemRetrievalMessage::OnCleanupInvalidItemRetrievalMessage(
		int responseId,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const NetworkId & creatorId,
		int reimburseAmt
) :
GameNetworkMessage("OnCleanupInvalidItemRetrievalMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_playerId(playerId),
m_creatorId(creatorId),
m_reimburseAmt(reimburseAmt)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_creatorId);
	AutoByteStream::addVariable(m_reimburseAmt);
}

OnCleanupInvalidItemRetrievalMessage::~OnCleanupInvalidItemRetrievalMessage()
{
}

