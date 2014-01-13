// ======================================================================
//
// FactionResponseMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/FactionResponseMessage.h"

// ======================================================================

const char * const FactionResponseMessage::MessageType = "FactionResponseMessage";

//----------------------------------------------------------------------

FactionResponseMessage::FactionResponseMessage(int rebel, int imperial, int criminal, const std::vector<std::string>& npcFactionNameList, const std::vector<float>& npcFactionValueList)
: GameNetworkMessage    (MessageType),
m_factionRebelValue   (rebel),
m_factionImperialValue(imperial),
m_factionCriminalValue(criminal),
m_npcFactionNameList(),
m_npcFactionValueList()
{
	m_npcFactionNameList.set(npcFactionNameList);
	m_npcFactionValueList.set(npcFactionValueList);

	addVariable(m_factionRebelValue);
	addVariable(m_factionImperialValue);
	addVariable(m_factionCriminalValue);
	addVariable(m_npcFactionNameList);
	addVariable(m_npcFactionValueList);
}

//-----------------------------------------------------------------------

FactionResponseMessage::FactionResponseMessage(Archive::ReadIterator & source)
: GameNetworkMessage(MessageType),
m_factionRebelValue(),
m_factionImperialValue(),
m_factionCriminalValue(),
m_npcFactionNameList(),
m_npcFactionValueList()
{
	addVariable(m_factionRebelValue);
	addVariable(m_factionImperialValue);
	addVariable(m_factionCriminalValue);
	addVariable(m_npcFactionNameList);
	addVariable(m_npcFactionValueList);
	unpack(source);
}

// ----------------------------------------------------------------------

const std::vector<std::string> & FactionResponseMessage::getNPCFactionNames()  const
{
	return m_npcFactionNameList.get();
}

// ----------------------------------------------------------------------

const std::vector<float> & FactionResponseMessage::getNPCFactionValues() const
{
	return m_npcFactionValueList.get();
}

// ----------------------------------------------------------------------

FactionResponseMessage::~FactionResponseMessage()
{
}

// ======================================================================
