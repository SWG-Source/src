// ======================================================================
//
// GameNetworkMessage.cpp
//
// copyright 2001 Verant Interactive
// Author: Justin Randall
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "sharedFoundation/CrcString.h"
#include "sharedNetworkMessages/NetworkMessageFactory.h"
#include <algorithm>
#include <unordered_map>

// ======================================================================

// id = date-time
std::string const GameNetworkMessage::NetworkVersionId = "20100225-17:43";

namespace GameNetworkMessageNamespace
{
	std::unordered_map<unsigned long, int>  gs_messageCount;

	// ----------------------------------------------------------------------

	struct SortPair
	{
		bool operator()(const std::pair<std::string, int> & x, const std::pair<std::string, int> & y) const
		{
			return (x.second < y.second);
		}
	};

	std::unordered_map<unsigned long, std::string> s_messageTypes;
}

using namespace GameNetworkMessageNamespace;

// ======================================================================

GameNetworkMessage::GameNetworkMessage(std::string const &newCmd) :
	MessageDispatch::Message<Archive::ByteStream>(newCmd.c_str()),
	cmd()
{
	unsigned long const cmdCrc = getType();
	cmd.set(cmdCrc);
	addVariable(cmd);
	++(gs_messageCount[cmdCrc]);
	s_messageTypes[cmdCrc] = newCmd;
}

// ----------------------------------------------------------------------

GameNetworkMessage::GameNetworkMessage(CrcString const &newCmd) :
	MessageDispatch::Message<Archive::ByteStream>(newCmd.getCrc()),
	cmd(newCmd.getCrc())
{
	unsigned long const cmdCrc = getType();
	addVariable(cmd);
	++(gs_messageCount[cmdCrc]);
	s_messageTypes[cmdCrc] = newCmd.getString();
}

// ----------------------------------------------------------------------

GameNetworkMessage::GameNetworkMessage(Archive::ReadIterator & source) :
	AutoByteStream(),
	cmd()
{
	setValue(source);
	addVariable(cmd);
	Archive::ReadIterator ri = getValue().begin();
	AutoByteStream::unpack(ri);
	unsigned long const cmdCrc = cmd.get();
	setType(cmdCrc);
}

// ----------------------------------------------------------------------

GameNetworkMessage::GameNetworkMessage(const GameNetworkMessage & source) :
	MessageDispatch::Message<Archive::ByteStream>(source),
	cmd(source.cmd)
{
}

// ----------------------------------------------------------------------

GameNetworkMessage::~GameNetworkMessage()
{
}

// ----------------------------------------------------------------------

const Archive::ByteStream & GameNetworkMessage::getByteStream() const
{
	return getValue();
}

// ----------------------------------------------------------------------

std::vector<std::pair<std::string, int> > const GameNetworkMessage::getMessageCount() // static
{
	std::vector<std::pair<std::string, int> > result;

	for (std::unordered_map <unsigned long, int>::const_iterator i = gs_messageCount.begin(); i != gs_messageCount.end(); ++i)
		result.push_back(std::make_pair(s_messageTypes[(*i).first], (*i).second));

	std::sort(result.begin(), result.end(), SortPair());

	return result;
}

// ----------------------------------------------------------------------

std::string const &GameNetworkMessage::getCmdName() const
{
	return getCmdName(getType());
}

// ----------------------------------------------------------------------

std::string const &GameNetworkMessage::getCmdName(unsigned long cmdCrc) // static
{
	std::string &val = s_messageTypes[cmdCrc];
	if (val.empty())
	{
		char buf[32];
		snprintf(buf, sizeof(buf)-1, "unknown(%08lx)", cmdCrc);
		buf[sizeof(buf)-1] = '\0';
		val = buf;
	}
	return val;
}

// ======================================================================

