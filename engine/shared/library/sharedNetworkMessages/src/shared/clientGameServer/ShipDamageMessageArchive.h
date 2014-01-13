//======================================================================
//
// ShipDamageMessageArchive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipDamageMessageArchive_H
#define INCLUDED_ShipDamageMessageArchive_H

//======================================================================

class ShipDamageMessage;

//----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
	class ReadIterator;

	void get(ReadIterator & source, ShipDamageMessage & target);
	void put(ByteStream & target, ShipDamageMessage const& source);
}

//======================================================================

#endif

