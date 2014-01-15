// ======================================================================
//
// ValidateAccountReplyMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ValidateAccountReplyMessage_H
#define INCLUDED_ValidateAccountReplyMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  Connection Server
 * Sent To:    Central
 * Action:     Do whatever validation Central can, then forward to LoginServer.
 *
 * Sent From:  Central
 * Sent To:    LoginServer
 * Action:     Check the permissions for the account (can create characters, etc.)
 *             Reply with ValidateAccountReplyMessage
 */

class ValidateAccountReplyMessage : public GameNetworkMessage
{
  public:
    ValidateAccountReplyMessage (StationId stationId, bool canLogin, bool canCreateRegular, bool canCreateJedi, bool canSkipTutorial, unsigned int track, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems);
    ValidateAccountReplyMessage (Archive::ReadIterator & source);
    virtual ~ValidateAccountReplyMessage ();

  public:
	StationId     getStationId         () const;
	bool          getCanLogin          () const;
	bool          getCanCreateRegular  () const;
	bool          getCanCreateJedi     () const;
	bool          getCanSkipTutorial   () const;
	unsigned int  getTrack             () const;
	std::vector<std::pair<NetworkId, std::string> > const & getConsumedRewardEvents() const;
	std::vector<std::pair<NetworkId, std::string> > const & getClaimedRewardItems() const;
	
  private:
    Archive::AutoVariable<StationId> m_stationId;
    Archive::AutoVariable<bool> m_canLogin;
	Archive::AutoVariable<bool> m_canCreateRegular;
	Archive::AutoVariable<bool> m_canCreateJedi;
	Archive::AutoVariable<bool> m_canSkipTutorial;
	Archive::AutoVariable<unsigned int> m_track;
	Archive::AutoArray<std::pair<NetworkId, std::string> > m_consumedRewardEvents;
	Archive::AutoArray<std::pair<NetworkId, std::string> > m_claimedRewardItems;

  private: // disable:
    ValidateAccountReplyMessage();
    ValidateAccountReplyMessage(const ValidateAccountReplyMessage&);
    ValidateAccountReplyMessage& operator= (const ValidateAccountReplyMessage&);

};

// ======================================================================

#endif
