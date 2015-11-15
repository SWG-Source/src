//======================================================================
//
// SuiEventSubscription.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiEventSubscription_H
#define INCLUDED_SuiEventSubscription_H

//======================================================================

class SuiEventSubscription
{
public:
	SuiEventSubscription();
	SuiEventSubscription(int suiEventType, std::string const & eventWidgetName);

	bool operator<(SuiEventSubscription const & rhs) const;

	int getSuiEventType() const;
	std::string const & getEventWidgetName() const;

private:
	int m_suiEventType;
	std::string m_eventWidgetName;
};

//----------------------------------------------------------------------

inline bool SuiEventSubscription::operator<(SuiEventSubscription const & rhs) const
{
	if (m_suiEventType < rhs.m_suiEventType)
		return true;
	if (m_suiEventType == rhs.m_suiEventType)
		return m_eventWidgetName < rhs.m_eventWidgetName;

	return false;
}

//----------------------------------------------------------------------

inline int SuiEventSubscription::getSuiEventType() const
{
	return m_suiEventType;
}

//----------------------------------------------------------------------

inline std::string const & SuiEventSubscription::getEventWidgetName() const
{
	return m_eventWidgetName;
}

//======================================================================

#endif
