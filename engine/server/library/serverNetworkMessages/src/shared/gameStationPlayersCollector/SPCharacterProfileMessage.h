#ifndef SPCharacterProfileMessage_h
#define SPCharacterProfileMessage_h

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class SPCharacterProfileMessage : public GameNetworkMessage
{

public:
	SPCharacterProfileMessage(
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
	);
	SPCharacterProfileMessage(Archive::ReadIterator & source);
	~SPCharacterProfileMessage();

	const std::string &        GetClusterName() const {return m_clusterName.get();}
	const NetworkId &          GetCharacterId() const {return m_characterId.get();}
	const std::string &        GetCharacterName() const {return m_characterName.get();}
	const std::string &        GetObjectName() const {return m_objectName.get();}
	float                      GetX() const {return m_x.get();}
	float                      GetY() const {return m_y.get();}
	float                      GetZ() const {return m_z.get();}
	const std::string &        GetSceneId() const {return m_sceneId.get();}
	float                      GetCash_Balance() const {return m_cash_balance.get();}
	float                      GetBank_Balance() const {return m_bank_balance.get();}
	const std::string &        GetObjectTemplateName() const {return m_objectTemplateName.get();}
	int                        GetStationId() const {return m_stationId.get();}
	const NetworkId &          GetContainedBy() const {return m_containedby.get();}
	int                        GetCreateTime() const {return m_createTime.get();}
	int                        GetPlayedTime() const {return m_playedTime.get();}
	int                        GetNumLots() const {return m_numLots.get();}

protected:

private:
	Archive::AutoVariable<std::string>      m_clusterName;
	Archive::AutoVariable<NetworkId>        m_characterId;
	Archive::AutoVariable<std::string>      m_characterName;
	Archive::AutoVariable<std::string>      m_objectName;
	Archive::AutoVariable<float>            m_x;
	Archive::AutoVariable<float>            m_y;
	Archive::AutoVariable<float>            m_z;
	Archive::AutoVariable<std::string>      m_sceneId;
	Archive::AutoVariable<float>            m_cash_balance;
	Archive::AutoVariable<float>            m_bank_balance;
	Archive::AutoVariable<std::string>      m_objectTemplateName;
	Archive::AutoVariable<int>              m_stationId;
	Archive::AutoVariable<NetworkId>        m_containedby;
	Archive::AutoVariable<int>              m_createTime;
	Archive::AutoVariable<int>              m_playedTime;
	Archive::AutoVariable<int>              m_numLots;

};

#endif
