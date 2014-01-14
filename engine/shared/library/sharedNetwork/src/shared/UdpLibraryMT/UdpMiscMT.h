// ======================================================================
//
// UdpMiscMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpMiscMT_H_
#define _UdpMiscMT_H_

// ======================================================================

class LogicalPacket;

// ======================================================================

class UdpMiscMT
{
public:
	static LogicalPacket const *CreateQuickLogicalPacket(void const *data, int dataLen, void const *data2 = 0, int dataLen2 = 0);
};

// ======================================================================

#endif // _UdpMiscMT_H_

