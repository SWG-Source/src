// MetricsInitiationMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 


#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "MetricsInitiationMessage.h"


//-----------------------------------------------------------------------


MetricsInitiationMessage::MetricsInitiationMessage(bool isDynamic, const std::string & primaryName,const std::string & secondaryName, const int index) :
		GameNetworkMessage("MetricsInitiationMessage"),
		m_isDynamic(isDynamic),
		m_primaryName(primaryName),
		m_secondaryName(secondaryName),
		m_index(index)
{
	addVariable(m_isDynamic);
	addVariable(m_primaryName);
	addVariable(m_secondaryName);
	addVariable(m_index);
}

//-----------------------------------------------------------------------
MetricsInitiationMessage::MetricsInitiationMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("MetricsInitiationMessage"),
		m_isDynamic(false),
		m_primaryName(),
		m_secondaryName(),
		m_index()
{
	addVariable(m_isDynamic);
	addVariable(m_primaryName);
	addVariable(m_secondaryName);
	addVariable(m_index);
	unpack(source);
}

//-----------------------------------------------------------------------

MetricsInitiationMessage::~MetricsInitiationMessage()
{
}

//-----------------------------------------------------------------------

