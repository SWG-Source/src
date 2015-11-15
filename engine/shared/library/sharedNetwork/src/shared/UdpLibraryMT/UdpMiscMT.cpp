// ======================================================================
//
// UdpMiscMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpMiscMT.h"
#include "UdpLibraryMT.h"
#include "sharedSynchronization/Guard.h"

// ======================================================================

LogicalPacket const *UdpMiscMT::CreateQuickLogicalPacket(void const *data, int dataLen, void const *data2, int dataLen2)
{
	Guard lock(UdpLibraryMT::getMutex());
	return UdpMisc::CreateQuickLogicalPacket(data, dataLen, data2, dataLen2);
}

// ======================================================================

