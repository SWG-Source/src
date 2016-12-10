// ======================================================================
//
// FactionResponseMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FactionResponseMessage_H
#define INCLUDED_FactionResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Connection Server
 * Sent to:    Client
 * Action:     Tells the client about it's factions
 *             connected to.
 */
class FactionResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	FactionResponseMessage(int rebel, int imperial, int criminal, const std::vector<std::string> & npcFactionNameList, const std::vector<float> & npcFactionValueList);
	explicit FactionResponseMessage(Archive::ReadIterator & source);
	virtual ~FactionResponseMessage();

public:
	int                getFactionRebel   () const;
	int                getFactionImperial() const;
	int                getFactionCriminal() const;
	const std::vector<std::string> & getNPCFactionNames()  const;
	const std::vector<float>       & getNPCFactionValues() const;
	
private:
	Archive::AutoVariable<int>         m_factionRebelValue;
	Archive::AutoVariable<int>         m_factionImperialValue;
	Archive::AutoVariable<int>         m_factionCriminalValue;
	Archive::AutoArray<std::string>    m_npcFactionNameList;
	Archive::AutoArray<float>          m_npcFactionValueList;

	FactionResponseMessage();
	FactionResponseMessage(const FactionResponseMessage&);
	FactionResponseMessage& operator= (const FactionResponseMessage&);
};

// ----------------------------------------------------------------------

inline int FactionResponseMessage::getFactionRebel() const
{
	return m_factionRebelValue.get();
}

// ----------------------------------------------------------------------

inline int FactionResponseMessage::getFactionImperial() const
{
	return m_factionImperialValue.get();
}

// ----------------------------------------------------------------------

inline int FactionResponseMessage::getFactionCriminal() const
{
	return m_factionCriminalValue.get();
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
