// FirstChatServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_FirstChatServer_H
#define	_INCLUDED_FirstChatServer_H

//-----------------------------------------------------------------------

#pragma warning ( disable : 4702 )

#include "sharedFoundation/FirstSharedFoundation.h"

// hack for gcc 3.4.x-3.5.0 bug
#if defined(__GNUC__) && (__GNUC__ == 3) && (__GNUC_MINOR__ > 3)
class TransferCharacterData;
namespace Archive
{
	class ByteStream;
	class ReadIterator;
	void get(ReadIterator &, TransferCharacterData &);
	void put(ByteStream &, TransferCharacterData const &);
}
#endif

#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetwork/Connection.h"
#include "ChatAPI/ChatAPI.h"
#include <unordered_map>
#include <string>

//-----------------------------------------------------------------------

#endif	// _INCLUDED_FirstChatServer_H
