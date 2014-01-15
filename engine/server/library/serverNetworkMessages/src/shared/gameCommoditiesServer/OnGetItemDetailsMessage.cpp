#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnGetItemDetailsMessage.h"

OnGetItemDetailsMessage::OnGetItemDetailsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("OnGetItemDetailsMessage"),
		m_responseId(),
		m_trackId(),
		m_itemId(),
		m_playerId(),
		m_userDescriptionLength(),
		m_userDescription(),
		m_oobLength(),
		m_oobData(),
		m_attributes(),
		m_resultCode()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_oobLength);
	AutoByteStream::addVariable(m_oobData);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnGetItemDetailsMessage::OnGetItemDetailsMessage(
	int responseId,
	int resultCode,
	const NetworkId & itemId,
	const NetworkId & playerId,
	int userDescriptionLength,
	const Unicode::String & userDescription,
	int oobLength,
	const Unicode::String & oobData,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes
	) :
		GameNetworkMessage("OnGetItemDetailsMessage"),
		m_responseId(responseId),
		m_trackId(0),
		m_itemId(itemId),
		m_playerId(playerId),
		m_userDescriptionLength(userDescriptionLength),
		m_userDescription(userDescription),
		m_oobLength(oobLength),
		m_oobData(oobData),
		m_attributes(),
		m_resultCode(resultCode)
{
	m_attributes.set(attributes);
	
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_oobLength);
	AutoByteStream::addVariable(m_oobData);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_resultCode);
}

OnGetItemDetailsMessage::~OnGetItemDetailsMessage()
{
}

