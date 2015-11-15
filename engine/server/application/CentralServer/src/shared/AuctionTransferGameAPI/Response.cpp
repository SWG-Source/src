#include "Response.h"

namespace AuctionTransfer
{
	GetIDResponse::GetIDResponse(RequestTypes type, void *user) 
	: GenericResponse( (short)type, TRANSFER_SERVER_TIME_OUT, user), m_transactionID(-1)
	{
	}

	void GetIDResponse::unpack(Base::ByteStream::ReadIterator &iter)
	{
		get(iter, m_type);
		get(iter, m_track);
		get(iter, m_result);
		get(iter, m_transactionID);
	}
};
