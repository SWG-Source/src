// 
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DeltasMessage.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"

//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(DeltasMessage, true, 0, 0, 0);

//===================================================================

DeltasMessage::DeltasMessage(const NetworkId & sourceObject, const Tag& tag, const Archive::AutoDeltaByteStream & sourcePackage, const unsigned char newPackageId) :
GameNetworkMessage("DeltasMessage"),
target(sourceObject),
typeId(tag),
packageId(newPackageId),
package()
{
	static Archive::ByteStream bs;
	AutoByteStream::addVariable(target);
	AutoByteStream::addVariable(typeId);
	AutoByteStream::addVariable(packageId);
	AutoByteStream::addVariable(package);
	bs.clear();
	sourcePackage.packDeltas(bs);
	package.set(bs);
}

//-----------------------------------------------------------------------

DeltasMessage::DeltasMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DeltasMessage"),
target(NetworkId::cms_invalid),
typeId(0),
packageId(0),
package()
{
	AutoByteStream::addVariable(target);
	AutoByteStream::addVariable(typeId);
	AutoByteStream::addVariable(packageId);
	AutoByteStream::addVariable(package);
	unpack(source);
}

//-----------------------------------------------------------------------

DeltasMessage::~DeltasMessage()
{
}

//-----------------------------------------------------------------------

void DeltasMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//-----------------------------------------------------------------------
