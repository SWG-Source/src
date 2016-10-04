// NetworkSetupData.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

NetworkSetupData::NetworkSetupData() :
crcBytes(ConfigSharedNetwork::getCrcBytes()),
hashTableSize(ConfigSharedNetwork::getHashTableSize()),
incomingBufferSize(ConfigSharedNetwork::getIncomingBufferSize()),
outgoingBufferSize(ConfigSharedNetwork::getOutgoingBufferSize()),
clockSyncDelay(ConfigSharedNetwork::getClockSyncDelay()),
keepAliveDelay(ConfigSharedNetwork::getKeepAliveDelay()),
maxConnections(ConfigSharedNetwork::getMaxConnections()),
maxConnectionsPerIP(ConfigSharedNetwork::getMaxConnectionsPerIP()),
maxRawPacketSize(ConfigSharedNetwork::getMaxRawPacketSize()),
maxInstandingPackets(ConfigSharedNetwork::getMaxInstandingPackets()),
maxOutstandingBytes(ConfigSharedNetwork::getMaxOutstandingBytes()),
maxOutstandingPackets(ConfigSharedNetwork::getMaxOutstandingPackets()),
processIcmpErrors(ConfigSharedNetwork::getProcessIcmpErrors()),
processOnSend(ConfigSharedNetwork::getProcessOnSend()),
fragmentSize(ConfigSharedNetwork::getFragmentSize()),
pooledPacketMax(ConfigSharedNetwork::getPooledPacketMax()),
pooledPacketSize(ConfigSharedNetwork::getPooledPacketSize()),
pooledPacketInitial(ConfigSharedNetwork::getPooledPacketInitial()),
packetHistoryMax(ConfigSharedNetwork::getPacketHistoryMax()),
logAllNetworkTraffic(ConfigSharedNetwork::getLogAllNetworkTraffic()),
oldestUnacknowledgedTimeout(ConfigSharedNetwork::getOldestUnacknowledgedTimeout()),
overflowLimit(ConfigSharedNetwork::getOverflowLimit()),
congestionWindowMinimum(ConfigSharedNetwork::getCongestionWindowMinimum()),
maxDataHoldTime(ConfigSharedNetwork::getMaxDataHoldTime()),
resendDelayAdjust(ConfigSharedNetwork::getResendDelayAdjust()),
resendDelayPercent(ConfigSharedNetwork::getResendDelayPercent()),
noDataTimeout(ConfigSharedNetwork::getNoDataTimeout()),
reliableOverflowBytes(ConfigSharedNetwork::getReliableOverflowBytes()),
icmpErrorRetryPeriod(ConfigSharedNetwork::getIcmpErrorRetryPeriod()),
maxDataHoldSize(ConfigSharedNetwork::getMaxDataHoldSize()),
port(0),
bindInterface(std::string("")),
compress(false),
allowPortRemapping(ConfigSharedNetwork::getAllowPortRemapping()),
useTcp(ConfigSharedNetwork::getUseTcp())
{
}

//-----------------------------------------------------------------------

NetworkSetupData::~NetworkSetupData()
{
}

//-----------------------------------------------------------------------

