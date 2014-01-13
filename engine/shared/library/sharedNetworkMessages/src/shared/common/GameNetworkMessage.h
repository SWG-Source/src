// ======================================================================
//
// GameNetworkMessage.h
//
// copyright 2001 Verant Interactive
// Author: Justin Randall
//
// ======================================================================

#ifndef	_INCLUDED_GameNetworkMessage_H
#define	_INCLUDED_GameNetworkMessage_H

// ======================================================================

#include "Archive/AutoByteStream.h"
#include "sharedMessageDispatch/Message.h"

// ======================================================================

class CrcString;

// ======================================================================

class GameNetworkMessage: public Archive::AutoByteStream, public MessageDispatch::Message<Archive::ByteStream>
{
public:

	static std::string const NetworkVersionId;

	explicit GameNetworkMessage(std::string const &newCommand);
	explicit GameNetworkMessage(CrcString const &newCommand);
	explicit GameNetworkMessage(Archive::ReadIterator &source);
	explicit GameNetworkMessage(GameNetworkMessage const &source);
	virtual ~GameNetworkMessage();

	GameNetworkMessage & operator=(GameNetworkMessage const &rhs);

	Archive::ByteStream const &getByteStream() const;
	std::string const &getCmdName() const;
	
	static std::vector<std::pair<std::string, int> > const getMessageCount();
	static std::string const &getCmdName(unsigned long cmdCrc);

private:
	GameNetworkMessage();
	Archive::AutoVariable<unsigned long> cmd;
};

// ======================================================================

#endif	// _INCLUDED_GameNetworkMessage_H

