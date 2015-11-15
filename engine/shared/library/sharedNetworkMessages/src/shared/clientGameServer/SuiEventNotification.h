//======================================================================
//
// SuiEventNotification.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiEventNotification_H
#define INCLUDED_SuiEventNotification_H

//======================================================================

#include "Archive/AutoDeltaVector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//----------------------------------------------------------------------

class SuiEventNotification : public GameNetworkMessage
{
public:

	static char const * const MessageType;

public:

	SuiEventNotification(int pageId, int subscribedEventIndex, int reservePropertyCount);
	explicit SuiEventNotification(Archive::ReadIterator & source);

	int getPageId() const;
	int getSubscribedEventIndex() const;
	int getPropertyCount() const;
	Unicode::String const & getProperty(int index) const;

	void addSubscribedProperty(Unicode::String const & propertyValue);

private:

	SuiEventNotification(SuiEventNotification const & rhs);
	SuiEventNotification operator=(SuiEventNotification const & rhs);

private:

	Archive::AutoDeltaVariable<int> m_pageId;
	Archive::AutoDeltaVariable<int> m_subscribedEventIndex;
	Archive::AutoDeltaVector<Unicode::String> m_subscribedProperties;
};

//======================================================================

#endif
