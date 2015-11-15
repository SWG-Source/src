// ConfigSharedNetwork.cpp
// Copyright 2000-2003, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/ConfigSharedNetwork.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"

#include <set>

//-----------------------------------------------------------------------

namespace ConfigSharedNetworkNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

	int   crcBytes;
	int   hashTableSize;
	int   incomingBufferSize;
	int   outgoingBufferSize;
	int   clockSyncDelay;
	int   maxConnections;
	int   maxRawPacketSize;
	int   maxInstandingPackets;
	int   maxOutstandingBytes;
	int   maxOutstandingPackets;
	bool  processIcmpErrors;
	bool  processOnSend;
	int   fragmentSize;
	int   pooledPacketMax;
	int   pooledPacketSize;
	int   packetHistoryMax;
	bool  logAllNetworkTraffic;
	int   oldestUnacknowledgedTimeout;
	int   overflowLimit;
	int   reportStatisticsInterval;
	int   packetSizeWarnThreshold;
	int   packetCountWarnThreshold;
	int   byteCountWarnThreshold;
	bool  reportMessages;
	int   congestionWindowMinimum;
	int   stallReportDelay;
	bool  enableFlushAndConfirmAllData;
	bool  fatalOnConnectionClosed;
	int   logBackloggedPacketThreshold;
	bool  useNetworkThread;
	int   networkThreadSleepTimeMs;
	int   keepAliveDelay;
	int   pooledPacketInitial;
	int   maxDataHoldTime;
	int   resendDelayAdjust;
	int   resendDelayPercent;
	int   networkThreadPriority;
	int   noDataTimeout;
	int   reliableOverflowBytes;
	int   icmpErrorRetryPeriod;
	int   maxDataHoldSize;
	bool  allowPortRemapping;
	bool  useTcp;
	int   tcpMinimumFrame;
	bool  reportUdpDisconnects;
	bool  reportTcpDisconnects;
	bool  logConnectionConstructionDestruction;
	bool  logConnectionDeferredMessagesWarning;
	bool  logConnectionOpenedClosed;
	int   logConnectionDeferredMessagesWarningInterval;

	typedef unsigned short ReservedPortInt;
	typedef std::set<ReservedPortInt> ReservedPortSet;
	ReservedPortSet reservedPorts;

	bool  networkHandlerDispatchThrottle;
	int   networkHandlerDispatchThrottleTimeMilliseconds;
	int   networkHandlerDispatchQueueSize;
	int   maxTCPRetries;

	bool  logSendingTooMuchData;
}

using namespace ConfigSharedNetworkNamespace;

#define KEY_INT(a,b)    (a = ConfigFile::getKeyInt("SharedNetwork", #a, b))
#define KEY_BOOL(a,b)   (a = ConfigFile::getKeyBool("SharedNetwork", #a, b))
#define KEY_REAL(a,b)   (a = ConfigFile::getKeyReal("SharedNetwork", #a, b))
#define KEY_STRING(a,b) (a = ConfigFile::getKeyString("SharedNetwork", #a, b))

// ======================================================================
// namespace ConfigSharedNetworkNamespace
// ======================================================================

void ConfigSharedNetworkNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("ConfigSharedNetwork not installed."));
	s_installed = false;
}

// ======================================================================
// class ConfigSharedNetwork
// ======================================================================

int ConfigSharedNetwork::getLogBackloggedPacketThreshold()
{
	return logBackloggedPacketThreshold;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getResendDelayAdjust()
{
	return resendDelayAdjust;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getResendDelayPercent()
{
	return resendDelayPercent;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxDataHoldTime()
{
	return maxDataHoldTime;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getFatalOnConnectionClosed()
{
	return fatalOnConnectionClosed;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getKeepAliveDelay()
{
	return keepAliveDelay;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getStallReportDelay()
{
	return stallReportDelay;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getCongestionWindowMinimum()
{
	return congestionWindowMinimum;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getEnableFlushAndConfirmAllData()
{
	return enableFlushAndConfirmAllData;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPooledPacketInitial()
{
	return pooledPacketInitial;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getCrcBytes()
{
	return crcBytes;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getHashTableSize()
{
	return hashTableSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getIncomingBufferSize()
{
	return incomingBufferSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getOutgoingBufferSize()
{
	return outgoingBufferSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getClockSyncDelay()
{
	//-- We must have this value properly initialized.
	DEBUG_FATAL(!s_installed, ("ConfigSharedNetwork not installed."));
	return clockSyncDelay;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxConnections()
{
	return maxConnections;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxRawPacketSize()
{
	return maxRawPacketSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxInstandingPackets()
{
	return maxInstandingPackets;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxOutstandingBytes()
{
	return maxOutstandingBytes;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxOutstandingPackets()
{
	return maxOutstandingPackets;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getProcessOnSend()
{
	return processOnSend;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getProcessIcmpErrors()
{
	return processIcmpErrors;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getFragmentSize()
{
	return fragmentSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getOverflowLimit()
{
	return overflowLimit;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPooledPacketMax()
{
	return pooledPacketMax;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPooledPacketSize()
{
	return pooledPacketSize;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPacketHistoryMax()
{
	return packetHistoryMax;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getLogAllNetworkTraffic()
{
	return logAllNetworkTraffic;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getOldestUnacknowledgedTimeout()
{
	return oldestUnacknowledgedTimeout;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getReportStatisticsInterval()
{
	return reportStatisticsInterval;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPacketSizeWarnThreshold()
{
	return packetSizeWarnThreshold;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getPacketCountWarnThreshold()
{
	return packetCountWarnThreshold;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getReportMessages()
{
	return reportMessages;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getByteCountWarnThreshold()
{
	return byteCountWarnThreshold;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getUseNetworkThread()
{
	if (useNetworkThread && useTcp)
	{
		WARNING(true, ("useNetworkThread is unsupported when useTcp is enabled.  useNetworkThread has been disabled."));
		useNetworkThread = false;
	}
	return useNetworkThread;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getNetworkThreadSleepTimeMs()
{
	return networkThreadSleepTimeMs;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getNetworkThreadPriority()
{
	return networkThreadPriority;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getNoDataTimeout()
{
	return noDataTimeout;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getReliableOverflowBytes()
{
	return reliableOverflowBytes;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getIcmpErrorRetryPeriod()
{
	return icmpErrorRetryPeriod;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxDataHoldSize()
{
	return maxDataHoldSize;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getAllowPortRemapping()
{
	return allowPortRemapping;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getUseTcp()
{
	return useTcp;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getTcpMinimumFrame()
{
	return tcpMinimumFrame;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getReportUdpDisconnects()
{
	return reportUdpDisconnects;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getReportTcpDisconnects()
{
	return reportTcpDisconnects;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getLogConnectionConstructionDestruction()
{
	return logConnectionConstructionDestruction;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getLogConnectionOpenedClosed()
{
	return logConnectionOpenedClosed;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getLogConnectionDeferredMessagesWarning()
{
	return logConnectionDeferredMessagesWarning;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getLogConnectionDeferredMessagesWarningInterval()
{
	return logConnectionDeferredMessagesWarningInterval;
}

// ----------------------------------------------------------------------

int ConfigSharedNetwork::getMaxTCPRetries()
{
	return maxTCPRetries;
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getIsPortReserved(unsigned short p)
{
	ReservedPortSet::const_iterator f = reservedPorts.find(p);
	return (f != reservedPorts.end());
}

//-----------------------------------------------------------------------

bool ConfigSharedNetwork::getNetworkHandlerDispatchThrottle()
{
	return networkHandlerDispatchThrottle;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getNetworkHandlerDispatchThrottleTimeMilliseconds()
{
	return networkHandlerDispatchThrottleTimeMilliseconds;
}

//-----------------------------------------------------------------------

int ConfigSharedNetwork::getNetworkHandlerDispatchQueueSize()
{
	return networkHandlerDispatchQueueSize;
}

//-----------------------------------------------------------------------

bool  ConfigSharedNetwork::getLogSendingTooMuchData()
{
	return 	logSendingTooMuchData;
}

//-----------------------------------------------------------------------

void ConfigSharedNetwork::install(int newClockSyncDelay)
{
	DEBUG_FATAL(s_installed, ("ConfigSharedNetwork already installed."));

	KEY_BOOL  (logAllNetworkTraffic, false);
	KEY_INT   (crcBytes, 2);
	KEY_INT   (hashTableSize, 100);
	KEY_INT   (incomingBufferSize, 4 * 1024 * 1024);
	KEY_INT   (outgoingBufferSize, 4 * 1024 * 1024);
	KEY_INT   (maxConnections, 1000);
	KEY_INT   (maxRawPacketSize, 496); 
	KEY_INT   (maxInstandingPackets, 400);
	KEY_INT   (maxOutstandingBytes, 200 * 1024);
	KEY_INT   (maxOutstandingPackets, 400);
	KEY_BOOL  (processOnSend, false);
	KEY_BOOL  (processIcmpErrors, true);
	KEY_INT   (fragmentSize, 496);
	KEY_INT   (pooledPacketMax, 1024);
	KEY_INT   (pooledPacketSize, -1);
	KEY_INT   (packetHistoryMax, 100);
	KEY_INT   (oldestUnacknowledgedTimeout, 90000);
	KEY_INT   (overflowLimit, 0);
	KEY_INT   (reportStatisticsInterval, 60000);
	KEY_INT   (packetSizeWarnThreshold, 0);
	KEY_INT   (packetCountWarnThreshold, 0);
	KEY_INT   (byteCountWarnThreshold, 0);
	KEY_BOOL  (reportMessages, false);
	KEY_INT   (congestionWindowMinimum, 0);
	KEY_INT   (stallReportDelay, 30000);
	KEY_BOOL  (enableFlushAndConfirmAllData, true);
	KEY_BOOL  (fatalOnConnectionClosed, false);
	KEY_INT   (logBackloggedPacketThreshold, 65000);
	KEY_BOOL  (useNetworkThread, false);
	KEY_INT   (networkThreadSleepTimeMs, 20);
	KEY_INT   (keepAliveDelay, 15000);
	KEY_INT   (pooledPacketInitial, 1024);
	KEY_INT   (maxDataHoldTime, 50);
	KEY_INT   (resendDelayAdjust, 500);
	KEY_INT   (resendDelayPercent, 125);
	KEY_INT   (networkThreadPriority, 3);
	KEY_INT   (noDataTimeout, 46000);
	KEY_INT   (reliableOverflowBytes, 2 * 1024 * 1024);
	KEY_INT   (icmpErrorRetryPeriod, 2000);
	KEY_INT   (maxDataHoldSize, -1);
	KEY_BOOL  (allowPortRemapping, true);
	KEY_BOOL  (useTcp, true);
	KEY_INT   (tcpMinimumFrame, 1000);
	KEY_BOOL  (reportUdpDisconnects, false);
	KEY_BOOL  (reportTcpDisconnects, true);
	KEY_BOOL  (logConnectionConstructionDestruction, false);
	KEY_BOOL  (logConnectionOpenedClosed, false);
	KEY_BOOL  (logConnectionDeferredMessagesWarning, false);
	KEY_INT   (logConnectionDeferredMessagesWarningInterval, 1000);
	KEY_INT   (maxTCPRetries,1);
	KEY_BOOL  (logSendingTooMuchData, true);
	{
		int i = 0;
		int p;
		do
		{
			p = ConfigFile::getKeyInt("SharedNetwork", "reservedPort", i, 0);
			if(p)
			{
				reservedPorts.insert(static_cast<ConfigSharedNetworkNamespace::ReservedPortInt>(p));
				++i;
			}
		} while(p > 0);
	}

	KEY_BOOL  (networkHandlerDispatchThrottle, false);
	KEY_INT   (networkHandlerDispatchThrottleTimeMilliseconds, 100);
	KEY_INT   (networkHandlerDispatchQueueSize, 1024);

	//-- Do not let a config file override this setting.
	//   It is critical that it be set properly in all client
	//   and server apps.
	//KEY_INT   (clockSyncDelay, 0);
	clockSyncDelay = newClockSyncDelay;

	s_installed = true;
	ExitChain::add(remove, "ConfigSharedNetwork");
}

// ======================================================================
