// MetricsDataMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 


#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "MetricsDataMessage.h"


//-----------------------------------------------------------------------


MetricsDataMessage::MetricsDataMessage() :
		GameNetworkMessage("MetricsDataMessage")
{
	addVariable(m_data);
}

//-----------------------------------------------------------------------
MetricsDataMessage::MetricsDataMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("MetricsDataMessage")
{
	addVariable(m_data);
	unpack(source);
}

//-----------------------------------------------------------------------

MetricsDataMessage::~MetricsDataMessage()
{
}

//-----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, MetricsPair &c)
	{
		get(source,c.m_label);
		get(source,c.m_value);
		get(source,c.m_description);
		get(source,c.m_persistData);
		get(source,c.m_summary);
	}

	void put(ByteStream & target, const MetricsPair &c)
	{
		put(target,c.m_label);
		put(target,c.m_value);
		put(target,c.m_description);
		put(target,c.m_persistData);
		put(target,c.m_summary);
	}
}

//-----------------------------------------------------------------------
