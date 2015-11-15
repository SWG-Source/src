#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "SPCharacterProfileMessage.h"

SPCharacterProfileMessage::SPCharacterProfileMessage(Archive::ReadIterator & source) :
GameNetworkMessage("SPCharacterProfileMessage"),
m_clusterName(),
m_characterId(),
m_characterName(),
m_objectName(),
m_x(),
m_y(),
m_z(),
m_sceneId(),
m_cash_balance(),
m_bank_balance(),
m_objectTemplateName(),
m_stationId(),
m_containedby(),
m_createTime(),
m_playedTime(),
m_numLots()
{
	AutoByteStream::addVariable(m_clusterName);
	AutoByteStream::addVariable(m_characterId);
	AutoByteStream::addVariable(m_characterName);
	AutoByteStream::addVariable(m_objectName);
	AutoByteStream::addVariable(m_x);
	AutoByteStream::addVariable(m_y);
	AutoByteStream::addVariable(m_z);
	AutoByteStream::addVariable(m_sceneId);
	AutoByteStream::addVariable(m_cash_balance);
	AutoByteStream::addVariable(m_bank_balance);
	AutoByteStream::addVariable(m_objectTemplateName);
	AutoByteStream::addVariable(m_stationId);
	AutoByteStream::addVariable(m_containedby);
	AutoByteStream::addVariable(m_createTime);
	AutoByteStream::addVariable(m_playedTime);
	AutoByteStream::addVariable(m_numLots);
	unpack(source);
}

SPCharacterProfileMessage::SPCharacterProfileMessage(
		const std::string & clusterName,
		const NetworkId & characterId,
		const std::string & characterName,
		const std::string & objectName,
		float x,
		float y,
		float z,
		const std::string & sceneId,
		float cash_balance,
		float bank_balance,
		const std::string & objectTemplateName,
		int   stationId,
		const NetworkId & containedBy,
		int   createTime,
		int   playedTime,
		int   numLots
) :
GameNetworkMessage("SPCharacterProfileMessage"),
m_clusterName(clusterName),
m_characterId(characterId),
m_characterName(characterName),
m_objectName(objectName),
m_x(x),
m_y(y),
m_z(z),
m_sceneId(sceneId),
m_cash_balance(cash_balance),
m_bank_balance(bank_balance),
m_objectTemplateName(objectTemplateName),
m_stationId(stationId),
m_containedby(containedBy),
m_createTime(createTime),
m_playedTime(playedTime),
m_numLots(numLots)
{
	AutoByteStream::addVariable(m_clusterName);
	AutoByteStream::addVariable(m_characterId);
	AutoByteStream::addVariable(m_characterName);
	AutoByteStream::addVariable(m_objectName);
	AutoByteStream::addVariable(m_x);
	AutoByteStream::addVariable(m_y);
	AutoByteStream::addVariable(m_z);
	AutoByteStream::addVariable(m_sceneId);
	AutoByteStream::addVariable(m_cash_balance);
	AutoByteStream::addVariable(m_bank_balance);
	AutoByteStream::addVariable(m_objectTemplateName);
	AutoByteStream::addVariable(m_stationId);
	AutoByteStream::addVariable(m_containedby);
	AutoByteStream::addVariable(m_createTime);
	AutoByteStream::addVariable(m_playedTime);
	AutoByteStream::addVariable(m_numLots);
}

SPCharacterProfileMessage::~SPCharacterProfileMessage()
{
}

