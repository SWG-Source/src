//-----------------------------------------------------------------------
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ErrorMessage.h"

//-----------------------------------------------------------------------

ErrorMessage::ErrorMessage(const std::string & newErrorName, const std::string & newDescription, const bool f) :
GameNetworkMessage("ErrorMessage"),
errorName(newErrorName),
description(newDescription),
fatal(f)
{
	AutoByteStream::addVariable(errorName);
	AutoByteStream::addVariable(description);
	AutoByteStream::addVariable(fatal);
}

//-----------------------------------------------------------------------

ErrorMessage::ErrorMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ErrorMessage"),
errorName(),
description(),
fatal()
{
	AutoByteStream::addVariable(errorName);
	AutoByteStream::addVariable(description);
	AutoByteStream::addVariable(fatal);
	unpack(source);
}

//-----------------------------------------------------------------------

ErrorMessage::~ErrorMessage()
{
}

//-----------------------------------------------------------------------
