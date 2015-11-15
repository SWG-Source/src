// ObjectChannelMessages.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"

#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedFoundation/MemoryBlockManager.h"

//-----------------------------------------------------------------------
namespace ObjectChannelMessagesNamespace
{
	MemoryBlockManager* ms_memoryBlockManager;
}

using namespace ObjectChannelMessagesNamespace;

//-------------------------------------------------------------------

void ObjControllerMessage::install (void)
{
	DEBUG_FATAL (ms_memoryBlockManager, ("ObjControllerMessage already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("ObjControllerMessage::ms_memoryBlockManager", true, sizeof(ObjControllerMessage), 0, 0, 0);
}

//-------------------------------------------------------------------

void ObjControllerMessage::remove (void)
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("ObjControllerMessage is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void *ObjControllerMessage::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (ObjControllerMessage), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("ObjControllerMessage installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void ObjControllerMessage::operator delete (void *pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

ObjControllerMessage::ObjControllerMessage(const NetworkId newNetworkId, 
										   const uint32 newMessage, 
										   const real newValue, 
										   const uint32 newFlags,
										   MessageQueue::Data*  newData) :
GameNetworkMessage("ObjControllerMessage"),
data(newData),
flags(newFlags),
message(newMessage),				//lint !e713 //loss of precision
networkId(newNetworkId),
value(newValue)
{
	addVariable(flags);
	addVariable(message);
	addVariable(networkId);
	addVariable(value);
	DEBUG_FATAL(networkId.get() == NetworkId::cms_invalid, ("Trying to send a controller message to networkId 0\n"));
	char msgDetail[256] = {"\0"};
	snprintf(msgDetail, 255, "send.ObjControllerMessage.%lu", newMessage);
	NetworkHandler::reportMessage(std::string(msgDetail), 0);
}

//-----------------------------------------------------------------------

ObjControllerMessage::ObjControllerMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ObjControllerMessage"),
data(0),
flags(0),
message(0),				
networkId(NetworkId::cms_invalid),
value(0)
{
	addVariable(flags);
	addVariable(message);
	addVariable(networkId);
	addVariable(value);
	AutoByteStream::unpack(source);
	long int i = message.get();
	data = ControllerMessageFactory::unpack(i, source); 
	DEBUG_FATAL(networkId.get() == NetworkId::cms_invalid, ("Trying to send a controller message to networkId 0\n"));
	char msgDetail[256] = {"\0"};
	snprintf(msgDetail, 255, "recv.ObjControllerMessage.%lu", message.get());
	NetworkHandler::reportMessage(std::string(msgDetail), 0);

}

//-----------------------------------------------------------------------

ObjControllerMessage::~ObjControllerMessage()
{
	data = 0;
}

//-----------------------------------------------------------------------

void ObjControllerMessage::pack(Archive::ByteStream & target) const
{
	GameNetworkMessage::pack(target);
	ControllerMessageFactory::pack(message.get(), data, target);
}



//-----------------------------------------------------------------------
