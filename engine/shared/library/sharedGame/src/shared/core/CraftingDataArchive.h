//======================================================================
//
// CraftingDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CraftingDataArchive_H
#define INCLUDED_CraftingDataArchive_H

#include "sharedGame/CraftingData.h"
#include "Archive/AutoByteStream.h"

//======================================================================
// archive functions

namespace Archive
{

	void get(ReadIterator & source, enum Crafting::CraftingStage & target);
	void get(ReadIterator & source, enum Crafting::IngredientType & target);

	void put(ByteStream & target, const enum Crafting::CraftingStage & source);
	void put(ByteStream & target, const enum Crafting::IngredientType & source);

}

//======================================================================

#endif
