// DeltasMessage.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_DeltasMessage_H
#define	_DeltasMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class DeltasMessage : public GameNetworkMessage
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:
	DeltasMessage(const NetworkId & sourceObject, const Tag& tag, const Archive::AutoDeltaByteStream & sourcePackage, const unsigned char packageId);
	DeltasMessage(Archive::ReadIterator & source);
	~DeltasMessage();


	const unsigned char          getPackageId  () const;
	const Archive::ByteStream &  getPackage    () const;
	const NetworkId              getTarget     () const;
	const Tag                    getTypeId     () const;

	void                         pack          (Archive::ByteStream & target) const;

	enum
	{
		DELTAS_CLIENT_ONLY,
		DELTAS_CLIENT_SERVER,
		DELTAS_SERVER,
		DELTAS_SHARED,
		DELTAS_CLIENT_SERVER_NP,
		DELTAS_SERVER_NP,
		DELTAS_SHARED_NP,
		DELTAS_UI,
		DELTAS_FIRST_PARENT_CLIENT_SERVER,
		DELTAS_FIRST_PARENT_CLIENT_SERVER_NP
	};

private:
	Archive::AutoVariable<NetworkId>            target;
	Archive::AutoVariable<Tag>                  typeId;
	Archive::AutoVariable<unsigned char>        packageId;
	Archive::AutoVariable<Archive::ByteStream>  package;

	static MemoryBlockManager* memoryBlockManager;

};

//-----------------------------------------------------------------------

inline const NetworkId DeltasMessage::getTarget() const
{
	return target.get();
}

//-----------------------------------------------------------------------

inline const unsigned char DeltasMessage::getPackageId() const
{
	return packageId.get();
}

//-----------------------------------------------------------------------

inline const Archive::ByteStream & DeltasMessage::getPackage() const
{
	return package.get();
}

//-----------------------------------------------------------------------

inline const Tag DeltasMessage::getTypeId() const
{
	return typeId.get();
}

//-----------------------------------------------------------------------

#endif // _DeltasMessage_H
