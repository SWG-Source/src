//======================================================================
//
// SuiEventSubscription.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiEventSubscription.h"

#include "sharedGame/SuiEventType.h"

//======================================================================

SuiEventSubscription::SuiEventSubscription() :
m_suiEventType(SuiEventType::SET_none),
m_eventWidgetName()
{
}

//----------------------------------------------------------------------

SuiEventSubscription::SuiEventSubscription(int suiEventType, std::string const & eventWidgetName) :
m_suiEventType(suiEventType),
m_eventWidgetName(eventWidgetName)
{
}

//======================================================================
