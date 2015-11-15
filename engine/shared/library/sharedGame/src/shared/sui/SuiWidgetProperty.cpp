//======================================================================
//
// SuiWidgetProperty.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiWidgetProperty.h"

//======================================================================

SuiWidgetProperty::SuiWidgetProperty() :
widgetName(),
propertyName()
{
}

//----------------------------------------------------------------------

SuiWidgetProperty::SuiWidgetProperty(std::string const & _widgetName, std::string const & _propertyName) :
widgetName(_widgetName),
propertyName(_propertyName)
{
}

//======================================================================
