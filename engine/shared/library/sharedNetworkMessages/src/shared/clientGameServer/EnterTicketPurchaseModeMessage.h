// ==================================================================
//
// EnterTicketPurchaseModeMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_EnterTicketPurchaseModeMessage_H
#define	INCLUDED_EnterTicketPurchaseModeMessage_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class EnterTicketPurchaseModeMessage : public GameNetworkMessage
{
public: 

	EnterTicketPurchaseModeMessage (const std::string& planetName, const std::string& travelPointName, bool instantTravel);
	explicit EnterTicketPurchaseModeMessage (Archive::ReadIterator &source);
	virtual ~EnterTicketPurchaseModeMessage();

	const std::string& getPlanetName () const;
	const std::string& getTravelPointName () const;
	bool getInstantTravel () const;

public:

	static const char* const cms_name;

private:

	Archive::AutoVariable<std::string> m_planetName;
	Archive::AutoVariable<std::string> m_travelPointName;
	Archive::AutoVariable<bool> m_instantTravel;
};

// ==================================================================

#endif 
