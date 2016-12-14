// ======================================================================
//
// ServerSecureTrade.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ServerSecureTrade_H
#define INCLUDED_ServerSecureTrade_H

#include "sharedFoundation/Watcher.h"

class CreatureObject;
class ServerObject;

class ServerSecureTrade
{

public:

	ServerSecureTrade(CreatureObject & inititaor, CreatureObject & recipient);
	~ServerSecureTrade();

	void acceptOffer(const CreatureObject & trader);
	void addItem(const CreatureObject & trader, const ServerObject & item);
	void beginTrading();
	void cancelTrade(const CreatureObject & trader);
	void giveMoney(const CreatureObject & trader, int amount);
	void rejectOffer(const CreatureObject &trader);
	void removeItem(const CreatureObject & trader, const ServerObject & item);
	void verifyTrade(const CreatureObject & trader);
	void unacceptOffer(const CreatureObject & trader);
	bool hasItemInSecureTrade( const CreatureObject & trader, const ServerObject & item) const;

private:
	void beginVerification();
	void completeTrade();
	void exitTrade();
	void logTradeitemContents(const CreatureObject & to, const ServerObject & item, const CreatureObject & from) const;

private:

	enum TradeState
	{
		TS_Initializing,
		TS_Trading,
		TS_Verifying,
		TS_Complete
	};

	int                             m_id;

	CreatureObject*                   m_initiator;
	std::vector<Watcher<ServerObject> >*   m_initiatorContents;
	int                               m_initiatorMoney;
	bool                              m_initiatorOk;
	bool                              m_initiatorVerify;

	CreatureObject*                    m_recipient;
	std::vector<Watcher<ServerObject> >*    m_recipientContents;
	int                                m_recipientMoney;
	bool                               m_recipientOk;
	bool                               m_recipientVerify;

	TradeState                         m_tradeState;

	

	

	ServerSecureTrade (const ServerSecureTrade &);
	ServerSecureTrade & operator= (const ServerSecureTrade &);

};


#endif

