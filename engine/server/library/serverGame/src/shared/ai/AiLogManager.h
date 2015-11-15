// ======================================================================
//
// AiLogManager.h
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiLogManager_H
#define	INCLUDED_AiLogManager_H

class NetworkId;

// ======================================================================
//
// AiLogManager
//
// ======================================================================

class AiLogManager
{
public:

	static void setLogging(NetworkId const & networkId, bool const enabled);
	static bool isLogging(NetworkId const & networkId);

private:

	AiLogManager();
	~AiLogManager();
	AiLogManager(AiLogManager const &);
	AiLogManager & operator =(AiLogManager const &);
};

// ======================================================================

#endif // INCLUDED_AiLogManager_H
