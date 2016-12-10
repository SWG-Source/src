// ======================================================================
//
// GetMoneyFromOfflineObjectMessage.h
// Copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetMoneyFromOfflineObjectMessage_H
#define INCLUDED_GetMoneyFromOfflineObjectMessage_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Game Server
 * Sent to:  DB Process
 * Action:  Take money from the specified object.
 * Note:  m_success is not meaningful when the message is used this way.
 *
 * Sent from:  DB Process
 * Sent to:  Game Server
 * Action:  Tell the game server whether the offline money grab worked.
 */
class GetMoneyFromOfflineObjectMessage : public GameNetworkMessage
{
  public:
	GetMoneyFromOfflineObjectMessage  (NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary, bool success);
	GetMoneyFromOfflineObjectMessage  (Archive::ReadIterator & source);
	~GetMoneyFromOfflineObjectMessage ();

  public:
	NetworkId const & getSourceObject() const;
	int getAmount() const;
	NetworkId const & getReplyTo() const;
	std::string const & getSuccessCallback() const;
	std::string const & getFailCallback() const;
	std::vector<int8> const & getPackedDictionary() const;
	bool getSuccess() const;
	
  private:
	Archive::AutoVariable<NetworkId> m_sourceObject;
	Archive::AutoVariable<int> m_amount;
	Archive::AutoVariable<NetworkId> m_replyTo;
	Archive::AutoVariable<std::string> m_successCallback;
	Archive::AutoVariable<std::string> m_failCallback;
	Archive::AutoVariable<std::vector<int8> > m_packedDictionary;
	Archive::AutoVariable<bool> m_success;
	
  private:        
	GetMoneyFromOfflineObjectMessage(const GetMoneyFromOfflineObjectMessage&);
	GetMoneyFromOfflineObjectMessage& operator= (const GetMoneyFromOfflineObjectMessage&);
};

// ======================================================================

#endif
