// ======================================================================
//
// GameServerMessageArchive.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/GameServerMessageArchive.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureStateArchive.h"
#include "sharedFoundation/ExitChain.h"


//-----------------------------------------------------------------------

GameServerMessageArchive::GameServerMessageArchive()
{
}

//-----------------------------------------------------------------------

GameServerMessageArchive::~GameServerMessageArchive()
{
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::install()
{
	if (getInstance() == nullptr)
	{
		setInstance(new GameServerMessageArchive);
		ExitChain::add(GameServerMessageArchive::remove, "GameServerMessageArchive::remove");
	}
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::remove()
{
	removeInstance();
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::pack(Archive::ByteStream & target, AiCreatureStateMessage const & msg)
{
	Archive::put(target, msg);
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::unpack(Archive::ReadIterator & source, AiCreatureStateMessage & msg)
{
	Archive::get(source, msg);
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::pack(Archive::ByteStream & target, AiMovementMessage const & msg)
{
	Archive::put(target, msg);
}

//-----------------------------------------------------------------------

void GameServerMessageArchive::unpack(Archive::ReadIterator & source, AiMovementMessage & msg)
{
	Archive::get(source, msg);
}

// ======================================================================
