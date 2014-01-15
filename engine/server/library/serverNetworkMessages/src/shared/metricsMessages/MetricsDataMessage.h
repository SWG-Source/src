// MetricsDataMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 


#ifndef	_INCLUDED_MetricsDataMessage_H
#define	_INCLUDED_MetricsDataMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

struct MetricsPair
{
	std::string  m_label;
	int          m_value;
	std::string  m_description;
	bool         m_persistData;
	bool         m_summary;
};

//-----------------------------------------------------------------------

class MetricsDataMessage : public GameNetworkMessage
{
public:
	MetricsDataMessage();
	explicit MetricsDataMessage(Archive::ReadIterator & source);
	~MetricsDataMessage();
	
	const std::vector<MetricsPair> & getData () const;
	void                             setData (const std::vector<MetricsPair>&);
	
private:
	MetricsDataMessage & operator = (const MetricsDataMessage & rhs);
	MetricsDataMessage(const MetricsDataMessage & source);

	Archive::AutoArray<MetricsPair> m_data;
};

//-----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, MetricsPair &c);
	void put(ByteStream & target, const MetricsPair &c);
}

//-----------------------------------------------------------------------
inline const std::vector<MetricsPair> & MetricsDataMessage::getData () const 
{
	return m_data.get();
}

//-----------------------------------------------------------------------

inline void MetricsDataMessage::setData(const std::vector<MetricsPair>& data)
{
	m_data.set(data);
}

//-----------------------------------------------------------------------

#ifndef ADD_METRICS_DATA

#define ADD_METRICS_DATA(label, defaultValue, persistData)   \
p.m_label = #label; \
p.m_value = defaultValue; \
p.m_description = ""; \
p.m_persistData = persistData; \
p.m_summary = false; \
m_ ## label = m_data.size(); \
m_data.push_back(p);

#endif //ADD_METRICS_DATA


//-----------------------------------------------------------------------





#endif
