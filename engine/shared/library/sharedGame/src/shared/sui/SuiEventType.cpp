//======================================================================
//
// SuiEventType.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiEventType.h"

//======================================================================

namespace SuiEventTypeNamespace
{
	std::string const names[] =
	{
		"none", //SET_none,
		"onButton", //SET_onButton,
		"onCheckbox", //SET_onCheckbox,
		"onEnabledChanged", //SET_onEnabledChanged,
		"onGenericSelection", //SET_onGenericSelection,
		"onSliderbar", //SET_onSliderbar,
		"onTabbedPane", //SET_onTabbedPane,
		"onTextbox", //SET_onTextbox,
		"onVisibilityChanged", //SET_onVisibilityChanged,
		"onClosedOk", //SET_onClosedOk,
		"onClosedCancel", //SET_onClosedCancel
	};
}

using namespace SuiEventTypeNamespace;

//----------------------------------------------------------------------

std::string const & SuiEventType::getNameForType(int eventType)
{ //lint !e1929 //returning a reference
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(SET_none, static_cast<SuiEventType::Type>(eventType), SET_numEventTypes);
	return names[static_cast<size_t>(eventType)];
}

//======================================================================
