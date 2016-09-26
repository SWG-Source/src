// NetworkSetupData.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_NetworkSetupData_H
#define	_INCLUDED_NetworkSetupData_H

//-----------------------------------------------------------------------

class NetworkSetupData
{
public:
	NetworkSetupData();
	virtual ~NetworkSetupData();

public:
	int             crcBytes;
	int             hashTableSize;
	int             incomingBufferSize;
	int             outgoingBufferSize;
	int             clockSyncDelay;
	int             keepAliveDelay;
	int             maxConnections;
	int 			maxConnectionsPerIP;
	int             maxRawPacketSize;
	int             maxInstandingPackets;
	int             maxOutstandingBytes;
	int             maxOutstandingPackets;
	bool            processIcmpErrors;
	bool            processOnSend;
	int             fragmentSize;
	int             pooledPacketMax;
	int             pooledPacketSize;
	int             pooledPacketInitial;
	int             packetHistoryMax;
	bool            logAllNetworkTraffic;
	int             oldestUnacknowledgedTimeout;
	int             overflowLimit;
	int             congestionWindowMinimum;
	int             maxDataHoldTime;
	int             resendDelayAdjust;
	int             resendDelayPercent;
	int             noDataTimeout;
	int             reliableOverflowBytes;
	int             icmpErrorRetryPeriod;
	int             maxDataHoldSize;
	unsigned short  port;
	std::string     bindInterface;
	bool            compress;
	bool            allowPortRemapping;
	bool            useTcp;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_NetworkSetupData_H
