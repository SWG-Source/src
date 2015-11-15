//======================================================================
//
// SuiWidgetProperty.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiWidgetProperty_H
#define INCLUDED_SuiWidgetProperty_H

//======================================================================

class SuiWidgetProperty
{
public:

	SuiWidgetProperty();
	SuiWidgetProperty(std::string const & _widgetName, std::string const & propertyName);

	std::string widgetName;
	std::string propertyName;
};

//======================================================================

#endif
