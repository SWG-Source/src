// ======================================================================
//
// LoginUpgradeAccountMessage.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginUpgradeAccountMessage_H
#define INCLUDED_LoginUpgradeAccountMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  GameServer
 * Sent To:    LoginServer (via Central)
 * Action:     Upgrade or adjust an account based on in-game player actions.
 *
 * Sent From:  LoginServer
 * Sent To:    Central
 * Action:     Acknowldege the upgrade.  isAck should be set to true.
 */

class LoginUpgradeAccountMessage : public GameNetworkMessage
{
  public:
	enum UpgradeType {UT_addJedi, UT_setSpectral};
	enum OccupyUnlockedSlotResponse {OUSR_success, OUSR_db_error, OUSR_account_has_no_unlocked_slot, OUSR_account_has_no_unoccupied_unlocked_slot, OUSR_cluster_already_has_unlocked_slot_character};
	enum VacateUnlockedSlotResponse {VUSR_success, VUSR_db_error, VUSR_account_has_no_unlocked_slot, VUSR_not_unlocked_slot_character, VUSR_no_available_normal_character_slot};
	enum SwapUnlockedSlotResponse   {SUSR_success, SUSR_db_error, SUSR_account_has_no_unlocked_slot, SUSR_not_unlocked_slot_character, SUSR_invalid_target_character, SUSR_target_character_already_unlocked_slot_character};

  public:
    LoginUpgradeAccountMessage (UpgradeType upgradeType, const StationId &stationId, const NetworkId &character, const NetworkId &replyToObject, const std::string &replyMessage, const std::string &packedMessageData, bool isAck);
    LoginUpgradeAccountMessage (Archive::ReadIterator & source);
    virtual ~LoginUpgradeAccountMessage ();

  public:
	UpgradeType         getUpgradeType() const;
	StationId           getStationId() const;
	const NetworkId &   getCharacter() const;
	const NetworkId &   getReplyToObject() const;
	const std::string & getReplyMessage() const;
	const std::string & getPackedMessageData() const;
	bool                getIsAck() const;

	void                setAck();
	
  private:
    Archive::AutoVariable<int> m_upgradeType;
	Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoVariable<NetworkId> m_character;
	Archive::AutoVariable<NetworkId> m_replyToObject;
	Archive::AutoVariable<std::string> m_replyMessage;
	Archive::AutoVariable<std::string> m_packedMessageData;
	Archive::AutoVariable<bool> m_isAck;	
  
  private: // disable:
    LoginUpgradeAccountMessage();
    LoginUpgradeAccountMessage(const LoginUpgradeAccountMessage&);
    LoginUpgradeAccountMessage& operator= (const LoginUpgradeAccountMessage&);
};

// ======================================================================

inline LoginUpgradeAccountMessage::UpgradeType LoginUpgradeAccountMessage::getUpgradeType() const
{
	return static_cast<UpgradeType>(m_upgradeType.get());
}

// ----------------------------------------------------------------------


inline StationId LoginUpgradeAccountMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & LoginUpgradeAccountMessage::getCharacter() const
{
	return m_character.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & LoginUpgradeAccountMessage::getReplyToObject() const
{
	return m_replyToObject.get();
}

// ----------------------------------------------------------------------

inline const std::string & LoginUpgradeAccountMessage::getReplyMessage() const
{
	return m_replyMessage.get();
}

// ----------------------------------------------------------------------

inline const std::string & LoginUpgradeAccountMessage::getPackedMessageData() const
{
	return m_packedMessageData.get();
}

// ----------------------------------------------------------------------

inline bool LoginUpgradeAccountMessage::getIsAck() const
{
	return m_isAck.get();
}

// ======================================================================

#endif
