// ======================================================================
//
// ServerSecureTradeManager.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#ifndef INCLUDED_ServerSecureTradeManager_H
#define INCLUDED_ServerSecureTradeManager_H

class CreatureObject;
class ServerSecureTrade;

class ServerSecureTradeManager
{
public:
	static void install();
	static void remove();

	static bool requestTradeWith (CreatureObject & initiator, CreatureObject & recipient);
	static void acceptTradeRequest (CreatureObject & initiator, CreatureObject & recipient);
	static void refuseTrade(CreatureObject & initiator, CreatureObject & recipient);
	static void clearTrade (CreatureObject & initiator, CreatureObject & recipient);

private:
	ServerSecureTradeManager();
	~ServerSecureTradeManager();

	static void   beginPreTrade(CreatureObject & initiator, CreatureObject & recipient);

};


#endif

