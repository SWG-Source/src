// ======================================================================
//
// ByteOrder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ByteOrder.h"

// ======================================================================

#if defined(_M_X64)
#include <stdlib.h>

ulong ntohl(ulong netLong)
{
	return _byteswap_ulong(netLong);
}

ulong htonl(ulong hostLong)
{
	return _byteswap_ulong(hostLong);
}

ushort ntohs(ushort netShort)
{
	return _byteswap_ushort(netShort);
}

ushort htons(ushort hostShort)
{
	return _byteswap_ushort(hostShort);
}
#else

// I'm using the arguments, but the compiler can't tell that
#pragma warning(disable: 4100)

__declspec(naked) ulong ntohl(ulong netLong)
{
	_asm
	{
		mov     eax, [esp+4]
		bswap   eax
		ret
	}
} //lint !e533 !e715 // function should return a value, argument not referenced

__declspec(naked) ulong htonl(ulong hostLong)
{
	_asm
	{
		mov     eax, [esp+4]
		bswap   eax
		ret
	}
} //lint !e533 !e715 // function should return a value, argument not referenced

__declspec(naked) ushort ntohs(ushort netShort)
{
	_asm
	{
		mov     eax, [esp+4]
		bswap   eax
		shr     eax, 16
		ret
	}
} //lint !e533 !e715 // function should return a value, argument not referenced

__declspec(naked) ushort htons(ushort hostShort)
{
	_asm
	{
		mov     eax, [esp+4]
		bswap   eax
		shr     eax, 16
		ret
	}
} //lint !e533 !e715 // function should return a value, argument not referenced

// ======================================================================

#endif
