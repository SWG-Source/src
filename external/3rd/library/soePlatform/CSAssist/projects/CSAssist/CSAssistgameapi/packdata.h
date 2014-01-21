#ifndef PACKDATA_H
#define PACKDATA_H

//------------------------------------------------------------------------------
//
// packdata.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// encoding/decoding API-user objects for transmission over the wire
//
//------------------------------------------------------------------------------

#include <Base/Archive.h>
#include "CSAssistgameapi.h"

namespace CSAssist
{
	extern void decodeTicketComment(Base::ByteStream::ReadIterator & msg,CSAssistGameAPITicketComment *dest);
	extern void decodeDocumentHeader(Base::ByteStream::ReadIterator & msg,CSAssistGameAPIDocumentHeader *dest);
	extern void decodeSearchResult(Base::ByteStream::ReadIterator & msg,CSAssistGameAPISearchResult *dest);


namespace Base
{
//using namespace CSAssist;

	void put(Base::ByteStream & target, const CSAssistGameAPITicket & source);
	void get(ByteStream::ReadIterator & source, CSAssistGameAPITicket & target);
	void get(ByteStream::ReadIterator & source, CSAssistGameAPITicket * target);
}
}

#endif
