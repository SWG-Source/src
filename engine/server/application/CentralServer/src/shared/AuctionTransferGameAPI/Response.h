#ifndef RESPONSE_H
#define RESPONSE_H

#include <Base/Archive.h>
#include <ATGenericAPI/GenericMessage.h>
#include "AuctionTransferEnum.h"

//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{

	class CommonResponse : public GenericResponse
	{
	public:
		CommonResponse(RequestTypes type, void *user) : GenericResponse((short)type, TRANSFER_SERVER_TIME_OUT, user) {}
		virtual ~CommonResponse() {}
	};

//////////////////////////////////////////////////////////////////////////////////////
	class GetIDResponse : public GenericResponse
	{
	public:
		GetIDResponse(RequestTypes type, void *user);
		virtual ~GetIDResponse() {}
		long long getNewID() { return m_transactionID; }
		virtual void unpack(Base::ByteStream::ReadIterator &iter);
	private:
		long long m_transactionID;
	};


}; // namespace AuctionTransfer
//////////////////////////////////////////////////////////////////////////////////////

#endif //RESPONSE_H
