#if !defined (CTSERVICEOBJECTS_H_)
#define CTSERVICEOBJECTS_H_

#pragma warning (disable : 4786)

#include <Base/Archive.h>
#include "CTServiceCharacter.h"
#include "CTServiceServer.h"
#include "CTServiceWebAPITransaction.h"
#include "CTServiceCustomer.h"
#include "CTServiceDBOrder.h"
#include "CTServiceDBTransaction.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Base
{
	void get(ByteStream::ReadIterator & source, CTService::CTServiceCharacter & target);
	void put(ByteStream & target, CTService::CTServiceCharacter & source);
	void get(ByteStream::ReadIterator & source, CTService::CTServiceServer & target);
	void put(ByteStream & target, CTService::CTServiceServer & source);
	void get(ByteStream::ReadIterator & source, CTService::CTServiceWebAPITransaction & target);
	void put(ByteStream & target, CTService::CTServiceWebAPITransaction & source);
	void get(ByteStream::ReadIterator & source, CTService::CTServiceCustomer & target);
	void put(ByteStream & target, CTService::CTServiceCustomer & source);
	void get(ByteStream::ReadIterator & source, CTService::CTServiceDBOrder & target);
	void put(ByteStream & target, CTService::CTServiceDBOrder & source);
	void get(ByteStream::ReadIterator & source, CTService::CTServiceTransaction & target);
	void put(ByteStream & target, CTService::CTServiceTransaction & source);
}

#ifdef EXTERNAL_DISTRO
}; // namespace
#endif

#endif 



