// 
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedFoundation/MemoryBlockManager.h"

//-----------------------------------------------------------------------

namespace BaselinesMessageNamespace
{
	MemoryBlockManager* ms_memoryBlockManager;
}

using namespace BaselinesMessageNamespace;

//-------------------------------------------------------------------

void BaselinesMessage::install (void)
{
	DEBUG_FATAL (ms_memoryBlockManager, ("BaselinesMessage already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("BaselinesMessage::ms_memoryBlockManager", true, sizeof(BaselinesMessage), 0, 0, 0);
}

//-------------------------------------------------------------------

void BaselinesMessage::remove (void)
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("BaselinesMessage is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void *BaselinesMessage::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (BaselinesMessage), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("BaselinesMessage installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void BaselinesMessage::operator delete (void *pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

BaselinesMessage::BaselinesMessage(const NetworkId &networkId, const Tag objectType, const Archive::AutoByteStream & sourcePackage, const unsigned char newPackageId) :
GameNetworkMessage("BaselinesMessage"),
target(networkId),
typeId(objectType),
packageId(newPackageId),
package()
{
	static Archive::ByteStream bs;
	int s = bs.getSize();
	AutoByteStream::addVariable(target);
	AutoByteStream::addVariable(typeId);
	AutoByteStream::addVariable(packageId);
	AutoByteStream::addVariable(package);
	bs.clear();
	sourcePackage.pack(bs);
	package.set(bs);
	char numbuf[256] = {"\0"};
	char tag[5];
	ConvertTagToString(objectType, tag);
	snprintf(numbuf, sizeof(numbuf), "send.BaselinesMessage.%s", tag);
	NetworkHandler::reportMessage(numbuf, s);
}

//-----------------------------------------------------------------------

BaselinesMessage::BaselinesMessage(const NetworkId &id, 
	                               const Tag objectType, 
	                               const unsigned char newPackageId, 
	                               const Archive::ByteStream & newPackage) :
GameNetworkMessage("BaselinesMessage"),
target(id),
typeId(objectType),
packageId(newPackageId),
package(newPackage)
{
	int s = newPackage.getSize();
	AutoByteStream::addVariable(target);
	AutoByteStream::addVariable(typeId);
	AutoByteStream::addVariable(packageId);
	AutoByteStream::addVariable(package);
	char numbuf[256] = {"\0"};
	char tag[5];
	ConvertTagToString(objectType, tag);
	snprintf(numbuf, sizeof(numbuf), "send.BaselinesMessage.%s", tag);
	NetworkHandler::reportMessage(numbuf, s);	
}

//-----------------------------------------------------------------------

BaselinesMessage::BaselinesMessage(Archive::ReadIterator & source) :
GameNetworkMessage("BaselinesMessage"),
target(NetworkId::cms_invalid),
typeId(0),
packageId(0),
package()
{
	int s = source.getSize();
	AutoByteStream::addVariable(target);
	AutoByteStream::addVariable(typeId);
	AutoByteStream::addVariable(packageId);
	AutoByteStream::addVariable(package);
	unpack(source);
	char numbuf[256] = {"\0"};
	char tag[5];
	ConvertTagToString(typeId.get(), tag);
	snprintf(numbuf, sizeof(numbuf), "recv.BaselinesMessage.%s", tag);
	NetworkHandler::reportMessage(numbuf, s);
	
}

//-----------------------------------------------------------------------

BaselinesMessage::~BaselinesMessage()
{
}

//-----------------------------------------------------------------------

void BaselinesMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//-----------------------------------------------------------------------
