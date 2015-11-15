// BaselinesMessage.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_BaselinesMessage_H
#define	_BaselinesMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------

class BaselinesMessage : public GameNetworkMessage
{
public: //ctor/dtor
	BaselinesMessage   (const NetworkId &networkId,
						const Tag objectType,
	                    const Archive::AutoByteStream & sourcePackage, 
	                    const unsigned char packageId);

	BaselinesMessage   (const NetworkId &id, 
		                const Tag objectType, 
	                    const unsigned char newPackageId, 
						const Archive::ByteStream & newPackage);

	explicit BaselinesMessage   (Archive::ReadIterator & source);

	~BaselinesMessage  ();

public: // methods
	const NetworkId              getTarget          () const;
	const unsigned char          getPackageId       () const;
	const Tag                    getTypeId          () const;
	const Archive::ByteStream &  getPackage         () const;

	void                         pack               (Archive::ByteStream & target) const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public: // types
	enum
	{
		BASELINES_CLIENT_ONLY,
		BASELINES_CLIENT_SERVER,
		BASELINES_SERVER,
		BASELINES_SHARED,
		BASELINES_CLIENT_SERVER_NP,
		BASELINES_SERVER_NP,
		BASELINES_SHARED_NP,
		BASELINES_UI,
		BASELINES_FIRST_PARENT_CLIENT_SERVER,
		BASELINES_FIRST_PARENT_CLIENT_SERVER_NP
	};

private: 
	Archive::AutoVariable<NetworkId>            target;
	Archive::AutoVariable<Tag>                  typeId;
	Archive::AutoVariable<unsigned char>        packageId;
	Archive::AutoVariable<Archive::ByteStream>  package;
};

//-----------------------------------------------------------------------

inline const unsigned char BaselinesMessage::getPackageId() const
{
	return packageId.get();
}

//-----------------------------------------------------------------------

inline const Archive::ByteStream & BaselinesMessage::getPackage() const
{
	return package.get();
}

//-----------------------------------------------------------------------

inline const NetworkId BaselinesMessage::getTarget() const
{
	return target.get();
}

//-----------------------------------------------------------------------

inline const Tag BaselinesMessage::getTypeId() const
{
	return typeId.get();
}

//-----------------------------------------------------------------------

#endif // _BaselinesMessage_H
