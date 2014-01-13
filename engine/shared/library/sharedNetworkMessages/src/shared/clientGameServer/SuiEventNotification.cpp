//======================================================================
//
// SuiEventNotification.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SuiEventNotification.h"

//======================================================================

char const * const SuiEventNotification::MessageType = "SuiEventNotification";

//----------------------------------------------------------------------

SuiEventNotification::SuiEventNotification(int pageId, int subscribedEventIndex, int reservePropertyCount) :
GameNetworkMessage(SuiEventNotification::MessageType),
m_pageId(pageId),
m_subscribedEventIndex(subscribedEventIndex),
m_subscribedProperties()
{
	addVariable(m_pageId);
	addVariable(m_subscribedEventIndex);
	addVariable(m_subscribedProperties);

	if (reservePropertyCount > 0)
		m_subscribedProperties.reserve(static_cast<size_t>(reservePropertyCount));
}

//----------------------------------------------------------------------

SuiEventNotification::SuiEventNotification(Archive::ReadIterator & source) :
GameNetworkMessage(SuiEventNotification::MessageType),
m_pageId(-1),
m_subscribedEventIndex(-1),
m_subscribedProperties()
{
	addVariable(m_pageId);
	addVariable(m_subscribedEventIndex);
	addVariable(m_subscribedProperties);
	AutoByteStream::unpack(source);
}

//----------------------------------------------------------------------

int SuiEventNotification::getPageId() const
{
	return m_pageId.get();

}
//----------------------------------------------------------------------

int SuiEventNotification::getSubscribedEventIndex() const
{
	return m_subscribedEventIndex.get();
}

//----------------------------------------------------------------------

int SuiEventNotification::getPropertyCount() const
{
	return static_cast<int>(m_subscribedProperties.size());
}

//----------------------------------------------------------------------

Unicode::String const & SuiEventNotification::getProperty(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getPropertyCount());
	return m_subscribedProperties.get(static_cast<size_t>(index));
}

//----------------------------------------------------------------------

void SuiEventNotification::addSubscribedProperty(Unicode::String const & propertyValue)
{
	m_subscribedProperties.push_back(propertyValue);
}

//======================================================================
