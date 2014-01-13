// ControllerMessageFactory.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "Unicode.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/CraftingData.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"

#include "sharedNetworkMessages/ObjectMenuRequestDataArchive.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedUtility/InstallationResourceDataArchive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

#include <map>
#include <set>

//-----------------------------------------------------------------------
namespace ControllerMessageFactoryNamespace
{
	static std::map<int32, ControllerMessageFactory::ControllerMessagePackFunction> s_packFunctions;
	static std::map<int32, ControllerMessageFactory::ControllerMessageUnpackFunction> s_unpackFunctions;
	static std::set<int32> s_allowFromClient;
	static bool s_installed = false;
}

using namespace ControllerMessageFactoryNamespace;

namespace ControllerGenericPackFunctions
{

}

//-----------------------------------------------------------------------

void ControllerMessageFactory::install ()
{
	FATAL(s_installed, ("ControllerMessageFactory::install already installed"));

	s_installed = true;
}

//-----------------------------------------------------------------------

void ControllerMessageFactory::remove ()
{
	FATAL(!s_installed, ("ControllerMessageFactory::remove not installed"));
	s_installed = false;
}

//-----------------------------------------------------------------------

void ControllerMessageFactory::registerControllerMessageHandler (int32 message, ControllerMessagePackFunction pack, ControllerMessageUnpackFunction unpack, bool allowFromClient/* = false*/)
{
	FATAL(!s_installed, ("ControllerMessageFactory::register not installed"));
	NOT_NULL(pack);
	NOT_NULL(unpack);
	DEBUG_WARNING(s_packFunctions.find(message) != s_packFunctions.end(), ("Registering pack function for a message %d that already exists", message));
	DEBUG_WARNING(s_unpackFunctions.find(message) != s_unpackFunctions.end(), ("Registering unpack function for a message %d that already exists", message));
	s_packFunctions[message] = pack;
	s_unpackFunctions[message] = unpack;

	if (allowFromClient)
		IGNORE_RETURN(s_allowFromClient.insert(message));
	else
		IGNORE_RETURN(s_allowFromClient.erase(message));
}

//-----------------------------------------------------------------------


void ControllerMessageFactory::pack(const int32 message, const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	FATAL(!s_installed, ("ControllerMessageFactory::pack not installed"));

	std::map<int32, ControllerMessageFactory::ControllerMessagePackFunction>::iterator i = s_packFunctions.find(message);
	if (i == s_packFunctions.end())
	{
		WARNING_STRICT_FATAL(true, ("ControllerMessageFactory::pack received a message %d which had no registered pack function.", message));
	}
	else
	{
		(i->second)(data, target);
	}
}

//-----------------------------------------------------------------------



MessageQueue::Data* ControllerMessageFactory::unpack(const int32 message, Archive::ReadIterator & source)
{
	MessageQueue::Data* data = 0;
	FATAL(!s_installed, ("ControllerMessageFactory::unpack not installed"));

	std::map<int32, ControllerMessageFactory::ControllerMessageUnpackFunction>::iterator i = s_unpackFunctions.find(message);
	if (i == s_unpackFunctions.end())
	{
		WARNING_STRICT_FATAL(true, ("ControllerMessageFactory::unpack received a message %d which had no registered unpack function.", message));
	}
	else
	{
		data = (i->second)(source);
	}
	return data;
}

//-----------------------------------------------------------------------

bool ControllerMessageFactory::allowFromClient(const int32 message)
{
	return (s_allowFromClient.find(message) != s_allowFromClient.end());
}

//-----------------------------------------------------------------------



// ======================================================================
