// MetricsInititaionMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 


#ifndef	_INCLUDED_MetricsInitiationMessage_H
#define	_INCLUDED_MetricsInitiationMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class MetricsInitiationMessage : public GameNetworkMessage
{
public:
	MetricsInitiationMessage(bool isDynamic, const std::string & primaryName,const std::string & secondaryName, const int index);
	explicit MetricsInitiationMessage(Archive::ReadIterator & source);
	~MetricsInitiationMessage();

	bool                getIsDynamic() const;
	const std::string & getPrimaryName() const;
	const std::string & getSecondaryName() const;
	const int           getIndex() const;

private:
	MetricsInitiationMessage & operator = (const MetricsInitiationMessage & rhs);
	MetricsInitiationMessage(const MetricsInitiationMessage & source);

	Archive::AutoVariable<bool>        m_isDynamic;
	Archive::AutoVariable<std::string> m_primaryName;
	Archive::AutoVariable<std::string> m_secondaryName;
	Archive::AutoVariable<int>         m_index;
};

//-----------------------------------------------------------------------

inline bool MetricsInitiationMessage::getIsDynamic() const
{
	return m_isDynamic.get();
}

//-----------------------------------------------------------------------
inline const std::string & MetricsInitiationMessage::getPrimaryName() const
{
	return m_primaryName.get();
}
//-----------------------------------------------------------------------
inline const std::string & MetricsInitiationMessage::getSecondaryName() const
{
	return m_secondaryName.get();
}

//-----------------------------------------------------------------------

inline const int MetricsInitiationMessage::getIndex() const
{
	return m_index.get();
}

//-----------------------------------------------------------------------

#endif
