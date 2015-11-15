// ======================================================================
//
// ByteOrder.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

// ======================================================================

ulong  __cdecl ntohl(ulong  netLong);
ushort __cdecl ntohs(ushort netShort);

ulong  __cdecl htonl(ulong  hostLong);
ushort __cdecl htons(ushort hostShort);

// ======================================================================

#endif
