// ConfigSharedNetwork.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_ConfigSharedNetwork_H
#define	_INCLUDED_ConfigSharedNetwork_H

//-----------------------------------------------------------------------

class ConfigSharedNetwork
{
public:
	static void  install (int clockSyncDelay);

	static int   getCrcBytes();
	static int   getHashTableSize();
	static int   getIncomingBufferSize();
	static int   getOutgoingBufferSize();
	static int   getClockSyncDelay();
	static int   getMaxConnections();
	static int   getMaxRawPacketSize();
	static int   getMaxInstandingPackets();
	static int   getMaxOutstandingBytes();
	static int   getMaxOutstandingPackets();
	static int   getPacketHistoryMax();
	static bool  getProcessIcmpErrors();
	static bool  getProcessOnSend();
	static int   getFragmentSize();
	static int   getPooledPacketMax();
	static int   getPooledPacketSize();
	static int   getOldestUnacknowledgedTimeout();
	static int   getOverflowLimit();
	static int   getReportStatisticsInterval();
	static int   getPacketSizeWarnThreshold();
	static int   getPacketCountWarnThreshold();
	static int   getByteCountWarnThreshold();
	static bool  getReportMessages();
	static int   getCongestionWindowMinimum();
	static int   getStallReportDelay();
	static bool  getEnableFlushAndConfirmAllData();
	static bool  getFatalOnConnectionClosed();
	static int   getLogBackloggedPacketThreshold();
	static bool  getLogAllNetworkTraffic();
	static bool  getUseNetworkThread();
	static int   getNetworkThreadSleepTimeMs();
	static int   getKeepAliveDelay();
	static int   getPooledPacketInitial();
	static int   getMaxDataHoldTime();
	static int   getResendDelayAdjust();
	static int   getResendDelayPercent();
	static int   getNetworkThreadPriority();
	static int   getNoDataTimeout();
	static int   getReliableOverflowBytes();
	static int   getIcmpErrorRetryPeriod();
	static int   getMaxDataHoldSize();
	static bool  getAllowPortRemapping();
	static bool  getUseTcp();
	static int   getTcpMinimumFrame();
	static bool  getReportUdpDisconnects();
	static bool  getReportTcpDisconnects();
	static bool  getLogConnectionConstructionDestruction();
	static bool  getLogConnectionOpenedClosed();
	static bool  getLogConnectionDeferredMessagesWarning();
	static int   getLogConnectionDeferredMessagesWarningInterval();
	static bool  getIsPortReserved(unsigned short p);
	static bool  getNetworkHandlerDispatchThrottle();
	static int   getNetworkHandlerDispatchThrottleTimeMilliseconds();
	static int   getNetworkHandlerDispatchQueueSize();
	static int   getMaxTCPRetries();
	static bool  getLogSendingTooMuchData();
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConfigSharedNetwork_H
