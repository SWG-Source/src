// ======================================================================
//
// GameServerMessageInterface.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerMessageInterface.h"


//-----------------------------------------------------------------------

GameServerMessageInterface * GameServerMessageInterface::ms_instance = nullptr;


//=======================================================================

GameServerMessageInterface::GameServerMessageInterface()
{
}

//-----------------------------------------------------------------------

GameServerMessageInterface::~GameServerMessageInterface()
{
	if (ms_instance == this)
		ms_instance = nullptr;
}

//-----------------------------------------------------------------------

void GameServerMessageInterface::setInstance(GameServerMessageInterface * instance)
{
	if (ms_instance == nullptr)
		ms_instance = instance;
	else if (ms_instance != instance)
	{
		WARNING(true, ("GameServerMessageInterface::setInstance called when an "
			"instance has already been created"));
		delete instance;
	}
}

//-----------------------------------------------------------------------

void GameServerMessageInterface::removeInstance()
{
	delete ms_instance;
}


//-----------------------------------------------------------------------

// Stub functions. The actual implementation should be in a derived class
	
void GameServerMessageInterface::pack(Archive::ByteStream &, AiCreatureStateMessage const &)
{
}

void GameServerMessageInterface::unpack(Archive::ReadIterator &, AiCreatureStateMessage &)
{
}

void GameServerMessageInterface::pack(Archive::ByteStream &, AiMovementMessage const &)
{
}

void GameServerMessageInterface::unpack(Archive::ReadIterator &, AiMovementMessage &)
{
}

// ======================================================================
