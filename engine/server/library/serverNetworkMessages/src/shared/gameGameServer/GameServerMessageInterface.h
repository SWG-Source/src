// ======================================================================
//
// GameServerMessageInterface.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#ifndef INCLUDED_GameServerMessageInterface_H
#define INCLUDED_GameServerMessageInterface_H

namespace Archive
{
	class ByteStream;
	class ReadIterator;
};

class AiMovementMessage;
class AiCreatureStateMessage;

//-----------------------------------------------------------------------

// This class is used as an interface to pack/unpack messages that rely on game 
// server specific code, so the serverGame library doesn't have to be linked
// with every server.
class GameServerMessageInterface
{
public:

	static GameServerMessageInterface * getInstance();

	virtual void pack(Archive::ByteStream & target, AiCreatureStateMessage const & msg);
	virtual void unpack(Archive::ReadIterator & source, AiCreatureStateMessage & msg);

	virtual void pack(Archive::ByteStream & target, AiMovementMessage const & msg);
	virtual void unpack(Archive::ReadIterator & source, AiMovementMessage & msg);

protected:
	
	         GameServerMessageInterface();
	virtual ~GameServerMessageInterface();

	static void setInstance(GameServerMessageInterface * instance);
	static void removeInstance();

protected:

	static GameServerMessageInterface * ms_instance;
};


inline GameServerMessageInterface * GameServerMessageInterface::getInstance()
{
	return ms_instance;
}



#endif	// INCLUDED_GameServerMessageInterface_H
