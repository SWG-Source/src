//======================================================================
//
// MessageQueueDraftSlotsDataOption.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSlotsDataOption_H
#define INCLUDED_MessageQueueDraftSlotsDataOption_H

//----------------------------------------------------------------------

#include "sharedGame/CraftingData.h"
#include "StringId.h"

//======================================================================

class MessageQueueDraftSlotsDataOption
{
public:
	StringId                       name;
	Unicode::String                ingredient;
	Crafting::IngredientType       type;
	int                            amountNeeded;

	MessageQueueDraftSlotsDataOption ();
};

//======================================================================

#endif
