//======================================================================
//
// SuiEventType.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiEventType_H
#define INCLUDED_SuiEventType_H

//======================================================================

namespace SuiEventType
{
	enum Type
	{
		SET_none,
		SET_onButton,
		SET_onCheckbox,
		SET_onEnabledChanged,
		SET_onGenericSelection,
		SET_onSliderbar,
		SET_onTabbedPane,
		SET_onTextbox,
		SET_onVisibilityChanged,
		SET_onClosedOk,
		SET_onClosedCancel,
		SET_numEventTypes
	};	

	std::string const & getNameForType(int eventType);
}

//======================================================================

#endif
