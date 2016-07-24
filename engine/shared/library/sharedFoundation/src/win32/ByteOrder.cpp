// ======================================================================
//
// ByteOrder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ByteOrder.h"

// ======================================================================

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
