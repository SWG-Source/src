//======================================================================
//
// CraftingDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CraftingDataArchive.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/Object.h"

//======================================================================
namespace Archive
{

	void get(ReadIterator & source, enum Crafting::CraftingStage & target)
	{
		source.get(&target, 4);
	}

	//------------------------------------------------------------------------------

	void put(ByteStream & target, const enum Crafting::CraftingStage & source)
	{
		target.put(&source, 4);
	}

	//------------------------------------------------------------------------------

	void get(ReadIterator & source, enum Crafting::IngredientType & target)
	{
		source.get(&target, 4);
	}

	//------------------------------------------------------------------------------

	void put(ByteStream & target, const enum Crafting::IngredientType & source)
	{
		target.put(&source, 4);
	}

}


//======================================================================
